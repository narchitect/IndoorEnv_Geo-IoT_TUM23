int loudness;
float db;
#define PIN_GROVE_POWER 38
void setup()
{
    // Powerup Seeeduino LoRaWAN Grove connectors
  pinMode(PIN_GROVE_POWER, OUTPUT);
  digitalWrite(PIN_GROVE_POWER, 1);
    Serial.begin(9600);
}

void loop()
{
    loudness = analogRead(0);
    db = 20*log10(loudness);
    Serial.println(db);
    delay(100);
   // Serial.println(loudness);
}
