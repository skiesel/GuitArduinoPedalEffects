int counter=0;
unsigned int ADC_low, ADC_high, effect_type=1;
 
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
   /****SELECTION MODE*****/
//enter in "selection effect mode", the program stays in this loop while the toggle switch is down
// you can change effect pushing buttons, the number of blinks will indicate the effect number
 
 
digitalWrite(LED, LOW); // switch-off the LED
 
while (digitalRead(TOGGLE) && digitalRead(FOOTSWITCH))
    { 
      digitalWrite(LED, HIGH); //light the LED
 
    if (!digitalRead(PUSHBUTTON_2))
        { 
         if (effect_type<4) effect_type++;
        }
        else if (!digitalRead(PUSHBUTTON_1))
        {
            if (effect_type>0) effect_type--;
        }
 
    //Write the OLED screen acording with the effect number
    switch(effect_type)
    {
    case 1: //1st effect
    u8g.firstPage();
    do {
        u8g.setFont(u8g_font_helvR14r);
        u8g.drawStr( 0, 16, " SELECTION");    
        u8g.drawStr( 0, 50, "     DELAY    ");
        } while( u8g.nextPage() );
    break;
 
    case 2: //2nd effect
    u8g.firstPage();
    do {
        u8g.setFont(u8g_font_helvR14r);
        u8g.drawStr( 0, 16, " SELECTION");    
        u8g.drawStr( 0, 50, "DISTORTION");
        } while( u8g.nextPage() );
    break;
 
    case 3: //3rd effect
    u8g.firstPage();
     do {
        u8g.setFont(u8g_font_helvR14r);
        u8g.drawStr( 0, 16, " SELECTION");    
        u8g.drawStr( 0, 50, "FUZZ");
        } while( u8g.nextPage() );
    break;
 
    case 4: //4th effect
    u8g.firstPage();
    do {
        u8g.setFont(u8g_font_helvR14r);
        u8g.drawStr( 0, 16, " SELECTION");    
        u8g.drawStr( 0, 50, "BIT CRUSHER");
        } while( u8g.nextPage() );
    break;
    }
  }    
 
    /****NORMAL MODE*****/
  //Turn on the LED and write the OLED if the effect is ON.
  if (digitalRead(FOOTSWITCH)) 
  {
 digitalWrite(LED, HIGH); //light the LED
    switch(effect_type)
    {
    case 1: //1st effect
    u8g.firstPage();
    do {
        u8g.setFont(u8g_font_helvR14r);
        u8g.drawStr( 0, 16, "  DELAY  ");    
        u8g.setPrintPos(10, 50); 
        u8g.setFont(u8g_font_helvR24r); 
        u8g.print(vol_variable);
        } while( u8g.nextPage() );
    break;
 
    case 2: //2nd effect
    u8g.firstPage();
    do {
        u8g.setFont(u8g_font_helvR14r);
        u8g.drawStr( 0, 16, "DISTORTION");    
        u8g.setPrintPos(10, 50); 
        u8g.setFont(u8g_font_helvR24r); 
        u8g.print(distortion_threshold);
        } while( u8g.nextPage() );
    break;
 
    case 3: //3rd effect
    u8g.firstPage();
     do {
        u8g.setFont(u8g_font_helvR14r);
        u8g.drawStr( 0, 16, "FUZZ");    
        u8g.setPrintPos(10, 50); 
        u8g.setFont(u8g_font_helvR24r); 
        u8g.print(distortion_threshold);
        } while( u8g.nextPage() );
    break;
 
    case 4: //4th effect
    u8g.firstPage();
    do {
        u8g.setFont(u8g_font_helvR14r);
        u8g.drawStr( 0, 16, "BIT CRUSHER");    
        u8g.setPrintPos(10, 50); 
        u8g.setFont(u8g_font_helvR24r); 
        u8g.print(bit_crush_variable);
        } while( u8g.nextPage() );
    break;
    } 
  }
    else  
  {  
   // digitalWrite(LED, LOW); // switch-off the LED
    u8g.firstPage(); 
    do {
        u8g.setFont(u8g_font_helvR24r);
        u8g.drawStr( 0, 30, "EFFECT");    
        u8g.drawStr( 0, 60, "  OFF ");   
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
if(counter==2000)
{ 
counter=0;
if (!digitalRead(PUSHBUTTON_2)) {
    if (vol_variable<32768)vol_variable=vol_variable+10; //increase the vol
    if (distortion_threshold<32768)distortion_threshold=distortion_threshold+25; //increase the vol
    if (bit_crush_variable<16)bit_crush_variable=bit_crush_variable+1; //increase the number of crushed bits
    if (Delay_Depth<MAX_DELAY)Delay_Depth=Delay_Depth+1; //increase delay
    digitalWrite(LED, LOW); //blinks the led
    }
 
    if (!digitalRead(PUSHBUTTON_1)) {
    if (vol_variable>0)vol_variable=vol_variable-10; //decrease vol
    if (distortion_threshold>0)distortion_threshold=distortion_threshold-25; //decrease vol
    if (bit_crush_variable>0)bit_crush_variable=bit_crush_variable-1; //decrease the number of crushed bits
    if (Delay_Depth>0)Delay_Depth=Delay_Depth-1; //decrease delay
  digitalWrite(LED, LOW); //blinks the led
    }
}
 
 
    //**** EFFECT AREA***///
    // The effects are selected depending on effect type.
    switch(effect_type)
    {
    case 1:
    /**** DELAY EFFECT ***/
    //The input_signal is stored and played using a circular buffer
    Delay_Buffer[DelayCounter] = (((ADC_high << 8) | ADC_low) + 0x8000);
    DelayCounter++;
    if(DelayCounter >= Delay_Depth) DelayCounter = 0;
    output = (Delay_Buffer[DelayCounter] +(((ADC_high << 8) | ADC_low) + 0x8000))>>1;
    break;
 
    case 2:
    /**** DISTORTION EFFECT ***/
    if(input>distortion_threshold) output=distortion_threshold;
    else output=input;
    break;
 
    case 3:
    /**** FUZZ EFFECT ***/
    if(input>distortion_threshold) output=32768;
    else if(input<-distortion_threshold) output=-32768;
    else output=input;
    break;
 
    case 4:
    /**** BIT CRUSHER EFFECT ***/
    output = input<<bit_crush_variable;
    break;
 
     case 5://echo
    /**** ECHO EFFECT ***/
    Delay_Buffer[DelayCounter]  = (input + Delay_Buffer[DelayCounter])>>1;
    DelayCounter++;    
    if(DelayCounter >= Delay_Depth) DelayCounter = 0;
    output = (input + (Delay_Buffer[DelayCounter]))>>1;
 
    break;
    }
 
  //write the PWM signal
  OCR4AL = ((output + 0x8000) >> 8); // convert to unsigned, send out high byte
  OCR4BL = output; // send out low byte
}
 

