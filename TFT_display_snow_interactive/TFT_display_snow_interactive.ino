#include "Adafruit_RA8875.h"
#include "pins.h"
#include "calibrate.h"

// TFT Display resolution
const int DISPLAY_WIDTH = 800;
const int DISPLAY_HEIGHT = 480;

// "Physics" engine grid size
const int PHY_DISP_RATIO = 11; // Physical cell size in pixels
const int BITS_PER_CELL = 1;
const int CELLS_PER_BYTE = 8 / BITS_PER_CELL;
const int PHY_WIDTH = DISPLAY_WIDTH / PHY_DISP_RATIO;
const int PHY_HEIGHT = DISPLAY_HEIGHT / PHY_DISP_RATIO;

const int GRID_TOTAL_CELLS = PHY_WIDTH * PHY_HEIGHT;
const int GRID_SIZE_BYTES = GRID_TOTAL_CELLS / CELLS_PER_BYTE;
byte grid[GRID_SIZE_BYTES];

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS_PIN, RA8875_RESET_PIN);

const uint16_t BACKGROUND_COLOR = 0b0000000000000111;
const uint16_t SNOW_COLOR       = 0b1111111111111111;

// To measure FPS
unsigned long loopSpeedCounter = 0;
unsigned long previousReportTime = millis();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("");
  Serial.println("====================================================");
  Serial.println("Arduino started");

  Serial.println("Initializing RA8875...");
  if (tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 display controller init OK");
  } else {
    Serial.println("RA8875 display controller init FAILED");
  }

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  Serial.print("TFT Status: "); Serial.println(tft.readStatus(), HEX);

  tft.fillScreen(RA8875_GREEN);
  tft.textMode();
  const char msg[] = "Copyright (c) 2021 River Champeimont";
  tft.textSetCursor(0, 0);
  tft.textColor(RA8875_BLACK, RA8875_GREEN);
  tft.textWrite(msg, sizeof(msg) - 1);
  tft.graphicsMode();
  delay(3000);

  // Setup touch screen
  tft.touchEnable(true);
  setupCalibration();

  Serial.print("Number of grid cells: ");
  Serial.println(GRID_TOTAL_CELLS);
  Serial.print("Size of grid in bytes: ");
  Serial.println(GRID_SIZE_BYTES);

  memset(grid, 0, GRID_SIZE_BYTES);

  tft.fillScreen(BACKGROUND_COLOR);
}

byte getCellValue(int row, int col) {
  int cellIndex = row * PHY_WIDTH + col;
  return (grid[cellIndex / CELLS_PER_BYTE] >> (cellIndex % CELLS_PER_BYTE)) & 1;
}

void setCellValueTo0(int row, int col) {
  int cellIndex = row * PHY_WIDTH + col;
  grid[cellIndex / CELLS_PER_BYTE] &= ~(1 << (cellIndex % CELLS_PER_BYTE));
  renderVoid(row, col);
}

void setCellValueTo1(int row, int col) {
  int cellIndex = row * PHY_WIDTH + col;
  grid[cellIndex / CELLS_PER_BYTE] |= 1 << (cellIndex % CELLS_PER_BYTE);
  renderFlake(row, col);
}

void renderFlake(int row, int col) {
  if (random(2)) {
    tft.drawFastVLine(col * PHY_DISP_RATIO + PHY_DISP_RATIO / 2, row * PHY_DISP_RATIO, PHY_DISP_RATIO - 1, SNOW_COLOR);
    tft.drawFastHLine(col * PHY_DISP_RATIO, row * PHY_DISP_RATIO + PHY_DISP_RATIO / 2, PHY_DISP_RATIO - 1, SNOW_COLOR);
  } else {
    tft.drawLine(col * PHY_DISP_RATIO, row * PHY_DISP_RATIO, col * PHY_DISP_RATIO + PHY_DISP_RATIO - 1, row * PHY_DISP_RATIO + PHY_DISP_RATIO - 1, SNOW_COLOR);
    tft.drawLine(col * PHY_DISP_RATIO, row * PHY_DISP_RATIO + PHY_DISP_RATIO - 1, col * PHY_DISP_RATIO + PHY_DISP_RATIO - 1, row * PHY_DISP_RATIO, SNOW_COLOR);
  }
}

void renderVoid(int row, int col) {
  tft.fillRect(col * PHY_DISP_RATIO, row * PHY_DISP_RATIO, PHY_DISP_RATIO, PHY_DISP_RATIO, BACKGROUND_COLOR);
}

void handleTouch() {
  // Create snow where user touches screen
  uint16_t tx, ty, dx, dy;
  int row, col;
  if (tft.touched()) {
    tft.touchRead(&tx, &ty);

    // Translate touch screen coordinates to display coordinates
    if (translateTouchCoords(tx, ty, &dx, &dy) == 0) {
      // Translate display coordinates to cell coordinates
      col = dx / PHY_DISP_RATIO;
      row = dy / PHY_DISP_RATIO;
      if (col >= 0 && col < PHY_WIDTH && row >= 0 && row < PHY_HEIGHT) {
        if (getCellValue(row, col) == 0) {
          setCellValueTo1(row, col);
        }
      } else {
        Serial.println("Touched position is outside grid.");
      }
    }
  }
}

void loop() {
  unsigned long now = millis();

  // Make it random
  randomSeed(millis());

  // Simulate falling snow
  for (int row = PHY_HEIGHT - 2; row >= 0; row--) {
    for (int col = 0; col < PHY_WIDTH; col++) {
      if (getCellValue(row, col) == 1) {
        int futureCol = random(col - 1, col + 2);
        // Check snow flake did not leave screen and future cell is empty.
        if (futureCol >= 0 && futureCol < PHY_WIDTH && getCellValue(row + 1, futureCol) == 0) {
          setCellValueTo1(row + 1, futureCol);
          setCellValueTo0(row, col);
        }
      }
    }
  }

  // Create some new random snow at the top of the screen
  for (int col = 0; col < PHY_WIDTH; col++) {
    if (random(500) < 1) {
      if (getCellValue(0, col) == 0) {
        setCellValueTo1(0, col);
      }
    }
  }

  // Create snow where the user touches
  handleTouch();

  // To compute FPS
  if (now >= previousReportTime + 1000) {
    Serial.print(loopSpeedCounter);
    Serial.println(" FPS");
    loopSpeedCounter = 0;
    previousReportTime = now;
  }

  loopSpeedCounter++;
}
