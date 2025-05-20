#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks/SensorTask.h"
#include "tasks/ControlTask.h"
#include "tasks/InputTask.h"
#include "tasks/WiFiTask.h"
#include "utils/Config.h"
#include "tasks/ODriveStatusTask.h"

// Start van alle FreeRTOS taken
void setup() {
  initSerial();
  initSensorTask(); // Task voor meten van ToF sensoren (core 0)
  initInputTask(); // Task voor checken of een persoon aanwezig is (core 0)
  initControlTask(); // Task voor tamme PID regeling en aansturen ODrives (core 1)
  initWiFiTask(); // Task voor wifi gebruik (core 0)
  initODriveStatusTask(); // Task voor status van motoren (core 1)

}

// Deze functie is niet nodig omdat FreeRTOS functies de taken laten draaien dus loop wordt beeindigd. 
void loop() {
  vTaskDelete(NULL);
}
