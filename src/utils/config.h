#ifndef CONFIG_H
#define CONFIG_H

#include <freertos/semphr.h>
#define SDA_R 8
#define SCL_R 9
#define SDA_L 36
#define SCL_L 35
#define TOUCH_PIN1 T4
#define TOUCH_PIN2 T5
#define ODRIVE_R_RX 21
#define ODRIVE_R_TX 14
#define ODRIVE_L_RX 17
#define ODRIVE_L_TX 18


#define WIFI_SSID "smartrollator" // http://smartrollator.local voor webinterface
#define WIFI_PASS "12345678"

// Standaardinstellingen voor waardes web interface
#define DEFAULT_LOOPAFSTAND 0.2f
#define DEFAULT_GEWICHT 100.0f
#define DEFAULT_LENGTE 1.8f
#define DEFAULT_ARMSTERKTE 10.0f
#define DEFAULT_DUWKACHT 0.5f

// Globale configuratie-variabelen die via de webinterface worden ingesteld
extern float loopAfstand;
extern float gewicht;
extern float lengte;
extern float armSterkte;
extern float duwkracht;

// Globale variabelen voor de loggers
extern bool logSensorTask;
extern bool logInputTask;
extern bool logControlTask;
extern bool logWiFiTask;
extern bool logODriveStateTask;
extern bool logPID;



SemaphoreHandle_t configMutex = NULL;

void initSerial(unsigned long baud = 115200);

#endif
