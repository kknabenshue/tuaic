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
#define C_X_MIN     0
#define C_Y_MIN     0
#define C_WIDTH     272
#define C_HEIGHT    480

#define C_TAB_W     ceil(C_WIDTH/3.0)
#define C_TAB_H     30

// <todo>: Create buffer size helper functions.
#define C_TXT_TX1   C_TAB_W/2 - (6 * tft.getFontWidth())/2
#define C_TXT_TX2   C_TAB_W + C_TAB_W/2 - (4 * tft.getFontWidth())/2
#define C_TXT_TX3   2*C_TAB_W + C_TAB_W/2 - (7 * tft.getFontWidth())/2
#define C_TXT_TY    C_TAB_H/2 - tft.getFontHeight()/2
#define C_TXT_TS    0.95

// Window object area/position definitions.
// ---------------------------------------------------------------------
#define C_TABLE_R   50
#define C_TABLE_X   C_WIDTH/2
#define C_TABLE_Y   C_TAB_H + C_TABLE_R + 30

#define C_CTRL_B_W  87
#define C_CTRL_B_H  60
#define C_CTRL_W    3 * C_CTRL_B_W + 1
#define C_CTRL_H    4 * C_CTRL_B_H + 1
#define C_CTRL_BRD  (C_WIDTH - C_CTRL_W) / 2 - 1
#define C_CTRL_X    C_CTRL_BRD
#define C_CTRL_Y    C_HEIGHT - C_CTRL_H - C_CTRL_BRD



RA8875 tft = RA8875(RA8875_CS, RA8875_RST);

const uint8_t C_SW_REV_MAJ = 0;
const uint8_t C_SW_REV_MIN = 0;
const uint8_t C_SW_REV_PCH = 1;

const float C_RAD_CONV = 0.0174532925;
const char C_KEY_LAB[4][3] = {
  {'7', '8', '9'},
  {'4', '5', '6'},
  {'1', '2', '3'},
  {'<', '0', '>'}
};

uint8_t cur_tab = 0;
int angle = 0;

// -------------------------------------------------------------------------------------------------
// Setup/initialize.
// -------------------------------------------------------------------------------------------------
void setup() {
  // Set up serial debug and clear terminal if using Putty.
  Serial.begin(74880);
  Serial.println();
  Serial.println("-----------------------------------------------------------------------------");
  Serial.println("RA8875 started...");

  // Start display.
  tft.begin(RA8875_480x272);
  tft.setRotation(3);

  // Start touch.
  #if defined(USE_FT5206_TOUCH)
  tft.useCapINT(RA8875_INT);
  tft.setTouchLimit(MAX_TOUCH_LIMIT);
  tft.enableCapISR(true);
  #else
  tft.println("Open RA8875UserSettings.h file and uncomment USE_FT5206_TOUCH!");
  #endif
  
  // Welcome screen.
  // ---------------------------------------------------------------------
  tft.clearScreen(C_WHITE);
  tft.setTextColor(C_BLACK);
  tft.setFontScale(2);
  tft.setCursor(CENTER, C_HEIGHT/2-57, false);
  tft.print("TUAIC");
  tft.setFontScale(0.5);
  tft.setCursor(CENTER, CENTER, false);
  tft.print("v" + String(C_SW_REV_MAJ) + "." + String(C_SW_REV_MIN) + "." + String(C_SW_REV_PCH));
  tft.setCursor(CENTER, C_HEIGHT/2+10, false);
  tft.setFontScale(0.5);
  tft.print("Touch screen to continue...");
  
  while (not tft.touched());
  drawTab(1);
  cur_tab = 1;
}

// -------------------------------------------------------------------------------------------------
// Main loop.
// -------------------------------------------------------------------------------------------------
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
    // drawCtrl(cur_tab);
    
    
    delay(50);
  }
}


// -------------------------------------------------------------------------------------------------
// Draw tab.
// -------------------------------------------------------------------------------------------------
void drawTab(char tab) {
  if (tab != cur_tab) {
    tft.clearScreen(C_WDW_BG);  // <todo>: Only clear changing tabs. Based on tab and cur_tab.
    
    tft.fillRect(0, 0, C_WIDTH, C_TAB_H, C_TAB_INACT);                    // Inactive tab background.
    tft.drawFastHLine(0, C_TAB_H-1, C_WIDTH, C_BLACK);                    // Inactive tab bottom line.
    tft.drawFastVLine(C_TAB_W-1, 0, C_TAB_H, C_BLACK);                    // Tab dividing lines.
    tft.drawFastVLine(2*C_TAB_W-1, 0, C_TAB_H, C_BLACK);
    tft.fillRect((tab-1) * C_TAB_W, 0, C_TAB_W-1, C_TAB_H+1, C_TAB_ACT);  // Active tab background.
    
    tft.setTextColor(C_BLACK);                  // Tab 1 text.
    tft.setFontScale(C_TXT_TS);
    tft.setCursor(C_TXT_TX1, C_TXT_TY, false);
    tft.print("MANUAL");
    tft.setTextColor(C_BLACK);                  // Tab 2 text.
    tft.setFontScale(C_TXT_TS);
    tft.setCursor(C_TXT_TX2, C_TXT_TY, false);
    tft.print("AUTO");
    tft.setTextColor(C_BLACK);                  // Tab 3 text.
    tft.setFontScale(C_TXT_TS);
    tft.setCursor(C_TXT_TX3, C_TXT_TY, false);
    tft.print("PROGRAM");
  }
  
  // Draw common objects.
  // ---------------------------------------------------------------------
  tft.fillCircle(C_TABLE_X, C_TABLE_Y, C_TABLE_R+3, C_BLACK);   // Draw turn table.
  tft.fillCircle(C_TABLE_X, C_TABLE_Y, C_TABLE_R+1, C_TABLE_BG);
  
  drawBridge(angle);
  drawCtrl(tab);
  
  return;
}


// -------------------------------------------------------------------------------------------------
// Draw table bridge.
// -------------------------------------------------------------------------------------------------
void drawBridge(int angle) {
  static boolean init = 1;
  static int x1, y1, x2, y2, angle_z1;
  
  if (init == 0) {
    // Remove old bridge.
    // ---------------------------------------------------------------------
    tft.drawLineAngle(C_TABLE_X, C_TABLE_Y, -angle_z1    , C_TABLE_R  , C_TABLE_BG, 0);
    tft.drawLineAngle(C_TABLE_X, C_TABLE_Y, -angle_z1+180, C_TABLE_R  , C_TABLE_BG, 0);
    tft.drawLineAngle(x1       , y1       , -angle_z1    , C_TABLE_R-1, C_TABLE_BG, 0);
    tft.drawLineAngle(x1       , y1       , -angle_z1+180, C_TABLE_R-1, C_TABLE_BG, 0);
    tft.drawLineAngle(x2       , y2       , -angle_z1    , C_TABLE_R-1, C_TABLE_BG, 0);
    tft.drawLineAngle(x2       , y2       , -angle_z1+180, C_TABLE_R-1, C_TABLE_BG, 0);
  }
  else {
    init = 0;
  }
  
  x1 = C_TABLE_X + floor(-10 * sin(C_RAD_CONV * angle));
  y1 = C_TABLE_Y + floor(-10 * cos(C_RAD_CONV * angle));
  x2 = C_TABLE_X - floor(-10 * sin(C_RAD_CONV * angle));
  y2 = C_TABLE_Y - floor(-10 * cos(C_RAD_CONV * angle));
  
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


// -------------------------------------------------------------------------------------------------
// Draw control pad.
// -------------------------------------------------------------------------------------------------
void drawCtrl(int tab) {
  int bc_x, bc_y, txt_w, txt_h;
  
  tft.setActiveWindow(C_CTRL_X, C_CTRL_X + C_CTRL_W, C_CTRL_Y, C_CTRL_Y + C_CTRL_H);
  tft.clearActiveWindow(false);
  // tft.fillWindow(C_BLACK);
  
  if (tab == 1) {
    tft.setTextColor(C_BLACK);
    tft.setFontScale(2);
    txt_w = tft.getFontWidth();
    txt_h = tft.getFontHeight();
    
    // Draw button rectangles.
    tft.drawRect(C_WIDTH/2 - C_CTRL_B_W - 10, C_CTRL_Y + C_CTRL_H - C_CTRL_B_H - 14, C_CTRL_B_W + 1, C_CTRL_B_H + 1, C_BLACK);
    tft.drawRect(C_WIDTH/2 + 10             , C_CTRL_Y + C_CTRL_H - C_CTRL_B_H - 14, C_CTRL_B_W + 1, C_CTRL_B_H + 1, C_BLACK);
    
    // Draw CCW arrow.
    bc_x = C_WIDTH/2 - C_CTRL_B_W - 10 + C_CTRL_B_W/2;
    bc_y = C_CTRL_Y + C_CTRL_H - C_CTRL_B_H - 14 + C_CTRL_B_H/2;
    
    tft.fillCircle(bc_x, bc_y + 1, txt_h/2 - 8, C_BLACK);
    tft.fillCircle(bc_x, bc_y + 1, txt_h/2 - 10, C_WDW_BG);
    tft.fillRect(bc_x, bc_y - 10, txt_h/2, 16, C_WDW_BG);
    tft.drawLine(bc_x + txt_h/2 - 15 - 1, bc_y     + 6, bc_x + txt_h/2 - 8 - 1, bc_y     + 6, C_BLACK);
    tft.drawLine(bc_x + txt_h/2 - 15 - 1, bc_y + 1 + 6, bc_x + txt_h/2 - 8 - 1, bc_y + 1 + 6, C_BLACK);
    tft.drawLine(bc_x + txt_h/2 - 8  - 1, bc_y     + 6, bc_x + txt_h/2 - 8 - 1, bc_y + 7 + 6, C_BLACK);
    tft.drawLine(bc_x + txt_h/2 - 7  - 1, bc_y     + 6, bc_x + txt_h/2 - 7 - 1, bc_y + 7 + 6, C_BLACK);
    
    // Draw CW arrow.
    bc_x = C_WIDTH/2 + 10 + C_CTRL_B_W/2;
    bc_y = C_CTRL_Y + C_CTRL_H - C_CTRL_B_H - 14 + C_CTRL_B_H/2;
    
    tft.fillCircle(bc_x, bc_y + 1, txt_h/2 - 8, C_BLACK);
    tft.fillCircle(bc_x, bc_y + 1, txt_h/2 - 10, C_WDW_BG);
    tft.fillRect(bc_x - txt_h/2, bc_y - 10, txt_h/2, 16, C_WDW_BG);
    tft.drawLine(bc_x - txt_h/2 + 15 + 1, bc_y     + 6, bc_x - txt_h/2 + 8 + 1, bc_y     + 6, C_BLACK);
    tft.drawLine(bc_x - txt_h/2 + 15 + 1, bc_y + 1 + 6, bc_x - txt_h/2 + 8 + 1, bc_y + 1 + 6, C_BLACK);
    tft.drawLine(bc_x - txt_h/2 + 8  + 1, bc_y     + 6, bc_x - txt_h/2 + 8 + 1, bc_y + 7 + 6, C_BLACK);
    tft.drawLine(bc_x - txt_h/2 + 7  + 1, bc_y     + 6, bc_x - txt_h/2 + 7 + 1, bc_y + 7 + 6, C_BLACK);
    
    // Speed slider.
    tft.drawFastHLine(C_CTRL_X + 20, C_CTRL_Y + C_CTRL_H - C_CTRL_B_H - 80, C_CTRL_W - 40, C_BLACK);
    tft.fillRect(C_CTRL_X + 60, C_CTRL_Y + C_CTRL_H - C_CTRL_B_H - 80 - 25, 25, 50, C_WDW_BG);  // <todo>: Replace x with current slider position value.
    tft.drawRect(C_CTRL_X + 60, C_CTRL_Y + C_CTRL_H - C_CTRL_B_H - 80 - 25, 25, 50, C_BLACK);
    
    tft.setFontScale(2);
    tft.setCursor(CENTER, C_CTRL_Y + C_CTRL_H - C_CTRL_B_H - 105 - txt_h - 35, false);
    tft.print("SPEED: " + String(20));  // <todo>: Replace with 0 to 99 mapped speed.
  }
  else if (tab == 2 || tab == 3) {
    tft.setTextColor(C_BLACK);
    tft.setFontScale(2);
    txt_w = tft.getFontWidth();
    txt_h = tft.getFontHeight();
    
    for (int i = 0; i <= 4; i++) {  // Draw horizontal keypad lines.
      tft.drawFastHLine(C_CTRL_X, C_CTRL_Y + (i * C_CTRL_B_H), C_CTRL_W, C_BLACK);
    }
    
    for (int i = 0; i <= 3; i++) {  // Draw vertical keypad lines.
      tft.drawFastVLine(C_CTRL_X + (i * C_CTRL_B_W), C_CTRL_Y, C_CTRL_H, C_BLACK);
    }
    
    for (int i = 0; i <= 2; i++) {
      for (int j = 0; j <= 3; j++) {
        bc_x = C_CTRL_X + (i * C_CTRL_B_W) + (C_CTRL_B_W / 2);
        bc_y = C_CTRL_Y + (j * C_CTRL_B_H) + (C_CTRL_B_H / 2);
        
        if (i == 0 && j == 3) {       // Draw CCW arrow.
          tft.fillCircle(bc_x, bc_y + 1, txt_h/2 - 8, C_BLACK);
          tft.fillCircle(bc_x, bc_y + 1, txt_h/2 - 10, C_WDW_BG);
          tft.fillRect(bc_x, bc_y - 10, txt_h/2, 16, C_WDW_BG);
          tft.drawLine(bc_x + txt_h/2 - 15 - 1, bc_y     + 6, bc_x + txt_h/2 - 8 - 1, bc_y     + 6, C_BLACK);
          tft.drawLine(bc_x + txt_h/2 - 15 - 1, bc_y + 1 + 6, bc_x + txt_h/2 - 8 - 1, bc_y + 1 + 6, C_BLACK);
          tft.drawLine(bc_x + txt_h/2 - 8  - 1, bc_y     + 6, bc_x + txt_h/2 - 8 - 1, bc_y + 7 + 6, C_BLACK);
          tft.drawLine(bc_x + txt_h/2 - 7  - 1, bc_y     + 6, bc_x + txt_h/2 - 7 - 1, bc_y + 7 + 6, C_BLACK);
        }
        else if (i == 2 && j == 3) {  // Draw CW arrow.
          tft.fillCircle(bc_x, bc_y + 1, txt_h/2 - 8, C_BLACK);
          tft.fillCircle(bc_x, bc_y + 1, txt_h/2 - 10, C_WDW_BG);
          tft.fillRect(bc_x - txt_h/2, bc_y - 10, txt_h/2, 16, C_WDW_BG);
          tft.drawLine(bc_x - txt_h/2 + 15 + 1, bc_y     + 6, bc_x - txt_h/2 + 8 + 1, bc_y     + 6, C_BLACK);
          tft.drawLine(bc_x - txt_h/2 + 15 + 1, bc_y + 1 + 6, bc_x - txt_h/2 + 8 + 1, bc_y + 1 + 6, C_BLACK);
          tft.drawLine(bc_x - txt_h/2 + 8  + 1, bc_y     + 6, bc_x - txt_h/2 + 8 + 1, bc_y + 7 + 6, C_BLACK);
          tft.drawLine(bc_x - txt_h/2 + 7  + 1, bc_y     + 6, bc_x - txt_h/2 + 7 + 1, bc_y + 7 + 6, C_BLACK);
        }
        else {
          tft.setCursor(bc_x - txt_w/2, bc_y - txt_h/2 - 2, false);
          tft.print(C_KEY_LAB[j][i]);
        }
      }
    }
  }
  
  tft.setActiveWindow();
  return;
}















