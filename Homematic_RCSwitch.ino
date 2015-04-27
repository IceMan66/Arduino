/*

// ATMEL ATTINY84 / ARDUINO
//
//                  +-\/-+
//            VCC  1|    |14  GND
//      (D 0) PB0  2|    |13  AREF
//      (D 1) PB1  3|    |12  PA1 (D 9)
//            PB3  4|    |11  PA2 (D 8)
//  PWM (D 2) PB2  5|    |10  PA3 (D 7)
//  PWM (D 3) PA7  6|    |9   PA4 (D 6)
//  PWM (D 4) PA6  7|    |8   PA5 (D 5) PWM
//                  +----+

*/

// ---------- included libraries ----------
#include <RCSwitch.h> //http://code.google.com/p/rc-switch/downloads/detail?name=RCswitch_2.51.zip&can=3&q=
#include <avr\sleep.h>



// ----------- hardware pin definitions ----------


// ---------- Utility macros ----------
#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

//this defines two convenient 'function macros' that allow the setting or clearing of individual bits in a register.
//Function macros are like normal #defines, but the arguments in the expressions are treated like place holders for actual variables to be inserted by the compiler
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))    //clear bit in byte at sfr adress
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))     //set bit in byte at sfr adress
#endif

// ---------- library initialization ---------
RCSwitch mySwitch = RCSwitch();



// ---------- constant / literals definitions ----------

// ---------- variable initialization ----------
const int pinCount = 8;
volatile boolean eventFlag = true; //force channel update for first use


typedef struct {
  byte HMInputPin; //
  boolean PinState;
  char familycode;
  byte groupnumber;
  byte devicenumber;
} HMChannel_t;


HMChannel_t HMRXPins[] = {
  {PA1, -1, 'a', 1, 1}, // RCSwitch No. A1
  {PA2, -1, 'a', 1, 2}, // RCSwitch No. A2
  {PA3, -1, 'a', 1, 3}, // RCSwitch No. A3
  {PA4, -1, 'b', 1, 1}, // RCSwitch No. B1
  {7, -1, 'b', 1, 2}, // RCSwitch No. B2
  {10, -1, 'b', 1, 3}, // RCSwitch No. B3
  {9, -1, 'c', 1, 1}, // RCSwitch No. C1
  {8, -1, 'c', 1, 2}, // RCSwitch No. C2
};



void setup() {

  byte pinTX  = PA0; //DATA PIN for 433MHz Transmitter
  mySwitch.enableTransmit(pinTX);

  //Set Input and enable PULLUP
  for (int i = 0; i < pinCount; i++) {
    pinMode(HMRXPins[i].HMInputPin, INPUT_PULLUP);
  }

  GIMSK  = (1 << PCIE0) | (1 << PCIE1); //enable Pin-Change-Interrupt

  sbi (PCMSK0, PCINT1); //enable PA1 for Interrupt
  sbi (PCMSK0, PCINT2); //enable PA2 for Interrupt
  sbi (PCMSK0, PCINT3); //enable PA3 for Interrupt
  sbi (PCMSK0, PCINT4); //enable PA4 for Interrupt
  sbi (PCMSK0, PCINT7); //enable PA7 for Interrupt
  sbi (PCMSK1, PCINT8); //enable PB0 for Interrupt
  sbi (PCMSK1, PCINT9); //enable PB1 for Interrupt
  sbi (PCMSK1, PCINT10); //enable PB2 for Interrupt
}

void loop() {

  // boolean newState;

  if (eventFlag) {
    for (int i = 0; i < pinCount; i++) {
      boolean newState = !digitalRead(HMRXPins[i].HMInputPin);
      if (newState != HMRXPins[i].PinState) {
        HMRXPins[i].PinState = newState;
        if (newState == HIGH) {
          mySwitch.switchOn(HMRXPins[i].familycode, HMRXPins[i].groupnumber, HMRXPins[i].devicenumber); // RCSwitch ON
        }
        else
        {
          mySwitch.switchOff(HMRXPins[i].familycode, HMRXPins[i].groupnumber, HMRXPins[i].devicenumber); // RCSwitch OFF
        }
      }
    }
  }
  eventFlag = false; //reset the event flag for the next event
  system_sleep();  // go back to sleep
}

//
void system_sleep() {
  cbi(ADCSRA, ADEN); // Switch Analog to Digital converter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode
  sleep_mode(); // System sleeps here
  sbi(ADCSRA, ADEN); // Switch Analog to Digital converter ON
}



//Interrupt Service Routine. This code is executed whenever a event (either rising or falling edge) occurs.
ISR (PCINT0_vect) {
  eventFlag = true; //set the event flag that tells the main loop that a event was triggert.
}

//Interrupt Service Routine. This code is executed whenever a event (either rising or falling edge) occurs.
ISR (PCINT1_vect) {
  eventFlag = true; //set the event flag that tells the main loop that a event was triggert.
}
