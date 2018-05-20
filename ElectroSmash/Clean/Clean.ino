// www.Electrosmash.com/rights CC-by-NC
// Based on OpenMusicLabs previous works.
// pedalshield_mega_clean_OLED.ino reads the ADC and writes the PWMs
 
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
int input;
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
  ADCSRB = 0x00; //free running mode
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
   u8g.setFont(u8g_font_helvR12r);
   u8g.drawStr( 0, 25, " CLEAN EFFECT ");    
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
 
//// No Digital Signal Processing beacuse is a CLEAN effect ////
 
  //write the PWM signal
  OCR4AL = ((input + 0x8000) >> 8); // convert to unsigned, send out high byte
  OCR4BL = input; // send out low byte
 
}
