// station météo sur upesy et tft 3.5 + bme280 ILI9488 Station Salle

#include <esp_now.h>
#include <WiFi.h>
#include "SPI.h"
#include "TFT_eSPI.h"
#include <BME280I2C.h> 
#include <Wire.h>
#include <time.h>

float temp_ext = 0;   float t_max = temp_ext;   float t_min = 30;
float pres;   float pres_max = pres;   float pres_min = 1040;

float humidite; float temp_moy;  int nb;
long temps; long temps_moy;

typedef struct struct_message {
    float c;
    float d;
} struct_message;
struct_message myData;

BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off, // --------------
#define TFT_GREY 0x7BEF

TFT_eSPI myGLCD = TFT_eSPI();       // Invoke custom library

void setup()                         // ----- Début du setup ----------------
{ 
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  while(!Serial) {} // Wait
  Wire.begin();
  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
 
  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");  
  }
  
// Setup the LCD
  myGLCD.init();
  myGLCD.setRotation(0);

   // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  esp_now_register_recv_cb(OnDataRecv);
  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.setTextColor(TFT_GREENYELLOW,TFT_BLACK);
  myGLCD.drawString("TEMP IN", 10, 50,4); //100
  //myGLCD.drawString( "PRES", 10, 150,4);  //180
  myGLCD.drawString("HUMIDITE", 10, 260,4);
  myGLCD.setTextDatum(TL_DATUM); // Remet text a default
  temps = millis();
}                                   // ---------------- Fin du setup ------------------

void loop()                        // --------------- Début de la loop ---------
{      
  if ( (millis() - temps) > 1000*60) {
   float temp(NAN), hum(NAN), pres(NAN);
   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);
   bme.read(pres, temp, hum, tempUnit, presUnit);
  
  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.setTextColor(TFT_GREENYELLOW,TFT_BLACK);
  myGLCD.drawString("TEMP IN", 10, 50,4);
  //myGLCD.drawString("PRES", 10, 150,4);
  myGLCD.drawString("HUMIDITE", 10, 260,4); 
  myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);
  myGLCD.drawFloat(temp, 1, 210, 40, 6);         //temp_in -3.7 TFT 2.8
  myGLCD.drawFloat(pres/100+16, 1, 150, 145, 6);   
  myGLCD.drawNumber(hum + 4, 250, 250, 6);

  if (pres/100+16 > pres_max) {pres_max=pres/100+16;}
  if (pres/100+16 < pres_min)  {pres_min=pres/100+16;}
  myGLCD.setTextColor(TFT_ORANGE,TFT_BLACK); myGLCD.drawNumber(pres_max, 10, 112,6);
  myGLCD.setTextColor(TFT_YELLOW,TFT_BLACK); myGLCD.drawNumber(pres_min, 10, 178,6);
  temps = millis() ;}       //  delay (1000*60); 
} 
// --------------- Fin de la loop -----------------

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {                                  // -------- réception données extérieur --------------
  memcpy(&myData, incomingData, sizeof(myData));
  temp_ext = myData.c;
  humidite = myData.d;
   if (temp_ext > t_max) {t_max = temp_ext;} else if(temp_ext < t_min and t_min > -30 and temp_ext > -50) {t_min = temp_ext;}    // -------- calcul mini et maxi température extérieur ---------------
  myGLCD.setTextColor(TFT_YELLOW,TFT_BLACK);
  myGLCD.drawNumber(humidite-2, 160, 250, 6);
  myGLCD.setTextColor(TFT_ORANGE,TFT_BLACK); myGLCD.drawFloat(temp_ext, 1, 130, 340, 8);
  myGLCD.setTextColor(TFT_ORANGE,TFT_BLACK); myGLCD.drawFloat(t_max, 1, 10, 330, 6); myGLCD.setTextColor(TFT_YELLOW,TFT_BLACK); myGLCD.drawFloat(t_min, 1, 10, 400, 6);  //affiche mini maxi
}
