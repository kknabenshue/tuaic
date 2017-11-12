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

#define MAX_TOUCH_LIMIT 1

// Color definitions.
// ---------------------------------------------------------------------
#define C_BLACK         0x0000
#define C_WHITE         0xFFFF
#define C_RED           0xF800
#define C_GREEN         0x07E0
#define C_BLUE          0x001F
#define C_GREY_25       0xC618
#define C_GREY_50       0x7BEF
#define C_GREY_75       0x39E7

#define C_WDW_BG        C_WHITE

#define C_TAB_ACT       C_WDW_BG
#define C_TAB_INACT     C_GREY_50

#define C_TABLE_BG      C_GREY_25

// Tab area/position definitions.
// ---------------------------------------------------------------------
#define C_TAB_W   160
#define C_TAB_H   50
#define C_X_MIN   0
#define C_Y_MIN   0
#define C_WIDTH   480
#define C_HEIGHT  272

#define C_TXT_TX1 C_TAB_W/2
#define C_TXT_TX2 C_TAB_W + C_TAB_W/2
#define C_TXT_TX3 2*C_TAB_W + C_TAB_W/2
#define C_TXT_TY  C_TAB_H/2

// Window object area/position definitions.
// ---------------------------------------------------------------------
#define C_TABLE_X 120
#define C_TABLE_Y ((C_HEIGHT - C_TAB_H) / 2) + C_TAB_H
#define C_TABLE_R 80


RA8875 tft = RA8875(RA8875_CS, RA8875_RST);

const float rad_conv = 0.0174532925;
char cur_tab = 0;
int angle = 0;


void setup() {
  // Set up serial debug and clear terminal if using Putty.
  Serial.begin(38400);
  Serial.println();
  Serial.println("-----------------------------------------------------------------------------");
  Serial.println("RA8875 started...");

  // Start display.
  tft.begin(RA8875_480x272);

  // Start touch.
  #if defined(USE_FT5206_TOUCH)
  tft.useCapINT(RA8875_INT);
  tft.setTouchLimit(MAX_TOUCH_LIMIT);
  tft.enableCapISR(true);
  #else
  tft.println("Open RA8875UserSettings.h file and uncomment USE_FT5206_TOUCH!");
  #endif
  
  // Welcome screen.
  // ---------------------------------------------------------------------------------------------
  // tft.clearScreen(C_WHITE);
  // tft.setTextColor(C_BLACK, C_WHITE);
  // tft.setCursor(C_WIDTH/2, C_HEIGHT/2-20, true);
  // tft.setFontScale(1.5);
  // tft.print("WELCOME TO THE TUAIC");
  // tft.setCursor(C_WIDTH/2, C_HEIGHT/2+20, true);
  // tft.setFontScale(0.5);
  // tft.print("Touch screen to continue...");
  
  // while (not tft.touched());
  drawTab(1);
  cur_tab = 1;
}

void loop() {
  if (tft.touched()) {
    tft.updateTS(); // Update touch data in library
    
    uint16_t loc_touch[MAX_TOUCH_LIMIT][2]; // To hold touch location coordinates
    tft.getTScoordinates(loc_touch);        // Get touch location.
    
    
    if (loc_touch[0][1] <= C_TAB_H) {   // Y touch location in tab region.
      if (loc_touch[0][0] < C_TAB_W) {  // X touch location in tab 1 region.
        drawTab(1);
        cur_tab = 1;
      }
      else if (loc_touch[0][0] >= C_TAB_W && loc_touch[0][0] < 2*C_TAB_W) {  // X touch location in tab 2 region.
        drawTab(2);
        cur_tab = 2;
      }
      else if (loc_touch[0][0] >= 2*C_TAB_W) {  // X touch location in tab 3 region.
        drawTab(3);
        cur_tab = 3;
      }
    }
    
    
    tft.enableCapISR(); // Rearm touch ISR.
  }
  else {
    if (angle < 360) {
      angle++;
    }
    else {
      angle = 0;
    }
    
    drawBridge(angle);
    
    
    delay(50);
  }
}


// -------------------------------------------------------------------------------------------------
// Draw tab.
// -------------------------------------------------------------------------------------------------
void drawTab(char tab) {
  if (tab == 1 && cur_tab != 1) {
    // Draw tab 1 active.
    // ---------------------------------------------------------------------
    tft.clearScreen(C_WDW_BG);
    
    tft.fillRect(0, 0, C_WIDTH, C_TAB_H, C_TAB_INACT);                // Inactive tab background.
    tft.drawFastHLine(0, C_TAB_H-1, C_WIDTH, C_BLACK);                // Inactive tab bottom line.
    tft.fillRect(0, 0, C_TAB_W, C_TAB_H+1, C_TAB_ACT);                // Active tab background.
    tft.drawFastVLine(C_TAB_W, 0, C_TAB_H, C_BLACK);                  // Tab dividing lines.
    tft.drawFastVLine(2*C_TAB_W, 0, C_TAB_H, C_BLACK);
    
    tft.setTextColor(C_BLACK, C_TAB_ACT);                             // Tab 1 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX1, C_TXT_TY, true);
    tft.print("MANUAL");
    tft.setTextColor(C_BLACK, C_TAB_INACT);                           // Tab 2 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX2, C_TXT_TY, true);
    tft.print("AUTO");
    tft.setTextColor(C_BLACK, C_TAB_INACT);                           // Tab 3 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX3, C_TXT_TY, true);
    tft.print("PROGRAM");
    
    // Draw tab window objects.
    // ---------------------------------------------------------------------
  }
  else if (tab == 2 && cur_tab != 2) {
    // Draw tab 2 active.
    // ---------------------------------------------------------------------
    tft.clearScreen(C_WDW_BG);
    
    tft.fillRect(0, 0, C_WIDTH, C_TAB_H, C_TAB_INACT);                // Inactive tab background.
    tft.drawFastHLine(0, C_TAB_H-1, C_WIDTH, C_BLACK);                // Inactive tab bottom line.
    tft.fillRect(C_TAB_W, 0, C_TAB_W, C_TAB_H+1, C_TAB_ACT);          // Active tab background.
    tft.drawFastVLine(C_TAB_W, 0, C_TAB_H, C_BLACK);                  // Tab dividing lines.
    tft.drawFastVLine(2*C_TAB_W, 0, C_TAB_H, C_BLACK);
    
    tft.setTextColor(C_BLACK, C_TAB_INACT);                           // Tab 1 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX1, C_TXT_TY, true);
    tft.print("MANUAL");
    tft.setTextColor(C_BLACK, C_TAB_ACT);                             // Tab 2 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX2, C_TXT_TY, true);
    tft.print("AUTO");
    tft.setTextColor(C_BLACK, C_TAB_INACT);                           // Tab 3 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX3, C_TXT_TY, true);
    tft.print("PROGRAM");
    
    // Draw tab window objects.
    // ---------------------------------------------------------------------
  }
  else if (tab == 3  && cur_tab != 3) {
    // Draw tab 3 active.
    // ---------------------------------------------------------------------
    tft.clearScreen(C_WDW_BG);
    
    tft.fillRect(0, 0, C_WIDTH, C_TAB_H, C_TAB_INACT);                // Inactive tab background.
    tft.drawFastHLine(0, C_TAB_H-1, C_WIDTH, C_BLACK);                // Inactive tab bottom line.
    tft.fillRect(2*C_TAB_W, 0, C_TAB_W, C_TAB_H+1, C_TAB_ACT);        // Active tab background.
    tft.drawFastVLine(C_TAB_W, 0, C_TAB_H, C_BLACK);                  // Tab dividing lines.
    tft.drawFastVLine(2*C_TAB_W, 0, C_TAB_H, C_BLACK);
    
    tft.setTextColor(C_BLACK, C_TAB_INACT);                           // Tab 1 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX1, C_TXT_TY, true);
    tft.print("MANUAL");
    tft.setTextColor(C_BLACK, C_TAB_INACT);                           // Tab 2 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX2, C_TXT_TY, true);
    tft.print("AUTO");
    tft.setTextColor(C_BLACK, C_TAB_ACT);                             // Tab 3 text.
    tft.setFontScale(1);
    tft.setCursor(C_TXT_TX3, C_TXT_TY, true);
    tft.print("PROGRAM");
    
    // Draw tab window objects.
    // ---------------------------------------------------------------------
  }
  
  // Draw common objects.
  // ---------------------------------------------------------------------
  tft.fillCircle(C_TABLE_X, C_TABLE_Y, C_TABLE_R+3, C_BLACK);   // Draw turn table.
  tft.fillCircle(C_TABLE_X, C_TABLE_Y, C_TABLE_R+1, C_TABLE_BG);
  
  drawBridge(angle);
  
  return;
}


// -------------------------------------------------------------------------------------------------
// Draw table bridge.
// -------------------------------------------------------------------------------------------------
void drawBridge(int angle) {
  static int x1, y1, x2, y2, angle_z1 = 0;
  
  // Remove old bridge.
  // ---------------------------------------------------------------------
  tft.drawLineAngle(C_TABLE_X, C_TABLE_Y, -angle_z1    , C_TABLE_R  , C_TABLE_BG, 0);
  tft.drawLineAngle(C_TABLE_X, C_TABLE_Y, -angle_z1+180, C_TABLE_R  , C_TABLE_BG, 0);
  tft.drawLineAngle(x1       , y1       , -angle_z1    , C_TABLE_R-1, C_TABLE_BG, 0);
  tft.drawLineAngle(x1       , y1       , -angle_z1+180, C_TABLE_R-1, C_TABLE_BG, 0);
  tft.drawLineAngle(x2       , y2       , -angle_z1    , C_TABLE_R-1, C_TABLE_BG, 0);
  tft.drawLineAngle(x2       , y2       , -angle_z1+180, C_TABLE_R-1, C_TABLE_BG, 0);
  
  x1 = C_TABLE_X + floor(-10 * sin(rad_conv * angle));
  y1 = C_TABLE_Y + floor(-10 * cos(rad_conv * angle));
  x2 = C_TABLE_X - floor(-10 * sin(rad_conv * angle));
  y2 = C_TABLE_Y - floor(-10 * cos(rad_conv * angle));
  
  // Draw new bridge.
  // ---------------------------------------------------------------------
  tft.drawLineAngle(C_TABLE_X, C_TABLE_Y, -angle       , C_TABLE_R  , C_RED     , 0);
  tft.drawLineAngle(C_TABLE_X, C_TABLE_Y, -angle+180   , C_TABLE_R  , C_BLACK   , 0);
  tft.drawLineAngle(x1       , y1       , -angle       , C_TABLE_R-1, C_BLACK   , 0);
  tft.drawLineAngle(x1       , y1       , -angle+180   , C_TABLE_R-1, C_BLACK   , 0);
  tft.drawLineAngle(x2       , y2       , -angle       , C_TABLE_R-1, C_BLACK   , 0);
  tft.drawLineAngle(x2       , y2       , -angle+180   , C_TABLE_R-1, C_BLACK   , 0);
  
  angle_z1 = angle;
  
  return;
}















