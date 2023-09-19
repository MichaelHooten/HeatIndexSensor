/***************************************************************************
  Heat Index SensorID

	Uses the following devices:
	Adafruit Feather M4 Express, Product ID: 3857
	Adafruit FeatherWing OLED - 128x32 OLED, Product ID: 2900
	Adafruit BME280 I2C or SPI Temperature Humidity Pressure Sensor, Product ID: 2652

 ***************************************************************************/

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIRE Wire

#define VBATPIN A6

Adafruit_BME280 bme; // I2C

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &WIRE);

unsigned long delayTime;
float airTemp = 0.0;
float relHumid = 0.0;
float heatIndex = 0.0;
float measuredvbat = 0.0;

void setup() {
  Serial.begin(9600);
  delay(delayTime);

  // Serial messages for debug
  Serial.println(F("Heat index sensor"));

  // OLED initialization
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  // Show splash screen
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  // set display defaults
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Serial messages for debug
  Serial.println(F("OLED begun"));

  // BME280 initialization
  unsigned status;

  // default settings
  status = bme.begin();

  // Serial messages for debug
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();

}


void loop() {
  readSensor();
  heatIndexCalc();

  // Serial messages
  Serial.print("Temperature = ");
  Serial.print(airTemp);
  Serial.println(" °F");

  Serial.print("Humidity = ");
  Serial.print(relHumid);
  Serial.println(" %");

  Serial.print("Heat Index = ");
  Serial.print(heatIndex);
  Serial.println(" °F");

  Serial.print("Battery: ");
  Serial.print(measuredvbat);
  Serial.println(" V");

  Serial.println();

  // OLED messages
  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("Temp: ");
  display.print(airTemp);
  display.println(" F");

  display.print("RH: ");
  display.print(relHumid);
  display.println(" %");

  display.print("Heat Index: ");
  display.print(heatIndex);
  display.println(" F");

  display.print("Battery: ");
  display.print(measuredvbat);
  display.println(" V");

  display.display();


  delay(delayTime);
}


void readSensor() {
  // Read the temperature, convert from C to F
  airTemp = bme.readTemperature();
  airTemp = airTemp * 9 / 5 + 32;

  // read the relative humidity
  relHumid = bme.readHumidity();

  // battery measurement
  measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage


}

void heatIndexCalc() {
  // These calculations are documented by the National Weather Service
  // https://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml

  float indexAdjustment = 0.0;

  // simple formula
  heatIndex = 0.5 * (airTemp + 61.0 + ((airTemp - 68.0) * 1.2) + (relHumid * 0.094));

  // If the heat index calculated with the simple equation is greater than 80F, use the more complex calculation
  if (heatIndex >= 80.0) {
    heatIndex = -42.379 + 2.04901523 * airTemp + 10.14333127 * relHumid - .22475541 * airTemp * relHumid - .00683783 * sq(airTemp) - .05481717 * sq(relHumid) + .00122874 * sq(airTemp) * relHumid + .00085282 * airTemp * sq(relHumid) - .00000199 * sq(airTemp) * sq(relHumid);
  }

  // There are two possible adjustments that might be made, depending on conditions
  // if the temp is between 80 and 112, and the humidity is less than 13%

  // if the temp is between 80 and 87 and the humidity is greater than 85%

  heatIndex = heatIndex + indexAdjustment;
}
