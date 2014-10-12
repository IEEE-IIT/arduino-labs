/* Simple Car game for a 16x2 LCD display 
   You can use any Hitachi HD44780 compatible LCD.
   Wiring explained at http://www.arduino.cc/en/Tutorial/LiquidCrystal
   (I used theLCD Electronic Brick on bus 1:
     rs on pin 2, rw on pin 3, enable on pin 4,
     data on pins 5,6,7,8)
   There's also a "steering wheel" potentiometer on analog input 1,
   and a Piezo speaker on pin 9 (PWM).
   
   Enjoy,
   @TheRealDod, Nov 25, 2010
*/


#include <SPI.h>
#include <MFRC522.h>


#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.
byte seven_seg_digits[11][7] = { { 1,1,1,1,1,1,0 },  // = 0
                                 { 0,1,1,0,0,0,0 },  // = 1
                                 { 1,1,0,1,1,0,1 },  // = 2
                                 { 1,1,1,1,0,0,1 },  // = 3
                                 { 0,1,1,0,0,1,1 },  // = 4
                                 { 1,0,1,1,0,1,1 },  // = 5
                                 { 1,0,1,1,1,1,1 },  // = 6
                                 { 1,1,1,0,0,0,0 },  // = 7
                                 { 1,1,1,1,1,1,1 },  // = 8
                                 { 1,1,1,0,0,1,1 },  // = 9
                                 { 0,0,0,0,0,0,0 },  // = off
                                 };
boolean stopped = false;
int randomnum = 0;



#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// LiquidCrystal display
// You can use any Hitachi HD44780 compatible. Wiring explained at
// http://www.arduino.cc/en/Tutorial/LiquidCrystal
//LiquidCrystal lcd(2, 3, 4, 5, 6, 7, 8);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
 
// Steering wheel potentiometer
const int POTPIN = 1;
const int MAXPOT = 800; // no need to turn the wheel all the way to 1023 :)
 
// Piezo speaker
const int SPEAKERPIN = 0;
 
const int RANDSEEDPIN = 0; // an analog pin that isn't connected to anything
const int MAXSTEPDURATION = 300; // Start slowly, each step is 1 millisec shorter.
const int MINSTEPDURATION = 150; // This is as fast as it gets
 
const int NGLYPHS = 6;
// the glyphs will be defined starting from 1 (not 0),
// to enable lcd.print() of null-terminated strings
byte glyphs[NGLYPHS][8] = {
  // 1: car up
  { B00000,
    B01110,
    B11111,
    B01010,
    B00000,
    B00000,
    B00000,
    B00000}
  // 2: car down
  ,{B00000,
    B00000,
    B00000,
    B00000,
    B01110,
    B11111,
    B01010,
    B00000}
  // 3: truck up
  ,{B00000,
    B11110,
    B11111,
    B01010,
    B00000,
    B00000,
    B00000,
    B00000}
  // 4: truck down
  ,{B00000,
    B00000,
    B00000,
    B00000,
    B11110,
    B11111,
    B01010,
    B00000}
  // 5: crash up
  ,{B10101,
    B01110,
    B01110,
    B10101,
    B00000,
    B00000,
    B00000,
    B00000}
  // 6: crash down
  ,{B00000,
    B00000,
    B00000,
    B10101,
    B01110,
    B01110,
    B10101,
    B00000}
};
 
const int NCARPOSITIONS = 4;

// Each position is mapped to a column of 2 glyphs
// Used to make sense when I had a 5th position
// where car or crash was drawn as 2 glyphs
// (can't do that since 0 terminates strings),
// so it's kinda silly now, but it ain't broke :)
const char BLANK=32;
char car2glyphs[NCARPOSITIONS][2] = {
  {1,BLANK},{2,BLANK},{BLANK,1},{BLANK,2}
};
char truck2glyphs[NCARPOSITIONS][2] = {
  {3,BLANK},{4,BLANK},{BLANK,3},{BLANK,4}
};
char crash2glyphs[NCARPOSITIONS][2] = {
  {5,BLANK},{6,BLANK},{BLANK,5},{BLANK,6}
};
 
const int ROADLEN = 15; // LCD width (not counting our car)
int road[ROADLEN]; // positions of other cars
char line_buff[2+ROADLEN]; // aux string for drawRoad()
int road_index;
int car_pos;
// Off-the-grid position means empty column, so MAXROADPOS
// determines the probability of a car in a column
// e.g. 3*NCARPOSITIONS gives p=1/3
const int MAXROADPOS = 3*NCARPOSITIONS;
int step_duration;
 
int crash; // true if crashed
unsigned int crashtime; // millis() when crashed
const int CRASHSOUNDDURATION = 250;

const char *INTRO1="Trucks ahead,";
const char *INTRO2="Drive carefully";
const int INTRODELAY = 2000;
 
void setupRFID() {
  pinMode(2, OUTPUT);   
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  writeDot(0);  // start with the "dot" off
  Serial.begin(9600);
  SPI.begin();		// Init SPI bus
  mfrc522.PCD_Init();	// Init MFRC522 card
}

void setupCar() { 
  crash = crashtime = road_index = 0;
  step_duration = MAXSTEPDURATION;
  line_buff[1+ROADLEN] = '\0'; // null terminate it
  randomSeed(analogRead(RANDSEEDPIN));
  for (int i=0; i<NGLYPHS; i++) {
    lcd.createChar(i+1,glyphs[i]);
  }
  for (int i=0; i<ROADLEN; i++) {
    road[i]=-1;
  }
  pinMode(SPEAKERPIN,OUTPUT);
  analogWrite(SPEAKERPIN,0); // to be on the safe side
  lcd.begin(16,2);
  getSteeringWheel();
  drawRoad();
  lcd.setCursor(1,0);
  lcd.print(INTRO1);
  lcd.setCursor(1,1);
  lcd.print(INTRO2);
  delay(INTRODELAY);
}

void setup()
{
  setupRFID();
  setupCar();
}

void writeDot(byte dot) {
  digitalWrite(9, dot);
}
    
void sevenSegWrite(byte digit) {
  byte pin = 2;
  for (byte segCount = 0; segCount < 7; ++segCount) {
    digitalWrite(pin, seven_seg_digits[digit][segCount]);
    ++pin;
  }
}

void loopRFID() {
  if (randomnum==0 && !mfrc522.PICC_IsNewCardPresent()) {
    sevenSegWrite(6);
  } else {
    if (!stopped) {
      randomnum = random(1,10);
      stopped = true;
    } else {
      if (mfrc522.PICC_IsNewCardPresent()) {
        stopped = false;
      }
      else sevenSegWrite(randomnum);
    }
  }
  //delay(25);
}

void loopCar() {
  if (digitalRead(13)==HIGH && crash) {
    /*int road[ROADLEN]; // positions of other cars
    char line_buff[2+ROADLEN]; // aux string for drawRoad()
    int road_index;
    int car_pos;
    int step_duration;
     
    int crash; // true if crashed
    unsigned int crashtime; // millis() when crashed
    
    setupCar();*/
    crash = crashtime = road_index = 0;
    /*line_buff[1+ROADLEN] = '\0'; // null terminate it
    randomSeed(analogRead(RANDSEEDPIN));
    for (int i=0; i<NGLYPHS; i++) {
      lcd.createChar(i+1,glyphs[i]);
    }
    for (int i=0; i<ROADLEN; i++) {
      road[i]=-1;
    }
    drawRoad();
    lcd.setCursor(1,0);
    lcd.print(INTRO1);
    lcd.setCursor(1,1);
    lcd.print(INTRO2);
    delay(INTRODELAY);*/
  }
  
  unsigned long now = millis()-INTRODELAY;
  if (!crash) {
    getSteeringWheel();
    crash = (car_pos==road[road_index]);
  }
  if (crash) {
    if (!crashtime) {
      crashtime=now;
      drawRoad();
      // Game over text
      // (keep first 2 "crash" columns intact)
      lcd.setCursor(2,0);
      lcd.print("Crashed after");
      lcd.setCursor(2,1);
      lcd.print(now/1000);
      lcd.print(" seconds.");
    }
    if ((now-crashtime)<CRASHSOUNDDURATION) {
      analogWrite(SPEAKERPIN,random(255)); // white noise
    } 
    else {
      analogWrite(SPEAKERPIN,0); // dramatic post-crush silence :)   
    }
    delay(10); // Wait a bit between writes
  } 
  else {
 
    int prev_pos = road[(road_index-1)%ROADLEN];
    int this_pos = random(MAXROADPOS);
    while (abs(this_pos-prev_pos)<2) { // don't jam the road
      this_pos = random(MAXROADPOS);
    }
    road[road_index] = this_pos;
    road_index = (road_index+1)%ROADLEN;
    drawRoad();
    delay(step_duration);
    if (step_duration>MINSTEPDURATION) {
      step_duration--; // go faster
    }
  }
}

void loop() {
  loopRFID();
  loopCar();
}
void getSteeringWheel() {
  car_pos = map(analogRead(POTPIN),0,1024,0,NCARPOSITIONS);
}
 
void drawRoad() {
  for (int i=0; i<2; i++) {
    if (crash) {
      line_buff[0]=crash2glyphs[car_pos][i];
    } 
    else {
      line_buff[0]=car2glyphs[car_pos][i];
    }
    for (int j=0; j<ROADLEN; j++) {
      int pos = road[(j+road_index)%ROADLEN];
      line_buff[j+1] = pos>=0 && pos<NCARPOSITIONS ? truck2glyphs[pos][i] : BLANK;
    }
    lcd.setCursor(0,i);
    lcd.print(line_buff);
  }
}
