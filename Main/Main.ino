#include "HX711.h"
#include <Preferences.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define LOADCELL_DOUT_PIN 5
#define LOADCELL_SCK_PIN 18
#define weight_of_object_for_calibration 500

const char *ssid = "Void";
const char *password = "Armin_11";

long sensor_Reading_Results;
float CALIBRATION_FACTOR;
bool show_Weighing_Results = true;
int weight_In_g;
float weight_In_kg;
int LOAD_CALIBRATION_FACTOR = 0;

HX711 LOADCELL_HX711;

Preferences preferences;

LiquidCrystal_I2C lcd(0x27, 16, 2);

String getObjectTitle() {
  Serial.println("\nEnter the title of the object to be weighed:");
  while (Serial.available() == 0) {
    delay(100);
  }
  String title = Serial.readString();
  title.trim();
  Serial.println("Object Title: " + title);
  return title;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(2000);

  Serial.println("Setup...");
  delay(1000);

  preferences.begin("CF", false);
  delay(100);

  Serial.println();
  Serial.println("Do not place any object or weight on the scale.");
  delay(1000);

  Serial.println();
  Serial.println("LOADCELL_HX711 begin.");
  LOADCELL_HX711.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  LOAD_CALIBRATION_FACTOR = preferences.getFloat("CFVal", 0);
  delay(500);
  LOADCELL_HX711.set_scale();
  LOADCELL_HX711.tare();
  LOADCELL_HX711.set_scale(LOAD_CALIBRATION_FACTOR);
  delay(500);

  Serial.println();
  Serial.print("CALIBRATION_FACTOR : ");
  Serial.println(LOAD_CALIBRATION_FACTOR);
  delay(2000);
  Serial.println();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  Serial.println("Setup finish.");
  delay(1000);
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Put on Scale And");
  lcd.setCursor(0, 1);
  lcd.print("Enter its Title");

  if (show_Weighing_Results) {
    String objectTitle = getObjectTitle();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(objectTitle);
    lcd.setCursor(0, 1);
    lcd.print("Weight: ");

    if (LOADCELL_HX711.is_ready()) {
      weight_In_g = LOADCELL_HX711.get_units(10);

      if (weight_In_g >= 1000) {
        weight_In_kg = weight_In_g / 1000.0;

        lcd.print(weight_In_kg, 3);
        lcd.print(" Kg");
      } else {
        lcd.print(weight_In_g);
        lcd.print(" g");
      }

      String sendingWeight = String(weight_In_g >= 1000 ? weight_In_kg : weight_In_g) + (weight_In_g >= 1000 ? " Kg" : " g");

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        String apiUrl = "https://api.apispreadsheets.com/data/b9UECbmzcyMRssfJ/";

        String jsonPayload = "{";
        jsonPayload += "\"data\": {";
        jsonPayload += "\"title\": \"" + objectTitle + "\",";
        jsonPayload += "\"weight\": \"" + sendingWeight + "\"";
        jsonPayload += "}}";

        Serial.println("JSON Payload:");
        Serial.println(jsonPayload);

        http.begin(apiUrl);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response code: " + String(httpResponseCode));
          Serial.println("Response: " + response);
        } else {
          Serial.println("Error in sending POST: " + String(httpResponseCode));
        }

        http.end();
      } else {
        Serial.println("WiFi Disconnected");
      }
    } else {
      Serial.println("HX711 not found.");
    }
  }

  delay(1000);
}
