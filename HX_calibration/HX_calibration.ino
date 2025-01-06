#include "HX711.h"
#include <Preferences.h>

#define LOADCELL_DOUT_PIN 5
#define LOADCELL_SCK_PIN  18
#define weight_of_object_for_calibration 500                                                                                   

long sensor_Reading_Results; 
float CALIBRATION_FACTOR;
bool show_Weighing_Results = false;
int weight_In_g;

HX711 LOADCELL_HX711;

Preferences preferences;

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

  Serial.println();
  Serial.println("Setup finish.");
  delay(1000);

  Serial.println();
  Serial.println("Sequence of commands for the calibration process :");
  Serial.println("1. Send the P character to prepare for the calibration process.");
  Serial.println("2. Send the C character to calibration.");
  Serial.println("The above commands must be sequential from 1 to 2.");
  delay(1000);
  Serial.println();
  Serial.println("Please enter/send the character P to prepare for the calibration process.");
}

void loop() {
  if(Serial.available()) {
    char inChar = (char)Serial.read();
    Serial.println();
    Serial.print("Received characters : ");
    Serial.println(inChar);

    if (inChar == 'P') {
      show_Weighing_Results = false;
      delay(1000);
      if (LOADCELL_HX711.is_ready()) {  
        Serial.println(); 
        Serial.println("Do not place any object or weight on the scale.");
        Serial.println("Please wait...");
        
        for (byte i = 5; i > 0; i--) {
          Serial.println(i);
          delay(1000);
        }
        
        LOADCELL_HX711.set_scale(); 
        Serial.println();
        Serial.println("Set the scales...");
        Serial.println("Please wait...");
        delay(1000);
        
        LOADCELL_HX711.tare();
        Serial.println();
        Serial.println("Please place an object whose weight is known on the scale.");
        Serial.flush();
        
        for (byte i = 5; i > 0; i--) {
          Serial.println(i);
          delay(1000);
        }
        
        Serial.println();
        Serial.println("Please enter/send the character C for calibration.");
      } else {
        Serial.println("HX711 not found.");
      }
    }

    if (inChar == 'C') {
      if (LOADCELL_HX711.is_ready()) {
        Serial.println(); 
        Serial.println("Start calibration...");
        Serial.println("Let the object whose weight is known remain on the scale.");
        Serial.println("Please wait...");
        Serial.println(); 
        
        for (byte i = 0; i < 5; i++) {
          sensor_Reading_Results = LOADCELL_HX711.get_units(10);
          Serial.print("Sensor reading results : ");
          Serial.println(sensor_Reading_Results);
          delay(1000);
        }

        CALIBRATION_FACTOR = sensor_Reading_Results / weight_of_object_for_calibration; 

        Serial.println(); 
        Serial.println("Save calibration values to flash memory.");
        preferences.putFloat("CFVal", CALIBRATION_FACTOR); 
        delay(500);

        Serial.println(); 
        Serial.println("Load calibration values from flash memory.");
        float LOAD_CALIBRATION_FACTOR; 
        LOAD_CALIBRATION_FACTOR = preferences.getFloat("CFVal", 0); 
        delay(500);

        Serial.println(); 
        Serial.println("Set the scale with the CALIBRATION_FACTOR value.");
        LOADCELL_HX711.set_scale(LOAD_CALIBRATION_FACTOR);
        delay(500);
        
        Serial.println(); 
        Serial.print("CALIBRATION_FACTOR : ");
        Serial.println(LOAD_CALIBRATION_FACTOR);
        delay(2000);
        
        show_Weighing_Results = true;

        Serial.println();
        Serial.println("Calibration complete.");
        Serial.println();
        Serial.println("If you want to recalibrate, please enter/send the character P.");
        Serial.println();
        Serial.println("The scales are ready to use.");
        Serial.println();
        delay(2000);
      } else {
        Serial.println("HX711 not found.");
      }
    }
  }

  if (show_Weighing_Results == true) {
    if (LOADCELL_HX711.is_ready()) {
      weight_In_g = LOADCELL_HX711.get_units(10); 
      
      Serial.print("Weight : ");
      Serial.print(weight_In_g);
      Serial.println(" g");
    } else {
      Serial.println("HX711 not found.");
    }
  }

  delay(1000);
}
