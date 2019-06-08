//******************************************************************************
#define FIRMWARE_VERSION 1.02
#define HOSTNAME "mqttnode"
//#define PRODUCTION_SERIAL true      //uncoment to turn the serial debuging off
#define SERIAL_SPEED 9600           // 9600 for BLE friend
//******************************************************************************

#include <Arduino.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
//   |--------------|-------|---------------|--|--|--|--|--|

WiFiClient espClient;

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

   WiFiManager wifiManager;
   wifiManager.autoConnect("AutoConnectAP");

   // ------------------------------------- OTA -----------------------------------
  #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
    Serial.print("Hostname: "); Serial.println(HOSTNAME);
  #endif
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([]() {
  #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
    Serial.println("Start updating ");
  #endif
  });

  ArduinoOTA.onEnd([]() {
  #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
    Serial.println("\nEnd");
  #endif
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  #endif
  });

  ArduinoOTA.onError([](ota_error_t error) {
  #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  #endif
  });
  ArduinoOTA.begin();
//------------------------------ end OTA ---------------------------------------
}

void loop() {
    ArduinoOTA.handle();
}
