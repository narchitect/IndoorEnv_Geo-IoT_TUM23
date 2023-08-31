#define PIN_GROVE_POWER 38
#include <Seeed_HM330X.h>
#include "rgb_lcd.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL_OUTPUT SerialUSB
#else
#define SERIAL_OUTPUT Serial
#endif

rgb_lcd lcd;

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

HM330X sensor;
uint8_t buf[30];

const char* str[] = {
  "sensor num: ",
  "PM1.0 concentration(CF=1,Standard particulate matter,unit:ug/m3): ",
  "PM2.5 concentration(CF=1,Standard particulate matter,unit:ug/m3): ",
  "PM10 concentration(CF=1,Standard particulate matter,unit:ug/m3): ",
  "PM1.0 concentration(Atmospheric environment,unit:ug/m3): ",
  "PM2.5 concentration(Atmospheric environment,unit:ug/m3): ",
  "PM10 concentration(Atmospheric environment,unit:ug/m3): ",
};

HM330XErrorCode print_result(const char* str, uint16_t value) {
  if (NULL == str) {
    return ERROR_PARAM;
  }
  SERIAL_OUTPUT.print(str);
  SERIAL_OUTPUT.println(value);
  return NO_ERROR;
}

uint16_t pm25Value;  // Variable to store PM2.5 concentration value

/*parse buf with 29 uint8_t-data*/
HM330XErrorCode parse_result(uint8_t* data) {
  uint16_t value = 0;
  if (NULL == data) {
    return ERROR_PARAM;
  }
  for (int i = 1; i < 8; i++) {
    value = (uint16_t)data[i * 2] << 8 | data[i * 2 + 1];
    print_result(str[i - 1], value);

    if (i == 2) {
      // Store PM2.5 concentration value
      pm25Value = value;
    }
  }

  return NO_ERROR;
}

HM330XErrorCode parse_result_value(uint8_t* data) {
  if (NULL == data) {
    return ERROR_PARAM;
  }
  for (int i = 0; i < 28; i++) {
    SERIAL_OUTPUT.print(data[i], HEX);
    SERIAL_OUTPUT.print("  ");
    if ((0 == (i) % 5) || (0 == i)) {
      SERIAL_OUTPUT.println("");
    }
  }
  uint8_t sum = 0;
  for (int i = 0; i < 28; i++) {
    sum += data[i];
  }
  if (sum != data[28]) {
    SERIAL_OUTPUT.println("wrong checkSum!!");
  }
  SERIAL_OUTPUT.println("");
  return NO_ERROR;
}

void setup() {
  // Power up Seeeduino LoRaWAN Grove connectors
  pinMode(PIN_GROVE_POWER, OUTPUT);
  digitalWrite(PIN_GROVE_POWER, HIGH);
  SERIAL_OUTPUT.begin(115200);
  delay(100);
  SERIAL_OUTPUT.println("Serial start");

  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);

  // Print a message to the LCD.
  lcd.print("PM2.5 =");

  if (sensor.init()) {
    SERIAL_OUTPUT.println("HM330X init failed!!");
    while (1)
      ;
  }
}

void loop() {
  if (sensor.read_sensor_value(buf, 29)) {
    SERIAL_OUTPUT.println("HM330X read result failed!!");
    buf[0] = 3;
  }
  parse_result_value(buf);
  parse_result(buf);
  SERIAL_OUTPUT.println("");
  lcd.setCursor(7, 1);   // Set cursor to the position where the value will be updated
  lcd.print(pm25Value);  // Update the LCD with the new PM2.5 value
  delay(5000);
}
