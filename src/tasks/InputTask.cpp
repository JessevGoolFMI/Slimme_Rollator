#include "InputTask.h"
#include <Arduino.h>
#include "utils/Config.h"
#include "freertos/semphr.h"
#include "tasks/SensorTask.h"

int touchwaarde1 = 0;
int touchwaarde2 = 0;
bool startVoorwaarde = false;

// Task voor het controleren of een persoon de rollator gebruikt
void inputTask(void *parameter) {

  int counter = 0;
  for(;;) { // Oneindige lus
    touchwaarde1 = touchRead(TOUCH_PIN1); // Touchwaarde lezen
    touchwaarde2 = touchRead(TOUCH_PIN2); // Touchwaarde lezen
    float afstand_R;
    float afstand_L;
    float offset = -0.05; // Ofsett waarde voor bepalen juiste afstand

    // Afstand opvragen
    xSemaphoreTake(sensorMutex, portMAX_DELAY);
    afstand_R = afstandR;
    afstand_L = afstandL;
    xSemaphoreGive(sensorMutex);

    // Als een persoon voor de rollator staat begin counten
    if (afstand_R > -1.2 && afstand_L > -1.2) counter++;
    else counter = 0;

    // Activeer de regelkring als persoon dichtbij is en er wordt gedrukt
    if (counter >= 1 && touchwaarde1 > 55000 && touchwaarde2 > 75000) {
      startVoorwaarde = true;
    } 
    else if ((afstand_R < loopAfstand + offset && afstand_L < loopAfstand + offset ) || ( afstand_R > loopAfstand - offset && afstand_L > loopAfstand - offset)) {
      startVoorwaarde = false;
    }
    else {
      startVoorwaarde = false;
    }

    if (logInputTask) {
      Serial.print("[InputTask] Touch1: ");
      Serial.print(touchwaarde1);
      Serial.print(" | Touch2: ");
      Serial.print(touchwaarde2);
      Serial.print("  StartVoorwaarde: ");
      Serial.println(startVoorwaarde);
    }

    // Wacht 200 ms voor volgende controle
    vTaskDelay(pdMS_TO_TICKS(200)); // Geef cpu vrij
  }
}

// Start de inputTask op core 0
void initInputTask() {
  xTaskCreatePinnedToCore(inputTask, "Input Task", 2048, NULL, 1, NULL, 0);
}
