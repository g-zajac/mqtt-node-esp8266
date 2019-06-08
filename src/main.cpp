//******************************************************************************
#define FIRMWARE_VERSION 1.01
#define HOSTNAME "mqttnode"
//#define PRODUCTION_SERIAL true      //uncoment to turn the serial debuging off
#define SERIAL_SPEED 9600           // 9600 for BLE friend
//******************************************************************************

#include <Arduino.h>

void setup() {
    #ifndef PRODUCTION_SERIAL
   Serial.begin(SERIAL_SPEED);          // compiling info
     Serial.println("\r\n---------------------------------");                  //NOTE \r\n - new line, return
     Serial.println("Compiled: " __DATE__ ", " __TIME__ ", " __VERSION__);
     Serial.print("Version: "); Serial.print(FIRMWARE_VERSION); Serial.println(" by Grzegorz Zajac");
     Serial.println("---------------------------------");
     Serial.println( F("\r\n--- ESP Info --- ") );
     Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
     Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
     Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
     Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
     Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
     Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
     Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
     Serial.printf("Free size: %u\n", ESP.getFreeSketchSpace());
     Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
     Serial.println();
 #endif
}

void loop() {
  // put your main code here, to run repeatedly:
}
