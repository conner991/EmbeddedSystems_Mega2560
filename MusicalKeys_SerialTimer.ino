
#include <arduino.h>

// Prototypes
void myDelay(unsigned int myFreq);

// Initialze Pointers to Registers
volatile unsigned char *myTCCR1A = (unsigned char*)0x80;
volatile unsigned char *myTCCR1B = (unsigned char*)0x81;
volatile unsigned char *myTCCR1C = (unsigned char*)0x82;
volatile unsigned char *myTIMSK1 = (unsigned char*)0x6F;
volatile unsigned int *myTCNT1H = (unsigned int*)0x85;
volatile unsigned int *myTCNT1L = (unsigned int*)0x84;
volatile unsigned char *myTIFR1 = (unsigned char*)0x36;
volatile unsigned char *portDDRB = (unsigned char*)0x24;
volatile unsigned char *portB = (unsigned char*)0x25;

unsigned char inputs[12] = {'A', 'a', 'B', 'C', 'c', 'D', 'd', 'E', 'F', 'f', 'G', 'g'}; 
unsigned int freq[12] = {440, 466, 494, 523, 554, 587, 624, 659, 698, 740, 784, 831};
unsigned int myFreq = 0;
bool quit = 0;

#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])

void setup() 
{
   // setup the UART
  Serial.begin(9600);

  // Set Port B to output, 0x40 = 0b0100 0000
  *portDDRB |= 0x40;
  // Initialize PB6 to low, 0xBF = 0b1011 1111
  *portB &= 0xBF;
  // Init timer control registers
  *myTCCR1A = 0x00;
  *myTCCR1B = 0x00;
  *myTCCR1C = 0x00;
}

void loop(){
 
  // if the UART received a character
  if(Serial.available())
  {
    
      // read in the character
      unsigned char in_char = Serial.read();
  
      if (in_char == 'q')
      {
         // Silence
         Serial.write(in_char);
         quit = 1;
         *portB &= 0xBF;
         myFreq = 0; 
      }
  
      else
      {
          // write the character back to the Serial Port
          for (int i = 0; i < ARRAY_SIZE(inputs); i++)
          {
               if (in_char == inputs[i])
               {
                  Serial.write(in_char);
                  quit = 0;
                  myFreq = freq[i];
                  
               }
             
          }
          
      }
  }

  if (!quit)
  {
    // set pb.6 to 1
    *portB |= 0x40;
    // call delay 50% DC 
    myDelay(myFreq);
    // set pb.6 to 0
    *portB &= 0xBF;
    // call delay 50% DC
    myDelay(myFreq);
  }
  
  
  
  // q function exit, when q set freq to 0
  // freq should start at 0 too 
 
}

void myDelay(unsigned int freq)
{
  // calc period
  double period = 1.0/double(freq);
  // 50% duty cycle
  double half_period = period/ 2.0f;
  // clock period def
  double clk_period = 0.0000000625;
  // calc ticks
  unsigned int ticks = half_period / clk_period;
  // This stops the timer
  *myTCCR1B &= 0xF8; // 0xF8 = 0b1111 1000
  *myTCNT1L = (unsigned int)(65536 - ticks);
  *myTCCR1B |= 0x01; // Normal Mode, no prescalar, start the timer
  // Wait for TOV1 to roll over, wait for overflow
  while ((*myTIFR1 & 0x01) == 0)
    ;
  // Stop the timer
  *myTCCR1B &= 0xF8; // 0b1111 1000
  // clear TOV1, reset it, write a 1 to reset to 0
  *myTIFR1 |= 0x01;
}
