/*

 The code for controlling the VS1053 was made by Marc "Trench" Tschudin, his full header is below. 
 There is also code by me (jonathan Whitaker) and a capacitive sense function that belongs to someone, somewhere. 
 All of this code is public domain - enjoy:)
 
 Marcs header:
 2-12-2011
 Spark Fun Electronics 2011
 Nathan Seidle
 Updated to Arduino 1.01 by Marc "Trench" Tschudin
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 This code works with the VS1053 Breakout Board and controls the VS1053 in what is called Real Time MIDI mode. 
 To get the VS1053 into RT MIDI mode, power up the VS1053 breakout board with GPIO0 tied low, GPIO1 tied high.
 
 I use the NewSoftSerial library to send out the MIDI serial at 31250bps. This allows me to print regular messages
 for debugging to the terminal window. This helped me out a ton.
 
 5V : VS1053 VCC
 GND : VS1053 GND
 D3 (SoftSerial TX) : VS1053 RX
 D4 : VS1053 RESET
 
 Attach a headphone breakout board to the VS1053:
 VS1053 LEFT : TSH
 VS1053 RIGHT : RSH
 VS1053 GBUF : GND
 
 When in the drum bank (0x78), there are not different instruments, only different notes.
 To play the different sounds, select an instrument # like 5, then play notes 27 to 87.
 
 To play "Sticks" (31):
 talkMIDI(0xB0, 0, 0x78); //Bank select: drums
 talkMIDI(0xC0, 5, 0); //Set instrument number
 //Play note on channel 1 (0x90), some note value (note), middle velocity (60):
 noteOn(0, 31, 60);
 
 */

#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX

byte note = 0; //The MIDI note value to be played
byte resetMIDI = 4; //Tied to VS1053 Reset line
byte ledPin = A1; //MIDI traffic inidicator
int  instrument = 0;

int prev_values[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int prev_prev_values[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int pins[] = {2, 5, 6, 7, 8, 9, 11, 12, 13};
int notes[] = {60, 62, 64, 65, 67, 69, 71, 72};
int note_states[] = {0, 0, 0, 0, 0, 0, 0, 0};
int pin_thresholds[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int pincount = 8;

void setup() {
  Serial.begin(57600);

  //Setup soft serial for MIDI control
  mySerial.begin(31250);
  
  //voltage to the capsense pins, not ideal but it works. Try to use the +5V if you can though
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
  
  //get the thresholds for capacitive sensing
  for (int i = 0; i < pincount; i ++){
    pin_thresholds[i] = readCapacitivePin(pins[i])+10;
  }

  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127)
  talkMIDI(0xB0, 0, 0x00);
  talkMIDI(0xC0, instrument, 0);
}

void loop() {
  
  for (int i = 0; i < pincount; i ++){
    if (readCapacitivePin(pins[i])>pin_thresholds[i]){
      if ((prev_values[i] == 1) && (prev_prev_values[i] == 0)){
        if (note_states[i] == 0){
          noteOn(0, notes[i], 80);
          Serial.println("noteon");
          note_states[i] = 1;
        }
      }
      prev_prev_values[i] = prev_values[i];
      prev_values[i] = 1; 
    }
    else {
      if ((prev_values[i] == 0) && (prev_prev_values[i] == 1)){
        if (note_states[i] == 1){
          noteOff(0, notes[i], 80);
          Serial.println("noteoff");
          note_states[i] = 0;
        }
      }
      prev_prev_values[i] = prev_values[i];
      prev_values[i] = 0;
    }
  }
  
  //change instruments
  //drums
  if ((prev_values[3] == 1) &&(prev_values[6] == 1)){
    talkMIDI(0xB0, 0, 0x78); //Bank select drums
    Serial.println("Changing to drums");
    talkMIDI(0xC0, instrument, 0);
  }
  //cycle through ordinary instruments
  if ((prev_values[0] == 1) &&(prev_values[6] == 1)){
    talkMIDI(0xB0, 0, 0x00); //Default bank GM1
    if (instrument < 127){
      instrument ++;
      
    }
    else {
      instrument = 0;
    }
    talkMIDI(0xC0, instrument, 0);
  }
  
}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

  digitalWrite(ledPin, LOW);
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
