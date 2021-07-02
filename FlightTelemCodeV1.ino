#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <avr/pgmspace.h>

#include "MPU9250.h"

int count=0;
float bias[3]= {0,0,0};
/* 
 * Connections:
 *    SD card shield
 *    CLK  - 13 
 *    MOSI - 11
 *    MISO - 12
 *    CS   - 10
 *
 * I2C
 *    SDA - 27
 *    SCL - 28
 * Notes:
 *  1. SD card communication occurs over SPI hardware pins. The SD cards needs to have a FAT32 / FAT16 filesystem loaded.
 *  2. EEPROM is size for micro is 1KB
 *  3. SD card library is slow, make it faster by following the follows https://forum.arduino.cc/index.php?topic=49649.0
 *  4. The SD library is huge at 13 kB, use a smaller more customized library for the SD card
 *  5. is the SD card a micro and mini, it depends on what it and the shield is to define
 *  6. files on SD library must use 8.3 convention 3 char extension, 8 char name
 *      https://www.arduino.cc/en/Reference/SDCardNotes
 *  7. Arudino uses AVR libc in user space, look its reference manual if needed.
 *  8. Arduino has no protection mechanisms as seen in normal OSes. You are running in kernel mode
 *     be careful and don't run out of SRAM
 *  9. Arduino sends data to computer at 38400 bit Hz 
 *
 * TODO: What is the maximum SPI speed for the SD card
 *
 *  Arduino Pro Micro
 *  8 MHz crystal 
 *  ATmega32U4
 *  20 digital I/O pins
 *  2.5K sram
 *  1K EEPROM
 *  
 *  RX - pin 0 for hardware serial (Serial1) UART
 *  TX - pin 1 for hardware serial
 *
 * Arduino Pro Micro
 * 
 * I2C 2, 3 (SDA, SCL)
 *
 * Connections
 * 2 I2C devices, IMU and baro (master read slave write)
 * 1 Serial device to communicate with SD card (master write slave read)
 */


#define SerialDebug true  // Set to true to get Serial output for debugging with computer

// SPI pins declr (for SD)
uint8_t CS = 10;

MPU9250 myIMU;

File logFile;

void setup()
{
  Wire.begin();
  // TWBR = 12;  // 400 kbit/sec I2C speed
  Serial.begin(9600);

  pinMode(CS, OUTPUT);

  // initialize SD card
  uint8_t sdopen = SD.begin(CS);

  if (sdopen) {
    logFile = SD.open(F("LOG.TXT"), FILE_WRITE);
  }

  if (!sdopen || !logFile) {
    // SD card initialization failed
    Serial.println(F("SD card initialization failed."));
   
    // TODO: maybe flash LED to indicate failures
  }

  // Read the WHO_AM_I register, this is a good test of communication
  // TODO: What is in the WHO_AM_I register
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
    Serial.print(c, HEX); Serial.print(" == "); Serial.println(0x71, HEX);

  if (c == 0x71) // WHO_AM_I should always be 0x71
  {
    byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    Serial.print(F("AK8963 "));Serial.print(d, HEX);Serial.print(" == ");Serial.println(0x48, HEX);

    
    myIMU.getAres();
    myIMU.getGres();
    myIMU.getMres();

    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    


    delay(2000); // Add delay to see results before serial spew of data

  } // if (c == 0x71)
  else
  {
    Serial.print(F("Could not connect to MPU9250: 0x"));
    Serial.println(c, HEX);
    while(1) ; // Loop forever if communication doesn't happen
  }
}



void loop()
{
  count+=1;
  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0] * myIMU.aRes; // - myIMU.accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1] * myIMU.aRes; // - myIMU.accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2] * myIMU.aRes; // - myIMU.accelBias[2];

  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)

  // Must be called before updating quaternions!
  myIMU.updateTime();

  
    char buf[15];
    logFile.print(millis()); logFile.print(","); 

    logFile.print(dtostrf(1000 * myIMU.ax, 14, 4, buf)); logFile.print(",");
    logFile.print(dtostrf(1000 * myIMU.ay, 14, 4, buf)); logFile.print(",");
    logFile.println(dtostrf(1000 * myIMU.az, 14, 4, buf));

    logFile.flush();

    if(SerialDebug)
    {
      // Print acceleration values in milligs!
      Serial.print(1000 * myIMU.ax-bias[0]);
      Serial.print(" mg, ");
      Serial.print(1000 * myIMU.ay-bias[1]);
      Serial.print(" mg, ");
      Serial.print(1000 * myIMU.az-bias[2]);
      Serial.print(" mg, ");

      Serial.print(((float) myIMU.readTempData()) / 333.87 + 14.0, 1);
      Serial.println(" C");
 
    }

    if(count==10);
    {
    bias[0] = (float)myIMU.accelCount[0] * myIMU.aRes; // - myIMU.accelBias[0];
    bias[1] = (float)myIMU.accelCount[1] * myIMU.aRes; // - myIMU.accelBias[1];
    bias[2] = (float)myIMU.accelCount[2] * myIMU.aRes; // - myIMU.accelBias[2];
    }
   
delay(1500);
}
