#include <Arduino.h>
#include <Wire.h>//Adafuruit Bus IO needs Wire.h
#include <SPI.h>
#include "Adafruit_MAX31855.h"

/*Pb-free-Profile*****************
 1.Heating stage 1  :       ~ 150°C
 2.Preheating stage : 150°C ~ 200°C : 60s ~ 120s
 3.Heating stage 2  : 200°C ~ 245°C :            :
 4.reflow stage     : 240°C ~ 245°C :     ~ 30s  : 217°C↑ 60s ~ 120s
 5.cooling stage    :               :            :
*********************************/
/*63Sn/37Pb-Profile*****************
 1.Heating stage 1  :       ~ 150°C
 2.Preheating stage :     150°C     : 60s ~ 120s
 3.Heating stage 2  : 150°C ~ 183°C :            :
 4.reflow stage     : 183°C ~ 230°C :     ~ 30s  :
 5.cooling stage    :               :            :
*********************************/
#define STAGE1_TEMP 150
#define STAGE2_TEMP 175
#define STAGE3_TEMP 230
#define STAGE4_TEMP_MAX 245
#define STAGE4_TEMP 230

#define HOTPLATE_PIN 3
#define CS_PIN 10

Adafruit_MAX31855 thermocouple(CS_PIN);

void temp_control(int pin, int current_temp, int target_temp);

void setup() {
  pinMode(HOTPLATE_PIN,OUTPUT);
  digitalWrite(HOTPLATE_PIN,LOW);
  Serial.begin(9600);
  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
  Serial.print("Initializing sensor...");
  if (!thermocouple.begin()) {
    Serial.println("ERROR.");
    while (1) delay(10);
  }

  Serial.println("DONE.");
  Serial.println("wait for start...");
  while(!(Serial.available())){
    Serial.println(thermocouple.readCelsius());
    delay(1000);
  }
}

void loop() {
  static int temperature = 0;
  static int stage = 1;
  static long timerstart = 0;
  static long times;

  //write temp getting function
  temperature = thermocouple.readCelsius();

  if(stage == 1){
    Serial.print(STAGE1_TEMP);
    if(temperature < STAGE1_TEMP){
      digitalWrite(HOTPLATE_PIN,HIGH);
    }else{
      stage = 2;
      timerstart = millis();
    }
  }else if(stage == 2){
    times = millis() - timerstart;

    if(times < 100000){//100s
      Serial.print(STAGE2_TEMP);
      //175°C
      if(temperature > STAGE2_TEMP){
        digitalWrite(HOTPLATE_PIN,LOW);
      }else{
        digitalWrite(HOTPLATE_PIN,HIGH);
      }
    }else{
      stage = 3;
    }
  }else if(stage == 3){
    Serial.print(STAGE3_TEMP);
    if(temperature < STAGE3_TEMP){
      digitalWrite(HOTPLATE_PIN,HIGH);
    }else{
      stage = 4;
      timerstart = millis();
    }
  }else if(stage == 4){
    times = millis() - timerstart;
    Serial.print(STAGE4_TEMP_MAX);
    if(times < 20000){//20s
      if(temperature < STAGE4_TEMP){
        digitalWrite(HOTPLATE_PIN,HIGH);
      }else{
        digitalWrite(HOTPLATE_PIN,LOW);
      }
    }else{
      stage = 5;
    }
  }else if(stage == 5){
    Serial.print(0);
    digitalWrite(HOTPLATE_PIN,LOW);
    if(temperature < 40){
      stage = 6;
    }
  }else if(stage == 6){
    Serial.println("Done.");
    while(true);
  }
  Serial.print(",");
  Serial.println(temperature);
  delay(1000);
}

void temp_control(int pin, int current_temp, int target_temp){
  int deviation = target_temp - current_temp;
  static int old_deviation = deviation;
  static int sigma = 0;
  float value;
  const float param_p = 1;
  const float param_i = 1;
  const float param_d = 1;
  //proportional
  value = param_p* deviation;
  //integral
  sigma += deviation;
  value += param_i* sigma;
  //differential
  value += param_d* (deviation - old_deviation);
  
}