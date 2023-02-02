/*
 * 
 * Meine Wetterstation 
 * 
 * 
 * 
 * Hier noch ein paar hilfreiche Links zu den Bauteilen:
 * 
 * Verbinde das LCD Display mit dem Arduino
 * https://www.arduino.cc/en/Tutorial/HelloWorld
 * 
 * DHT11 für Arduino
 * http://playground.arduino.cc/Main/DHT11Lib
 * 
 * DS3231 Modul 
 * http://misclab.umeoce.maine.edu/boss/Arduino/bensguides/DS3231_Arduino_Clock_Instructions.pdf
 */


//************ Bibliotheken ************//

#include "dht11.h"                                              // DHT11 Temperatur & Feuchtigkeits Sensor

#include <LiquidCrystal.h>                                      // LCD Display  

#include "DS3231.h"                                             // DS3231 Echtzeituhr Modul

#include "Wire.h"                                               // I2C interface f�r DS3231


//************ Objekte ************//

dht11 DHT11;                                                    // Feuchtigkeits und Temperatur Sensor

DS3231 Clock;                                                   // Echtzeituhr DS3231

//************ Pin Definitionen ************//

#define DHT11PIN 6                                              // DHT11 auf Pin 6

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);                          // Die Pins f�r das LCD sind D12,D11,D5,D4,D3,D2
                                                                /*
                                                                LCD RS pin zu digital pin 12
                                                                LCD Enable pin zu digital pin 11
                                                                LCD D4 pin zu digital pin 5 
                                                                LCD D5 pin zu digital pin 4
                                                                LCD D6 pin zu digital pin 3
                                                                LCD D7 pin zu digital pin 2
                                                                */                                              
//Echtzeituhr DS3231 Pins 
//SCL -> A5
//SDA -> A4


//************Button Zuweisung************//
const int nextButton = 7;                                       // WeiterButton auf Pin 7
const int editButton = 8;                                       // EditButton auf Pin 8
const int increaseButton = 9;                                   // + Button auf Pin 9
const int descreaseButton = 10;                                 // - Button auf Pin 10



//************ Variablen & Funktionen ************//

//DHT11
int checkDHT11(void);                                           // Checkt den Status des Sonsors


//DS3231
int second,minute,hour,date,month,year,DStemperature;           // 
void ReadDS3231(void);                                          // Liest die Daten der Echtzeituhr aus
bool Century=false;                                             //
bool h12;                                                       // 12 oder 24 Std Modus
bool PM;                                                        // AM PM anzeige nur bei 12 Std
byte ADay, AHour, AMinute, ASecond, ABits;                      //
bool ADy, A12h, Apm;                                            //

byte Byear, Bmonth, Bdate, BDoW, Bhour, Bminute, Bsecond;       //


//DisplayIntro Variablen
int startPoint;                                                 // F�ge Startpunkt hinzu
int endPoint;                                                   // F�ge Endpunkt hinzu
int i, j;                                                       // Integriere "i" und "j"
int speed = 50;                                                 // Scrollgeschwindigkeit der TextMessage

//Dargestelleter Text
String txtMsg =      "Meine";                                   // String TxtMsg "Meine"
String txtMsg1 = "Wetterstation";                               // String TextMsg1 "Wetterstation"

//Definiere Verz�gerungszeiten
#define DisplayDelay 1000                                       // Verz�gerung der LCD aktualisierung beim anzeigen der Daten
#define EditDelay 1000                                          // Verz�gerung der LCD aktualisierung im Einstellungsmodus

void DisplayIntro(void);                                        // Zeigt das DisplayIntro beim Starten an
char* getDayofweek(int d, int m, int y);                        // Wochentag anzeigen
int isLeapYear(int y);                                          // Gibt eine 1 wieder wenn es ein Schaltjahr ist, oder eine 0 wenn nicht

//************Allgemeine Einstellungen************//
int displayValue=0;                                             // Zeigt die verschiedenen Menus des LCD Displays an 
/*
 * 0 - Alle
 * 1 - Zeit
 * 2 - Datum
 * 3 - Temperatur
 * 4 - Feuchtigkeit
 */
 
int editValue=0;                                                // Einstellungswerte der Echtzeituhr
/*
 * 0 - Stunden
 * 1 - Minuten
 * 2 - Tag
 * 3 - Monat
 * 4 - Jahr
 */
 
bool edit=false;                                                // Definition ob Daten der Sensoren gezeigt werden oder der Einstellungsmodus der Echtzeituhr

void printAll(void);                                            // Zeigt Daten von allen Sensoren an
void printTime(void);                                           // Zeigt die Uhrzeit vom DS3231 an
void printHourMinute(void);                                     // Zeigt Stunden/Minuten vom DS3231 an
void printDate(void);                                           // Zeigt das Datum vom DS3231 an
void printTemperature(void);                                    // Zeigt die Temperatur vom DHT11 an
void printHumidity(void);                                       // Zeigt die Feuchtigkeit vom DHT11 an


//************Symbole************//
byte termometru[8] =                                            // Thermometer Symbol
{
    B00100,                                                     //   0  0  1  0  0
    B01010,                                                     //   0  1  0  1  0
    B01010,                                                     //   0  1  0  1  0
    B01110,                                                     //   0  1  1  1  0
    B01110,                                                     //   0  1  1  1  0
    B11111,                                                     //   1  1  1  1  1
    B11111,                                                     //   1  1  1  1  1
    B01110,                                                     //   0  1  1  1  0
};

byte picatura[8] =                                              // Wassertropfen Symbol
{
    B00100,                                                     //   0  0  1  0  0
    B00100,                                                     //   0  0  1  0  0
    B01010,                                                     //   0  1  0  1  0
    B01010,                                                     //   0  1  0  1  0
    B10001,                                                     //   1  0  0  0  1
    B10001,                                                     //   1  0  0  0  1
    B10001,                                                     //   1  0  0  0  1
    B01110,                                                     //   0  1  1  1  0
};


void setup() {

  pinMode(nextButton, INPUT);                                   // Weiter Button als Input gesetzt
  pinMode(editButton, INPUT);                                   // Edit Button als Input gesetzt
  pinMode(increaseButton, INPUT);                               // + Button als Input gesetzt
  pinMode(descreaseButton, INPUT);                              // - Button als Input gesetzt

  lcd.begin(16, 2);                                             // Einstellen des LCD Display mit 16 Zeilen und 2 Reihen

  Serial.begin(115200);                                         // Initialisiere Serielle SChnittstelle 

  Wire.begin();                                                 // Initialisiere I2C interface
  DisplayIntro();                                               // Zeige das "Meine Wetterstation" Intro

  lcd.clear();                                                  // L�sche das LCD display

  lcd.createChar(1,termometru);                                 // Erstelle Charakter "1" f�r das Temperatur Symbol
  lcd.createChar(2,picatura);	                                  // Erstelle Charaker "2" f�r das Wassertropfen Symbol
}


void loop() {

if ( digitalRead(editButton)==HIGH)                             // Wenn Edit-Button gedr�ckt
    {
    edit=!edit;                                                 // dann nach 1 sek Einstellungsmodus
    editValue=0;                                                // wenn Einstellungsmodus wieder aus ist
    displayValue=0;                                             // Zeigt das Display wieder alle Daten an 
    delay(EditDelay);                                           // nach 1 sek Verz�gerung
   }

if(edit==false)                                                 // Wenn Einstellungsmodus aus = Zeigt Werte aller Sensoren an
{
//Kein Blinken wenn Daten angezeigt werden  
lcd.noCursor();                                                 // Schaltet den Cursor im LCD Display aus
lcd.noBlink();                                                  // Schaltet das Blinken im LCD Display aus


//************DisplayMenus************//
  if( digitalRead(nextButton)==HIGH)                            // Wenn Weiter-Button gedr�ckt
  {
    displayValue++;                                             // dann geht das Display ins n�chste Menu
  }
  switch(displayValue)                                          // �ndere Display Menus
  {
    case 0:                                                     // Menupunkt 0 Zeigt Daten aller Sensoren an
      lcd.clear();                                              // Display l�schen
      printAll();                                               // Zeigt Daten aller Sensoren an
    break;
    case 1:                                                     // Menupunkt 1 Zeigt Uhrzeit an
      ReadDS3231();                                             // Liest Daten der Echtzeituhr aus(Zeit)
      lcd.clear();                                              // Display l�schen
      printTime();                                              // Zeigt die Uhrzeit in HH:MM:SS
    break;
    case 2:                                                     // Menupunkt 2 Zeigt Wochentag und Datum an
      ReadDS3231();                                             // Liest Daten der Echtzeituhr aus(Datum)
      lcd.clear();                                              // Display l�schen
      printDate();                                              // Zeigt Wochentag und das Datum an
    break;
    case 3:                                                     // Menupunkt 3 zeigt Temperatur an
      checkDHT11();                                             // Liest Daten des DHT11 aus
      lcd.clear();                                              // Display l�schen
      printTemperature();                                       // Zeigt Temperatur an,
    break;      
    case 4:                                                     // Menupunkt 4 zeigt die Feuchtigkeit an
      checkDHT11();                                             // Liest die Daten des DHT11 aus
      lcd.clear();                                              // Display l�schen
      printHumidity();                                          // Zeigt die Luftfeuchtigkeit an 
    break;   
  
    default:                                                    // Standartfenster was immer als erstes gezeigt wird
      lcd.clear();                                              // Display l�schen
      displayValue=0;                                           // Display wert "0" = Hauptfenster 
      printAll();                                               // Zeigt Daten aller Sensoren an
    break;
  }

  delay(DisplayDelay);                                          // Verz�gerung "1 Sekunde"
  
}
else                                                            // Einstellungsmodus an, Zeit und Datum einstellen
{ 
delay(EditDelay);                                               // Verz�gerung f�r den Einstellungsmodus
lcd.clear();                                                    // Display l�schen

  if( digitalRead(nextButton) ==HIGH){                          // Ausw�hlen welchen Wert man �ndern m�chte Minuten/Stunden/Tag/Monat/Jahr
    editValue++;
  }


//************Einstellungsmodus************//
  switch(editValue){
    case 0:                                                     // Fall 0. Die Stunden einstellen
      lcd.noCursor();
      ReadDS3231();                                             // Liest die Daten der Echtzeituhr aus (Zeit)
      printHourMinute();                                        // Zeigt die Uhrzeit an
      //lcd.setCursor(5, 0);
        if(digitalRead(increaseButton)==HIGH)                   // Wenn der +Button gedr�ckt wird
          {
          hour++;                                               // Werden die Stunden erh�ht
          if(hour>23) hour=0;                                   // Wenn die Stunden gr��er als "23" sind, dann bei "0" anfangen 
          }
        if(digitalRead(descreaseButton)==HIGH)                  // Wenn der -Button gedr�ckt wird 
          {
          hour--;                                               // Werden die Stunden gesenkt
          if(hour<0) hour=23;                                   // Wenn die Stunden kleiner als "0" sind, dann bei "23" anfangen
          }
       Clock.setHour(hour);                                     // Speichert die Stunden ab
       printHourMinute();                                       // Zeigt die Uhrzeit an
       lcd.setCursor(5, 0);                                     // Display Cursor an stelle "6" in Reihe "1" 
       lcd.cursor();                                            // Display Cursor einschalten
    break;
    case 1:                                                     // Fall 1. Minuten einstellen
      lcd.noCursor();                                           // Display Cursor ausschalten
      ReadDS3231();                                             // Liest die Daten der Echtzeituhr aus (Zeit)
      printHourMinute();                                        // Zeigt die Uhrzeit an
      //lcd.setCursor(8, 0);
        if( digitalRead(increaseButton)==HIGH)                  // Wenn +Button gedr�ckt wird
          {
          minute++;                                             // Werden die Minuten erh�ht
          if(minute>59) minute=0;                               // Wenn die Minuten gr��er als "59" sind, dann bei "0" anfangen
          }
        if(digitalRead(descreaseButton)==HIGH)                  // Wenn -Button gedr�ckt wird
          {
          minute--;                                             // Werden die Minuten gesenkt
          if(minute<0) minute=59;                               // Wenn die Minuten kleiner als "0" sind, dann bei "59" anfangen 
          }
       Clock.setMinute(minute);                                 // Spiechert die Minuten ab
       printHourMinute();                                       // Ziegt die Uhrzeit an 
       lcd.setCursor(8, 0);                                     // Display Cursor auf Stelle "9" in Reihe "1" 
       lcd.cursor();                                            // Display Cursor einschalten
    break;
    case 2:                                                     // Fall 2. Tag einstellen
      lcd.noCursor();                                           // Display Cursor ausschalten
      ReadDS3231();                                             // Liest die Daten der Echtzeituhr aus (Datum)
      printDate();                                              // Zeigt das Datum an
      //lcd.setCursor(9, 0);
      if( digitalRead(increaseButton)==HIGH)                    // Wenn +Button gedr�ckt wird
          {
          date++;                                               // Werden die Tage erh�ht
          if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) // Wenn es ein Monate mit 31 Tagen ist
          {
            if (date>31) date=1;                                // Wenn die Tage gr��er als "31" sind, dann bei "1" anfangen
          }
          else if(month==2)                                     // Oder wenn im Februar
          {
            if(isLeapYear(year)==1)                             // Ein Schaltjahr ist 
            {
              if (date>29) date=1;                              // Wenn die Tage gr��er als "29" sind, dann wieder bei "1" anfangen
            }
            else if(isLeapYear(year)==0)                        // Oder wenn kein Schaltjahr ist
            {
              if (date>28) date=1;                              // Wenn die Tage gr��er als "28" sind, dann bei "1" anfangen
            }
          }
          }
        
        if(digitalRead(descreaseButton)==HIGH)                  // Wenn der -Button ger�ckt wird 
          {
          date--;                                               // Dann werden die Tage gesenkt
          if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) //Wenn Monate mit "31" tagen  
          {
            if (date<1) date=31;                                // Wenn die Tage kleiner als "1" sind, dann bei "31" Tagen anfangen 
          }
          else if(month == 4 || month == 6 || month == 9 || month == 11 )  // Oder wenn Monate mit "30" Tagen
          {
            if (date<1) date=30;                                // Wenn Tage kleiner als "1" sind, dann bei "30" Tagen anfangen
          }
          else if(month==2)                                     // Wenn der Februar
          {
            if(isLeapYear(year)==1)                             // In einem Schaltjahr liegt
            {
              if (date<1) date=29;                              // Und die Tage kleiner als 1 sind, dann bei "29" anfangen
            }
            else if(isLeapYear(year)==0)                        // Wenn kein Schaltjahr ist
            {
              if (date<1) date=28;                              // Wenn die Tage kleiner als "1" sind, dann bei "28" anfangen
            }
          }
          }
       Clock.setDate(date);                                     // Speichert den Tag ab
       printDate();                                             // Zeigt das Datum an 
       lcd.setCursor(5, 1);                                     // Display Cursor an Stelle "6" in Reihe "1"
       lcd.cursor();                                            // Display Cursor einschalten
    break;
    case 3:                                                     // Fall 3. Den Monat einstellen
      lcd.noCursor();
      ReadDS3231();                                             // Liest Daten der Echtzeituhr aus (Datum)
      printDate();                                              // Zeigt Datum an
      
        if(digitalRead(increaseButton)==HIGH)                   // Wenn der +Button gedr�ckt wird
          {
          month++;                                              // Werden die Monate erh�ht
            if (month>12) month=1;                              // Wenn die Monate mehr als "12" sind, dann bei "1" anfangen
          }
        if(digitalRead(descreaseButton)==HIGH)                  // Wenn der -Button gedr�ckt wird
          {
          month--;                                              // Werden die Monate gesenkt
            if (month<1) month=12;                              // Wenn die Monate kleiner als "1" sind, dann bei "12" anfangen
          }  
       Clock.setMonth(month);                                   // Speichert den Monat ab
       printDate();                                             // Zeigt das Datum an 
       lcd.setCursor(8, 1);                                     // Display Cursor auf Stelle "9" in Reihe "1"
       lcd.cursor();                                            // Display Cursor einschalten
    break;
    case 4:                                                     // Fall 4. Das Jahr einstellen
      lcd.noCursor();                                           // Display Cursor ausschalten
      ReadDS3231();                                             // Liest Daten der Echtzeituhr aus (Datum)
      printDate();                                              // Zeigt das Datum an
      //lcd.setCursor(8, 0);
        if(digitalRead(increaseButton)==HIGH)                   // Wenn +Button gedr�ckt wird
         {
          year++;                                               // Wird das Jahr erh�ht
            if (year>99) year=1;                                // Wenn die Jahre mehr als "99" sind, dann bei "1" anfangen
          }
        if(digitalRead(descreaseButton)==HIGH)                  // Wenn -Button gedr�ckt wird 
          {
          year--;                                               // Werden die Jahre gesenkt
            if (year<1) year=99;                                // Wenn die Jahre kleiner als "1" sind, dann bei "99" anfangen
          }
          
       Clock.setYear(year);                                     // Speichert das Jahr ab 
       printDate();                                             // Zeigt das Datum an
       lcd.setCursor(11, 1);                                    // Display Cursor auf Stelle 12 in Reihe 1 
       lcd.cursor();                                            // Display Cursor einschalten
    break;   
    default:
      editValue=0;                                              // Wenn Einstellungsmodus beendet
      lcd.clear();                                              // Display l�schen
      lcd.setCursor(2,0);                                       // Cursor auf Stelle 3 in Rheihe 1
      lcd.print("Zeit & Datum");                                // Zeigt "Zeit und Datum"
      lcd.setCursor(2,1);                                       // Cursor auf stelle 4 in Reihe 2
      lcd.print("eingestellt!");                                // Zeigt "eingestellt!" an
      lcd.noCursor();                                           // Display Cursor ausgeschaltet
      delay (3000);                                             // Verz�gerung von 3 Sekunden
    break;
  } 
}
  
}


//************"Wochentag und Datum" Menu************//
void printDate()                                                // Zeigt Datum an
{
  ReadDS3231();                                                 // Liest die Daten der Echtzeituhr aus (Datum)

  lcd.setCursor(4, 0);                                          // Display Cursor auf Stelle "4" in Reihe "1"
  lcd.print(getDayofweek(date, month, year+2000));              // Zeigt Wochentag an
    
  lcd.setCursor(4, 1);                                          // Display Cursor auf Stelle "5" in Reihe "2"
  if(date<10) lcd.print('0');                                   // Wenn die Tage kleiner als "10" sind dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(date);                                              // Zeigt den Tag an
  lcd.print('.');                                               // Zeigt einen "." an
  if(month<10) lcd.print('0');                                  // Wenn die Monate kleiner als "10" sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(month);                                             // Zeigt den Monat an
  lcd.print('.');                                               // Zeigt einen "." an
  if(year<10) lcd.print('0');                                   // Wenn die Jahre kleiner als "10" sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(year);                                              // Zeigt das Jahr an
}  


//************"UhrZeit" Menu************//
void printTime()                                                // Zeigt Uhrzeit an 
{
  ReadDS3231();                                                 // Liest Daten der Echtzeituhr aus (Zeit)
  lcd.setCursor(4,0);                                           // Display Cursor an Stelle "5" in Reihe "1"
  lcd.print("Uhrzeit:");                                        // Zeigt "Uhrzeit:" an
  lcd.setCursor(4, 1);                                          // Display Cursor an Stelle "5" in Reihe "2"
  if(hour<10) lcd.print('0');                                   // Wenn die Stunden kleiner als "10" sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(hour);                                              // Zeigt die Stunden an
  lcd.print(':');                                               // Zeigt einen ":" an
  if(minute<10) lcd.print('0');                                 // Wenn die Minuten kleiner als "10" sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(minute);                                            // Zeigt die Minuten an
  lcd.print(':');                                               // Zeigt einen ":" an
  if(second<10) lcd.print('0');                                 // Wenn die Sekunden kleiner als "10" sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(second);                                            // Zeigt die Sekunden an
}

void printHourMinute()                                          // Zeigt Uhrzeit an
{
  ReadDS3231();                                                 // Liest Daten der Echtzeituhr aus (Zeit)
  lcd.setCursor(4, 0);                                          // Display Cursor an Stelle 5 in Reihe 1
  if(hour<10) lcd.print('0');                                   // Wenn die Stunden kleiner als "10" sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(hour);                                              // Zeigt die Stunden an
  lcd.print(':');                                               // Zeigt einen ":" an
  if(minute<10) lcd.print('0');                                 // Wenn die Minuten kleiner als "10" sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(minute);                                            // Zeigt die Minuten an
  lcd.print(':');                                               // Zeigt einen ":" an
  if(second<10) lcd.print('0');                                 // Wenn die Sekunden kleiner als "10" sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(second);                                            // Zeigt die Sekunden an
}


//************Zeigt "Alle Daten" Menu an************//
void printAll(void)                                             // Zeigt "Alle Daten" an
{
  if (checkDHT11()== 1)                                         // Liest Daten des DHT11 aus (Temperatur und Feuchtigkeit)
  {
    lcd.print("");
  }
  ReadDS3231();                                                 // Liest Daten der Echtzeituhr aus (Zeit und Datum)
  //Zeit
  lcd.setCursor(1, 0);                                          // Display Cursor an Stelle 2 in Reihe 1
  if(hour<10) lcd.print('0');                                   // Wenn die Stunden kleiner als 10 sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(hour);                                              // Zeigt die Stunden an
  lcd.print(':');                                               // Zeigt einen ":" an
  if(minute<10) lcd.print('0');                                 // Wenn die Minuten kleiner als 10 sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(minute);                                            // Zeigt die Minuten an

  //Datum
  lcd.setCursor(7, 0);                                          // Display Cursor an Stelle 2 in Reihe 1
  if(date<10) lcd.print('0');                                   // Wenn die Tage kleiner als 10 sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(date);                                              // Zeigt den Tag an
  lcd.print('.');                                               // Zeigt einen "." an
  if(month<10) lcd.print('0');                                  // Wenn die Monate kleiner als 10 sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(month);                                             // Zeigt den Monat an
  lcd.print('.');                                               // Zeigt einen "." an
  if(year<10) lcd.print('0');                                   // Wenn die Jahre kleiner als 10 sind, dann eine "0" vor der eigentlichen Zahl anzeigen
  lcd.print(year);                                              // Zeigt das Jahr an

  //Temperatur
  lcd.setCursor(1, 1);                                          // Display Cursor an Stelle 2 in Reihe 2
  lcd.write (1);                                                // Zeigt das Thermometer Symbol
  lcd.print(DHT11.temperature);                                 // Zeigt die Temperatur an
  lcd.print((char)223);                                         // Zeigt das Grad "�" Symbol an
  lcd.print("C");                                               // Zeigt ein "C" f�r Celsius an

  //Luftfeuchtigkeit
  lcd.setCursor(9, 1);                                          // Display Cursor an Stelle 10 in Reihe 2
  lcd.write(2);                                                 // Zeigt das Wassertropfen Symbol an 
  lcd.setCursor(11, 1);                                         // Display Cursor an Stelle 12 in Reihe 2
  lcd.print(DHT11.humidity);                                    // Zeigt die Luftfeuchtigkeit an
  lcd.print('%');                                               // Zeigt ein "%" an
}


//************DisplayIntro einstellungen************//
void DisplayIntro(void)                                         // DisplayIntro einstellungen
{
   startPoint = 0;                                              // Startpunkt einstellen
   endPoint = 15;                                               // Endpunkt einstellen
   lcd.clear();                                                 // Display l�schen

  for (i = txtMsg.length() - 1; i >= 0; i--)                    // F�r jeden Buchstaben aus dem String angefangen vom letzten
  {
    startPoint = 0;                                             // Startpunkt "0"
    for (j = 0; j < endPoint; j++)                              // F�r jede Position auf dem LCD display
    {
      lcd.setCursor(startPoint, 0);                             // Display Cursor auf Startpunkt setzen
      lcd.print(txtMsg[i]);                                     // Zeigt die TxtMsg "Meine" an 
      delay(speed);                                             // Schnelligkeit der Textnachricht
      if (startPoint != endPoint - 1)                           // Wenn der Startpunkt ...
      {
        lcd.setCursor(startPoint, 0);                           // Display Cursor an Startpunkt setzen
        lcd.print(' ');                                         // Zeigt "" an
      }
      startPoint++;                                             // Startpunkt wird mehr
    }
    startPoint = 0;                                             // Starpunkt auf "0" setzen
    for (j = 0; j < endPoint; j++)                              // j=0 Wenn j kleiner als der Endpunkt ist, dann wird j mehr
    {
      lcd.setCursor(startPoint, 1);                             // Display Cursor auf Startpunkt "1" setzen
      lcd.print(txtMsg1[i]);                                    // Zeigt die TxtMsg1 "Wetterstation" an

      delay(speed);                                             // Geschwindigkeit der Textnachricht

      if (startPoint != endPoint - 1)                           // Wenn Startpunkt ...
      {
        lcd.setCursor(startPoint, 1);                           // Display Cursor auf Startpunkt 1
        lcd.print(' ');                                         // Zeigt "" an
      }
      startPoint++;                                             // Startpunkt wird mehr 
    }
    endPoint--;                                                 // Endpunkt wird weniger
    delay(speed);                                               // Geschwindigkeit der Textnachrichten
  }
    delay(3000);                                                // Verz�gert das DisplayIntro auf dem Display f�r "3 Sekunden"
}

//************Daten der Echtzeituhr auslesen************//
void ReadDS3231(void)                                           // Liest die Werte der Echtzeituhr aus
{
  second=Clock.getSecond();                                     // Gibt die Sekunden wieder
  minute=Clock.getMinute();                                     // Gibt die Minuten wieder
  hour=Clock.getHour(h12, PM);                                  // Gibt die Stunden wieder
  date=Clock.getDate();                                         // Gibt den Tag wieder
  month=Clock.getMonth(Century);                                // Gibt den Monat wieder
  year=Clock.getYear();                                         // Gibt das Jahr wieder
  DStemperature=Clock.getTemperature();                         // Gibt die Temperatur wieder
}


//************"Temperatur" Menu***************//
void printTemperature()                                         // Zeigt die Temperatur auf dem LCD Display an.
{
  lcd.setCursor(2, 0);                                          // Display Cursor an Stelle "3" in Reihe "1"
  lcd.print("Temperatur:");                                     // Zeigt "Temperatur:" an
  lcd.setCursor(4, 1);                                          // Display Cursor an Stelle "5" in Reihe "2"
  lcd.write(1);                                                 // Zeigt das Themometer Symbol an
  lcd.setCursor(6, 1);                                          // Display Cursor an Stelle "7" in Reihe "2" 
  lcd.print(DHT11.temperature);                                 // Zeigt die Temperatur an 
  lcd.print((char)223);                                         // Zeigt das Grad "�" Symbol an 
  lcd.print("C");                                               // Zeigt "C" f�r Celsius an
}


//************"Luftfeuchtigkeit" Menu************//
void printHumidity()                                            // Zeigt die Luftfeuchtigkeit auf dem LCD Display an.
{
    lcd.setCursor(1, 0);                                        // Display Cursor auf Selle "2" in Reihe "1"
    lcd.print("Feuchtigkeit:");                                 // Zeigt "Feuchtigkeit:" an
    lcd.setCursor(4, 1);                                        // Display Cursor auf Stelle "5" in Reihe "2"
    lcd.write(2);                                               // Zeigt das Wassertropfen Symbol an 
    lcd.setCursor(6, 1);                                        // Diplay Cursor auf Stelle "7" in Reihe "2"
    lcd.print(DHT11.humidity);                                  // Zeigt die Luftfeuchtigkeit an
    lcd.print('%');                                             // Zeigt "%" an
}


int checkDHT11()                                                // Auslesen des DHT11 Sensorsstatus
{ 
int chk = DHT11.read(DHT11PIN);                                 // Liest den "Pin 6" aus

Serial.print("Lese DHT11 Sensor aus: ");                        // Zeigt "Lese DHT11 Sensor aus: " auf dem Seriellen Monitor an
  switch (chk)
  {
    case DHTLIB_OK: 
    Serial.println("DHT11:OK\n");                               // Zeigt "OK" auf dem Seriellen Monitor an
    return 0;
    break;
    case DHTLIB_ERROR_CHECKSUM: 
    Serial.println("DHT11:Checksum Fehler\n");                  // Zeigt "Checksum Fehler" auf dem Seriellen Monitor an
    return 1;
    break;
    case DHTLIB_ERROR_TIMEOUT: 
    Serial.println("DHT11:Timeout Fehler\n");                   // Zeigt "Timeout Fehler" auf dem Seriellen Monitor an
    return 1;
    break;
    default: 
    Serial.println("DHT11: Unbekannter Fehler\n");              // Zeigt "Unbekannter Fehler" auf dem Seriellen Monitor an
    return 1;
    break;
  }
}

//************Wochentage************//
char* getDayofweek(int d, int m, int y)                         //Zeigt den Wochentag an
{
    int day;
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    y -= m < 3;
    day = ( y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
     switch(day){
      case 0 :return("Sonntag");
      case 1 :return("Montag");
      case 2 :return("Dienstag");
      case 3 :return("Mittwoch");
      case 4 :return("Donnerstag");
      case 5 :return("Freitag");
      case 6 :return("Samstag");

      default:return("Fehler: Ung?ltiges Argument");
      }
}


int isLeapYear(int y){                                          //Schaltjahr Ja oder Nein?
  if ( y%400 == 0)
    return 1;
  else if ( y%100 == 0)
    return 0;
  else if ( year%4 == 0 )
    return 1;
  else
    return 0;   
  return 0;
}



