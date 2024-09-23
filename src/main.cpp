#include <Arduino_GFX_Library.h>
#include <Arduino.h>

//Parameters for Screen
#define TFT_CS 5
#define TFT_DC 27
#define TFT_RST 33
#define TFT_SCK 18
#define TFT_MOSI 23
#define TFT_MISO 29
#define TFT_LED 22
#define E1_OUT_A 26
#define E1_OUT_B 25
#define E1_PUSH 35
#define E2_OUT_A 12
#define E2_OUT_B 14
#define E2_PUSH 21
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
unsigned long _lastIncReadTime = micros();
unsigned long _lastDecReadTime = micros();
int _pauseLength = 25000;
int _fastIncrement = 10;

int counter = 0;
int counter_2 = 0;

bool push = 0;
bool push_2 = 0;
bool lastPush = 0;

void read_encoder();
void read_encoder_2();

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

bool lastMenu = 0;

//Cursor Variable
short x = 0;
short y = 0;


//Funciton Defintiions
void A_CHANGE_1(); 
void A_CHANGE_2();
void Push_1();
void Push_2();
void updateScreen(bool forward1, bool backwards1, bool forward2, bool backward2, bool push, bool back);
uint16_t randomColor();
void squareTransition(uint16_t color);

//Reset Numbers
void resetAdd();
void resetSub();
void resetMult();
void resetDiv();
void resetQuiz();

//Different Functions
void addMode(int push, int back);
void subtractMode(int push, int back);
void multiplyMode(int push, int back);
void divideMode(int push, int pubacksh2);

void setup() {
pinMode(TFT_LED, OUTPUT);
digitalWrite(TFT_LED, HIGH);
//Setup Screen
gfx->begin();
gfx->fillScreen(WHITE);
Serial.begin(9600);

#ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
#endif
  //Finalize screen setup and do opening menu screen
    gfx->setTextSize(5, 5);
    gfx->setCursor(20, gfx->height()/2-5);
    char Title[10] = {'M', 'a', 't', 'h', 0x20, 'Q', 'u', 'e', 's', 't'};
    for (int i = 0; i < 10; i++) {
      gfx->setTextColor(randomColor());
      gfx->print(Title[i]);
      delay(100);
    }

    delay(1000);

gfx->fillScreen(WHITE);
gfx->setCursor(20, gfx->height()/2-5);
gfx->print("Math Quest");
gfx->setCursor(20, gfx->height()/2+50);
gfx->setTextSize(2, 2);
gfx->print("Press Enter to Continue");

// Set encoder pins and attach interrupts
pinMode(E1_OUT_A, INPUT_PULLUP);
pinMode(E1_OUT_B, INPUT_PULLUP);
pinMode(E2_OUT_A, INPUT_PULLUP);
pinMode(E2_OUT_B, INPUT_PULLUP);
pinMode(E1_PUSH, INPUT_PULLUP);
pinMode(E2_PUSH, INPUT_PULLUP);
pinMode(MENU_BUTTON, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(E1_OUT_A), read_encoder, CHANGE);
attachInterrupt(digitalPinToInterrupt(E1_OUT_B), read_encoder, CHANGE);
attachInterrupt(digitalPinToInterrupt(E2_OUT_A), read_encoder_2, CHANGE);
attachInterrupt(digitalPinToInterrupt(E2_OUT_B), read_encoder_2, CHANGE);
}

void loop()
{
  static int lastCounter = 0;
  static int lastCounter_2 = 0;
  // If count has changed print the new value to serial
  if(counter != lastCounter){
    if (counter_2 < 0) counter_2 = 0;
    if (counter_2 > 99) counter_2 = 99;
    if (counter_2 < lastCounter_2) updateScreen(1, 0, 0, 0, 0, 0);
    if (counter_2 > lastCounter_2) updateScreen(0, 1, 0, 0, 0, 0);
    lastCounter_2 = counter_2;
  }

  if(counter_2 != lastCounter_2){
    if (counter_2 < 0) counter_2 = 0;
    if (counter_2 > 99) counter_2 = 99;
    if (counter_2 < lastCounter_2) updateScreen(0, 0, 1, 0, 0, 0);
    if (counter_2 > lastCounter_2) updateScreen(0, 0, 0, 1, 0, 0);
    lastCounter_2 = counter_2;
  }

  bool push_input = digitalRead(E1_PUSH) | digitalRead(E2_PUSH);

  if (push_input && !lastPush) {
    updateScreen(0, 0, 0, 0, 1, 0);
    lastPush = true;
  }
  else if (!push_input && lastPush) {
    lastPush = false;
  }

  if (digitalRead(MENU_BUTTON) && !lastMenu) {
    updateScreen(0, 0, 0, 0, 0, 1);
    lastMenu = true;
  }
  else if (!digitalRead(MENU_BUTTON) && !lastMenu) {
    lastMenu = false;
  }
}

void updateScreen(bool forward1, bool backwards1, bool forward2, bool backward2, bool push, bool back) {
  switch(mode) {
    //Splash Screen
    case 0:
      if (push == 1 || back == 1) {
        gfx->fillScreen(WHITE);
        mode = 1;
      }
      break;
  //Main Menu
   case 1:
    if (forward1 || forward2) {
      menu1++;
      if (menu1 > 6) menu1 = 0;
    }
    else if(backwards1 || backward2) {
      menu1--;
      if (menu1 < 0) menu1 = 6;
    } 
      switch(menu1) {
        case 0:
          gfx->fillScreen(RED);
          gfx->setTextColor(WHITE);
          gfx->setCursor(gfx->width()/2 - 30, gfx->height()/2+5);
          gfx->print(addName);
          if (push && !back) {
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
          if (push && !back) {
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
          if (push && !back) {
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
          if (push && !back) {
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
          if (push && !back) {
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
      addMode(push, back);
      break;
    //Subtraction
    case 3:
      gfx->fillScreen(BLUE);
      gfx->setTextColor(WHITE);
      subtractMode(push, back);
      break;
    //Multiplication
    case 4:
      gfx->fillScreen(GREEN);
      gfx->setTextColor(BLACK);
      multiplyMode(push, back);
      break;
    //Division
    case 5:
      gfx->fillScreen(PURPLE);
      gfx->setTextColor(WHITE);
      divideMode(push, back);
      break;
    //Quiz Mode
    case 6:
      gfx->fillScreen(BLACK);
      gfx->setTextColor(WHITE);
      switch(quizMode) {
        case 0:
          addMode(push, back);
          break;
        case 1:
          subtractMode(push, back);
          break;
        case 2:
          multiplyMode(push, back);
          break;
        case 3:
          divideMode(push, back);
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
  counter = 0;
  counter_2 = 0;
}

void resetSub() {
  if (mode != 3) {
    currentScore = 0;
  }
  subNum1 = random(0, 99);
  subNum2 = random(0, addNum1);
  difference = subNum1 - subNum2;
  counter = 0;
  counter_2 = 0;
}

void resetMult() {
  if (mode != 4) {
    currentScore = 0;
  }
  multNum1 = random(0, 9);
  multNum2 = random(0, 9);
  product = multNum1 * multNum2;
  counter = 0;
  counter_2 = 0;
}

void resetDiv() {
  if (mode != 5) {
    currentScore = 0;
  }
  divNum2 = random(0, 9);
  quotient = random(0, 9);
  divNum1 = divNum2 * quotient;
  counter = 0;
  counter_2 = 0;
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

void addMode(int push, int back) {
  gfx->setTextSize(3,3);
  gfx->setCursor(gfx->width()/2-10, gfx->height() + 10);
  gfx->print(currentScore);
  gfx->setCursor(0, gfx->height()/2);
  gfx->print(addNum1); gfx->print(" + "); gfx->print(addNum2); gfx->print(" = ");
  if (counter != 0) gfx->print(counter % 10);
  else gfx->print(" ");
  gfx->print(counter_2 % 10);
  if(push && !back) {
    short calculatedSum = (counter%10)*10 + counter_2%10;
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
  if (back) {
    mode = 1;
    currentScore = 0;
    updateScreen(0,0,0,0,0,0);
  }
}
void subtractMode(int push, int back) {
  gfx->setTextSize(3,3);
  gfx->setCursor(gfx->width()/2-10, gfx->height() + 10);
  gfx->print(currentScore);
  gfx->setCursor(0, gfx->height()/2);
  gfx->print(subNum1); gfx->print(" - "); gfx->print(subNum2); gfx->print(" = ");
  if (counter != 0) gfx->print(counter % 10);
  else gfx->print(" ");
  gfx->print(counter_2 % 10);
  if((push && !back)) {
    short calculatedDifference = (counter%10)*10 + counter_2%10;
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
  if (back) {
    mode = 1;
    currentScore = 0;
    updateScreen(0,0,0,0,0,0);
  }
}
void multiplyMode(int push, int back) {
  gfx->setTextSize(3,3);
  gfx->setCursor(gfx->width()/2-10, gfx->height() + 10);
  gfx->print(currentScore);
  gfx->setCursor(0, gfx->height()/2);
  gfx->print(multNum1); gfx->print(" * "); gfx->print(multNum2); gfx->print(" = ");
  if (counter != 0) gfx->print(counter % 10);
  else gfx->print(" ");
  gfx->print(counter_2 % 10);
  if(push && !back) {
    short calculatedProduct = (counter%10)*10 + counter_2%10;
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
  if (back) {
    mode = 1;
    currentScore = 0;
    updateScreen(0,0,0,0,0,0);
  }
}
void divideMode(int push, int back) {
  gfx->setTextSize(3,3);
  gfx->setCursor(gfx->width()/2-10, gfx->height() + 10);
  gfx->print(currentScore);
  gfx->setCursor(0, gfx->height()/2);
  gfx->print(divNum1); gfx->print(" ÷ "); gfx->print(divNum2); gfx->print(" = ");
  if (counter != 0) gfx->print(counter % 10);
  else gfx->print(" ");
  gfx->print(counter_2 % 10);
  if(push && !back) {
    short calculatedQuotient = (counter%10)*10 + counter_2%10;
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
  if (back) {
    mode = 1;
    currentScore = 0;
    updateScreen(0,0,0,0,0,0);
  }
}

void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(E1_OUT_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(E1_OUT_B)) old_AB |= 0x01; // Add current state of pin B
 
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue;
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue;
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;              // Update counter
    encval = 0;
  }
}

void read_encoder_2() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
 
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(E2_OUT_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(E2_OUT_B)) old_AB |= 0x01; // Add current state of pin B
 
  encval += enc_states[( old_AB & 0x0f )];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 1;
    if((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue;
    }
    _lastIncReadTime = micros();
    counter_2 = counter_2 + changevalue;              // Update counter
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -1;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue;
    }
    _lastDecReadTime = micros();
    counter_2 = counter_2 + changevalue;              // Update counter
    encval = 0;
  }
} 