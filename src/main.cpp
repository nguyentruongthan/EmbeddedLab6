#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <cstdlib>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID ""
#define WLAN_PASS ""
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME ""
#define AIO_KEY ""

// Setup MQTT
WiFiClient client;
Adafruit_MQTT_Client mqtt(
  &client, AIO_SERVER, AIO_SERVERPORT,
  AIO_USERNAME, AIO_KEY);

// Set publish
Adafruit_MQTT_Publish temp_pub = 
  Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

void setup() {
  // This serial is used to connect to STM32
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);

  // Set busy to HIGH
  digitalWrite(5, HIGH);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);    
  }

  while(mqtt.connect() != 0){
    delay(500);
  }

  // Finish setup, set busy to LOW
  digitalWrite(5, LOW);
}

uint8_t is_begin = 0;
uint8_t serial_buffer[100];
uint8_t length = 0;

void handle_serial_msg(uint8_t byte){
  if(byte == '!'){
    is_begin = 1;
  }else{
    if(is_begin){
      if(byte != '#'){
        serial_buffer[length ++] = byte;
      }else{
        serial_buffer[length] = '\0';
        float temp_value = std::atof((char*)serial_buffer);
        temp_pub.publish(temp_value);
        is_begin = 0;
        length = 0;
      }
    }

  }
}
int count = 0;
int blink_led_count = 10;
uint8_t blink_led_status = 0;
void loop() {
  blink_led_count -= 10;
  if(blink_led_count <= 0)
  {
    blink_led_count = 1000;
    if(blink_led_status == 0){
      blink_led_status = 1;
      digitalWrite(2, HIGH);
    }else{
      blink_led_status = 0;
      digitalWrite(2, LOW);
    }
  }

  if(Serial.available()) {
    while(Serial.available()) {
      uint8_t byte = Serial.read();
      handle_serial_msg(byte);
    }
  }
  delay(10);
}

