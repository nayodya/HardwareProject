#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>
#include <DFRobotDFPlayerMini.h>

#define mcuserial Serial1     //node mcu
#define mySerial Serial2      //fingerprint sensor
#define dfSerial Serial3      //df player

int reactionTime;
String gameMode;

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
DFRobotDFPlayerMini myDFPlayer;

#include <avr/io.h>
#include <avr/interrupt.h>
#include <TimerOne.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

/////////////////////////////////////////
int trigPin = 11;
int echoPin = 12;

long duration, distance;

int t=0;
int count=0;
//////////////////////////////

LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20 column and 4 rows

#define LED1 2
#define LED2 3
#define LED3 4
#define LED4 5

#define BUTTON1 6
#define BUTTON2 7
#define BUTTON3 8
#define BUTTON4 9

#define SPEAKER 10

int State = 0;

int buttonPins[] = {BUTTON1, BUTTON2, BUTTON3, BUTTON4};
int ledPins[] = {LED1, LED2, LED3, LED4};
String btnColor[] = {"Grren Win", "Yellow Win", "Red Win", "Blue Win"};

bool firstButtonPressed = false;  // Corrected the variable declaration
bool exitFlag = false;

//game2
const int speakerPin = 10;
const int numLeds = 4;
const int maxLeds = 10; // Maximum number of blinks in the final level

// Variables to store the patterns
int ledPattern[maxLeds];
int userPattern[maxLeds];
int userPatternIndex = 0;
int currentLevel = 1;
bool userInputComplete = false;

// Tones for each button
const int tones[] = {500, 600, 700, 800}; // Frequencies for C4, D4, E4, F4 notes

//////////////////////////////////////////////


const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 4; //four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
char prevKey;

//////////////////////////
//variables for serial communication
int level = 0;
int confirm_id = 0;
int confirm_finger_temp = 0;
String userID = "";
String command="";
///////////////////////////

byte pin_rows[ROW_NUM] = {29, 28, 27, 26}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {25, 24, 23, 22}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

// Define custom inverted characters
byte inverted_1[8] = {
  B11111,
  B11011,
  B10011,
  B11011,
  B11011,
  B11011,
  B10001,
  B11111
};

byte inverted_2[8] = {
  B11111,
  B11011,
  B10101,
  B11101,
  B11011,
  B10111,
  B10001,
  B11111
};

byte inverted_dot[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B10011,
  B10011,
  B11111
};

void checkExit(){
  //Serial.println("in check exit value 6");
  char key= keypad.getKey();

  if (key) {
    Serial.println(key);  // Print key to Serial

    // Check for specific key press
    if (key == '9') {
      
      exitFlag=true;
      Serial.println("exit");
    }
  }
    ////////////////////////////////////
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10); // transmit waves for 10 microseconds

    duration = pulseIn(echoPin, HIGH); // receive reflected waves

    distance = duration / 58.2; // distance in centimetres

    //Serial.print("distance");
    //Serial.println(distance);
    if (distance <= 25 && distance >= 4) {
        count++;
        if(count>10){
        t=0;
        }
    }else{
        t+=1;
    }

    /////////////////////////
}

////////////////////////////////////////////////game1
void playButtonTone(int button) {
  switch (button) {
    case 0:
      tone(SPEAKER, 500); // Generate a 500 Hz tone
      break;
    case 1:
      tone(SPEAKER, 600); // Generate a 600 Hz tone
      break;
    case 2:
      tone(SPEAKER, 700); // Generate a 700 Hz tone
      break;
    case 3:
      tone(SPEAKER, 800); // Generate a 800 Hz tone
      break;
  }
  delay(500);
  noTone(SPEAKER);
}

////////////////////////////////////////////////////////////////////game2  
// Function to generate a random LED pattern for the current level
void generateLedPattern(int level) {

  for (int i = 0; i < level + 3; i++) { // Start with 4 LEDs in level 1 and increase by 1 each level
    ledPattern[i] = random(0, numLeds);
  }
}

// Function to display the LED pattern
void displayLedPattern(int level) {
        lcd.clear();
        lcd.setCursor(3,1);
        lcd.print("level");
        lcd.setCursor(10,1);
        lcd.print(level);
        delay(2000);
  lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("pattern playing!");
  for (int i = 0; i < level + 3; i++) { // Start with 4 LEDs in level 1 and increase by 1 each level
    tone(SPEAKER,tones[ledPattern[i]],500);
    digitalWrite(ledPins[ledPattern[i]], HIGH);
    delay(500);
    digitalWrite(ledPins[ledPattern[i]], LOW);
    noTone(SPEAKER);
    delay(500);
    if (exitFlag == true) {
      break;
    }
  }
  delay(800);
}

// Function to check the user input pattern
bool checkPattern(int level) {
  for (int i = 0; i < level + 3; i++) { // Start with 4 LEDs in level 1 and increase by 1 each level
    if (userPattern[i] != ledPattern[i]) {
      return false;
    }
  }
  return true;
}

// Function to play a tone
void playTone(int buttonIndex) {
  tone(speakerPin, tones[buttonIndex], 500);
  delay(500);
  noTone(speakerPin);
}

////////////////////////////////////////////////////////////////////


void setup() {
  Serial.begin(9600);


    ///////////////////////////////////////////////////df player
  dfSerial.begin(9600);

  if (!myDFPlayer.begin(dfSerial)) {  // Use softwareSerial to communicate with MP3
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(25);

  ////////////////////////////
  mcuserial.begin(9600);
  ///////

 
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  
  
  
  lcd.init(); // initialize the lcd
  lcd.backlight();

  lcd.createChar(0, inverted_1);
  lcd.createChar(1, inverted_2);
  lcd.createChar(2, inverted_dot);

  myDFPlayer.play(14);
  delay(300);
  lcd.setCursor(5, 1);            // move cursor the first row
  lcd.print("Welcome to");
  lcd.setCursor(5, 2);            // move cursor the first row
  lcd.print("NeuroNest!");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1. Memory Game");
  lcd.setCursor(0, 1);
  lcd.print("2. Reaction Game");

  // Initialize LEDs
for (int i = 0; i < 4; i++) {
  pinMode(ledPins[i], OUTPUT);
}

// Initialize Buttons
for (int i = 0; i < 4; i++) {
  pinMode(buttonPins[i], INPUT_PULLUP);
}

// Initialize Speaker
pinMode(SPEAKER, OUTPUT);

// Set up Timer1 interrupt to call the checkDistance function every 100ms
Timer1.initialize(50000); // 200ms = 200,000us
Timer1.attachInterrupt(checkExit); // checkExit is a function that checks whether exit button is pressed

////////////////////////game2
// Seed the random generator
randomSeed(analogRead(0));
//////////////////////////

////////////////////////////////////////////////////////////////
//////////////////////////for the fingerprint
Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
////////////////////////////////////////////////
}

void loop() {
  for (int i = 0; i < 4; i++) {
  digitalWrite(ledPins[i], LOW);
}
  //Serial.println("in void loop");
  char key = keypad.getKey();

  if (key) {
    t=0;
    Serial.println(key);  // Print key to Serial

    // Check for specific key press
    if (key == '5') {
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      lcd.write(byte(2));
      lcd.setCursor(17, 0);
      lcd.print("<--");
      lcd.setCursor(17, 1);
      lcd.print("   ");
      lcd.setCursor(0, 1);
      lcd.print("2.");
    }else if (key == '9') {
      delay(2000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1. Memory Game");
      lcd.setCursor(0, 1);
      lcd.print("2. Reaction Game");

    }else if (key == '8') {
      lcd.setCursor(0, 1);
      lcd.write(byte(1));
      lcd.write(byte(2));
      lcd.setCursor(17, 0);
      lcd.print("   ");
      lcd.setCursor(17, 1);
      lcd.print("<--");
      lcd.setCursor(0, 0);
      lcd.print("1.");
    }else if (key == '6'){
      if (prevKey == '5' && State == 1){
          lcd.clear();
          lcd.setCursor(5,1);
          lcd.print("New Player");
          currentLevel=1;
          State = 0;
          ////////////////////////////////////
          resetGame();
          finger_enrolling();
          command = "GET";
          Serial.println(confirm_id);
          userID = confirm_id;
          currentLevel = datareceive(command, String(confirm_id));
          ////////////////////////////////////
          delay(1000);
          lcd.clear();
          lcd.setCursor(3,1);
          lcd.print("start!");

        delay(1000);
          while (true) {
    if (exitFlag == true) {
        exitFlag = false;
        break;
    }
    if(t>600){
    ////relay handling 
    Serial.println("pause");
    Serial.println(t);
    while(t!=0){
      lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("Game Paused!");
    delay(30);
    Serial.println("Paused");
    }
    t=0;
    Serial.println("back");
    delay(10);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start in..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("3..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("2...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("1...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start!");
    delay(600);
    lcd.clear();
    delay(30);

    }
      lcd.clear();
    // Generate and display the LED pattern once
    if (!userInputComplete) {

        generateLedPattern(currentLevel);
        if (exitFlag == true) {
            exitFlag = false;
            break;
        }
        displayLedPattern(currentLevel);
        if (exitFlag == true) {
            exitFlag = false;
            break;
        }
        userInputComplete = true;
    }
    delay(100);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("enter pattern!");
    delay(200);
    // Read button inputs
    if(t>600){
    ////relay handling 
    Serial.println("pause");
    Serial.println(t);
    while(t!=0){
      lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("Game Paused!");
    delay(30);
    Serial.println("Paused");
    }
    t=0;
    Serial.println("back");
    delay(10);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start in..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("3..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("2...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("1...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start!");
    delay(600);
    lcd.clear();
    delay(30);

    }
    for (int i = 0; i < numLeds; i++) {
        
        if (exitFlag == true) {
            break;
        }
        if (digitalRead(buttonPins[i]) == LOW) {
          t=0;
            userPattern[userPatternIndex] = i;
            userPatternIndex++;
            digitalWrite(ledPins[i], HIGH);
            if (exitFlag == true) {
                break;
            }
            playTone(i); // Play the unique tone for the pressed button
            delay(10); // Debounce delay
            digitalWrite(ledPins[i], LOW);
            if (exitFlag == true) {
                break;
            }

            // If user has entered the complete pattern for the current level
            if (userPatternIndex >= currentLevel + 3) { // Start with 4 LEDs in level 1 and increase by 1 each level
                Serial.print("User pattern: ");
                for (int j = 0; j < currentLevel + 3; j++) {
                    Serial.print(userPattern[j]);
                    Serial.print(" ");
                }
                Serial.println();
                if (exitFlag == true) {
                    break;
                }

                // Check the user pattern
                if (checkPattern(currentLevel)) {
                  t=0;
                    Serial.println("Correct pattern! Moving to the next level.");
                    lcd.clear();
                    lcd.setCursor(2,1);
                    lcd.print("Correct pattern!");
                    delay(1000);
                    lcd.clear();
                    lcd.setCursor(3,1);
                    lcd.print("Moving to the");
                    lcd.setCursor(5,2);
                    lcd.print("next level");
                    command = "POST";
                    level = currentLevel + 1 ;
                    resendUpdatedData(command, userID, reactionTime, gameMode, level);
                    currentLevel++;
                    if (currentLevel > 10) {
                        Serial.println("Congratulations! You've completed all levels.");
                        lcd.clear();
                        lcd.setCursor(2,1);
                        lcd.print("Congratulations!");
                        lcd.setCursor(2,2);
                        lcd.print("You've completed");
                        lcd.setCursor(4,3);
                        lcd.print("all levels");
                        
                        if (exitFlag == true) {
                            break;
                        }
                        // Stop execution
                    }
                } else {
                  t=0;
                    Serial.println("Incorrect pattern. Try again.");
                    lcd.clear();
                    lcd.setCursor(1,1);
                    lcd.print("Incorrect pattern");
                    lcd.setCursor(5,2);
                    lcd.print("Try again");
                }
                if (exitFlag == true) {
                    break;
                }

                // Reset for next attempt or next level
                userPatternIndex = 0;
                userInputComplete = false;
                delay(5000); // Wait before showing the pattern again. Interrupt will not work here
            }
        }
    }
    if (exitFlag == true) {
        exitFlag = false;
        break;
    }
}
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1. Memory Game");
      lcd.setCursor(0, 1);
      lcd.print("2. Reaction Game");
        }else if (prevKey == '8' && State == 1){
          lcd.clear();
          lcd.setCursor(5,1);
          lcd.print("Old Player");

          //old player

          State = 0;

          ////////////////////////////////////////////
          resetGame();
          while(confirm_finger_temp!=1){
          getFingerprintID();
          delay(50);
          }
          Timer1.initialize(500000000000);
          Serial.print("fingerprint id:");
          Serial.println(confirm_id);
          userID = confirm_id;
          mySerial.flush();
          mySerial.end();
          Serial.flush();
          command = "GET";
          currentLevel = datareceive(command, String(confirm_id));
          Serial.println(currentLevel);
          Timer1.initialize(50000);
          ////////////////////////////////////////////
           delay(1000);
          lcd.clear();
          lcd.setCursor(3,1);
          lcd.print("start!");

        delay(1000);
          while (true) {
    if (exitFlag == true) {
        exitFlag = false;
        break;
    }
    if(t>600){
    ////relay handling 
    Serial.println("pause");
    Serial.println(t);
    while(t!=0){
      lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("Game Paused!");
    delay(30);
    Serial.println("Paused");
    }
    Serial.println("back");
    delay(10);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start in..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("3..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("2...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("1...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start!");
    delay(600);
    lcd.clear();
    delay(30);

    }
    // Generate and display the LED pattern once
    if (!userInputComplete) {
        generateLedPattern(currentLevel);
        if (exitFlag == true) {
            exitFlag = false;
            break;
        }
        displayLedPattern(currentLevel);
        if (exitFlag == true) {
            exitFlag = false;
            break;
        }
        userInputComplete = true;
    }
     delay(100);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("enter pattern!");
    delay(500);

    // Read button inputs
    for (int i = 0; i < numLeds; i++) {
        if (exitFlag == true) {
            break;
        }
        if (digitalRead(buttonPins[i]) == LOW) {
          t=0;
            userPattern[userPatternIndex] = i;
            userPatternIndex++;
            digitalWrite(ledPins[i], HIGH);
            if (exitFlag == true) {
                break;
            }
            playTone(i); // Play the unique tone for the pressed button
            delay(10); // Debounce delay
            digitalWrite(ledPins[i], LOW);
            if (exitFlag == true) {
                break;
            }

            // If user has entered the complete pattern for the current level
            if (userPatternIndex >= currentLevel + 3) { // Start with 4 LEDs in level 1 and increase by 1 each level
                Serial.print("User pattern: ");
                for (int j = 0; j < currentLevel + 3; j++) {
                    Serial.print(userPattern[j]);
                    Serial.print(" ");
                }
                Serial.println();
                if (exitFlag == true) {
                    break;
                }

                // Check the user pattern
                if (checkPattern(currentLevel)) {
                  t=0;
                    Serial.println("Correct pattern! Moving to the next level.");
                    lcd.clear();
                    lcd.setCursor(2,1);
                    lcd.print("Correct pattern!");
                    delay(1000);
                    lcd.clear();
                    lcd.setCursor(3,1);
                    lcd.print("Moving to the");
                    lcd.setCursor(5,2);
                    lcd.print("next level");
                    command = "POST";
                    level = currentLevel +1;
                    resendUpdatedData(command, userID, reactionTime, gameMode, level);
                    currentLevel++;
                    if (currentLevel > 10) {
                        Serial.println("Congratulations! You've completed all levels.");
                        lcd.clear();
                        lcd.setCursor(2,1);
                        lcd.print("Congratulations!");
                        lcd.setCursor(2,2);
                        lcd.print("You've completed");
                        lcd.setCursor(4,3);
                        lcd.print("all levels");
                        if (exitFlag == true) {
                            break;
                        }
                        // Stop execution
                    }
                } else {
                  t=0;
                    Serial.println("Incorrect pattern. Try again.");
                    lcd.clear();
                    lcd.setCursor(1,1);
                    lcd.print("Incorrect pattern");
                    lcd.setCursor(5,2);
                    lcd.print("Try again");
                }
                if (exitFlag == true) {
                    break;
                }

                // Reset for next attempt or next level
                userPatternIndex = 0;
                userInputComplete = false;
                
                delay(5000); // Wait before showing the pattern again. Interrupt will not work here
            }
        }
    }
    if (exitFlag == true) {
        exitFlag = false;
        break;
    }
}
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1. Memory Game");
      lcd.setCursor(0, 1);
      lcd.print("2. Reaction Game");
        }else if (prevKey == '5'){
        lcd.clear();
        lcd.setCursor(5, 1);
        lcd.print("Memory Game");

        delay(2000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("1. New Player");
        lcd.setCursor(0, 1);
        lcd.print("2. Old Player");

        

        State = 1;
      }else if (prevKey == '8'){
        lcd.clear();
        lcd.setCursor(4, 1);
        lcd.print("Reaction Game");

        delay(2000);

        //reaction game
// Blink all LEDs and play tone once at startup
for (int i = 0; i < 4; i++) {
  digitalWrite(ledPins[i], HIGH);          
}
delay(500);
for (int i = 0; i < 4; i++) {
  digitalWrite(ledPins[i], LOW);
}
delay(1000);
while(true){
  delay(1000);
  Serial.println(exitFlag);
  if(exitFlag==true){
    exitFlag=false;
    break;
  }
   if(t>600){
    ////relay handling 
    Serial.println("pause");
    Serial.println(t);
    while(t!=0){
      lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("Game Paused!");
    delay(30);
    Serial.println("Paused");
    }
    t=0;
    Serial.println("back");
    delay(10);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start in..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("3..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("2...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("1...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start!");
    delay(600);
    lcd.clear();
    delay(30);

    }
    delay(100);
    Serial.println("back here");
    lcd.clear();
  delay(1000);
  Serial.println("wait!");
  lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("wait!");
  delay(3000);
  Serial.println("start!");
  lcd.clear();
  lcd.setCursor(6,1);
  lcd.print("start!");
  if(exitFlag==true){
    exitFlag=false;
    break;
  }
  // Set random seed for random number generation
  randomSeed(analogRead(0));

  // Generate a random delay between 0 and 10000 milliseconds (20 seconds)
  long randomDelay = random(0, 10001);
  delay(randomDelay);

  // Play tone once at a random time within the first 30 seconds
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], HIGH);
  }
  
  
  tone(SPEAKER, 1000); // Generate a 1000 Hz tone
  delay(500);
  if(exitFlag==true){
    exitFlag=false;
    break;
  }
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  noTone(SPEAKER);
  if(exitFlag==true){
    exitFlag=false;
    break;
  }
  firstButtonPressed = false;
  // Check which button is pressed
  while(firstButtonPressed == false){
    if(exitFlag==true){
      break;
    }
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) { // Button pressed
      t=0;
        if (!firstButtonPressed) {
          firstButtonPressed = true;
          Serial.print("Button ");
          Serial.print(i);
          Serial.println(" pressed first.");
          lcd.clear();
          lcd.setCursor(5,1);
          lcd.print(btnColor[i]);
        }
        if(exitFlag==true){
          break;
        }

        digitalWrite(ledPins[i], HIGH);          
    
        playButtonTone(i);
        delay(200); // Debounce delay

  digitalWrite(ledPins[i], LOW);


      }
    }
    if(exitFlag==true){
      break;
    }
 if(t>600){
    ////relay handling 
    Serial.println("pause");
    Serial.println(t);
    while(t!=0){
      lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("Game Paused!");
    delay(30);
    Serial.println("Paused");
    }
    t=0;
    Serial.println("back");
    delay(10);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start in..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("3..");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("2...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("1...");
    delay(600);
    lcd.clear();
    lcd.setCursor(3,1);
    lcd.print("start!");
    delay(600);
    lcd.clear();
    delay(30);

    }
  }
  if(exitFlag==true){
    exitFlag=false;
    break;
  }
      }
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("1. Memory Game");
      lcd.setCursor(0, 1);
      lcd.print("2. Reaction Game");
    }

    }
    prevKey = key;
  }

  if(t>600){
    ////relay handling 
    Serial.println("need to stop");
  }

  Serial.println(t);
}
uint8_t findEmptyID() {
  for (uint8_t id = 1; id <= finger.capacity; id++) {
    Serial.print("Checking ID "); Serial.println(id);
    uint8_t p = finger.loadModel(id);
    if (p == FINGERPRINT_OK) {
      Serial.print("ID "); Serial.print(id); Serial.println(" is occupied");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.print("Communication error while checking ID "); Serial.println(id);
    } else {
      Serial.print("ID "); Serial.print(id); Serial.println(" is empty");
      return id;
    }
  }
  return 0;  // No empty ID found
}

void enrollFingerprint(uint8_t id) {
  int p = -1;
  Serial.println("Waiting for valid finger to enroll...");

  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("put finger");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return;
    default:
      Serial.println("Unknown error");
      return;
  }

  Serial.println("Remove finger");

  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("remove finger");

  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);

  p = -1;
  Serial.println("Place same finger again...");

  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("place same finger");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return;
    default:
      Serial.println("Unknown error");
      return;
  }

  // Create model
  Serial.print("Creating model for #");  Serial.println(id);
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");

    lcd.clear();
    lcd.setCursor(3, 1);
    lcd.print("finger not match..");
    delay(1500);
    lcd.clear();

    finger_enrolling();

    return;
  } else {
    Serial.println("Unknown error");
    return;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");

    lcd.clear();
    lcd.setCursor(3, 1);
    lcd.print("stored..");
    delay(1000);
    lcd.clear();
    lcd.setCursor(4, 1);
    lcd.print("your id is");
    lcd.setCursor(7,3);
    lcd.print(id);
    delay(2000);
    lcd.clear();
    
    reactionTime = 0;  
    gameMode = 1;

    // Format the data
    String data = String(id) + "," + String(reactionTime) + "," + gameMode;

    // Send data to mcuserial
    mcuserial.print(data);
    Serial.print("Data sent to mcuserial: "); Serial.println(data);
    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return;
  } else {
    Serial.println("Unknown error");
    return;
  }
}
void finger_enrolling(){
  mcuserial.begin(9600);
  while (!Serial);  // For Leonardo/Micro/Zero

  finger.begin(57500);


  while(finger.verifyPassword()){
    Serial.println("Fingerprint sensor not found.");
  }
  Serial.println("Fingerprint sensor found!");


  // Optionally enable the debug output
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.print("Capacity: "); Serial.println(finger.capacity);


  if (finger.setSecurityLevel(3) == FINGERPRINT_OK) {
    Serial.println("Security level set to 3");
  } else {
    Serial.println("Failed to set security level");
  }


             uint8_t id = findEmptyID();
  if (id == 0) {
    Serial.println("No empty IDs found.");
  } else {
    //noInterrupts();
     Timer1.initialize(500000000000);
    Serial.print("Enrolling to ID "); Serial.println(id);
    enrollFingerprint(id);
    confirm_id = id;
    Timer1.initialize(50000); 
    delay(1000);
    //interrupts();
  }


}


int datareceive(String command, String userID) {
  mcuserial.flush();
  String sendData = command + "," + userID + "," + String(reactionTime) + "," + gameMode + "," + String(level);
  mcuserial.println(sendData);
  Serial.println("Data sent to NodeMCU: " + sendData);

  while (!mcuserial.available()) {
    // Waiting for the data
  }

  if (mcuserial.available()) {
    while(mcuserial.available()){
    String data = mcuserial.readStringUntil('\n');

    Serial.println("Raw data received from NodeMCU: " + data);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return 0;
    }

    String receivedUserID = doc["userid"].as<String>();
    int receivedReactionTime = doc["reactionTime"];
    String receivedGameMode = doc["gameMode"].as<String>();
    int receivedLevel = doc["level"];

  
    return receivedLevel;
    }
  }
  return -1; // Return an error value if no data is received
}

void resendUpdatedData(String command, String userID, int reactionTime, String gameMode, int level) {
  StaticJsonDocument<200> doc;
  doc["command"] = command;
  doc["userid"] = userID;
  doc["reactionTime"] = reactionTime;
  doc["gameMode"] = gameMode;
  doc["level"] = level;

  String sendData;
  serializeJson(doc, sendData);

  mcuserial.println(sendData);
  Serial.println("Resent updated data to NodeMCU: " + sendData);
}


uint8_t getFingerprintID() {
  Serial.println("Enter the fingerprint");

  lcd.clear();
  lcd.setCursor(3, 1);
  lcd.print("put finger");


  while (1){
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
    //  Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    confirm_id = finger.fingerID;
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    confirm_finger_temp = 1;

  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");

    lcd.clear();
    lcd.setCursor(3, 1);
    lcd.print("not matching finger");
    delay(1000);
    lcd.clear();
    getFingerprintID();

    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  delay(50);
  return finger.fingerID;
  }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  confirm_id = finger.fingerID;
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  
  Serial.println("id passed");
  delay(50);
  return finger.fingerID;

}

void resetGame() {
  userPatternIndex = 0;
  userInputComplete = false;
  memset(ledPattern, 0, sizeof(ledPattern));
  memset(userPattern, 0, sizeof(userPattern));
}
