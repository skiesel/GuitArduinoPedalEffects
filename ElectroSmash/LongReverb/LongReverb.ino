// CC-by-NC-www.Electrosmash.com/rights
// pedalshield_mega_reverb.ino uses 3 Delay Buffers to mimic the reflexion from 3 walls/
// pressing the pushbutton_1 or 2 makes the reverb longer or shorter, changing the "distance" of the walls
 
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
 
#define MAX_DELAY1  6100
#define MAX_DELAY2  1100
#define MAX_DELAY3   555
byte DelayBuffer1[MAX_DELAY1];
byte DelayBuffer2[MAX_DELAY2];
byte DelayBuffer3[MAX_DELAY3];
unsigned int DelayCounter1 = 0;
unsigned int DelayCounter2 = 0;
unsigned int DelayCounter3 = 0;
unsigned int Delay_Depth1 = MAX_DELAY1;
unsigned int Delay_Depth2 = MAX_DELAY2;
unsigned int Delay_Depth3 = MAX_DELAY3;
 
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
    //Turn on the LED if the effect is ON.
  if (digitalRead(FOOTSWITCH)) digitalWrite(LED, HIGH); 
     else  digitalWrite(LED, LOW);
 
  //write the OLED while waiting for the Timer 4 interruption.
  u8g.firstPage(); 
 do {
   u8g.setFont(u8g_font_helvR14r);
   u8g.drawStr( 0, 14, "LONGREVERB"); 
 
    u8g.drawStr( 0, 32, "ECHO 1: ");   
    u8g.setPrintPos(80, 32);
    u8g.print(Delay_Depth1);
 
    u8g.drawStr( 0, 48, "ECHO 2: "); 
    u8g.setPrintPos(80, 48);
    u8g.print(Delay_Depth2);
 
    u8g.drawStr( 0, 64, "ECHO 3: "); 
    u8g.setPrintPos(80, 64);
    u8g.print(Delay_Depth3);
 
  } while( u8g.nextPage() );
}
 
ISR(TIMER4_CAPT_vect) 
{
  // get ADC data
  ADC_low = ADCL; // ADC_low always 0 to save space
  ADC_high = ADCH;
 
  //store the high bit only for 
  DelayBuffer1[DelayCounter1] = (ADC_high+DelayBuffer1[DelayCounter1])>>1;
  DelayBuffer2[DelayCounter2] = (ADC_high+DelayBuffer1[DelayCounter2])>>1;
  DelayBuffer3[DelayCounter3] = (ADC_high+DelayBuffer1[DelayCounter3])>>1;
 
counter++; //to save resources, the pushbuttons are checked every 100 times.
if(counter==100)
{ 
counter=0;
if (!digitalRead(PUSHBUTTON_1)) {
  if (Delay_Depth1<MAX_DELAY1)Delay_Depth1=Delay_Depth1+1; 
  if (Delay_Depth2<MAX_DELAY2)Delay_Depth2=Delay_Depth2+1; 
  if (Delay_Depth3<MAX_DELAY3)Delay_Depth3=Delay_Depth3+1; 
    digitalWrite(LED, LOW); //blinks the led
    }
 
    if (!digitalRead(PUSHBUTTON_2)) {
  if (Delay_Depth1>0)Delay_Depth1=Delay_Depth1-1; 
  if (Delay_Depth2>0)Delay_Depth2=Delay_Depth2-1; 
  if (Delay_Depth3>0)Delay_Depth3=Delay_Depth3-1; 
  digitalWrite(LED, LOW); //blinks the led
    }
}
 
//Increse/reset delay counter.   
  DelayCounter1++;
  if(DelayCounter1 >= Delay_Depth1) DelayCounter1 = 0; 
  DelayCounter2++;
  if(DelayCounter2 >= Delay_Depth2) DelayCounter2 = 0; 
  DelayCounter3++;
  if(DelayCounter3 >= Delay_Depth3) DelayCounter3 = 0; 
 
//the output signal is = delayed signal + current signal.
  input = (DelayBuffer1[DelayCounter1]+DelayBuffer2[DelayCounter2]+DelayBuffer3[DelayCounter3]+ADC_high)>>2;
 
  //write the PWM signal
  OCR4AL = input ; // convert to unsigned, send out high byte
 // OCR4BL = input; // send out low byte
 
}
