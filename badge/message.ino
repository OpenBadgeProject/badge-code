#include "constants.h"

/*
 * Functions that display various messages to the screen
 * 
 */

bool progmemMessage = true;

// Print the score on the screen then return
void printScore(unsigned int score) {
  uint8_t i = 0;
  unsigned int digit;
  unsigned char scoreString[10];
  
  // Turn the score into a string
  // Using sprintf takes up 2K of program space
  printMessage(scoreMessage, true);

  scoreString[i++] = ' ';

  // We have to extract the score digits
  // They are reversed
  do {
    digit = score % 10;
    score = score / 10;
    scoreString[i++] = digit + 0x30;
  } while (score);

  // We need to reverse the string to fix the digits
  for(int j = 1; j <= i/2; j++) {
    scoreString[0] = scoreString[j]; // We are using slot 0 as our temp variable
    scoreString[j] = scoreString[i-j];
    scoreString[i-j] = scoreString[0];
  }
  scoreString[0] = ' '; // Don't forget to put this back
  scoreString[i++] = ' ';
  scoreString[i] = 0x00; // NULL terminator

  printMessage(scoreString, false);
}

// Print a message then return
void printMessage(unsigned char *newMessage, bool progmemMessage) {

  setMessage(newMessage, progmemMessage);
  while (true) {
    LOOP(0);
    showMessage();
    if (donePrinting) return;
  }
}

// Set the message to be displayed
void setMessage(unsigned char *newMessage, bool progMem) {

  progmemMessage = progMem;
  message = newMessage;
  messageCount = 0;
  donePrinting = false;
  byte messageByte;

  // Clear the framebuffer here. It's common to set the message then
  // display it. It makes life easier
  clearFrameBuffer();

  // We have to write our own strlen as PROGMEM strings are different
  messageLen = 0;
  while (true) {
    if (progmemMessage) messageByte = pgm_read_byte(message + messageLen);
    else messageByte = message[messageLen];
    if (messageByte == 0) {
      break;
    } else {
      messageLen++;
    }
  }
}


// Show the message. This has to be called multiple times. You probably want printMessage()
void showMessage()
{
  static unsigned int lastTick = 0;
  // Scroll the display at a reasonable speed
  if (lastTick + messageDelay < currentTick) {
    lastTick = currentTick;

    /*
     * Copy message data into framebuffer
     * 
     * We shift the screen one pixel to the left, then shift in whatever
     * the rightmost column is
     * 
     */

    for (int i = 0; i < 7; i++) {
      frameBuffer[i] = frameBuffer[i+1];
    }
    /*
     * There will always be 2 letters on the screen at a time. The letters
     * are 6 pixels wide, the screen is 8
     * 
     * Don't think of the screen in terms of the letter we're showing
     * think of it in terms of the column we are showing in the total message
     */

    // letterPos will track the current letter we are displaying
    uint8_t letterPos;

    // This block figures out where the curent letter we're to display is
    // messageCount is the current row we are to display for the message

    uint8_t pgmIndex = ((messageCount) / 6) % messageLen;
    if (progmemMessage) {
      letterPos = pgm_read_word(message + pgmIndex) - 0x20;
    } else {
      letterPos = message[pgmIndex] - 0x20;
    }


    // pos contians the actual row we are going to display
    uint8_t pos = (messageCount) % 6;
    if (pos == 5) {
      // Insert a gap between letters
      frameBuffer[7] = 0x00;
    } else {
      byte letterChar = pgm_read_byte(font + (letterPos * 5) + pos);
      frameBuffer[7] = letterChar;
    }


    // messageCount is the row we're displaying, when we get to the end of the
    // string, we flip back to zero
    messageCount++;
    if (messageCount >= messageLen * 6) {
      messageCount = 0;
      donePrinting = true;
    }
    blit();
  }

}
