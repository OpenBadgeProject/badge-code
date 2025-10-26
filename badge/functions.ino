#include "constants.h"

/*
 * Various helper functions are here
 * 
 */

// The RNG state
uint32_t rngState = 0;

/* The state must be initialized to non-zero */
uint32_t random32()
{

  // The state can't be zero ever, we set it the first time
  if (rngState == 0) {
    rngState = currentTick;
    if (rngState == 0) rngState = 7;
  }
  
  /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
  uint32_t x = rngState;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return rngState = x;
}


//void runTick() {
ISR (TIMER1_OVF_vect) {
  
  // We count the number of loops instead of using millis()
  // On the ATTINY85 chip the millis() retuns odd things
  currentTick++;

  if (CUR_BUTTON) {
    lastButton = 0;
  } else {
    lastButton++;
  }
  OLD_BUTTON = CUR_BUTTON;

  shiftRegisters();
}


/*
 * The way we use shift registes is very odd, but given the extreme constraints
 * for this project, it works.
 * 
 * We drive the LED matrix with the two serial to parallel registers.
 * We read the buttons with the parallel to serial register.
 * 
 * All 3 shift registes share a common clock and latch
 * 
 * As soon as we trip the latch, we start to write and read bits
 * 
 */
void shiftRegisters() {
      uint8_t currentRow = 1; // A bit vector for the LED row
      uint8_t currentCol;
      uint8_t frameMask; // The reverse bitmask of currentCol
      uint8_t i, j;
      cli();
      PORTB |= (1 << latchPin); // HIGH

      // Loop for each row
      for (j = 0; j < 8; j++) {
        currentCol = 0x01;
        frameMask = 0x80;
        PORTB &= ~(1 << latchPin); // LOW
        // Loop for each col, writing/reading one bit per clock
        // Set the button to 0, we will fill in the bits as we go
        CUR_BUTTON = 0;
        for (i = 0; i < 8; i++)  {

          // In this code we have to extract the relevant bits to shift into 
          // the register
          if (!!(currentRow & currentCol)) {
            PORTB |= (1 << dataPin1); // HIGH
          } else {
            PORTB &= ~(1 << dataPin1); // LOW
          }

          // We write the data in backwards because it's shifted in
          if (!(blitBuffer[j] & frameMask)) {
            PORTB |= (1 << dataPin2); // HIGH
          } else {
            PORTB &= ~(1 << dataPin2); // LOW
          }

          // Shift in the button presses
          CUR_BUTTON = CUR_BUTTON << 1;
          // READ the button input
          if (PINB & (1 << buttonPin))
            CUR_BUTTON = CUR_BUTTON | 0x01;

          // Pulse the clock
          PORTB |= (1 << clockPin); // HIGH
          PORTB &= ~(1 << clockPin); // LOW
          currentCol = currentCol << 1;
          frameMask = frameMask >> 1;
          for (uint8_t ll = 0; ll < 100; ll++) {
            // We have to kill some time. If we don't the last column is dim
            __asm__("nop\n\t");
          }
        }
        // Shift the row bit
        currentRow = currentRow << 1;
        PORTB |= (1 << latchPin); // HIGH
        PORTB &= ~(1 << latchPin); // LOW
      }
      // Clear the screen
      // If we don't shift in nothing after a short delay,
      // the right most column is noticably brighter than the others
//      for (i = 0; i < 200; i++) {
//        // We have to kill some time. If we don't the last column is dim
//        __asm__("nop\n\t");
//      }
      PORTB &= ~(1 << latchPin); // LOW
      PORTB &= ~(1 << dataPin1); // LOW
      PORTB &= ~(1 << dataPin2); // LOW
      for (j = 0; j < 8; j++) {
        PORTB |= (1 << clockPin); // HIGH
        PORTB &= ~(1 << clockPin); // LOW
      }
      PORTB |= (1 << latchPin); // HIGH
      PORTB &= ~(1 << latchPin); // LOW
      sei();
}

// Get a string from the user and return in the theString variable
void getString(unsigned char *theString) {
  int8_t currentChar = 0x21;
  uint8_t currentPos = 0;
  uint8_t maxLen = 0;
  clearFrameBuffer();
  while(1) {
    LOOP(0);

    for (int i = 0; i < 5; i++) {
      byte the_char = pgm_read_byte(font + (currentChar * 5) + i);
      frameBuffer[i] = the_char;
    }
    frameBuffer[6] = currentPos;
    frameBuffer[7] = maxLen;

    if (PUSH_BUTTON(BTN_DOWN)) {
      if (TICK(30)) {
        if (currentChar == 94) {
          currentChar = 0;
        } else {
          currentChar++;
        }
      }
    } else if (PUSH_BUTTON(BTN_UP)) {
      if (TICK(30)) {
        if (currentChar == 0) {
          currentChar = 94;
        } else {
          currentChar--;
        }
      }
    } else if (NEW_BUTTON(BTN_LEFT)) {
      if (currentPos != 0) currentPos--;
      currentChar = theString[currentPos]-0x20;
    } else if (NEW_BUTTON(BTN_RIGHT)) {
      if (currentPos < 100) currentPos++;
      if (currentPos > maxLen) {
        maxLen = currentPos;
        currentChar = 0;
      } else {
        currentChar = theString[currentPos]-0x20;
      }
    } else if (NEW_BUTTON(BTN_B)) {
      theString[maxLen+1] = 0;
      break;
    }
    theString[currentPos] = currentChar + 0x20;
  }
}

// Write all zeros to the framebuffer
// Basically, clear the screen
void clearFrameBuffer() {
  memset(frameBuffer, 0, 8);
  messageCount = 0;
}

// Set one point on the framebuffer
void setFrameBuffer(uint8_t x, uint8_t y) {
  if (x < 8 && y < 8)
    frameBuffer[x] |= 1 << y;
}

// unSet one point on the framebuffer
void unSetFrameBuffer(uint8_t x, uint8_t y) {
  if (x < 8 && y < 8)
    frameBuffer[x] &= ((1 << y) ^ 0xFF);
}

void blit() {
  cli();
  for (uint8_t i = 0; i < 8; i++) {
    blitBuffer[i] = frameBuffer[i];
  }
  sei();
  
}
