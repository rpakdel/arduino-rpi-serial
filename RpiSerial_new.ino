/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 2); // RX, TX

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define I2C_ADDR    0x27
LiquidCrystal_I2C  lcd(I2C_ADDR, 8, 2);

#define DHTPIN 4 // Pin which is connected to the DHT sensor.
#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);

unsigned long dhtLastRead = 0;
void readDHT()
{
    // read every 5 seconds
    unsigned long m = millis();
    
    if (m - dhtLastRead >= 5000)
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
       
        char buffer[32];
        sprintf(buffer, "{ \"temp\": %s, \"relhumidity\": %s }\0", tempStr, humidStr);
        mySerial.println(buffer);
        
    }
}

void setup() 
{
  dht.begin();

  lcd.init();
  lcd.blink();
  lcd.backlight();
  Serial.begin(9600);
  
  mySerial.begin(9600);
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
    while (mySerial.available())
    {
        char c = mySerial.read();
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
        mySerial.write(Serial.read());
    }

    readDHT();
}
