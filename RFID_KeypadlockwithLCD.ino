/*This code is written for a Project called RFID door lock.
 * It scans any RFID and asks for password from the known tags
 * An 4x3 or 4x4 matrix keypad can be used to enter the password 
 * Entering correct password unlocks the door for 10 seconds and
 * a countdown is shown on the lcd display
 */
#include <SPI.h>  //Including SPI communication library
                  //A Serial Peripheral Interface (SPI) bus is a system for serial communication,
                  //which uses up to four conductors, commonly three. 
                  //One for receiving data, another for sending, 
                  //one for synchronization and one for selecting a device to communicate with.
#include <MFRC522.h> //Including MFRC522 library, used for programming the RFID module
#include <Wire.h>   // Include Arduino Wire library for I2C
#include <LCD_I2C.h>  // Include LCD display library for I2C. 
                      //plz Install exactly the same library.
                      //installing a different library may not work
#include <Keypad.h>  //Including classical matrix type keypad library
int i;
const int doorlock=8; //connect pin 8 of the arduino uno with door lock sensor input pin
                      //lock should be connected with a proper bjt of FET switch
                      //To drive the solinoid lock or any type of relay opto-isolator is used 
                      //to protect the microcontroller from surge currents and back emf 
const int  buzzer=7;  //connect pin 7 of the arduino uno with a buzzer
                      //through atleast 100 ohm resistor not overload the GPIO pin
#define Password_Length 7  // Length of password + 1 for null character
char Master[Password_Length] = "123456";  // Password, change according to your wish 
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);
const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'}, 
  {'4', '5', '6', 'B'}, 
  {'7', '8', '9', 'C'}, 
  {'*', '0', '#', 'D'}  
};

byte rowPins[ROWS] = {14, 15, 16, 17}; //connect r1,r2,r3,r4 of keypad with these pins
byte colPins[COLS] = {2, 3, 4, 5}; //Connect c1,c2,c3,c4 of keypad with these pins


Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
LCD_I2C lcd(0x27, 16, 2); // Creates LCD object
/*
 MFRC-522 RFID door lock
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */
constexpr uint8_t RST_PIN = 9;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 10;     // Configurable, see typical pin layout above
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[7]; //change the size if your id shows more than 6 bytes

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  lcd.begin(); //Init LCD
  lcd.backlight(); //Turns on LCD backlight
  pinMode(doorlock,OUTPUT);
  pinMode(buzzer,OUTPUT);

  for (byte i = 0; i < 6; i++) {  //change the condition for tags that provides more than 6 bytes 
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  //printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
 
void loop() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Scan Your ID");
  //Look for new cards
    if( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
    for (byte i = 0; i < 7; i++) {     //Store NUID into nuidPICC array
      nuidPICC[i] = rfid.uid.uidByte[i];
      }
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    /*Below type of section should be added with each new RFID keys
     * please change the decimal values with your rfid tags values 
     * you can get those values by uploading this script and scanning 
     * the ids while your serial monitor is on 
     * please note that your tags may have more than 4 bytes of identities 
     * this code is written for upto 6 bytes, you may need to modify if your
     * tags provide more bytes. 
     */
    if(nuidPICC[0]==195&&nuidPICC[1]==171&&nuidPICC[2]==233&&nuidPICC[3]==24){
      Serial.print(F("Hi Roboboy.PLease enter your password"));
      password();
      for(i=0;i<7;i++){
        nuidPICC[i]=0;
      }
    }
    //section ends
    else if(nuidPICC[0]==140&&nuidPICC[1]==103&&nuidPICC[2]==217&&nuidPICC[3]==56){
      Serial.print(F("Welcome KBM. Please Enter your password"));
      password();
      for(i=0;i<7;i++){
        nuidPICC[i]=0;
      }
    }
    else{
      Serial.print(F("unknown id, access denied"));
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   Unknown ID");
      lcd.setCursor(0,1);
      lcd.print(" Access denied");
      delay(2000);
    }
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
byte data_count = 0;  // Counter for character entries
char Data[Password_Length];// Character to hold password input
char customKey;// Character to hold key input
int TimeCount; 

void password(){  // a function that checks Password and opnes the lock 
  delay(500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");
  Serial.println("Enter Password:");
  while(data_count < Password_Length - 1){
  customKey = customKeypad.getKey(); // Look for keypress
    if (customKey) {
      Data[data_count] = customKey; // Enter keypress into array and increment counter
      lcd.setCursor(data_count, 1);
      lcd.print(Data[data_count]);
      Serial.print(Data[data_count]);
      data_count++;
    }
  }  // See if we have reached the password length
    lcd.clear();
    if (!strcmp(Data, Master)) {      // Password is correct
      lcd.print("Correct");
      Serial.print("Correct\n");
      delay(500);
      digitalWrite(doorlock, HIGH);
      for(TimeCount=0;TimeCount<10;TimeCount++){
        lcd.clear();
        lcd.print("Door is Open for");
        Serial.print("Door is Open for");
        lcd.setCursor(4,1);
        lcd.print(10-TimeCount);
        Serial.print(10-TimeCount);
        lcd.print(" Seconds");
        Serial.print(" Seconds\n");
        delay(1000);
      }
      digitalWrite(doorlock, LOW);
      lcd.clear();
      lcd.print("Door Closed");
      Serial.print("Door closed\n");
      delay(2000);
    }
    else {
      // Password is incorrect
      lcd.clear();
      lcd.print("Incorrect");
      Serial.print("Incorrect\n");
      delay(1000);
    }
    lcd.clear();    // Clear data 
    while (data_count != 0) {
      Data[data_count--] = 0;
    }
  }
