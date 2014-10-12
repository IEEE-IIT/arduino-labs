// Arduino 7 segment display example software
// http://www.hacktronics.com/Tutorials/arduino-and-7-segment-led.html
// License: http://www.opensource.org/licenses/mit-license.php (Go crazy)
 
// Define the LED digit patters, from 0 - 9
// Note that these patterns are for common cathode displays
// For common anode displays, change the 1's to 0's and 0's to 1's
// 1 = LED on, 0 = LED off, in this order:
//                                    Arduino pin: 2,3,4,5,6,7,8
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() {                
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
  
  
  
  lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines, turn on backlight

// ------- Quick 3 blinks of backlight  -------------
  for(int i = 0; i< 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight(); // finish with backlight on  

  //-------- Write characters on the display ------------------
  // NOTE: Cursor Position: Lines and Characters start at 0  
  lcd.setCursor(3,0); //Start at character 4 on line 0
  lcd.print("Hello, world!");
  delay(1000);
  lcd.setCursor(2,1);
  lcd.print("From YourDuino");
  delay(1000);  
  lcd.setCursor(0,2);
  lcd.print("20 by 4 Line Display");
  lcd.setCursor(0,3);
  delay(2000);   
  lcd.print("http://YourDuino.com");
  delay(8000);
  // Wait and then tell user they can start the Serial Monitor and type in characters to
  // Display. (Set Serial Monitor option to "No Line Ending")
  lcd.setCursor(0,0); //Start at character 0 on line 0
  lcd.print("Start Serial Monitor");
  lcd.setCursor(0,1);
  lcd.print("Type chars 2 display");
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

void loop() {
  if (randomnum==0 && !mfrc522.PICC_IsNewCardPresent()) {
    sevenSegWrite(10);
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
  delay(25);
  
  
  
  // when characters arrive over the serial port...
  /*if (Serial.available()) {
    // wait a bit for the entire message to arrive
    delay(100);
    // clear the screen
    lcd.clear();
    // read all the available characters
    while (Serial.available() > 0) {
      // display each character to the LCD
      lcd.write(Serial.read());
    }
  }*/
}
