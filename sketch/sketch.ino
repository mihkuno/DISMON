#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin configuration
#define ACID_PIN A1    // pH sensor pin
#define TURB_PIN A2    // Turbidity sensor pin
#define TDS_PIN A3     // TDS sensor pin

// Constants for ADC and voltage reference
#define VREF 5.0
#define ADC_RES 1023.0

// Initialize LCD instances with known I2C addresses
LiquidCrystal_I2C lcd4(0x27, 16, 2);

void setup() {
  Serial.begin(9600); // Start serial communication for debugging
  Wire.begin(); // Initialize I2C bus
  
  pinMode(TURB_PIN, INPUT);
  pinMode(TDS_PIN, INPUT);
  pinMode(ACID_PIN, INPUT);

  if (initializeLCD(0x27, lcd4)) {
    lcd4.setCursor(0, 0);
    lcd4.print("Device 4 ready");
  }

  delay(1000); // Display readiness message for 1 second
  lcd4.clear();
}


// use the given data to map the analog values to PPM (Parts Per Million) values using a linear relationship.
// TEMP	ANALOG	PPM	  DESC
// 25	  510	    792	  salt
// 25	  5	      0	    none 
// 25	  318	    575	  sewage
float analogToPPM(int analogValue) {
    // Calibration data
    int analogVals[] = {5, 318, 510};  // Analog readings
    int ppmVals[] = {0, 575, 792};    // Corresponding PPM values
    int numPoints = 3;                // Number of calibration points

    // Check if the value is outside the calibration range
    if (analogValue <= analogVals[0]) {
        return ppmVals[0];
    } 
    if (analogValue >= analogVals[numPoints - 1]) {
        return ppmVals[numPoints - 1];
    }

    // Interpolation between two calibration points
    for (int i = 0; i < numPoints - 1; i++) {
        if (analogValue >= analogVals[i] && analogValue <= analogVals[i + 1]) {
            // Linear interpolation
            return ppmVals[i] + 
                   (float)(analogValue - analogVals[i]) * 
                   (ppmVals[i + 1] - ppmVals[i]) / 
                   (analogVals[i + 1] - analogVals[i]);
        }
    }

    // Default return value (shouldn't reach here if data is correct)
    return 0;
}


void loop() {
  // Read TDS data
  int tdsAnalogValue = analogRead(TDS_PIN); // Get raw analog value from TDS sensor
  float tdsVoltage = (tdsAnalogValue / ADC_RES) * VREF; // Convert raw ADC value to voltage
  int ppmValue = analogToPPM(tdsAnalogValue);

  // Read turbidity
  int turbAnalogValue = analogRead(TURB_PIN);
  float turbVoltage = (turbAnalogValue / ADC_RES) * VREF; // Convert to voltage
  turbVoltage = map(turbVoltage, 0, 4.45, 100, 0);

  // Read acidity
  float acidValue = analogToPh(analogRead(ACID_PIN));
  
  lcd4.print(String(acidValue) + " PH");
  lcd4.setCursor(11, 0);
  lcd4.print(String((int)turbVoltage) + " %"); 
  lcd4.setCursor(0, 1);
  lcd4.print(String(ppmValue) + " PPM"); 

  delay(300);
  lcd4.clear();
}

// Function to check and initialize an LCD at a specific I2C address
bool initializeLCD(byte address, LiquidCrystal_I2C &lcd) {
  Wire.beginTransmission(address);
  if (Wire.endTransmission() == 0) { // Device found
    lcd.init();        // Initialize the LCD
    lcd.backlight();   // Turn on the backlight
    Serial.print("LCD found at 0x");
    Serial.println(address, HEX);
    return true;       // Initialization successful
  } else {
    Serial.print("No device found at 0x");
    Serial.println(address, HEX);
    return false;      // No device at this address
  }
}


float analogToPh(int x) {
  // Define the slope (m) and intercept (b)
  float m = -0.0286; // Slope
  float b = 24.163;  // Intercept

  // Apply the formula: y = mx + b
  float y = m * x + b;

  return y;
}




