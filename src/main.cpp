//******************************************************************************
#define FIRMWARE_VERSION 1.10
#define HOSTNAME "mqttnode"
//#define PRODUCTION_SERIAL true      //uncoment to turn the serial debuging off
#define SERIAL_SPEED 9600           // 9600 for BLE friend
//******************************************************************************

extern "C" {
#include "user_interface.h"                                                     //NOTE needed for esp info
}

#include <Arduino.h>
#include "credentials.h"
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
//   |--------------|-------|---------------|--|--|--|--|--|

WiFiClient espClient;

#include <PubSubClient.h>
PubSubClient client(espClient);

#include "DHT.h"
#include "Adafruit_Sensor.h"
#define DHTPIN 12     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#include "Adafruit_CCS811.h"
Adafruit_CCS811 ccs;

unsigned long previousMillis, currentMillis = 0;
int interval = 3 * 1000;

#include <Adafruit_NeoPixel.h>
#define NEO_PIXEL_PIN 14
int pixelBrightness = 25;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, NEO_PIXEL_PIN, NEO_RGB + NEO_KHZ800);

// ----------------------- set neopixel colors --------------------------------
uint32_t black = strip.Color(0, 0, 0);
uint32_t red = strip.Color(255, 0, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t magenta = strip.Color(255, 0, 255);
uint32_t blue_light = strip.Color(110, 205, 240);
uint32_t orange = strip.Color(217, 190, 47);
uint32_t white = strip.Color(255,255,255);
uint32_t yellow = strip.Color(255,100,0);

enum list {SETUP, OTA, PORTAL, WIFI, MQTT, OFF, ALARM, TEST, SENSOR};

void set_neo_pixel(list status){
  uint32_t color;
  switch (status){
    case SETUP : color = green; break;
    case OTA : color = magenta; break;
    case PORTAL : color = orange; break;
    case WIFI : color = blue_light; break;
    case MQTT : color = blue; break;
    case OFF : color = black; break;
    case ALARM : color = red; break;
    case TEST : color = white; break;
    case SENSOR : color = yellow; break;
  }
  strip.setPixelColor(0, color);
  strip.setBrightness(pixelBrightness);
  strip.show();
}
// -----------------------------------------------------------------------------

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';
    String strTopic = String((char*)topic);
    Serial.print("Received MQTT: "); Serial.println(strTopic);

    if (strTopic == "node/setup/neo") {
      String msg1 = (char*)payload;
      if (msg1 == "true"){ set_neo_pixel(TEST); }
      if (msg1 == "false"){ set_neo_pixel(OFF); }
    }
    if (strTopic == "node/setup/interval") {
      String msg2 = (char*)payload;
      Serial.println(msg2);
      interval = msg2.toInt();
    }
    if (strTopic == "node/setup/brightness") {
      String msg3 = (char*)payload;
      Serial.println(msg3);
      pixelBrightness = msg3.toInt();
    }
}

void reconnect() {                                                               //TODO dodac zabezpieczenie, sprawdzic status rain przy reset, odswiezeniu polaczenia
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client,mqtt_user,mqtt_pass)) {
      Serial.println("connected");
      set_neo_pixel(MQTT);
      // Once connected, publish an announcement...
      client.publish("node/system/status", "connected");
      char ver[4];
      dtostrf(FIRMWARE_VERSION, 3, 2, ver);
      client.publish("node/system/firmware", ver);
      // ... and resubscribe
      client.subscribe("node/setup/#");
    } else {
      set_neo_pixel(ALARM);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
    strip.begin();
    delay(200);
    set_neo_pixel(SETUP);
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

   dht.begin();

   Serial.println("CCS811 test");
   if(!ccs.begin()){
       Serial.println("Failed to start sensor! Please check your wiring.");
       set_neo_pixel(ALARM);
   while(1);
 }

 //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

   // ------------------------------------- OTA -----------------------------------
  #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
    Serial.print("Hostname: "); Serial.println(HOSTNAME);
  #endif
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onStart([]() {
  #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
    Serial.println("Start updating ");
  #endif
  set_neo_pixel(OTA);
  });

  ArduinoOTA.onEnd([]() {
  #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
    Serial.println("\nEnd");
  #endif
  set_neo_pixel(OFF);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    set_neo_pixel(OTA);
      #ifndef PRODUCTION_SERIAL // Not in PRODUCTION
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      #endif
    set_neo_pixel(OFF);
  });

  ArduinoOTA.onError([](ota_error_t error) {
      set_neo_pixel(ALARM);
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

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    delay(500);
    #ifndef PRODUCTION_SERIAL
        Serial.print("\r\nIP address: ");
        Serial.println(WiFi.localIP());
    #endif
    set_neo_pixel(WIFI);
}

void loop() {
    ArduinoOTA.handle();
    if (!client.connected()) {
            reconnect();
    }
    client.loop();

    currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
        set_neo_pixel(SENSOR);
        previousMillis = currentMillis;

        client.publish("node/system/interval", String(interval/1000).c_str());
        client.publish("node/system/brightness", String(pixelBrightness).c_str());

        float h = dht.readHumidity();
        client.publish("node/sensor/dht/humidity", String(h).c_str());

        float t = dht.readTemperature();
        client.publish("node/sensor/dht/temperature", String(t).c_str());

        // Check if any reads failed and exit early (to try again).
        if (isnan(h) || isnan(t)) {
            set_neo_pixel(ALARM);
            Serial.println(F("Failed to read from DHT sensor!"));
            return;
        }

        //Compute heat index in Celsius (isFahreheit = false)
        float hic = dht.computeHeatIndex(t, h, false);
        client.publish("node/sensor/dht/heatindex", String(hic).c_str());
        set_neo_pixel(OFF);

        if(ccs.available()){
            if(!ccs.readData()){
                float t = ccs.calculateTemperature();
                client.publish("node/sensor/ccs/temperature", String(t).c_str());
                int co2 = ccs.geteCO2();
                client.publish("node/sensor/ccs/CO2", String(co2).c_str());
                int tvoc = ccs.getTVOC();
                client.publish("node/sensor/ccs/TVOC", String(tvoc).c_str());
            }
            else{
              Serial.println("ERROR!");
              set_neo_pixel(ALARM);
              while(1);
            }
        }
    }
}
