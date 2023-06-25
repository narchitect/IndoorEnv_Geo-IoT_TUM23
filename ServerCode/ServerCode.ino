// Seeeduino LoRaWAN ------------------------------------------------------------
#define PIN_GROVE_POWER 38
#define SerialUSB Serial
 
// LoRaWAN -----------------------------------------------------------------------
#include <LoRaWan.h>

//Ligth library -----------
#include <Wire.h>
#include <Digital_Light_TSL2561.h>

//Dust Library ----------
#include <Seeed_HM330X.h>

//SCD30 Library ---------
#include <SCD30.h>

//BME680 Library ---------
#include "seeed_bme680.h"
#define IIC_ADDR  uint8_t(0x76)
Seeed_BME680 bme680(IIC_ADDR);

// Put your LoRa keys here
#define DevEUI "004F85D928E85BEC"
#define AppEUI "70B3D57ED002F952"
#define AppKey "90383AB0225F58102FBAE9B23F66B263"
 
// CayenneLPP --------------------------------------------------------------------
#include <CayenneLPP.h>  // Include Cayenne Library
CayenneLPP lpp(51);      // Define the buffer size: Keep as small as possible -> how big the data package is 
// the numebr 51 means the number of bits 
// the bits numbers should be minimum, so roughly calculate the expected result.
 
// SETUP -------------------------------------------------------------------------
// vars
char buffer[256];

//dust
HM330X sensor;
uint8_t buf[30];

void setup(void)
{
  // Setup Serial connection
  delay(5000);
  Serial.begin(115200);
 
  // Powerup Seeeduino LoRaWAN Grove connectors
  pinMode(PIN_GROVE_POWER, OUTPUT);
  digitalWrite(PIN_GROVE_POWER, 1);

  // light sensor configuration
  Wire.begin();
  Serial.begin(9600);
  TSL2561.init();

  // SCD30 sensor config
  Wire.begin();
    scd30.initialize();

  //bme680 sensor config
  bme680.init();

  // Config LoRaWAN
  lora.init();
 
  memset(buffer, 0, 256);
  lora.getVersion(buffer, 256, 1);
  if (Serial) {
    Serial.print(buffer);
  }
 
  memset(buffer, 0, 256);
  lora.getId(buffer, 256, 1);
  if (Serial) {
    Serial.print(buffer);
  }
 
  // void setId(char *DevAddr, char *DevEUI, char *AppEUI);
  // replace the xxxxxx and the yyyyyy below with the DevEUI and the
  // AppEUI obtained from your registered sensor node and application
  // in The Things Network (TTN). The numbers are hexadecimal strings
  // without any leading prefix like "0x" and must have exactly the
  // same number of characters as given below.
  // lora.setId(NULL, "xxxxxxxxxxxxxxxx", "yyyyyyyyyyyyyyyy");
  lora.setId(NULL, DevEUI, AppEUI);
 
  // setKey(char *NwkSKey, char *AppSKey, char *AppKey);
  // replace the zzzzzz below with the AppKey obtained from your registered
  // application in The Things Network (TTN). The numbers are hexadecimal
  // strings without any leading prefix like "0x" and must have exactly
  // the same number of characters as given below.
  // lora.setKey(NULL, NULL, "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz");
  lora.setKey(NULL, NULL, AppKey);
 
  lora.setDeciveMode(LWOTAA);
  lora.setDataRate(DR0, EU868);      // DR5 = SF7, DR0 = SF 12
  lora.setAdaptiveDataRate(true);
 
  lora.setChannel(0, 868.1);
  lora.setChannel(1, 868.3);
  lora.setChannel(2, 868.5);
  lora.setChannel(3, 867.1);
  lora.setChannel(4, 867.3);
  lora.setChannel(5, 867.5);
  lora.setChannel(6, 867.7);
  lora.setChannel(7, 867.9);
 
  lora.setDutyCycle(false);
  lora.setJoinDutyCycle(false);
 
  lora.setPower(14);
  lora.setPort(33);
 
  unsigned int nretries;
  nretries = 0;
  while (!lora.setOTAAJoin(JOIN, 20)) {
    nretries++;
    if (Serial) {
      Serial.println((String)"Join failed, retry: " + nretries);
    }
  }
  Serial.println("Join successful!");
}
 
// LOOP --------------------------------------------------------------------
unsigned int nloops = 0;
void loop(void) {
  nloops++;
  if (Serial) {
    Serial.println((String)"Loop " + nloops + "...");
  }
 
  bool result = false;
 
 /*--------- customized part ---------*/
  // Reset Cayenne buffer and add new data
  lpp.reset();                             
  
  //add light observations
  lpp.addLuminosity(1, TSL2561.readVisibleLux());     // luminionity

  //add dust observation
  sensor.read_sensor_value(buf, 29);      
  lpp.addAnalogOutput(2,buf[0]);                      // dust

  //add SCD30 observations 
  float result_SCD30[3] = {0};
  scd30.getCarbonDioxideConcentration(result_SCD30);
  lpp.addTemperature(3, result_SCD30[1]);             // temperature
  lpp.addRelativeHumidity(4, result_SCD30[2]);        // humidity
  lpp.addAnalogOutput (5, result_SCD30[0]);           // Co2 
  
  //add BME68 obeservation
  bme680.read_sensor_data();
  lpp.addAnalogOutput(6, (bme680.sensor_result_value.gas/1000.0)); // VOC gas
  /*--------- customized part ---------*/

  // Transfer LoRa package
  result = lora.transferPacket(lpp.getBuffer(), lpp.getSize(), 5);                  // sends the Cayenne encoded data packet (n bytes) with a default timeout of 5 secs
  // result = lora.transferPacketWithConfirmed(lpp.getBuffer(), lpp.getSize(), 5);  // sends the Cayenne encoded data packet (n bytes) with a default timeout of 5 secs, using confirmed LoRa package
 
  if (result) {
    short length;
    short rssi;
 
    // Receive LoRaWAN package (LoraWAN Class A)
    char rx[256];
    length = lora.receivePacket(rx, 256, &rssi);
 
    // Check, if a package was received
    if (length)
    {
      if (Serial) {
        Serial.print("Length is: ");
        Serial.println(length);
        Serial.print("RSSI is: ");
        Serial.println(rssi);
        Serial.print("Data is: ");
 
        // Print received data as HEX
        for (unsigned char i = 0; i < length; i ++)
        {
          Serial.print("0x");
          Serial.print(rx[i], HEX);
          Serial.print(" ");
        }
 
        // Convert received package to int
        int rx_data_asInteger = atoi(rx);
 
        Serial.println();
        Serial.println("Received data: " + String(rx_data_asInteger));
      }
    }
  }
   
  if (Serial) {
    Serial.println((String)"Loop " + nloops + "...done!\n");
  }
  delay(30000);
 
}