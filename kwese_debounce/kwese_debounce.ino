// This code is intended to run on the kwese - an arduino based musical
// instrument. The idea is to use pins as capacitive sensors (each pin used as a sensor needs
// to be tied to +5V by a >1M resistor) and send data about which pins are down to a phone or computer.
// The output is serial data - bluetooth can be used by attaching an HC-06
// bluetooth board to RX, TX, +V and GND. 

//Set everything up

//some variables
int LEDpin = 13; //LED as an indicator
int pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, A0}; //input pins
int pincount = 12; //len(input pins)
//thresholds (autocalibration)
int pin_thresholds[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//stuff for debouncing...
int debounce_threshold = 5;
//int debounce_counts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int prev_values[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup(){
  //serial
  Serial.begin(9600);
  //LED
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW); 
  //get the thresholds
  for (int i = 0; i < pincount; i ++){
    pin_thresholds[i] = readCapacitivePin(pins[i])+10;
  }
}

//main loop
void loop(){
  for (int i = 0; i < pincount; i ++){
    int touch = 0;
    int touch_count = 0;
    for (int d = 0; d < debounce_threshold; d++){
      if (readCapacitivePin(pins[i])>pin_thresholds[i]){
        touch_count ++;
      }
    }
    if (touch_count > 2){
      touch = 1;
    }
    else if (touch_count<2){
      touch = 0;
    }
    else {
      touch = prev_values[i];
    }
    Serial.print(touch);
    Serial.print("A");
    prev_values[i] = touch*1;
  }
  Serial.println("");
}





//The following code was not written by me. Sadly, I cannot track down the 
//origonal source. 


// readCapacitivePin
//  Input: Arduino pin number
//  Output: A number, from 0 to 17 expressing
//          how much capacitance is on the pin
//  When you touch the pin, or whatever you have
//  attached to it, the number will get higher
//  In order for this to work now,
// The pin should have a 1+Megaohm resistor pulling
//  it up to +5v.
uint8_t readCapacitivePin(int pinToMeasure){
  // This is how you declare a variable which
  //  will hold the PORT, PIN, and DDR registers
  //  on an AVR
  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;
  // Here we translate the input pin number from
  //  Arduino pin number to the AVR PORT, PIN, DDR,
  //  and which bit of those registers we care about.
  byte bitmask;
  if ((pinToMeasure >= 0) && (pinToMeasure <= 7)){
    port = &PORTD;
    ddr = &DDRD;
    bitmask = 1 << pinToMeasure;
    pin = &PIND;
  }
  if ((pinToMeasure > 7) && (pinToMeasure <= 13)){
    port = &PORTB;
    ddr = &DDRB;
    bitmask = 1 << (pinToMeasure - 8);
    pin = &PINB;
  }
  if ((pinToMeasure > 13) && (pinToMeasure <= 19)){
    port = &PORTC;
    ddr = &DDRC;
    bitmask = 1 << (pinToMeasure - 13);
    pin = &PINC;
  }
  // Discharge the pin first by setting it low and output
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  delay(1);
  // Make the pin an input WITHOUT the internal pull-up on
  *ddr &= ~(bitmask);
  // Now see how long the pin to get pulled up
  int cycles = 16000;
  for(int i = 0; i < cycles; i++){
    if (*pin & bitmask){
      cycles = i;
      break;
    }
  }
  // Discharge the pin again by setting it low and output
  //  It's important to leave the pins low if you want to 
  //  be able to touch more than 1 sensor at a time - if
  //  the sensor is left pulled high, when you touch
  //  two sensors, your body will transfer the charge between
  //  sensors.
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  
  return cycles;
}
