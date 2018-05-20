// CC-by-NC-www.Electrosmash.com/rights
// Based on OpenMusicLabs previous works.
// pedalshield_mega_delay.ino reads the ADC, saves it into the DelayBuffer[] array and plays it into the PWM output.
// pressing the pushbutton_1 or 2 makes the delay longer or shorter
 
#include "U8glib.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send ACK
 
//defining harware resources.
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
int input, vol_variable=512;
int counter=0;
unsigned int ADC_low, ADC_high;
 
#define MAX_DELAY 7500
byte DelayBuffer[MAX_DELAY];
unsigned int DelayCounter = 0;
unsigned int Delay_Depth = MAX_DELAY;
 
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
  ADCSRB = 0x00; // free running mode
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
   u8g.setFont(u8g_font_helvR14r);
   u8g.drawStr( 0, 16, "  ECHO  ");    
   u8g.setPrintPos(10, 50); 
   u8g.setFont(u8g_font_helvR24r); 
   u8g.print(Delay_Depth);
  } while( u8g.nextPage() );
  }
    else  
  {  
    digitalWrite(LED, LOW); // switch-off the LED
    u8g.firstPage(); 
    do {
        u8g.setFont(u8g_font_helvR24r);
        u8g.drawStr( 0, 32, "EFFECT ");    
        u8g.drawStr( 0, 64, "   OFF  ");    
        } while( u8g.nextPage() );
  }
}
 
 
ISR(TIMER4_CAPT_vect) 
{
  // get ADC data
  ADC_low = ADCL; // ADC_low always 0 to save space
  ADC_high = ADCH;
 
  //store the high bit only for 
  DelayBuffer[DelayCounter] = (ADC_high + DelayBuffer[DelayCounter])>>1 ; 
 
 
counter++; //to save resources, the pushbuttons are checked every 100 times.
if(counter==100)
{ 
counter=0;
if (!digitalRead(PUSHBUTTON_1)) {
  if (Delay_Depth<MAX_DELAY)Delay_Depth=Delay_Depth+1; //increase the vol
    digitalWrite(LED, LOW); //blinks the led
    }
 
    if (!digitalRead(PUSHBUTTON_2)) {
  if (Delay_Depth>0)Delay_Depth=Delay_Depth-1; //decrease vol
  digitalWrite(LED, LOW); //blinks the led
    }
}
 
  //Increse/reset delay counter.   
  DelayCounter++;
  if(DelayCounter >= Delay_Depth) DelayCounter = 0; 
 
  //the output signal is = (delayed signal + current signal)/2
  input = (DelayBuffer[DelayCounter] +(ADC_high))>>1;
 
  //write the PWM signal
  OCR4AL = input;
  //OCR4BL = input; // send out low byte
 
}
