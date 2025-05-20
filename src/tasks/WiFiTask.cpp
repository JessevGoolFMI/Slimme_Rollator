#include "WiFiTask.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <Arduino.h>
#include "utils/Config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

WebServer server(80);
Preferences prefs;

float loopAfstand = DEFAULT_LOOPAFSTAND;
float gewicht = DEFAULT_GEWICHT;
float lengte = DEFAULT_LENGTE;
float armSterkte = DEFAULT_ARMSTERKTE;
float duwkracht = DEFAULT_DUWKACHT;

void loadStoredValues() {
  prefs.begin("rollator", true);
  loopAfstand = prefs.getFloat("loopAfstand", DEFAULT_LOOPAFSTAND);
  gewicht = prefs.getFloat("gewicht", DEFAULT_GEWICHT);
  lengte  = prefs.getFloat("lengte", DEFAULT_LENGTE);
  armSterkte = prefs.getFloat("armSterkte", DEFAULT_ARMSTERKTE);
  duwkracht = prefs.getFloat("duwkracht", DEFAULT_DUWKACHT);
  prefs.end();
}

void saveValue(const char* key, float value) {
  prefs.begin("rollator", false);
  prefs.putFloat(key, value);
  prefs.end();
}

String htmlPage() {
  String page = "<!DOCTYPE html><html><head><title>Rollator Configuratie</title>";
  page += "<style>body{font-family:sans-serif;background:#f5f5f5;padding:20px;}h2{color:#333;}";
  page += ".container{display:flex;gap:20px;align-items:flex-start;}form{background:#fff;padding:15px;border-radius:8px;box-shadow:0 0 10px rgba(0,0,0,0.1);}input[type=text]{width:120px;margin-bottom:10px;padding:5px;}input[type=submit]{padding:8px 15px;background:#4285f4;color:#fff;border:none;border-radius:4px;cursor:pointer;}input[type=submit]:hover{background:#3367d6;}img{width:300px;border-radius:8px;}";
  page += "</style></head><body><h2>Instellingen Slimme Rollator</h2><div class='container'><form action='/update' method='POST'>";
  page += "Loop Afstand (m): <input type='text' name='loopAfstand' value='" + String(loopAfstand) + "'><br>";
  page += "Gewicht Gebruiker (kg): <input type='text' name='gewicht' value='" + String(gewicht) + "'><br>";
  page += "Lengte Gebruiker (m): <input type='text' name='lengte' value='" + String(lengte) + "'><br>";
  page += "Armsterkte (0-10): <input type='text' name='armSterkte' value='" + String(armSterkte) + "'><br>";
  page += "Duwkracht Rollator: <input type='text' name='duwkracht' value='" + String(duwkracht) + "'><br>";
  page += "<input type='submit' value='Opslaan'>";
  page += "</form><img src='/InformatieRollator.jpg' alt='Rollator'></div></body></html>";
  return page;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleUpdate() {
  if (xSemaphoreTake(configMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    if (server.hasArg("loopAfstand")) {
      loopAfstand = server.arg("loopAfstand").toFloat();
      saveValue("loopAfstand", loopAfstand);
      Serial.print("Loopafstand: "); Serial.println(loopAfstand);
    }
    if (server.hasArg("gewicht")) {
      gewicht = server.arg("gewicht").toFloat();
      saveValue("gewicht", gewicht);
      Serial.print("Gewicht: "); Serial.println(gewicht);
    }
    if (server.hasArg("lengte")) {
      lengte = server.arg("lengte").toFloat();
      saveValue("lengte", lengte);
      Serial.print("Lengte: "); Serial.println(lengte);
    }
    if (server.hasArg("armSterkte")) {
      armSterkte = server.arg("armSterkte").toFloat();
      saveValue("armSterkte", armSterkte);
      Serial.print("Armsterkte: "); Serial.println(armSterkte);
    }
    if (server.hasArg("duwkracht")) {
      duwkracht = server.arg("duwkracht").toFloat();
      saveValue("duwkracht", duwkracht);
      Serial.print("Duwkracht: "); Serial.println(duwkracht);
    }
    xSemaphoreGive(configMutex);
  } else {
    Serial.println("[WiFiTask] Failed to take mutex for update");
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void wifiTask(void *parameter) {
  Serial.println("[WiFiTask] Starten van WiFi AP...");
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  delay(1000);
  Serial.print("[WiFiTask] AP IP address: ");
  Serial.println(WiFi.softAPIP());

  loadStoredValues();

  if (!LittleFS.begin()) {
    Serial.println("[WiFiTask] LittleFS kon niet starten!");
  } else {
    Serial.println("[WiFiTask] LittleFS gestart.");
  }

  if (!MDNS.begin("smartrollator")) {
    Serial.println("[WiFiTask] mDNS fout");
  } else {
    MDNS.addService("http", "tcp", 80);
    Serial.println("[WiFiTask] mDNS gestart als smartrollator.local");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.serveStatic("/InformatieRollator.jpg", LittleFS, "/InformatieRollator.jpg");
  server.begin();
  Serial.println("[WiFiTask] Webserver gestart.");

  for (;;) {
    server.handleClient();
    if (logWiFiTask) {
      //logger
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void initWiFiTask() {
  // Create mutex before starting tasks
  configMutex = xSemaphoreCreateMutex();
  if (configMutex == NULL) {
    Serial.println("[WiFiTask] Failed to create mutex!");
  }

  xTaskCreatePinnedToCore(wifiTask, "WiFi Task", 4096, NULL, 1, NULL, 0);
}
