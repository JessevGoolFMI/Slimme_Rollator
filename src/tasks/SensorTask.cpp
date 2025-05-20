#include <Arduino.h>
#include "SensorTask.h"
#include <Wire.h>
#include "VL53L1X.h"
#include "utils/Config.h"
#include "freertos/semphr.h"

// Twee I2C-bussen worden gedefinieerd voor de afzonderlijke TOF-sensoren
TwoWire I2C_bus1(0);
TwoWire I2C_bus2(1);
VL53L1X sensorR;
VL53L1X sensorL;

// Globale afstandsvariabelen gedeeld met andere taken
float afstandR = 0;
float afstandL = 0;
SemaphoreHandle_t sensorMutex; // Mutex voor toegang tot afstandsdata

// Deze taak wordt continu uitgevoerd op core 0 en leest beide TOF-sensoren uit
void sensorTask(void *parameter) {
  // Initialiseer de rechter sensor (sensorR) op I2C-bus 1
  I2C_bus1.begin(SDA_R, SCL_R);
  sensorR.setBus(&I2C_bus1);
  sensorR.init();
  sensorR.setDistanceMode(VL53L1X::Short);
  sensorR.setMeasurementTimingBudget(33000);

  // Initialiseer de linker sensor (sensorL) op I2C-bus 2
  I2C_bus2.begin(SDA_L, SCL_L);
  sensorL.setBus(&I2C_bus2);
  sensorL.init();
  sensorL.setDistanceMode(VL53L1X::Short);
  sensorL.setMeasurementTimingBudget(33000);

  Serial.println("[SensorTask] Sensoren geïnitialiseerd.");

  for(;;) { // Oneindige lus
    // Lees ruwe afstandsmetingen van beide sensoren
    float distR = sensorR.readRangeSingleMillimeters();
    float distL = sensorL.readRangeSingleMillimeters();

    // Zet om naar meters en maak waarden negatief
    xSemaphoreTake(sensorMutex, portMAX_DELAY); // Task wacht oneindig lang tot mutex beschikbaar is deze geeft toegang tot de sensor waardes
    afstandR = -distR / 1000.0;
    afstandL = -distL / 1000.0;
    xSemaphoreGive(sensorMutex);

    // Print debuginformatie naar de seriële monitor
    if (logSensorTask) {
      Serial.print("[SensorTask] AfstandR: ");
      Serial.print(afstandR);
      Serial.print(" m, AfstandL: ");
      Serial.println(afstandL);
    }

    // Wacht 30 ms voor volgende meting (33 Hz updatesnelheid)
    vTaskDelay(pdMS_TO_TICKS(30)); // Geef cpu vrij
  }
}

// Functie maakt een mutex aan en start de sensorTask op core 0
void initSensorTask() {
  sensorMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(sensorTask, "Sensor Task", 4096, NULL, 1, NULL, 0);
}
