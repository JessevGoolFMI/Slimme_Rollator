#include "ControlTask.h"
#include "utils/Config.h"
#include "tasks/SensorTask.h"
#include "tasks/InputTask.h"
#include "pid/PID.h"
#include <ODriveUART.h>
#include <Arduino.h>

// Twee UART kanalen voor communicatie met de ODrive motorcontrollers
ODriveUART odriveR(Serial1);
ODriveUART odriveL(Serial2);

// Deze taak draait op core 1 en voert de PID regeling en motorsturing uit
void controlTask(void *parameter) {
  Serial1.begin(115200, SERIAL_8N1, ODRIVE_R_RX, ODRIVE_R_TX);
  Serial2.begin(115200, SERIAL_8N1, ODRIVE_L_RX, ODRIVE_L_TX);

  Serial.println("[ControlTask] ODrive wordt geïnitialiseerd...");
  odriveR.clearErrors();
  odriveR.setState(AXIS_STATE_IDLE);
  odriveL.clearErrors();
  odriveL.setState(AXIS_STATE_IDLE);

  // Creeer tamme PID regelingen
  DiscretePID pidR(-0.2, true);
  DiscretePID pidL(-0.2, false);

  bool inClosedLoop = false;  // Houd bij of de ODrives al in closed-loop zijn

  for(;;) {
    // Als er geen startvoorwaarde is, zet ODrives in IDLE om veiligheid te waarborgen
    if (!startVoorwaarde) {
      if (inClosedLoop) {
        odriveR.setState(AXIS_STATE_IDLE);
        odriveL.setState(AXIS_STATE_IDLE);
        odriveL.clearErrors();
        odriveR.clearErrors();
        Serial.println("[ControlTask] ODrives naar IDLE gezet.");
        inClosedLoop = false;
      }
      vTaskDelay(pdMS_TO_TICKS(30));
      continue;
    }

    // Indien nog niet actief, zet ODrives nu pas in CLOSED_LOOP_CONTROL
    if (startVoorwaarde) {
      odriveR.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
      odriveL.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
      odriveL.clearErrors();
      odriveR.clearErrors();
      inClosedLoop = true;
    }
    
    odriveL.clearErrors();
    odriveR.clearErrors();

    float r, l;
    // Afstand data wordt opgehaald
    xSemaphoreTake(sensorMutex, portMAX_DELAY);
    r = afstandR;
    l = afstandL;
    xSemaphoreGive(sensorMutex);

    // Bereken torq
    float torqueR = pidR.compute(r);
    float torqueL = pidL.compute(l);

    // Set de torq naar de ODrives
    odriveR.setTorque(torqueR);
    odriveL.setTorque(torqueL);

    if (logControlTask) {
      Serial.print("[ControlTask] TorqueR: ");
      Serial.print(torqueR);
      Serial.print(" | TorqueL: ");
      Serial.println(torqueL);
    }

    // Regelcyclus van 33 Hz
    vTaskDelay(pdMS_TO_TICKS(30)); // Geef cpu vrij
  }
}

// Start de ControlTask op core 1 met prioriteit
void initControlTask() {
  xTaskCreatePinnedToCore(controlTask, "Control Task", 4096, NULL, 1, NULL, 1);
}
