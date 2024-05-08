#include <Arduino_GFX_Library.h>
#include <Arduino.h>

//Parameters for Screen
#define TFT_CS 33
#define TFT_DC 26
#define TFT_RST 25
#define TFT_SCK 14
#define TFT_MOSI 27
#define TFT_MISO -1  // no data coming back
#define TFT_LED_MOSFET 12
#define E1_OUT_A 34
#define E1_OUT_B 35
#define E1_PUSH 32
#define E2_OUT_A 22
#define E2_OUT_B 21
#define E2_PUSH 23
#define MENU_BUTTON 13


/* More dev device declaration: https://github.com/moononournation/Arduino_GFX/wiki/Dev-Device-Declaration */
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
//Arduino_DataBus *bus = create_default_Arduino_DataBus();
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, VSPI /* spi_num */);

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST, 1 /* rotation */, false /* IPS */);
//Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);


#endif /* !defined(DISPLAY_DEV_KIT) */
/*******************************************************************************
 * End of Arduino_GFX setting
 ******************************************************************************/

//Rotary Encoder Settings
int pulseCount1 = 0; // Rotation step count
int SIG_A_1 = 0; // Pin A output
int SIG_B_1 = 0; // Pin B output
int lastSIG_A_1 = 0; // Last state of SIG_A
int lastSIG_B_1 = 0; // Last state of SIG_B
int SIG_PUSH_1 = 0; //Push

int pulseCount2 = 0;
int SIG_A_2 = 0; // Pin A output
int SIG_B_2 = 0; // Pin B output
int lastSIG_A_2 = 0; // Last state of SIG_A
int lastSIG_B_2 = 0; // Last state of SIG_B
int SIG_PUSH_2 = 0; //Push

//Mode Settings
short mode = 0;
//0 is Splash Screen, 1 is Game Selection, 2 is Addition Mode, 3 is Subtraction Mode, 4 is Multiplicaiotn, 5 is Division, 6 is Quiz Mode

//Text Game Selection
char addName[3] = {'A', 'd', 'd'};
short sum = 0;
short addNum1 = 0;
short addNum2 = 0;

char subName[8] = {'S', 'u', 'b', 't', 'r', 'a', 'c', 't'};
short difference = 0;
short subNum1 = 0;
short subNum2 = 0;

char multiName[8] = {'M', 'u', 'l', 't', 'i', 'p', 'l', 'y'};
short product = 0;
short multNum1 = 0;
short multNum2 = 0;

char divName[8] = {'D', 'i', 'v', 'i', 's', 'i', 'o', 'n'};
char quizName[4] = {'Q', 'u', 'i', 'z'};
short quotient = 0;
short divNum1 = 0;
short divNum2 =0;

short menu1 = 0;

short quizMode = 0;

short currentScore = 0;

//Cursor Variable
short x = 0;
short y = 0;

//Funciton Defintiions
void A_CHANGE_1(); 
void A_CHANGE_2();
void Push_1();
void Push_2();
void updateScreen(bool forward1, bool backwards1, bool forward2, bool backward2, bool push1, bool push2);
uint16_t randomColor();
void squareTransition(uint16_t color);

//Reset Numbers
void resetAdd();
void resetSub();
void resetMult();
void resetDiv();
void resetQuiz();

//Different Functions
void addMode(int push1, int push2);
void subtractMode(int push1, int push2);
void multiplyMode(int push1, int push2);
void divideMode(int push1, int push2);

void setup() {
    //Setup Screen
    gfx->begin();
    gfx->fillScreen(WHITE);

#ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif
  //Finalize screen setup and do opening menu screen
    gfx->setTextSize(5, 5);
    gfx->setCursor(gfx->width()/2-20, gfx->height()/2);
    char Title[10] = {'M', 'a', 't', 'h', 0x20, 'Q', 'u', 'e', 's', 't'};
    for (int i = 0; i < 10; i++) {
      gfx->setTextColor(randomColor());
      gfx->print(Title[i]);
      delay(10);
    }

    delay(1000);

    gfx->fillRect(gfx->width()/2 -20, gfx->height()/2 - 6, 40, 20, WHITE);
    gfx->setCursor(gfx->width()/2-20, gfx->height()/2);
    gfx->print(Title);
    gfx->setCursor(gfx->width()/2-50, gfx->height()/2+6);
    gfx->setTextSize(3, 3);
    gfx->print("Press Enter to Continue");

    //Setup Encoder 1
    SIG_B_1 = digitalRead (E1_OUT_B); // Current state of B
    SIG_A_1 = SIG_B_1 > 0 ? 0 : 1; // Let them be different
    // Attach iterrupt for state change, not rising or falling edges
    attachInterrupt (digitalPinToInterrupt (E1_OUT_A), A_CHANGE_1, CHANGE);
    
    //Setup Encoder 2
    SIG_B_2 = digitalRead (E2_OUT_B); // Current state of B
    SIG_A_2 = SIG_B_1 > 0 ? 0 : 1; // Let them be different
    // Attach iterrupt for state change, not rising or falling edges
    attachInterrupt (digitalPinToInterrupt (E2_OUT_A), A_CHANGE_2, CHANGE);

    // Attach Interrupt to Push-Part of Board
    attachInterrupt (digitalPinToInterrupt(E1_PUSH), Push_1, CHANGE);
    attachInterrupt (digitalPinToInterrupt(E2_PUSH), Push_2, CHANGE);

    updateScreen(0, 0, 0, 0, 0, 0);
}

void loop()
{

}

void A_CHANGE_1() { // Interrupt Service Routine (ISR)
  detachInterrupt (0); // Important
  SIG_A_1 = digitalRead (E1_OUT_A); // Read state of A
  SIG_B_1 = digitalRead (E1_OUT_B); // Read state of B
 
  if ((SIG_B_1 == SIG_A_1) && (lastSIG_B_1 != SIG_B_1)) {
    pulseCount1--; // Counter-clockwise rotation
    lastSIG_B_1 = SIG_B_1;
    updateScreen(0, 1, 0, 0, 0, 0);
  }
 
  else if ((SIG_B_1 != SIG_A_1) && (lastSIG_B_1 == SIG_B_1)) {
    pulseCount1++; // Clockwise rotation
    lastSIG_B_1 = SIG_B_1 > 0 ? 0 : 1; // Save last state of B
    updateScreen(1, 0, 0, 0, 0, 0);
  }
  attachInterrupt (digitalPinToInterrupt (E1_OUT_A), A_CHANGE_1, CHANGE);
}

void A_CHANGE_2() { // Interrupt Service Routine (ISR)
  detachInterrupt (0); // Important
  SIG_A_2 = digitalRead (E2_OUT_A); // Read state of A
  SIG_B_2 = digitalRead (E2_OUT_B); // Read state of B
 
  if ((SIG_B_2 == SIG_A_2) && (lastSIG_B_2 != SIG_B_2)) {
    pulseCount2--; // Counter-clockwise rotation
    lastSIG_B_2 = SIG_B_2;
    updateScreen(0, 0, 0, 1, 0, 0);
  }
 
  else if ((SIG_B_2 != SIG_A_2) && (lastSIG_B_2 == SIG_B_2)) {
    pulseCount2++; // Clockwise rotation
    lastSIG_B_2 = SIG_B_2 > 0 ? 0 : 1; // Save last state of B
    updateScreen(0, 0, 1, 0, 0, 0);
  }
  attachInterrupt (digitalPinToInterrupt (E2_OUT_A), A_CHANGE_2, CHANGE);
}

void Push_1() {
  detachInterrupt(0);
  SIG_PUSH_1 = digitalRead(E1_PUSH);
  SIG_PUSH_2 = 0;
  for (int d = 0; d < 10; d++) {
    if (digitalRead(E2_PUSH) == 1) {
      SIG_PUSH_2 = digitalRead(E2_PUSH);
      break;
    }
    delay(1);
  }
  
  if (SIG_PUSH_1 == 1 & SIG_PUSH_2 == 1) {
    updateScreen(0, 0, 0, 0, 1, 1);
  }
  else {
    updateScreen(0, 0, 0, 0, 1, 0);
  }
  attachInterrupt (digitalPinToInterrupt(E1_PUSH), Push_1, CHANGE);
}

void Push_2() {
  detachInterrupt(0);
  SIG_PUSH_1 = 0;
  SIG_PUSH_2 = digitalRead(E2_PUSH);
  for (int d = 0; d < 10; d++) {
    if (digitalRead(E1_PUSH) == 1) {
      SIG_PUSH_1 = digitalRead(E1_PUSH);
      break;
    }
    delay(1);
  }

  if (SIG_PUSH_1 == 1 && SIG_PUSH_2 == 1) {
    updateScreen(0, 0, 0, 0, 1, 1);
  } else {
    updateScreen(0, 0, 0, 0, 0, 1);
  }
  attachInterrupt (digitalPinToInterrupt(E2_PUSH), Push_2, CHANGE);
}

void updateScreen(bool forward1, bool backwards1, bool forward2, bool backward2, bool push1, bool push2) {
  switch(mode) {
    //Splash Screen
    case 0:
      if (push1 == 1 || push2 == 1) {
        gfx->fillScreen(WHITE);
        mode = 1;
      } else {
        gfx->drawEllipse(random(0, 255), random(0, 255), random(1, 10), random(1, 10), randomColor());
        delay(5000);
      }
      break;
  //Main Menu
   case 1:
      switch(menu1) {
        case 0:
          gfx->fillScreen(RED);
          gfx->setTextColor(WHITE);
          gfx->setCursor(gfx->width()/2 - 30, gfx->height()/2+5);
          gfx->print(addName);
          if (push2 == 1 || push2 == 1) {
            gfx->fillScreen(RED);
            gfx->setTextColor(WHITE);
            resetAdd();
            mode = 2;
          }
          break;
        case 1:
          gfx->fillScreen(BLUE);
          gfx->setTextColor(WHITE);
          gfx->setCursor(gfx->width()/2 - 50, gfx->height()/2+5);
          gfx->print(subName);
          if (push2 == 1 || push2 == 1) {
            gfx->fillScreen(BLUE);
            gfx->setTextColor(WHITE);
            resetSub();
            mode = 3;
          }
          break;
        case 2:
          gfx->fillScreen(GREEN);
          gfx->setTextColor(BLACK);
          gfx->setCursor(gfx->width()/2 - 50, gfx->height()/2+5);
          gfx->print(multiName);
          if (push2 == 1 || push2 == 1) {
            gfx->fillScreen(GREEN);
            gfx->setTextColor(BLACK);
            resetMult();
            mode = 4;
          }
          break;
        case 3:
          gfx->fillScreen(PURPLE);
          gfx->setTextColor(WHITE);
          gfx->setCursor(gfx->width()/2 - 50, gfx->height()/2+5);
          gfx->print(divName);
          if (push2 == 1 || push2 == 1) {
            gfx->fillScreen(PURPLE);
            gfx->setTextColor(WHITE);
            resetDiv();
            mode = 5;
          }
          break;
        case 4:
          gfx->fillScreen(BLACK);
          gfx->setTextColor(WHITE);
          gfx->setCursor(gfx->width()/2 - 50, gfx->height()/2+5);
          gfx->print(quizName);
          if (push2 == 1 || push2 == 1) {
            gfx->fillScreen(BLACK);
            gfx->setTextColor(WHITE);
            resetQuiz();
            mode = 6;
          }
          break;
      }
      break;
    //Additon
    case 2:
      gfx->fillScreen(RED);
      gfx->setTextColor(WHITE);
      addMode(push1, push2);
      break;
    //Subtraction
    case 3:
      gfx->fillScreen(BLUE);
      gfx->setTextColor(WHITE);
      subtractMode(push1, push2);
      break;
    //Multiplication
    case 4:
      gfx->fillScreen(GREEN);
      gfx->setTextColor(BLACK);
      multiplyMode(push1, push2);
      break;
    //Division
    case 5:
      gfx->fillScreen(PURPLE);
      gfx->setTextColor(WHITE);
      divideMode(push1, push2);
      break;
    //Quiz Mode
    case 6:
      gfx->fillScreen(BLACK);
      gfx->setTextColor(WHITE);
      switch(quizMode) {
        case 0:
          addMode(push1, push2);
          break;
        case 1:
          subtractMode(push1, push2);
          break;
        case 2:
          multiplyMode(push1, push2);
          break;
        case 3:
          divideMode(push1, push2);
          break;
      }
      break;
  }
}

uint16_t randomColor() {
  short red = random(0, 255);
  short green = random(0, 255);
  short blue = random(0, 255);
  
  return ( ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3) );
}

void squareTransition(uint16_t color) {
  return; //may or may not be implemented
}
void resetAdd() {
  if (mode != 2) {
    currentScore = 0;
  }
  sum = random(0, 99);
  addNum1 = random(0, 99);
  addNum2 = sum - addNum1;
  pulseCount1 = 0;
  pulseCount2 = 0;
}

void resetSub() {
  if (mode != 3) {
    currentScore = 0;
  }
  subNum1 = random(0, 99);
  subNum2 = random(0, addNum1);
  difference = subNum1 - subNum2;
  pulseCount1 = 0;
  pulseCount2 = 0;
}

void resetMult() {
  if (mode != 4) {
    currentScore = 0;
  }
  multNum1 = random(0, 9);
  multNum2 = random(0, 9);
  product = multNum1 * multNum2;
  pulseCount1 = 0;
  pulseCount2 = 0;
}

void resetDiv() {
  if (mode != 5) {
    currentScore = 0;
  }
  divNum2 = random(0, 9);
  quotient = random(0, 9);
  divNum1 = divNum2 * quotient;
  pulseCount1 = 0;
  pulseCount2 = 0;
}

void resetQuiz() {
  if (mode != 6) {
    currentScore = 0;
  }
  quizMode = random(0, 3);
  switch(quizMode) {
    case 0:
      resetAdd();
      break;
    case 1:
      resetSub();
      break;
    case 2:
      resetMult();
      break;
    case 3:
      resetDiv();
  }
}

void addMode(int push1, int push2) {
  gfx->setCursor(gfx->width()/2-10, gfx->height() + 10);
  gfx->print(currentScore);
  gfx->setCursor(0, gfx->height()/2);
  gfx->print(addNum1); gfx->print(" + "); gfx->print(addNum2); gfx->print(" = ");
  if (pulseCount1 != 0) gfx->print(pulseCount1 % 10);
  else gfx->print(" ");
  gfx->print(pulseCount2 % 10);
  if((push1 && !push2) ||(!push1 && push2)) {
    short calculatedSum = (pulseCount1%10)*10 + pulseCount2%10;
    if (sum == calculatedSum) {
      gfx->setCursor(10, 10);
      gfx->setTextSize(3, 3);
      gfx->print("CORRECT!");
      gfx->setTextSize(5,5);
      currentScore++;
      if (mode == 6) resetQuiz();
      else resetAdd();
      delay(1000);
      updateScreen(0,0,0,0,0,0);
    } else {
      gfx->setCursor(10, 10);
      gfx->setTextSize(3, 3);
      gfx->print("WRONG!");
      gfx->setTextSize(5,5);
      delay(1000);
      updateScreen(0,0,0,0,0,0);
    }
  }
  if (push1 && push2) {
    mode = 1;
    currentScore = 0;
    updateScreen(0,0,0,0,0,0);
  }
}
void subtractMode(int push1, int push2) {
  gfx->setCursor(gfx->width()/2-10, gfx->height() + 10);
  gfx->print(currentScore);
  gfx->setCursor(0, gfx->height()/2);
  gfx->print(subNum1); gfx->print(" - "); gfx->print(subNum2); gfx->print(" = ");
  if (pulseCount1 != 0) gfx->print(pulseCount1 % 10);
  else gfx->print(" ");
  gfx->print(pulseCount2 % 10);
  if((push1 && !push2) ||(!push1 && push2)) {
    short calculatedDifference = (pulseCount1%10)*10 + pulseCount2%10;
    if (difference == calculatedDifference) {
      gfx->setCursor(10, 10);
      gfx->setTextSize(3, 3);
      gfx->print("CORRECT!");
      gfx->setTextSize(5,5);
      currentScore++;
      if (mode == 6) resetQuiz();
      else resetSub();
      delay(1000);
      updateScreen(0,0,0,0,0,0);
    } else {
      gfx->setCursor(10, 10);
      gfx->setTextSize(3, 3);
      gfx->print("WRONG!");
      gfx->setTextSize(5,5);
      delay(1000);
      updateScreen(0,0,0,0,0,0);
    }
  }
  if (push1 && push2) {
    mode = 1;
    currentScore = 0;
    updateScreen(0,0,0,0,0,0);
  }
}
void multiplyMode(int push1, int push2) {
  gfx->setCursor(gfx->width()/2-10, gfx->height() + 10);
  gfx->print(currentScore);
  gfx->setCursor(0, gfx->height()/2);
  gfx->print(multNum1); gfx->print(" * "); gfx->print(multNum2); gfx->print(" = ");
  if (pulseCount1 != 0) gfx->print(pulseCount1 % 10);
  else gfx->print(" ");
  gfx->print(pulseCount2 % 10);
  if((push1 && !push2) ||(!push1 && push2)) {
    short calculatedProduct = (pulseCount1%10)*10 + pulseCount2%10;
    if (product == calculatedProduct) {
      gfx->setCursor(10, 10);
      gfx->setTextSize(3, 3);
      gfx->print("CORRECT!");
      gfx->setTextSize(5,5);
      currentScore++;
      if (mode == 6) resetQuiz();
      else resetMult();
      delay(1000);
      updateScreen(0,0,0,0,0,0);
    } else {
      gfx->setCursor(10, 10);
      gfx->setTextSize(3, 3);
      gfx->print("WRONG!");
      gfx->setTextSize(5,5);
      delay(1000);
      updateScreen(0,0,0,0,0,0);
    }
  }
  if (push1 && push2) {
    mode = 1;
    currentScore = 0;
    updateScreen(0,0,0,0,0,0);
  }
}
void divideMode(int push1, int push2) {
  gfx->setCursor(gfx->width()/2-10, gfx->height() + 10);
  gfx->print(currentScore);
  gfx->setCursor(0, gfx->height()/2);
  gfx->print(divNum1); gfx->print(" ÷ "); gfx->print(divNum2); gfx->print(" = ");
  if (pulseCount1 != 0) gfx->print(pulseCount1 % 10);
  else gfx->print(" ");
  gfx->print(pulseCount2 % 10);
  if((push1 && !push2) ||(!push1 && push2)) {
    short calculatedQuotient = (pulseCount1%10)*10 + pulseCount2%10;
    if (quotient == calculatedQuotient) {
      gfx->setCursor(10, 10);
      gfx->setTextSize(3, 3);
      gfx->print("CORRECT!");
      gfx->setTextSize(5,5);
      currentScore++;
      if (mode == 6) resetQuiz();
      else resetDiv();
      delay(1000);
      updateScreen(0,0,0,0,0,0);
    } else {
      gfx->setCursor(10, 10);
      gfx->setTextSize(3, 3);
      gfx->print("WRONG!");
      gfx->setTextSize(5,5);
      delay(1000);
      updateScreen(0,0,0,0,0,0);
    }
  }
  if (push1 && push2) {
    mode = 1;
    currentScore = 0;
    updateScreen(0,0,0,0,0,0);
  }
}