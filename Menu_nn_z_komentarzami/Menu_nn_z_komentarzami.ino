#include <LiquidCrystal.h> //załączanie biblioteki LCD
#include <Wire.h> //załączanie biblioteki one wire
#include <TimerOne.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //inicjalizacja biblioteki LCD przez powiązanie wymaganych pinów LCD...
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  //...z pinami ARD do których są podłączone

#define BTN_BACK  6 //definiowanie określonych pinów jako przycisków
#define BTN_NEXT  7
#define BTN_PREV  8
#define BTN_OK    9
#define REL_PIN   13

typedef struct  //deklaracja struktury składającej się z elementów poniżej (Każdy element STRUCT_MENUPOS będzie reprezentacją                       pojedynczej opcji w menu)
{
  String label; //nagłówek
  float minVal;   //wartość minimalna
  float maxVal;   //wartość maksymalna
  float currentVal; //wartość chwilowa
  void (*handler)();
  /*void (*handler)(); - wskaźnik do funkcji nie przyjmującej oraz nie zwracającej żadnych parametrów. Ma 3 funkcje:
    1)Jeżeli minVal będzie >= maxVal funkcja ta będzie akcją, którą chcemy wykonać (bez wchodzenia na drugi poziom menu)
    2)Nadpisanie domyślnego zachowania programu (czyli wyświetlania wartości currentVal – będziemy mogli wstawić tam własne stringi)
    3)Wpisanie tam wartości NULL – wtedy program zachowa się domyślnie, czyli wyświetli currentVal bez żadnego formatowania.*/
} STRUCT_MENUPOS;

void rel_timer(void);
int flaga_srak = 0;
int cycle_number=0;
int cycle_number_max=0;

typedef enum
{
  BACK, NEXT, PREV, OK, NONE
} ENUM_BUTTON;
//Typy wyliczeniowe definiowane są przez słowo kluczowe enum. Definicja typu wyliczeniowego zawiera, ograniczoną klamrami, listę słownych wartości (listę wyliczeniową), które mogą zostać przypisane do zmiennych tego typu*/

STRUCT_MENUPOS menu[5]; // tablica zawierająca pozycje menu

int currentMenuPos = 0; //informacja która opcja jest aktualnie wybrana
int menuSize;   //rozmiar menu obliczany automatycznie na podstawie deklaracji
bool isInLowerLevel = false; //informacja czy jesteśmy w drugim poziomie menu
float tempVal;    //tymczasowa wartość dla currentVal ze struktury STRUCT_MENUPOS

float REL_T_OFF = 3000;
float REL_T_ON = 3000;
unsigned long int rel_time;
int pozycja = 0;

void setup()    //funkcja główna
{
  Serial.begin(9600);   //ustawianie datarate w bps
  lcd.begin(16, 2); //inicjalizacja LCD
  lcd.print("ONE FRANKE xD");   //komunikat początkowy
  delay(5000);  //czas wyświetlania komunikatu początkowego



  Timer1.initialize(1000);
  Timer1.attachInterrupt(rel_timer);


  pinMode(BTN_NEXT, INPUT_PULLUP);  //ustawianie trybu działania pinów cyfrowych z wew. rezystorem podciągającym
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(REL_PIN, OUTPUT);

  menu[0] = {"cycle number", 0, 3600, 10, NULL};   //ustawianie pozycji menu, kolejno: label, min, max, cur, handler
  menu[1] = {"Rel t_off [s]", 0.25, 3600, 1.5, NULL};
  menu[2] = {"Rel t_on [s]", 0.25, 3600, 1.5, NULL};
  menu[3] = {"Ulamki", 0, 30, 15, formatUlamki};
  menu[4] = {"PEŁNE ZANURZENIE", 0, 0, 0, actionPortSzeregowy};

  menuSize = sizeof(menu) / sizeof(STRUCT_MENUPOS); //obliczanie rozmiarów menu
}

void loop()     //pętla rysowania programu
{

  drawMenu();
  //Serial.println(REL_T_OFF);
  Serial.println(rel_time);
  Serial.println(flaga_srak);
  Serial.println(pozycja);
  Serial.println(digitalRead(13));
  Serial.println(cycle_number);

  switch (pozycja) {          //w zależności od odczytanego przycisku wybieramy konkretny przypadek
    case 0:
      {
        digitalWrite(13, HIGH);
        if (rel_time >= REL_T_OFF && cycle_number <= cycle_number_max)
        {
          rel_time = 0;
          pozycja = 1;
          cycle_number++;
        }
       
        break;
      }
    case 1:
      {
        digitalWrite(13, LOW);
        if (rel_time >= REL_T_ON)
        {
          rel_time = 0;
          pozycja = 0;

        }
        break;
      }
  }
  if (cycle_number==(cycle_number_max +1))
  {
            cycle_number=0;
          flaga_srak=0;
          rel_time=0;
          pozycja=0;
  }
        
}

ENUM_BUTTON getButton() {   //funkcja getbutton do odczytywania przycisków
  if (!digitalRead(BTN_BACK)) return BACK;
  if (!digitalRead(BTN_NEXT)) return NEXT;
  if (!digitalRead(BTN_PREV)) return PREV;
  if (!digitalRead(BTN_OK)) return OK;

  return NONE;
}

void drawMenu() {
  static unsigned long lastRead = 0;
  static ENUM_BUTTON lastPressedButton = OK;
  static unsigned int isPressedSince = 0;
  int autoSwitchTime = 500;

  ENUM_BUTTON pressedButton = getButton(); //odczyt wartości z funkcji getbutton

  if (pressedButton == NONE && lastRead != 0) {
    isPressedSince = 0;
    return;
  }
  if (pressedButton != lastPressedButton) {
    isPressedSince = 0;
  }

  if (isPressedSince > 3) autoSwitchTime = 10;
  if (lastRead != 0 && millis() - lastRead < autoSwitchTime && pressedButton == lastPressedButton) return;

  isPressedSince++;
  lastRead = millis();
  lastPressedButton = pressedButton;

  switch (pressedButton) {          //w zależności od odczytanego przycisku wybieramy konkretny przypadek
    case NEXT: handleNext(); break;
    case PREV: handlePrev(); break;
    case BACK: handleBack(); break;
    case OK: handleOk(); break;
  }

  lcd.home();   //kursor na początek
  lcd.clear();  //czyszczenie LCD
  if (isInLowerLevel)   //sprawdzanie czy jesteśmy na 2 poziomie
  {
    lcd.print(menu[currentMenuPos].label);  //drukowanie w nagłówku etykiety opcji w której jesteśmy
    lcd.setCursor(0, 1);    //przesuwanie kursora do 2giej linijki
    lcd.print((">"));     //drukowanie 'znaku zachęty'
    if (menu[currentMenuPos].handler != NULL)   //sprawdzanie czy jest coś w handlerze
    {
      (*(menu[currentMenuPos].handler))();    //jeżeli coś jest w handlerze, to wywołujemy jego funkcję która obsłuży wyświetlanie
    } else lcd.print(tempVal); //jeżeli nie ma nic w handlerze, drukujemy wartość obecną
  } else {
    lcd.print(cycle_number);    //jeżeli nie jesteśmy na 2 poziomie drukujemy 'Menu glowne'...
    lcd.print("/");
    lcd.print(cycle_number_max);
    lcd.setCursor(0, 1);            //...ustawiamy kursor w 2giej linii...
    lcd.print((">"));             //...drukujemy znak zachęty
    lcd.print(menu[currentMenuPos].label);  //...i obecną etykietę menu
  }
}

void handleNext() { //obsluga przycisku w prawo
  if (isInLowerLevel) { //jeżeli jesteśmy w 2gim poziomie, zwiększamy wartość do maxa, jak w 1 to przesuwamy pozycje menu w kółko
    if(currentMenuPos == 0) tempVal++;
    else tempVal = tempVal + 0.1;
    if (tempVal > menu[currentMenuPos].maxVal) tempVal = menu[currentMenuPos].maxVal;
  } else {
    currentMenuPos = (currentMenuPos + 1) % menuSize;
  }
}

void handlePrev() { //obsluga przycisku w lewo
  if (isInLowerLevel) { //analogicznie jak przycisk w prawo
    if(currentMenuPos == 0) tempVal--;
    else tempVal = tempVal - 0.1;
    if (tempVal < menu[currentMenuPos].minVal) tempVal = menu[currentMenuPos].minVal;
  } else {
    currentMenuPos--;
    if (currentMenuPos < 0) currentMenuPos = menuSize - 1;
  }
}

void handleBack() { //obsługa przycisku wstecz
  if (isInLowerLevel) { //jeżeli jesteśmy w 2 poziomie to z niego wychodzimy
    isInLowerLevel = false;
  }
}
//*Zobaczmy co kryje pod sobą funkcja obsługująca przycisk OK. Jeżeli pole handler nie jest NULLem oraz maxVal jest mniejszy bądź równy minVal to wykonujemy funkcję zdefiniowaną jako handler i wychodzimy z handleOk(). W przeciwnym wypadku jeżeli jesteśmy na niższym poziomie to zapisujemy tempVal do pola currentVal i przechodzimy poziom wyżej. Jeżeli jesteśmy w menu głównym to robimy dokładnie odwrotnie: zapisujemy currentVal do tempVal oraz wchodizmy na niższy poziom menu.*//
void handleOk() {
  if (menu[currentMenuPos].handler != NULL && menu[currentMenuPos].maxVal <= menu[currentMenuPos].minVal) {
    (*(menu[currentMenuPos].handler))();
    return;
  }
  if (isInLowerLevel) {
    menu[currentMenuPos].currentVal = tempVal;
    if (currentMenuPos == 0)cycle_number_max = tempVal;
    if (currentMenuPos == 1)REL_T_OFF = tempVal * 1000;
    if (currentMenuPos == 2)REL_T_ON = tempVal * 1000;
    isInLowerLevel = false;
  } else {
    tempVal = menu[currentMenuPos].currentVal;
    isInLowerLevel = true;
  }
}

/* Funkcje-uchwyty użytkownika */
void actionPortSzeregowy() {
  Serial.println("Wywolano akcje: Port szeregowy");
  lcd.print("Wywolano akcje: Port szeregowy");
  flaga_srak = !flaga_srak;
}

void formatNapisy() {
  String dictonary[] = {"Napis 1", "Napis 2", "Napis 3 :)"};
  //lcd.print(dictonary[tempVal]);
}

void formatUlamki() {
  lcd.print(tempVal / 10.0);
}
void rel_timer()
{
  if (flaga_srak == 1) rel_time++;
}
