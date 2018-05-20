// www.Electrosmash.com/pedalshield-mega
// pedalshield_mega_fuzz.ino reads the ADC signal and clips the signal depending on the variable fuzz_threshold
// pressing the pushbutton_1 or 2 turns the distortion harder or softer.
 
#include "U8glib.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send ACK
 
//defining hardware resources.
#define LED 13
#define FOOTSWITCH 12
#define TOGGLE 2
#define PUSHBUTTON_1 A5
#define PUSHBUTTON_2 A4

//defining the output PWM parameters
#define PWM_FREQ 0x00FF // pwm frequency - 31.3KHz
#define PWM_MODE 0 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // 2 PWMs in parallel

//other variables
int input, fuzz_threshold=6000; //initial value adjusted by try and error.
int counter=0;
unsigned int ADC_low, ADC_high;
 
void setup() {
 
  //setup IO
  pinMode(FOOTSWITCH, INPUT_PULLUP);
  pinMode(TOGGLE, INPUT_PULLUP);
  pinMode(PUSHBUTTON_1, INPUT_PULLUP);
  pinMode(PUSHBUTTON_2, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(6, OUTPUT); //PWM0 as output
  pinMode(7, OUTPUT); //PWM1 as output
 
  // setup ADC
  ADMUX = 0x60; // left adjust, adc0, internal vcc
  ADCSRA = 0xe5; // turn on adc, ck/32, auto trigger
  ADCSRB = 0x00; // ADC free running mode
  DIDR0 = 0x01; // turn off digital inputs for adc0
 
  // setup PWM
  TCCR4A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); //
  TCCR4B = ((PWM_MODE << 3) | 0x11); // ck/1
  TIMSK4 = 0x20; // interrupt on capture interrupt
  ICR4H = (PWM_FREQ >> 8);
  ICR4L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // turn on outputs
  sei(); // turn on interrupts - not really necessary with arduino
  }
 
void loop() 
{
  //Turn on the LED and write the OLED if the effect is ON.
  if (digitalRead(FOOTSWITCH)) 
  {
  digitalWrite(LED, HIGH); //light the LED
    u8g.firstPage(); 
 do {
   u8g.setFont(u8g_font_helvR18r);
   u8g.drawStr( 0, 30, "    FUZZ");    
   u8g.setPrintPos(30, 64); 
   u8g.setFont(u8g_font_helvR18r); 
   u8g.print(fuzz_threshold);
  } while( u8g.nextPage() );
  }
    else  
  {  
    digitalWrite(LED, LOW); // switch-off the LED
    u8g.firstPage(); 
    do {
        u8g.setFont(u8g_font_helvR24r);
        u8g.drawStr( 0, 30, "EFFECT");    
        u8g.drawStr( 0, 60, "   OFF ");   
        } while( u8g.nextPage() );
  }
}
ISR(TIMER4_CAPT_vect) 
{
  // get ADC data
  ADC_low = ADCL; // you need to fetch the low byte first
  ADC_high = ADCH;
  //construct the input sumple summing the ADC low and high byte.
  input = ((ADC_high << 8) | ADC_low) + 0x8000; // make a signed 16b value
 
counter++; //to save resources, the pushbuttons are checked every 1000 times.
if(counter==1000)
{ 
counter=0;
if (!digitalRead(PUSHBUTTON_2)) {
  if (fuzz_threshold<32768)fuzz_threshold=fuzz_threshold+25; //increase the vol
    digitalWrite(LED, LOW); //blinks the led
    }
 
    if (!digitalRead(PUSHBUTTON_1)) {
  if (fuzz_threshold>0)fuzz_threshold=fuzz_threshold-25; //decrease vol
  digitalWrite(LED, LOW); //blinks the led
    }
}
 
  //the input signal is 16bits (values from -32768 to +32768
  //the value is clipped to the fuzz_threshold value
    if(input>fuzz_threshold) input=32768;
    else if(input<-fuzz_threshold) input=-32768;
 
  //write the PWM signal
  OCR4AL = ((input + 0x8000) >> 8); // convert to unsigned, send out high byte
  OCR4BL = input; // send out low byte
}
 
