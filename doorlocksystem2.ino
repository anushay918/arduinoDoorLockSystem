// First store the secret passcode.
// Let user input a passcode (check for * key)
// If passcode same as stored passcode, power up green light and  solenoid for thrity seconds (else try again)
// While green led is turned on, if the # key is pressed, turn on red led and blink green led at 2HZ and prompt user to enter new code.
// six digits input and * key pressed. Save this code.
// Green led stops blinking for 1.5 seconds then starts blinking again.
// Same code is input and # key pressed.
// if second code and saved code match then both green and red led turn on for 1.5 seconds and new code saved.

// lcd.clear(); clears the screen
// lcd.home(); puts cursor at 0,0

//TO DO:
//save to eeprom
//# key for reentering code

#include "LiquidCrystal_I2C.h"
#include "Keypad.h"
#include <EEPROM.h>

#define GREEN_LED  10
#define RED_LED  12
#define LOCK 13

String secretCode = "123456";
String passcode = "";
String inputNewPasscode = "";
String inputString = "";
bool validCode = false;
int ledState = LOW;
unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long currentMillis = 0;

const char rows = 4;
const char cols = 4;

int i = 10;
char row_pins[rows] = {2,3,4,5};
char col_pins[cols] = {6,7,8,9};

char keys[rows][cols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad keypad = Keypad(makeKeymap(keys), row_pins, col_pins, rows, cols);

LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  // put your setup code here, to run once
  pinMode(GREEN_LED,OUTPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(LOCK,OUTPUT);

  Serial.begin(9600);
  lcd.init();                      
  lcd.backlight();
  lcd.setCursor(0,0);
  // if saved code is 0, then do this
  if (EEPROM.read(0) == 255) {
    lcd.print("Enter secret");
    lcd.setCursor(0,1);
    lcd.print("passcode: ");
    // if code is correct then let user enter new passcode
    while (!(checkCode(secretCode))) {
    }

    lcd.clear();
    lcd.home();
    lcd.print("Enter new code:");

    while (validCode == false) {
      getNewCode();
    }
    validCode = false;
    inputNewPasscode = "";

    reenterPasscode();
    while (!(checkCode(passcode))) {
    }
    
    lcd.clear();
    lcd.home();
    lcd.print("Code saved!");
    delay(3000);

    digitalWrite(RED_LED,HIGH);
    digitalWrite(GREEN_LED,HIGH);
    delay(1500);
    digitalWrite(RED_LED,LOW);
    digitalWrite(GREEN_LED,LOW);
  } else {
    passcode = readStringFromEEPROM();
    passcode.trim();
  }
  // else go to loop
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.clear();
  lcd.home();
  lcd.print("Enter passcode:");
  Serial.println(passcode);
  //Let user input passcode and check if this passcode is same as saved passcode.
  while (!checkCode(passcode)) {
  }
  // if same then:
  digitalWrite(GREEN_LED,HIGH);
  //open lock
  digitalWrite(LOCK,HIGH);
  //Keep it like this for thirty seconds and keep checking if # key is pressed.
  previousMillis = 0;  // will store last time LED was updated
  const long interval = 30000;
  bool changeCode = false;
  unsigned long startTime = millis();
  unsigned long timeElapsed = millis();
  while ((timeElapsed - startTime < interval) && (!(changeCode))) {
    char key = keypad.getKey();
    if (key) {
      if (key=='#') {
        changeCode = true;
      }
    }
    timeElapsed = millis();
  }

  //close lock
  digitalWrite(LOCK,LOW);
  digitalWrite(GREEN_LED,LOW);

  if (changeCode) {
    digitalWrite(RED_LED,HIGH);
    digitalWrite(GREEN_LED,LOW);
    lcd.setCursor(0,0);
    lcd.print("Enter new code:");
    // do a while loop and use millis to check if half a second has passed and then toggle green light.
    while (validCode == false) {
      getNewCode();
      toggleGreenLight();
    }
    previousMillis = 0; 
    validCode = false;
    inputNewPasscode = "";
    digitalWrite(GREEN_LED,LOW);
    delay(1500);
    
    reenterPasscode();
    while (!(checkCode(passcode))) {
      toggleGreenLight();
    }
    lcd.clear();
    lcd.home();
    lcd.print("Code saved!");
    delay(3000);

    previousMillis = 0;
    digitalWrite(RED_LED,HIGH);
    digitalWrite(GREEN_LED,HIGH);
    delay(1500);
    digitalWrite(RED_LED,LOW);
    digitalWrite(GREEN_LED,LOW);
  }

}

void getNewCode() {
  char key = keypad.getKey();
  if (key) {
    Serial.println(key);
    if ((key != '*') && (inputNewPasscode.length() == 6)) {
      i = 10;
      lcd.setCursor(10, 1);
      lcd.print("      ");
      inputNewPasscode = "";
    }
    displayPasscode(key);

    if (key >= '0' && key <= '9') {     // only act on numeric keys
      inputNewPasscode += key;               // append new character to input string
      //WHAT IF USER KEEPS ADDING DIGITS?
    } else if (key == '*') {
      if (inputNewPasscode.length() == 6) {
        validCode = true;
        // lcd.clear();
        // lcd.home();
        // lcd.print("Code saved!");
        // delay(3000);
        passcode = inputNewPasscode;
        writeStringToEEPROM(passcode);
        i = 10;
        //SAVE CODE TO EEPROM
      }
    } else {
      inputNewPasscode = "";                 // clear input
      invalidCode();
      i = 10;
      lcd.setCursor(10, 1);
      lcd.print("      ");
    }
  }
}

bool checkCode(String pw) {
  bool sameCode = false;
  char key = keypad.getKey();
  if (key) {
    displayPasscode(key);

    if (key >= '0' && key <= '9') {     // only act on numeric keys
      inputString += key;               // append new character to input string
    } else {
      inputString = "";                 // clear input
      invalidCode();
    }
    if (inputString.length() == 6) {
      if (inputString == pw) {
        sameCode = true;
        correctCode();
      } else {
        invalidCode();
      }
      i = 10;
      inputString = "";
    }
  }
  return sameCode;
}

void toggleGreenLight() {
  const long interval = 500;
  currentMillis = millis();
  if (previousMillis == 0) {
    previousMillis = millis();
  }

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(GREEN_LED, ledState);
  }
}

void invalidCode() {
  lcd.clear();
  lcd.home();
  lcd.print("Invalid code. ");
  lcd.setCursor(0,1);
  lcd.print("Try again:");
}

void correctCode() {
  lcd.clear();
  lcd.home();
  lcd.print("Correct code!");
  delay(3000);
}

void displayPasscode(char keyPressed) {
    lcd.setCursor(i, 1);
    lcd.print(keyPressed);
    i += 1;
}

void reenterPasscode() {
  lcd.clear();
  lcd.home();
  lcd.print("Reenter code:");
}

void writeStringToEEPROM(String strToWrite) {
  for (int i = 0; i < 6; i++)
  {
    EEPROM.write(i, strToWrite[i]);
  }
}

String readStringFromEEPROM() {
  char data[6];
  for (int i = 0; i < 6; i++)
  {
    data[i] = EEPROM.read(i);
  }
  return String(data);
}



