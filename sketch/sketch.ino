#include <EEPROM.h>
#include <GravityTDS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define ACID_PIN A1    // pH sensor pin
#define TURB_PIN A2    // Turbidity sensor pin
#define TDS_PIN A3     // TDS sensor pin

#define VREF 5.0
#define ADC_RES 1024

float acid_val = 0;
float turb_val = 0;
float tds_val = 0;  // TDS value in PPM

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 or 0x3F for a 16 chars and 2-line display
GravityTDS tds;

void setup() {
    pinMode(TURB_PIN, INPUT);
    pinMode(TDS_PIN, INPUT);
    pinMode(ACID_PIN, INPUT);

    lcd.init();                // Initialize the LCD
    lcd.backlight();          // Turn on the backlight

    tds.setPin(TDS_PIN);      // Set TDS sensor pin
    tds.setAref(VREF);        // Set analog reference voltage
    tds.setAdcRange(ADC_RES); // Set ADC resolution for Arduino Uno (10-bit ADC)
    tds.begin();              // Initialize TDS sensor
}

float readPH() {
    int buffer_arr[10];
    unsigned long avgval = 0;

    // Read analog values into the buffer
    for (int i = 0; i < 10; i++) {
        buffer_arr[i] = analogRead(ACID_PIN);
        delay(30); // Delay for stabilization
    }

    // Sort buffer array
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (buffer_arr[i] > buffer_arr[j]) {
                int temp = buffer_arr[i];
                buffer_arr[i] = buffer_arr[j];
                buffer_arr[j] = temp;
            }
        }
    }

    // Calculate average of middle values
    for (int i = 2; i < 8; i++) {
        avgval += buffer_arr[i];
    }

    float volt = (float)avgval * VREF / ADC_RES / 6; // Convert to voltage
    return volt; 
}

void loop() {
    // Update TDS data
    tds.update();
    tds_val = tds.getTdsValue(); // Get TDS value in PPM

    // Read turbidity
    turb_val = analogRead(TURB_PIN);
    turb_val *= (VREF / ADC_RES); // Convert to voltage
    
    // Read pH value using the separate function
    acid_val = readPH();

    // Clear LCD and display readings
    lcd.clear();
    lcd.setCursor(0, 0);

    // Display Turbidity and pH on the first line
    lcd.print("TB ");
    lcd.print(turb_val, 2); // Display turbidity voltage
    lcd.print(" PH ");
    lcd.print(acid_val, 2);   // Display pH value

    // Move to the second line and display TDS and temperature
    lcd.setCursor(0, 1);
    lcd.print("TD ");
    lcd.print(tds_val); // Display TDS value in PPM

    delay(1000); // Delay for stability before the next reading
}
