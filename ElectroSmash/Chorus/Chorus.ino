// CC-by-NC-www.Electrosmash.com/rights
// Based on OpenMusicLabs previous works.
// pedalshield_mega_chorus.ino the ADC read signal is delayed with a triangular LFO and mixed with the clean guitar signal.
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
int input, output, p, vol_variable=512;
int counter=0;
int count_up=0;
unsigned int ADC_low, ADC_high;
 
unsigned int max_delay=100;
unsigned int MIN_DELAY=10;
unsigned int DelayBuffer[1000];
unsigned int DelayCounter = 0;
unsigned int Delay_Depth = 100;
unsigned int flanger_speed = 5;
 
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
   u8g.drawStr( 0, 20, "  CHORUS  ");    
   u8g.setFont(u8g_font_helvR24r); 
   u8g.setPrintPos(10, 60); 
   u8g.print(max_delay);
   u8g.setFont(u8g_font_helvR14r); 
   u8g.setPrintPos(100, 60); 
   u8g.print(count_up);
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
 
  //store the current reading
  DelayBuffer[DelayCounter] = (((ADC_high << 8) | ADC_low) + 0x8000);
 
counter++; //to save resources, the pushbuttons are checked every 100 times.
 
if(counter==1000)
{ 
    counter=0;
    if (!digitalRead(PUSHBUTTON_2)) if (max_delay<1000)max_delay++; //increase the vol
    if (!digitalRead(PUSHBUTTON_1)) if (max_delay>20)max_delay--; //decrease delay
}
 
 
DelayCounter++;
  if(DelayCounter >= Delay_Depth) 
  {
    DelayCounter = 0; 
    if(count_up)
    {
       digitalWrite(LED, HIGH);
       for(p=0;p<10;p++)DelayBuffer[Delay_Depth+p]=DelayBuffer[Delay_Depth-1]; 
       Delay_Depth=Delay_Depth+1;
 
       if (Delay_Depth>=max_delay)count_up=0;
    }
    else
    {
       digitalWrite(LED, LOW); 
       Delay_Depth=Delay_Depth-1;
       if (Delay_Depth<=MIN_DELAY)count_up=1;
    }
  }
 
 // output = DelayBuffer[DelayCounter];
  output = (DelayBuffer[DelayCounter] + (((ADC_high << 8) | ADC_low) + 0x8000))>>1 ; //chorus
  //write the PWM signal
  OCR4AL = ((output + 0x8000) >> 8); // convert to unsigned, send out high byte
  OCR4BL = output; // send out low byte
 
}
