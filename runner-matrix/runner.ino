#include "LedControl.h"

LedControl lc = LedControl(7, 6, 5, 1);

unsigned long delaytime = 300;
int currpos[] = {7, 3};

void setup() {
  lc.shutdown(0, false);
  lc.setIntensity(0, 3);
  lc.clearDisplay(0);
}

void loop() {
  initializePlayer();
  
  for (int i = 0; i < 10; i++) {
    unsigned long tlInd = random(0, 7);
    cubeFall(tlInd);
  }
  scatter();
}

void initializePlayer() {
  lc.setLed(0, currpos[0], currpos[1], true);
}

void moveLeft() {
  lc.setLed(0, currpos[0], currpos[1], false);
  currpos[1] -= 1; 
  lc.setLed(0, currpos[0], currpos[1], true);
}

void cubeFall(unsigned long tlInd) {
  lc.setLed(0, 0, tlInd, true);
  lc.setLed(0, 0, tlInd + 1, true);
  lc.setLed(0, 1, tlInd, true);
  lc.setLed(0, 1, tlInd + 1, true);
  
  for (int row = 0; row < 8; row++) {
    lc.setLed(0, row, tlInd, false);
    lc.setLed(0, row, tlInd + 1, false);
    lc.setLed(0, row - 1, tlInd, false);
    lc.setLed(0, row - 1, tlInd + 1, false);
    
    lc.setLed(0, row, tlInd, true);
    lc.setLed(0, row, tlInd + 1, true);
    lc.setLed(0, row + 1, tlInd, true);
    lc.setLed(0, row + 1, tlInd + 1, true);
    delay(delaytime / 2);
  }
  lc.setLed(0, 7, tlInd, false);
  lc.setLed(0, 7, tlInd + 1, false);

  // If cube touches player
  if (tlInd == currpos[1] || tlInd + 1 == currpos[1]) {
    lc.clearDisplay(0);
    death();
  }

}

void scatter() {
  for (int dots = 0; dots < 30; dots++) {
    unsigned long dotInd = random(0, 7);

    for (int row = 0; row < 8; row++) {
      lc.setLed(0, row - 1, dotInd, false);
      lc.setLed(0, row, dotInd, true);
      delay(delaytime / 10);
    }
    lc.setLed(0, 7, dotInd, false);

    // If dot touches player
    if (dotInd == currpos[1]) {
      lc.clearDisplay(0);
      death();
    }
    
  }
}

void death() {
  
  byte d[8] = { B00111000, B00100100, B00100010, B00100010, B00100010, B00100010, B00100100, B00111000 };
  byte e[8] = { B00111000, B00100000, B00100000, B00111000, B00100000, B00100000, B00100000, B00111000 };
  byte a[8] = { B00011000, B00100100, B01000010, B01000010, B01111110, B01000010, B01000010, B01000010 };

  for (int i = 0; i < 7; i++) {
    lc.setRow(0, i, d[i]);
  }
  delay(500);

  for (int i = 0; i < 7; i++) {
    lc.setRow(0, i, e[i]);
  }
  delay(500);

  for (int i = 0; i < 7; i++) {
    lc.setRow(0, i, a[i]);
  }
  delay(500);
  
  for (int i = 0; i < 7; i++) {
    lc.setRow(0, i, d[i]);
  }
  delay(5000);
  lc.clearDisplay(0);
  initializePlayer();
}