/* ***************************************************************************** */
/* You can use this file to define the low-level hardware control fcts for       */
/* LED, button and LCD devices.                                                  */
/* Note that these need to be implemented in Assembler.                          */
/* You can use inline Assembler code, or use a stand-alone Assembler file.       */
/* Alternatively, you can implement all fcts directly in master-mind.c,          */
/* using inline Assembler code there.                                            */
/* The Makefile assumes you define the functions here.                           */
/* ***************************************************************************** */


#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif

#define	PAGE_SIZE		(4*1024)
#define	BLOCK_SIZE		(4*1024)

#define	INPUT			 0
#define	OUTPUT			 1

#define	LOW			 0
#define	HIGH			 1


// APP constants   ---------------------------------

// Wiring (see call to lcdInit in main, using BCM numbering)
// NB: this needs to match the wiring as defined in master-mind.c

#define STRB_PIN 24
#define RS_PIN   25
#define DATA0_PIN 23
#define DATA1_PIN 10
#define DATA2_PIN 27
#define DATA3_PIN 22

// -----------------------------------------------------------------------------
// includes 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

// -----------------------------------------------------------------------------
// prototypes

int failure (int fatal, const char *message, ...);

// -----------------------------------------------------------------------------
// Functions to implement here (or directly in master-mind.c)

/* this version needs gpio as argument, because it is in a separate file */
void digitalWrite (volatile uint32_t *gpio, int pin, int value){
  //offset variable
  int off;
  //get offset
  if(value == HIGH){
    off = 7;
  } 
  else{
    off = 10;
  } 
  // assign to memory address
  *(gpio + off) = 1 << (pin & 31);
}

// adapted from setPinMode
void pinMode(volatile uint32_t *gpio, int pin, int mode){
  //get shift
  int shift =3 * (pin % 10);
  //get fsel register
  int fsel = pin/10;
  //if statement depends on mode 1 = output , 0 = input
  if(mode == 1) *(gpio + fsel) = (*(gpio + fsel) & ~(7 << shift)) | (1 << shift);
  else *(gpio + fsel) = *(gpio + fsel) & ~(7 << shift);
}

void writeLED(uint32_t *gpio, int led, int value){
  //just uses digital write
  digitalWrite(gpio, led, value);
}

int readButton(volatile uint32_t *gpio, int button){
  //read the memory address and return
  return (*(gpio + 13) & (1 << (button & 31)));
}

void waitForButton (uint32_t *gpio, int button){
  // waits for value to not be zero
  while( (*(gpio + 13) & (1 << (button & 31))) == 0);
}
