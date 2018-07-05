#include <LiquidCrystal.h>  // biblioteka LCD
#include <OneWire.h> // biblioteka 1W do kominikacji z czujnikiem temp.
#include <DS18B20.h>  //biblioteka czujnika temp.


const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //inicjalizacja biblioteki LCD przez powiązanie wymaganych pinów LCD...
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  //...z pinami ARD do których są podłączone


byte address[8] = {0x28, 0xFF, 0xA5, 0x38, 0xA3, 0x15, 0x4, 0x9D}; //adres czujnika temperatury

OneWire onewire(10);  //numer pinu, do którego podłączona jest magistrala 1W
DS18B20 sensors(&onewire);

void setup()
  {
    while(!Serial);
    Serial.begin(9600);
    sensors.begin();
    sensors.request(address);
    lcd.begin(16, 2); //inicjalizacja LCD - liczba znaków, wierszy
  }

void loop()
  {

        float temperature = sensors.readTemperature(address);
    
        Serial.print(temperature,1);
        Serial.println("°C");
        sensors.request(address);
        
        lcd.print(temperature,1);
        lcd.print((char)223);
        lcd.print("C");
        
        delay(2000);
        
        lcd.clear(); //czyszczenie ekranu 
      
  }
