#include "PID.h"
#include <Arduino.h>
#include "utils/Config.h"

DiscretePID::DiscretePID(float setpoint, bool invert) : setpoint(setpoint), invertOutput(invert) {
  kt = 0.951; ka = 0.072;
  pi = 3.14159;
  Mp_raw = gewicht/2; // kg massa persoon
  Mr_raw = 7.5; //kg massa rollator  **afhankelijk per persoon**
  k = 100 * (armSterkte * 0.1); //Nm/rad buistijfheid arm  **afhankelijk per persoon**
  itot = 1/0.085; // totale overbrenging
  Mp = Mp_raw; // formule terugredenering van fysische modellen
  Mr = Jm * (itot * itot) + Mr_raw; // formule terugredenering van fysische modellen
  Wr = 3.1; //rad/s
  War = 1.3; //rad/s
  Wb = Wr * 0.1; //bandbreedte
  Me = Mr + Mp;
  ksl = Wb * Wb * Me;
  Serial.println("in PID");
  Serial.println(duwkracht);
  kpl = (ksl / (ka * kt)) * (duwkracht / 2); //kp last kant
  beta = 0.8; //1/3 1/3 1/3 bewingsprofiel
  kv = 2 * beta * Me * Wb;
  kta = kv / (ka * kt); // Servo-demping

  Ts = 0.03; // sampletijd 
  // tau I tau D en tau F actie
  taudm = (1 / (Wb / 3));
  tauim = 1 / (Wb / 10);
  taufm = 1 / (3 * Wb);

  // Vereenvoudingingen functie tamme PID regelaar
  a1 = 1 - (2 * taudm)/Ts;
  a2 = 1 + (2 * taudm)/Ts;
  a3 = 1 - (2 * taufm)/Ts;
  a4 = 1 + (2 * taufm)/Ts;
  a5 = 1 - (2 * tauim)/Ts;
  a6 = 1 + (2 * tauim)/Ts;
  a7 = (2 * tauim)/Ts;
  a8 = -(2 * tauim)/Ts;

  // Verder vereenvoudingen functie tamme PID regelaar
  b1 = a4 * a7;
  b2 = a3 * a7+a4 * a8;
  b3 = a3 * a8;
  c1 = a2 * a6;
  c2 = a1 * a6+a2 * a5;
  c3 = a1 * a5;

  // Declareren error en spanning window
  for (int i = 0; i < 3; ++i) {
    error_window[i] = 0;
    u_window[i] = 0;
  }
}

float DiscretePID::compute(float measurement) {
  // Update dynamische parameters bij elke cyclus (gewicht, armSterkte, duwkracht)
  Mp_raw = gewicht / 2; // halve massa gebruiker
  Mp = Mp_raw;
  Mr = Jm * (itot * itot) + Mr_raw;
  Me = Mr + Mp;

  // Update k op basis van armSterkte
  k = 80 * (armSterkte * 0.1);

  // Bandbreedte en afgeleiden
  Wb = Wr * 0.1;
  ksl = Wb * Wb * Me;
  kpl = (ksl / (ka * kt)) * (duwkracht / 2); //kp last kant

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

if (logPID){ 
    Serial.print("Error1: ");
    Serial.println(error_window[0]);
    Serial.print("Error2: ");
    Serial.println(error_window[1]);
    Serial.print("Error3: ");
    Serial.println(error_window[2]);
    Serial.print("Setpoint: ");
    Serial.println(setpoint);
    Serial.print("measurement: ");
    Serial.println(measurement);
    Serial.print("u_window0: ");
    Serial.println(u_window[0]);
    Serial.print("u_window1: ");
    Serial.println(u_window[1]);
    Serial.print("u_window2: ");
    Serial.println(u_window[2]);
    Serial.print("output: ");
    Serial.println(output);
    Serial.print("measurement: ");
    Serial.println(measurement);
    Serial.print("c1: ");
    Serial.println(c1);
    Serial.print("c2: ");
    Serial.println(c2);
    Serial.print("c3: ");
    Serial.println(c3);
    Serial.print("b1: ");
    Serial.println(b1);
    Serial.print("b2: ");
    Serial.println(b2);
    Serial.print("b3: ");
    Serial.println(b3);
  } 

  return constrain(output, -9, 9);
}
