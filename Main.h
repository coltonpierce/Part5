//***********************************      Main.h      ***********************************//

/*------------------------------------------------------------------*-

   Main.H (v1.00)

  ------------------------------------------------------------------
   
   'Project Header' (see Chap 5).


   COPYRIGHT
   ---------

   This code is associated with the book:

   EMBEDDED C by Michael J. Pont 
   [Pearson Education, 2002: ISBN: 0-201-79523-X].

   This code is copyright (c) 2001 by Michael J. Pont.
 
   See book for copyright details and other information.

-*------------------------------------------------------------------*/
#ifndef _Main_H
#define _Main_H

//------------------------------------------------------------------
// WILL NEED TO EDIT THIS SECTION FOR EVERY PROJECT
//------------------------------------------------------------------

// Must include the appropriate microcontroller header file here
#include "AT89C51RC2.h"

// Oscillator / resonator frequency (in Hz) e.g. (11059200UL)
#define OSC_FREQ (18432000UL)

// Number of oscillations per instruction (12, etc)
// 12 - Original 8051 / 8052 and numerous modern versions
//  6 - Various Infineon and Philips devices, etc.
//  4 - Dallas 320, 520 etc.
//  1 - Dallas 420, etc.
#define OSC_PER_INST (6)

//------------------------------------------------------------------
// SHOULD NOT NEED TO EDIT THE SECTIONS BELOW
//------------------------------------------------------------------

// Typedefs (see Chap 5)  
typedef unsigned char uint8_t;
typedef unsigned int  uint16_t;
typedef unsigned long uint32_t;





#define T2_100US 110 // 11 ms

#define TIMER2HL (65536- (uint16_t) (OSC_FREQ*T2_100US)/(OSC_PER_INST*10000)) // Value needed to run timer for 11 ms for timer 2

#define TIMER0HL (65536- (uint16_t) ((OSC_FREQ*(T2_100US-10))/(OSC_PER_INST*10000))) // Value needed to run timer for 11 ms for timer 2

#define TIMER2H TIMER2HL / 256
#define TIMER2L TIMER2HL % 256

#define TIMER0H TIMER0HL / 256
#define TIMER0L TIMER0HL % 256

#define ACTIVE						0
#define INACTIVE					1

#define LOAD_BUFFER_1			1
#define LOAD_BUFFER_2			2
#define FIND_CLUSTER_1		3
#define FIND_CLUSTER_2		4
#define DATA_SEND_1				5
#define DATA_SEND_2				6
#define DATA_IDLE_1				7
#define DATA_IDLE_2				8
#define PAUSE							9

// Interrupts (see Chap 7)  
#define Timer_0_Overflow 1
#define Timer_1_Overflow 3
#define Timer_2_Overflow 5

#endif

/*------------------------------------------------------------------*-
  ---- END OF FILE -------------------------------------------------
-*------------------------------------------------------------------*/
