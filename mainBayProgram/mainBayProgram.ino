//Download IMU library from https://github.com/bolderflight/MPU9250 and place extracted zip file in to arduino library folder

#include <SPI.h>
#include <SD.h>
#include "MPU9250.h"
File mainFile;
MPU9250 IMU(Wire, 0x68);
int status;
int timeCounter = 0;
float recData[12];
float initData[12];

void setup() {
  
  //Initializing Serial Connections: Telem, FC1, FC2
  Serial1.begin(9600);
  while (!Serial){
    ;
  }
  Serial1.println("Telemetry Serial Initialized");

  Serial2.begin(9600);
  while (!Serial2){
    ;
  }
 Serial1.println("Flight Computer 1 Serial Initialized");
  
  Serial3.begin(9600);
  while (!Serial3){
    ;
  } 
 Serial1.println("Flight Computer 2 Serial Initialized");
  
 
  //Initializing SPI Interface to Memory Unit
  if (!SD.begin(10)) {
    Serial1.println("Memory initialization failed!");
    while (1);
  }
  mainFile = SD.open("flight.txt", FILE_WRITE);
  if(mainFile){
    Serial1.println("Initialization Successful...");
    mainFile.print("Time");
    mainFile.close();
  }
  else{
    Serial1.println("Error when creating text file on SD card...");
  }

  // Initialize I2C communication with IMU 
  status = IMU.begin();
  if (status < 0) {
    Serial1.println("IMU initialization failed!");
    Serial1.println("Check IMU wiring or try cycling power");
    Serial1.print("Status: ");
    Serial1.println(status);
    while(1) {}
  }
  //Zeroing IMU
initData[0]=IMU.getAccelX_mss();
initData[1]=IMU.getAccelY_mss();
initData[2]=IMU.getAccelZ_mss();
initData[3]=IMU.getGyroX_rads();
initData[4]=IMU.getGyroY_rads();
initData[5]=(IMU.getGyroZ_rads(),6);

}

//Outputs Acceleration, Rotatation, Magnetometer, Temperature, and Altitude Data from IMU and Flight Computers to recData
void readSensors(float recData[12]){
  IMU.readSensor();
  //Change of coordinates: Z-direction: +y, Y-direction: +x, X-direction: +z
  recData[0] = IMU.getAccelX_mss()-initData[0];
  recData[1] = IMU.getAccelY_mss()-initData[1];
  recData[2] = IMU.getAccelZ_mss()-initData[2];
  recData[3] = IMU.getGyroX_rads()-initData[3];
  recData[4] = IMU.getGyroY_rads()-initData[4];
  recData[5] = (IMU.getGyroZ_rads(),6)-initData[5];
  recData[6] = IMU.getMagX_uT();
  recData[7] = IMU.getMagY_uT();
  recData[8] = IMU.getMagZ_uT();
  recData[9] = IMU.getTemperature_C();
  recData[10] = Serial2.parseInt();
  recData[11] = Serial3.parseInt();
}

//Writes recData onto file in Memory Unit
void writeMem(float recData[12], int timeCounter){
  mainFile = SD.open("flight.txt", FILE_WRITE);
  mainFile.print(timeCounter);
  for (int i = 1; i < 12; i++){
    mainFile.print(recData[i]);
    mainFile.print("\t");
  }
  mainFile.println("");
  mainFile.close();  
}

void loop() {
  //Reads from Sensors and Writes to Memory every 0.1s
  timeCounter += 1;
  readSensors(recData);

  
  writeMem(recData, timeCounter);
  
  Serial1.print("TIME: ");
  Serial1.print(timeCounter*0.25);
  Serial1.println("s");

  Serial1.print("Acceleration in X: ");
  Serial1.print(recData[0]);
  Serial1.println("m/s^2");

  Serial1.print("Acceleration in Y: ");
  Serial1.print(recData[1]);
  Serial1.println("m/s^2");
  
  Serial1.print("Acceleration in Z: ");
  Serial1.print(recData[2]);
  Serial1.println("m/s^2");
  
  
  Serial1.print("Rotation in X: ");
  Serial1.print(recData[3]);
  Serial1.println("rad/s");

  Serial1.print("Rotation in Y: ");
  Serial1.print(recData[4]);
  Serial1.println("rad/s");
  
  Serial1.print("Rotation in Z: ");
  Serial1.print(recData[5]);
  Serial1.println("rad/s");

  Serial1.print("Magnetometer in X: ");
  Serial1.print(recData[6]);
  Serial1.println("uT");

  Serial1.print("Magnetometer in Y: ");
  Serial1.print(recData[7]);
  Serial1.println("uT");

  Serial1.print("Magnetometer in Z: ");
  Serial1.print(recData[8]);
  Serial1.println("uT");

  Serial1.print("Temperature: ");
  Serial1.print(recData[9]);
  Serial1.println(" C");

  Serial1.print("Altitude Reading #1: ");
  Serial1.print(recData[10]);
  Serial1.println("ft.");

  Serial1.print("Altitude Reading #2: ");
  Serial1.print(recData[11]);
  Serial1.println("ft.");

  delay(250);

}
