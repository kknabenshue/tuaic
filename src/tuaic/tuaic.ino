#include <Arduino.h>

// TUAIC top level source.

#include <SPI.h>
#include <RA8875.h>
#include <Wire.h>

// Uses built in SPI:
// MOSI: Pin 11.
// MISO: Pin 12.
// SCK:  Pin 13.

#define RA8875_CS 10
#define RA8875_RST 9
#define RA8875_INT 2

#define MAX_TOUCH_LIMIT 2

RA8875 tft = RA8875(RA8875_CS, RA8875_RST);


void setup() {
  // Set up serial debug and clear terminal if using Putty.
  Serial.begin(38400);
  Serial.println();
  Serial.println("-----------------------------------------------------------------------------");
  Serial.println("RA8875 started...");

  // Start display.
  tft.begin(RA8875_480x272);
  tft.println("RA8875 started...");
  tft.println("Tap the screen to continue...");
  tft.setTextColor(RA8875_WHITE,RA8875_BLACK);

  // Start touch.
  #if defined(USE_FT5206_TOUCH)
  tft.useCapINT(RA8875_INT);
  tft.setTouchLimit(MAX_TOUCH_LIMIT);
  tft.enableCapISR(true);
  #else
  tft.println("You should open RA8875UserSettings.h file and uncomment USE_FT5206_TOUCH!");
  #endif
}

void loop() {
  #if defined(USE_FT5206_TOUCH)
  if (tft.touched()){//if touched(true) detach isr
  // //at this point we need to fill the FT5206 registers...
  //   tft.updateTS();//now we have the data inside library
  //   tft.setCursor(CENTER,CENTER);
  //   tft.print("                              ");
  //   tft.setCursor(CENTER,CENTER);
  //   tft.print("touches:");
  //   tft.print(tft.getTouches());
  //   tft.print(" | gesture:");
  //   tft.print(tft.getGesture(),HEX);
  //   tft.print(" | state:");
  //   tft.print(tft.getTouchState(),HEX);
  //   //you need to get the coordinates? We need a bidimensional array
  //   uint16_t coordinates[MAX_TOUCH_LIMIT][2];//to hold coordinates
  //   tft.getTScoordinates(coordinates);//done
  //   //now coordinates has the x,y of all touches
  //   //now draw something....
  //   uint16_t tempCol;
  //   for (uint8_t i=1;i<=tft.getTouches();i++){
  //     if (i == 1)tempCol = RA8875_RED;
  //     if (i == 2)tempCol = RA8875_GREEN;
  //     if (i == 3)tempCol = RA8875_MAGENTA;
  //     if (i == 4)tempCol = RA8875_CYAN;
  //     if (i == 5)tempCol = RA8875_YELLOW;
  //     tft.fillCircle(coordinates[i-1][0],coordinates[i-1][1],10,tempCol);
  //   }
  //   tft.enableCapISR();//rearm ISR if needed (touched(true))
  //   //otherwise it doesn't do nothing...
    Serial.println("Screen was touched");
    tft.clearScreen(RA8875_WHITE);
    tft.setTextColor(RA8875_BLACK, RA8875_WHITE);
    tft.setCursor(CENTER, CENTER);
    // tft.setFontSize(20);
    tft.setFontScale(1.5);
    tft.print("WELCOME TO THE TUAIC");
  }
  #endif
}
