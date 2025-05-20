#include "utils/Config.h"
#include <Arduino.h>

// loggers voor alle Tasks
bool logSensorTask = false;
bool logInputTask = false;
bool logControlTask = true;
bool logWiFiTask = false;
bool logODriveStateTask = false;
bool logPID = true;

// Serial init functie
void initSerial(unsigned long baud) {
  static bool initialized = false;
  if (!initialized) {
    Serial.begin(baud);
    while (!Serial); // Wacht tot klaar (optioneel voor debugging via USB)
    Serial.println("[System] Seriële monitor gestart.");
    initialized = true;
  }
}
