#include "PID.h"
#include <Arduino.h>
#include "utils/Config.h"

DiscretePID::DiscretePID(float setpoint, bool invert) : setpoint(setpoint), invertOutput(invert) {
  if (xSemaphoreTake(configMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    kt = 0.951; ka = 0.072;
    pi = 3.14159;
    Mp_raw = gewicht / 2; // kg massa persoon
    Mr_raw = 7.5; // kg massa rollator
    k = 100 * (armSterkte * 0.1); // Nm/rad buistijfheid arm
    itot = 1 / 0.085; // totale overbrenging
    Mp = Mp_raw;
    Mr = Jm * (itot * itot) + Mr_raw;
    Wr = 3.1; // rad/s
    War = 1.3; // rad/s
    Wb = Wr * 0.1; // bandbreedte
    Me = Mr + Mp;
    ksl = Wb * Wb * Me;
    kpl = (ksl / (ka * kt)) * (duwkracht / 2); // kp last kant
    beta = 0.8; // 1/3 1/3 1/3 bewingsprofiel
    kv = 2 * beta * Me * Wb;
    kta = kv / (ka * kt); // Servo-demping

    Ts = 0.03; // sampletijd
    taudm = (1 / (Wb / 3));
    tauim = 1 / (Wb / 10);
    taufm = 1 / (3 * Wb);

    a1 = 1 - (2 * taudm) / Ts;
    a2 = 1 + (2 * taudm) / Ts;
    a3 = 1 - (2 * taufm) / Ts;
    a4 = 1 + (2 * taufm) / Ts;
    a5 = 1 - (2 * tauim) / Ts;
    a6 = 1 + (2 * tauim) / Ts;
    a7 = (2 * tauim) / Ts;
    a8 = -(2 * tauim) / Ts;

    b1 = a4 * a7;
    b2 = a3 * a7 + a4 * a8;
    b3 = a3 * a8;
    c1 = a2 * a6;
    c2 = a1 * a6 + a2 * a5;
    c3 = a1 * a5;

    for (int i = 0; i < 3; ++i) {
      error_window[i] = 0;
      u_window[i] = 0;
    }
    xSemaphoreGive(configMutex);
  } else {
    Serial.println("[PID] Failed to take mutex in constructor");
  }
}

float DiscretePID::compute(float measurement) {
  if (xSemaphoreTake(configMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    // Update dynamische parameters
    Mp_raw = gewicht / 2; // halve massa gebruiker
    Mp = Mp_raw;
    Mr = Jm * (itot * itot) + Mr_raw;
    Me = Mr + Mp;
    k = 80 * (armSterkte * 0.1); // Update k
    Wb = Wr * 0.1;
    ksl = Wb * Wb * Me;
    kpl = (ksl / (ka * kt)) * (duwkracht / 2); // kp last kant

    // Fout en vorige fouten opslaan
    error_window[2] = error_window[1];
    error_window[1] = error_window[0];
    error_window[0] = setpoint - measurement;

    u_window[2] = u_window[1];
    u_window[1] = u_window[0];

    // Tamme PID berekening
    u_window[0] = (c1 / b1) * error_window[0] +
                  (c2 / b1) * error_window[1] +
                  (c3 / b1) * error_window[2] -
                  (b2 / b1) * u_window[1] -
                  (b3 / b1) * u_window[2];

    float output = kpl * u_window[0] * ka;
    output = invertOutput ? -output : output;

    xSemaphoreGive(configMutex);

    if (logPID) {
      Serial.print("Error1: "); Serial.println(error_window[0]);
      Serial.print("Error2: "); Serial.println(error_window[1]);
      Serial.print("Error3: "); Serial.println(error_window[2]);
      Serial.print("Setpoint: "); Serial.println(setpoint);
      Serial.print("measurement: "); Serial.println(measurement);
      Serial.print("u_window0: "); Serial.println(u_window[0]);
      Serial.print("u_window1: "); Serial.println(u_window[1]);
      Serial.print("u_window2: "); Serial.println(u_window[2]);
      Serial.print("output: "); Serial.println(output);
      Serial.print("measurement: "); Serial.println(measurement);
      Serial.print("c1: "); Serial.println(c1);
      Serial.print("c2: "); Serial.println(c2);
      Serial.print("c3: "); Serial.println(c3);
      Serial.print("b1: "); Serial.println(b1);
      Serial.print("b2: "); Serial.println(b2);
      Serial.print("b3: "); Serial.println(b3);
    }

    return constrain(output, -9, 9);
  } else {
    Serial.println("[PID] Failed to take mutex in compute");
    return 0; // Return safe default or previous output
  }
}
