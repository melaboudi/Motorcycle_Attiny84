#include <Arduino.h>

#include <TinyWire.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#define DEBUG
#ifdef DEBUG
byte own_address = 8;
const int buttonPin1 = 8;
const int buttonPin2 = 10;
const int buttonPin3 = 1;
const int ledPin = 7;
volatile bool ledState = LOW;
int buttonState1;
int buttonState2;
int buttonState3;
int lastButtonState1 = LOW;
int lastButtonState2 = LOW;
int lastButtonState3 = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
volatile int state = 0;
volatile bool ON = true;
volatile bool request = false;
volatile bool received = false;
bool change=false;
unsigned long counter=0;
uint8_t retour=0;
unsigned int oldRetour=0;
volatile int x=8;
volatile int y=9;

// these define cbi and sbi, for as far they are not known yet
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
char reception[3] = {0};
void onI2CRequest();
void checkButtons();
void setup_watchdog(int ii);
void system_sleep();
void onI2CReceive(int howMany);
void blinkLED(int k);

void setup() {
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  TinyWire.begin( own_address );
  TinyWire.onRequest( onI2CRequest );
  TinyWire.onReceive( onI2CReceive );
}

void loop() {
  
  if(received){
    while (TinyWire.available() > 0) {
      char buffer = TinyWire.read();
      sprintf(reception, "%c", buffer);
    }
    if (strcmp(reception, "n") == 0) {blinkLED(1);}
    else if (strcmp(reception, "f") == 0) {
    //blinkLED(1);
    system_sleep();
    }else if (strcmp(reception, "r") == 0) {
      x=0;y=0;
    }
    received = false;
  }
  
    checkButtons();
  if(change&&(retour!=oldRetour)){counter++;oldRetour=retour;}
    if(((counter%2)==0)&&(change)){
    //digitalWrite(7,HIGH);
    x=retour;
    change=false;
    }
    if(((counter%2)==1)&&(change)){
    //digitalWrite(7,LOW);
    y=retour;
    change=false;}
}
void checkButtons(){
  int reading1 = digitalRead(buttonPin1);
  int reading2 = digitalRead(buttonPin2);
  int reading3 = digitalRead(buttonPin3);
  if ((reading1 != lastButtonState1) || (reading2 != lastButtonState2) || (reading3 != lastButtonState3)) {
    lastDebounceTime = millis();}
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if ((reading1 != buttonState1) || (reading2 != buttonState2) || (reading3 != buttonState3)) {
      change=true;
      buttonState1 = reading1;
      buttonState2 = reading2;
      buttonState3 = reading3;
      if ((buttonState1 == LOW) && (buttonState2 == HIGH) && (buttonState3 == HIGH)) {retour= 1;}
      if ((buttonState1 == HIGH) && (buttonState2 == LOW) && (buttonState3 == HIGH)) {retour= 2;}
      if ((buttonState1 == HIGH) && (buttonState2 == HIGH) && (buttonState3 == LOW)) {retour= 3;}
      if ((buttonState1 == LOW) && (buttonState2 == LOW) && (buttonState3 == HIGH)) {retour= 4;}
      if ((buttonState1 == LOW) && (buttonState2 == HIGH) && (buttonState3 == LOW)) {retour= 5;}
      if ((buttonState1 == HIGH) && (buttonState2 == LOW) && (buttonState3 == LOW)) {retour= 6;}
      if ((buttonState1 == LOW) && (buttonState2 == LOW) && (buttonState3 == LOW)) {retour= 7;}
    }
  }
  lastButtonState1 = reading1;
  lastButtonState2 = reading2;
  lastButtonState3 = reading3;
}

void onI2CRequest() {
  byte data[2]={lowByte(x),highByte(x)};
  byte data1[2]={lowByte(y),highByte(y)};
  TinyWire.write(data[0]);
  TinyWire.write(data[1]);
  TinyWire.write(data1[0]);
  TinyWire.write(data1[1]);
}

void setup_watchdog(int ii) {
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

  uint8_t bb;
  if (ii > 9 ) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

// system wakes up when watchdog is timed out
void system_sleep() {
  cbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter OFF
  setup_watchdog(9);                   // approximately 8 seconds sleep

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sei();                               // Enable the Interrupts so the wdt can wake us up
  sleep_mode();                        // System sleeps here

  sleep_disable();                     // System continues execution here when watchdog timed out
  power_all_enable();
  sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON
}

// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
  //blinkLED(1);
}

void onI2CReceive(int howMany) {
  received = true;
}
void blinkLED(int k) {
  for (int i = 0; i < k; i++) {
    digitalWrite(7, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(25);                       // wait for a second
    digitalWrite(7, LOW);    // turn the LED off by making the voltage LOW
    delay(25);
  }
}
