
// web: http://www.henningkarlsen.com/electronics
/*.
 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.
 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
 ------------------ Pensez a renommez le User_Setup.h
 */
#include <esp_now.h>
#include <WiFi.h>
#include "SPI.h"
#include "TFT_eSPI.h"
#include <BME280I2C.h> 
#include <Wire.h>

float temp_ext = 0;   float t_max = temp_ext;   float t_min = 30;
long temps;

typedef struct struct_message {
    float c;
    //float d;
} struct_message;

struct_message myData;

BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off, // --------------
#define TFT_GREY 0x7BEF

TFT_eSPI myGLCD = TFT_eSPI();       // Invoke custom library
int mq2 = 0; int mq2_old = 0;
int tableau_mq[10];

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
  myGLCD.setRotation(2);

   // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  esp_now_register_recv_cb(OnDataRecv);
  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);
  myGLCD.drawString("TEMP  IN", 10,20,4);
  myGLCD.drawString("POLLUANT", 10, 100,4);
  myGLCD.drawString("PRESSION", 10, 180,4);
  myGLCD.drawString("HUMIDITE", 10, 260,4);
  temps = millis();
}                                   // ---------------- Fin du setup ------------------

void loop()                        // --------------- Début de la loop ---------
{      
  if ( (millis() - temps) > 1000*60) {
   float temp(NAN), hum(NAN), pres(NAN);
   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);
   bme.read(pres, temp, hum, tempUnit, presUnit);
          Serial.print("Temperature  ");Serial.print(temp-3);Serial.print("   Humidite  ");Serial.print(hum+9);Serial.print("   Pression. ");Serial.print(pres/100+17);Serial.print("   Qualite AIR  ");Serial.println(mq2);
  
  int i = 0; float tableau_mq[10]; int total = 0;                                                            // ----Calcul moyenne PPM -------------------------

    for(i = 0; i < 10; i++) { tableau_mq[i] = analogRead(33)/4;  delay(100); }
    for (int x = 0;x<10;x++) { total = total + tableau_mq[x]; }
    mq2 = total / 10;

  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);
  myGLCD.drawString("TEMP  IN", 10,20,4);
  myGLCD.drawString("POLLUANT", 10, 100,4);
  myGLCD.drawString("PRESSION", 10, 180,4);
  myGLCD.drawString("HUMIDITE", 10, 260,4);  

  myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);
  myGLCD.drawFloat(temp + 0.5, 1, 210, 15, 6);         //temp_in -3.7 TFT 2.8
  myGLCD.drawNumber(pres/100+18, 200, 170, 6);
  myGLCD.drawNumber(hum + 4, 220, 250, 6);
  if (mq2 > 125) {myGLCD.setTextColor(TFT_RED,TFT_BLACK); myGLCD.drawNumber(mq2, 220, 90, 6); }
  else if (mq2 > mq2_old + 10 || mq2 > 400) {myGLCD.setTextColor(TFT_ORANGE,TFT_BLACK); myGLCD.drawNumber(mq2, 220, 90, 6); }
  else {myGLCD.setTextColor(TFT_VIOLET,TFT_BLACK); myGLCD.drawNumber(mq2, 220, 90, 6); }
  mq2_old = mq2;
    temps = millis() ;}       //  delay (1000*60);
}                               
// --------------- Fin de la loop -----------------

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {                                  // -------- réception données extérieur --------------
  memcpy(&myData, incomingData, sizeof(myData));
  temp_ext = myData.c;
   if (temp_ext > t_max) {t_max = temp_ext;} else if(temp_ext < t_min and t_min > -30 and temp_ext > -50) {t_min = temp_ext;}    // -------- calcul mini et maxi température extérieur ---------------
  myGLCD.setTextColor(TFT_RED,TFT_BLACK);
  //if (temp_ext < 10)  { myGLCD.drawFloat(temp_ext, 1, 220, 90, 6); } else { myGLCD.drawFloat(temp_ext, 1, 210, 90, 6); }  // affiche température extérieur
  //myGLCD.setTextColor(TFT_WHITE,TFT_BLACK); myGLCD.drawFloat(t_max, 1, 150, 80, 5); myGLCD.drawFloat(t_min, 1, 150, 110, 5);  //affiche mini maxi

  myGLCD.setTextColor(TFT_ORANGE,TFT_BLACK); myGLCD.drawFloat(temp_ext, 1, 130, 330, 8);
  myGLCD.setTextColor(TFT_RED,TFT_BLACK); myGLCD.drawFloat(t_max, 1, 10, 320, 6); myGLCD.setTextColor(TFT_DARKCYAN,TFT_BLACK); myGLCD.drawFloat(t_min, 1, 10, 390, 6);  //affiche mini maxi

}
