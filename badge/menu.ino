/*
 * menu
 * 
 * Display a menu
 * 
 * The custom message function is just a menu
 */

#include "constants.h"
#include <EEPROM.h>

// Menu static text
const char customMenu0[] PROGMEM = "Custom Message ";
const char customMenu1[] PROGMEM = "Show Message ";
const char customMenu2[] PROGMEM = "Set Message ";
const char customMenu4[] PROGMEM = "Back";

badgeStruct customMenu[] = {
  {
    customMenu0,
    NULL
  },
  {
    customMenu1,
    (*showCustomMessage)
  },
  {
    customMenu2,
    (*setCustomMessage)
  },
  {
    customMenu4,
    NULL
  }
};

void showCustomMessage() {
  unsigned char messageString[100];

  // Read the message from the eeprom
  for (int i = 0; i < 100; i++) {
    byte the_char = EEPROM.read(i);
    messageString[i] = the_char;
    if (the_char == 0) {
      break;
    }
  }

  setMessage(messageString, false);

  // Print the message until a keypress
  while(1) { 
    LOOP(0); 
    if (CUR_BUTTON && (CUR_BUTTON ^ OLD_BUTTON)) {
      return;
    }
    showMessage();
  }
}

void setCustomMessage() {
  unsigned char newString[100];
  getString(newString);
  for (int i = 0; i < 100; i++) {
    EEPROM.write(i, newString[i]);
    if (newString[i] == 0) break;
  }
}

uint8_t customMenuLen = sizeof(customMenu) / sizeof(customMenu[0]);

void customMessage() {
  showMenu(customMenu, customMenuLen, 10000);
}


void showMenu(badgeStruct *thePrograms, uint8_t menuMax, uint16_t menuTimeout) {
  uint8_t menuIndex = 0;
  bool menuChanged = false;
  
  setMessage(thePrograms[0].programName, true);
  while (true) {
    LOOP(menuTimeout);

    
    if (NEW_BUTTON(BTN_DOWN)) {

      if (menuIndex < (menuMax - 1)) {
        menuIndex++;
        menuChanged = true;
      }
    } else if (NEW_BUTTON(BTN_UP)) {
      if (menuIndex > 0) {
        menuIndex--;
        menuChanged = true;
      }
    } else if (NEW_BUTTON(BTN_A)) {
      if (thePrograms[menuIndex].badgeFunction != NULL) {
        (*thePrograms[menuIndex].badgeFunction)();
        menuIndex = 0;
        menuChanged = true;
      } else {
        if (menuIndex > 0) return;
      }
    }
    if (menuChanged) {
      menuChanged = false;
      setMessage(thePrograms[menuIndex].programName, true);
    }

    showMessage();
  }
}
