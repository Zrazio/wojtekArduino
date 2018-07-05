#include <LiquidCrystal.h>

#include <Wire.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; //inicjalizacja biblioteki LCD przez powiązanie wymaganych pinów LCD...
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  //...z pinami ARD do których są podłączone

#define BTN_BACK  6
#define BTN_NEXT  7
#define BTN_PREV  8
#define BTN_OK    9

typedef struct {
  String label;
  int minVal;
  int maxVal;
  int currentVal;
  void (*handler)();
} STRUCT_MENUPOS;

typedef enum {
  BACK, NEXT, PREV, OK, NONE
} ENUM_BUTTON;

STRUCT_MENUPOS menu[5];

int currentMenuPos = 0;
int menuSize;
bool isInLowerLevel = false;
int tempVal;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("ONE FRANKE xD");
  delay(5000);
  

  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);

  menu[0] = {"Cyfry", 0, 9, 5, NULL};
  menu[1] = {"Liczby", 10, 1000, 15, NULL};
  menu[2] = {"Napisy", 0, 2, 0, formatNapisy};
  menu[3] = {"Ulamki", 0, 30, 15, formatUlamki};
  menu[4] = {"Port szer.", 0, 0, 0, actionPortSzeregowy};

  menuSize = sizeof(menu)/sizeof(STRUCT_MENUPOS);
}

void loop() {
  drawMenu();
}

ENUM_BUTTON getButton() {
  if(!digitalRead(BTN_BACK)) return BACK;
  if(!digitalRead(BTN_NEXT)) return NEXT;
  if(!digitalRead(BTN_PREV)) return PREV;
  if(!digitalRead(BTN_OK)) return OK;

  return NONE;
}

void drawMenu() {
  static unsigned long lastRead = 0;
  static ENUM_BUTTON lastPressedButton = OK;
  static unsigned int isPressedSince = 0;
  int autoSwitchTime = 500;

  ENUM_BUTTON pressedButton = getButton();

  if(pressedButton == NONE && lastRead != 0) {
    isPressedSince = 0;
    return;
  }
  if(pressedButton != lastPressedButton) {
    isPressedSince = 0;
  }

  if(isPressedSince > 3) autoSwitchTime = 70;
  if(lastRead != 0 && millis() - lastRead < autoSwitchTime && pressedButton == lastPressedButton) return;

  isPressedSince++;
  lastRead = millis();
  lastPressedButton = pressedButton;
  
  switch(pressedButton) {
    case NEXT: handleNext(); break;
    case PREV: handlePrev(); break;
    case BACK: handleBack(); break;
    case OK: handleOk(); break;
  }

  lcd.home();
  lcd.clear();
  if(isInLowerLevel) {
    lcd.print(menu[currentMenuPos].label);
    lcd.setCursor(0, 1);
    lcd.print(F("> "));

    if(menu[currentMenuPos].handler != NULL) {
      (*(menu[currentMenuPos].handler))();
    } else {
      lcd.print(tempVal);
    }
  } else {
    lcd.print(F("Menu glowne"));
    lcd.setCursor(0, 1);
    lcd.print(F("> "));

    lcd.print(menu[currentMenuPos].label);
  }
}

void handleNext() {
  if(isInLowerLevel) {
    tempVal++;
    if(tempVal > menu[currentMenuPos].maxVal) tempVal = menu[currentMenuPos].maxVal;
  } else {
    currentMenuPos = (currentMenuPos + 1) % menuSize;
  }
}

void handlePrev() {
  if(isInLowerLevel) {
    tempVal--;
    if(tempVal < menu[currentMenuPos].minVal) tempVal = menu[currentMenuPos].minVal;
  } else {
    currentMenuPos--;
    if(currentMenuPos < 0) currentMenuPos = menuSize - 1;
  }
}

void handleBack() {
  if(isInLowerLevel) {
    isInLowerLevel = false;
  }
}

void handleOk() {
  if(menu[currentMenuPos].handler != NULL && menu[currentMenuPos].maxVal <= menu[currentMenuPos].minVal) {
    (*(menu[currentMenuPos].handler))();
    return;
  }
  if(isInLowerLevel) {
    menu[currentMenuPos].currentVal = tempVal;
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
}

void formatNapisy() {
  String dictonary[] = {"Napis 1", "Napis 2", "Napis 3 :)"};
  lcd.print(dictonary[tempVal]);
}

void formatUlamki() {
  lcd.print(tempVal / 10.0);
}
