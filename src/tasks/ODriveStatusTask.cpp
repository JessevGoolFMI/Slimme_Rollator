#include "ODriveStatusTask.h"
#include <Arduino.h>
#include <ODriveUART.h>
#include "utils/Config.h"

// Gebruik dezelfde seriële poorten als in ControlTask.cpp
extern ODriveUART odriveR;
extern ODriveUART odriveL;

void odriveStatusTask(void *parameter) {
  for (;;) {
    ODriveFeedback feedbackL = odriveL.getFeedback();
    ODriveFeedback feedbackR = odriveR.getFeedback();

    if (logControlTask) {
      Serial.print("[ODriveStatus] posL: ");
      Serial.print(feedbackL.pos);
      Serial.print(", velL: ");
      Serial.print(feedbackL.vel);
      Serial.print(" || posR: ");
      Serial.print(feedbackR.pos);
      Serial.print(", velR: ");
      Serial.println(feedbackR.vel);
    }

    vTaskDelay(pdMS_TO_TICKS(100));  // 10 Hz update
  }
}

void initODriveStatusTask() {
  xTaskCreatePinnedToCore(odriveStatusTask, "ODrive Status Task", 4096, NULL, 1, NULL, 1);
}
