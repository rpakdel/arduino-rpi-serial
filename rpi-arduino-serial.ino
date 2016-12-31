#include <SoftwareSerial.h>

SoftwareSerial rpiSerial(3, 2); // RX, TX

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <RF24_config.h>
#include <nRF24L01.h>
#include <SPI.h>
#include <RF24.h>

#include "pta.h"

#define I2C_ADDR    0x27
LiquidCrystal_I2C  lcd(I2C_ADDR, 8, 2);

#define DHTPIN 4 // Pin which is connected to the DHT sensor.
#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);

RF24 radio(9, 10);
static const int receiverAdd[2] = { 0xF0F0F0F0AA, 0xF0F0F0F066 };

unsigned long dhtLastRead = 0;
void readDHT()
{
    // read every 5 seconds
    unsigned long m = millis();
    
    if (m - dhtLastRead >= 10000)
    {
        dhtLastRead = m;
        sensors_event_t event;

        dht.temperature().getEvent(&event);
        float temp = event.temperature;
        bool isTempValid = false;
        char tempStr[5 + 1];
        if (!isnan(temp)) 
        {
            isTempValid = true;
            lcd.setCursor(10, 0);
            lcd.print(temp);
            lcd.print("C");
            lcd.home();

            dtostrf(temp, 5, 2, tempStr);
        }
        else
        {
            strcpy(tempStr, "null");
        }

        dht.humidity().getEvent(&event);
        float humid = event.relative_humidity;
        bool isHumidValid = false;
        char humidStr[5 + 1];
        if (!isnan(event.relative_humidity))
        {
            isHumidValid = true;
            lcd.setCursor(10, 1);
            lcd.print(event.relative_humidity);
            lcd.print("%");
            lcd.home();

            dtostrf(humid, 5, 2, humidStr);
        }
        else
        {
            strcpy(humidStr, "null");
        }
       
        char buffer[64];
        sprintf(buffer, "{ \"stationid\": 0, \"temp\": %s, \"relhumidity\": %s }\0", tempStr, humidStr);
        Serial.println(buffer);
        rpiSerial.println(buffer);
        
    }
}

void readPTA()
{
    uint8_t pipeNum = 0;
    if (radio.available(&pipeNum))
    {
        PTA pta;
        radio.read(&pta, sizeof(PTA));
        //PrintPTA(pta, Serial);

        char pressureStr[7 + 1];
        dtostrf(pta.pressure, 7, 2, pressureStr);

        char tempStr[5 + 1];
        dtostrf(pta.temperature, 5, 2, tempStr);

        char altStr[5 + 1];
        dtostrf(pta.altitude, 5, 2, altStr);

        char buffer[128];
        sprintf(buffer, 
            "{ \"stationid\": 1, \"pressure\": %s, \"temp\": %s, \"altitude\": %s }\0", 
            pressureStr,
            tempStr, 
            altStr);
        Serial.println(buffer);
        rpiSerial.println(buffer);
    }
}

void setup() 
{
  dht.begin();

  lcd.init();
  lcd.blink();
  lcd.backlight();

  radio.begin();
  radio.setAutoAck(true);
  radio.setPALevel(RF24_PA_LOW);
  radio.setRetries(15, 15);
  radio.openReadingPipe(1, receiverAdd[0]);
  radio.openReadingPipe(2, receiverAdd[1]);

  // Start the radio listening for data
  radio.startListening();


  Serial.begin(9600);
  rpiSerial.begin(9600);

  Serial.println(F("Ready"));
  lcd.home();
  lcd.print("Arduino ready");
  delay(1000);
  lcd.home();
  lcd.clear();
}

int row = 0;
void loop() 
{
    bool lcdclear = true;
    while (rpiSerial.available())
    {
        char c = rpiSerial.read();
        if ((int)c == 13 || (int)c == 10) // new line
        {
            row++;
            if (row > 1)
            {
                row = 0;
                lcd.clear();
                lcd.home();
            }
            lcd.setCursor(0, row);
        }
        else
        {
            lcd.print(c);
        }
        Serial.println((int)c);
    }
  
    while (Serial.available()) 
    {
        rpiSerial.write(Serial.read());
    }

    readDHT();

    readPTA();
}
