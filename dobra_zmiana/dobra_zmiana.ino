#include <OneWire.h> //zalaczanie bibliotek 1W
#include <DS18B20.h> //zalaczanie bibliotek termometru
#include <LiquidCrystal.h> //zalaczanie bibliotek LCD


byte address[8] = {0x28, 0xFF, 0xA5, 0x38, 0xA3, 0x15, 0x4, 0x9D}; //adres czujnika temp.

OneWire onewire(10); //deklaracja obiektu onewire odpowiedzialnego za komunikacje 1W (argument - pin)
DS18B20 sensors(&onewire); //deklaracja obiektu sensors odpowiedzialnego za obsługe DS18B20 (argument - wskaznik do obiektu magistrali 1W do ktorej podlaczony jest termometr)

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //inicjalizacja biblioteki LCD przez polaczenie odpowiednich pinow LCD z pinami ARD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  while(!Serial);
  Serial.begin(9600);

  sensors.begin(); //wyszukiwanie czujnikow i ustawianie ich rozdzielczosci, domyslnie = 12
  sensors.request(address); //rozkaz obliczania temp. dla czujnika o adresie podanym w argumencie
  
  lcd.begin(16, 2); //ustawienie liczby kolumn i rzedow LCD
}

void loop() {
  //if (sensors.available()) //sprawdza czy czujnik juz obliczyl temp. -> jak nie program leci dalej -> jak jest temp to ja odczytujemy
  {
    float temperature = sensors.readTemperature(address); //odczyt temp. z czujnika o podanym adresie
    lcd.setCursor(0, 0); //ustawianie kursora LCD na poczatek

    Serial.print(temperature,1); //drukowanie do monitora temperatury, powinno byc robione dopiero gdy available da wartosc 1; zaokraglenie do 1 miejsca
    Serial.println("°C");

    lcd.print(temperature,1);
    lcd.print((char)223);
    lcd.print("C");
    delay(1000);
    lcd.clear(); //czyszczenie ekranu 
    
    sensors.request(address); //rozkaz obliczania temp. dla czujnika o adresie podanym w argumencie
  }

  // tu umieść resztę twojego programu
  // Będzie działał bez blokowania
}
