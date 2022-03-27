/*
 * MasterMind implementation: template; see comments below on which parts need to be completed
 * CW spec: https://www.macs.hw.ac.uk/~hwloidl/Courses/F28HS/F28HS_CW2_2022.pdf
 * This repo: https://gitlab-student.macs.hw.ac.uk/f28hs-2021-22/f28hs-2021-22-staff/f28hs-2021-22-cwk2-sys

 * Compile: 
 gcc -c -o lcdBinary.o lcdBinary.c
 gcc -c -o master-mind.o master-mind.c
 gcc -o master-mind master-mind.o lcdBinary.o
 * Run:     
 sudo ./master-mind

 OR use the Makefile to build
 > make all
 and run
 > make run
 and test
 > make test

 ***********************************************************************
 * The Low-level interface to LED, button, and LCD is based on:
 * wiringPi libraries by
 * Copyright (c) 2012-2013 Gordon Henderson.
 ***********************************************************************
 * See:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
*/

/* ======================================================= */
/* SECTION: includes                                       */
/* ------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

/* --------------------------------------------------------------------------- */
/* Config settings */
/* you can use CPP flags to e.g. print extra debugging messages */
/* or switch between different versions of the code e.g. digitalWrite() in Assembler */
#define DEBUG
#undef ASM_CODE

// =======================================================
// Tunables
// PINs (based on BCM numbering)
// For wiring see CW spec: https://www.macs.hw.ac.uk/~hwloidl/Courses/F28HS/F28HS_CW2_2022.pdf
// GPIO pin for green LED
#define LED 13
// GPIO pin for red LED
#define LED2 5
// GPIO pin for button
#define BUTTON 19

// =======================================================
// delay for loop iterations (mainly), in ms
// in mili-seconds: 0.4s
#define DELAY   400 
// in micro-seconds: 3.5s
#define TIMEOUT 3500000
// =======================================================
// APP constants   ---------------------------------
// number of colours and length of the sequence
#define COLS 3
#define SEQL 3
// =======================================================

// generic constants

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

/* Constants */ 
static const int colors = COLS;
static const int seqlen = SEQL;

static char* color_names[] = { "red", "green", "blue" };

static int* theSeq = NULL;

static int *seq1, *seq2, *cpy1, *cpy2;

/* --------------------------------------------------------------------------- */

// Mask for the bottom 64 pins which belong to the Raspberry Pi
//	The others are available for the other devices
#define	PI_GPIO_MASK	(0xFFFFFFC0)

static unsigned int gpiobase ;
static uint32_t *gpio ;

static int timed_out = 0;

/* ------------------------------------------------------- */
// misc prototypes

int failure (int fatal, const char *message, ...);
void waitForEnter (void);
void waitForButton (uint32_t *gpio, int button);

/* ======================================================= */
/* SECTION: hardware interface (LED, button, LCD display)  */
/* ------------------------------------------------------- */
/* low-level interface to the hardware */

/* ********************************************************** */
/* COMPLETE the code for all of the functions in this SECTION */
/* Either put them in a separate file, lcdBinary.c, and use   */
/* inline Assembler there, or use a standalone Assembler file */
/* You can also directly implement them here (inline Asm).    */
/* ********************************************************** */

/* These are just prototypes; you need to complete the code for each function */

/* send a @value@ (LOW or HIGH) on pin number @pin@; @gpio@ is the mmaped GPIO base address */
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

/* set the @mode@ of a GPIO @pin@ to INPUT or OUTPUT; @gpio@ is the mmaped GPIO base address */
void pinMode(volatile uint32_t *gpio, int pin, int mode){
  //get shift
  int shift =3 * (pin % 10);
  //get fsel register
  int fsel = pin/10;
  //if statement depends on mode 1 = output , 0 = input
  if(mode == 1) *(gpio + fsel) = (*(gpio + fsel) & ~(7 << shift)) | (1 << shift);
  else *(gpio + fsel) = *(gpio + fsel) & ~(7 << shift);
}

/* send a @value@ (LOW or HIGH) on pin number @pin@; @gpio@ is the mmaped GPIO base address */
/* can use digitalWrite(), depending on your implementation */
void writeLED(uint32_t *gpio, int led, int value){
  //just uses digital write
  digitalWrite(gpio, led, value);
}

/* read a @value@ (LOW or HIGH) from pin number @pin@ (a button device); @gpio@ is the mmaped GPIO base address */
int readButton(volatile uint32_t *gpio, int button){
  //read the memory address and return
  return (*(gpio + 13) & (1 << (button & 31)));
}

/* wait for a button input on pin number @button@; @gpio@ is the mmaped GPIO base address */
/* can use readButton(), depending on your implementation */
void waitForButton (uint32_t *gpio, int button){
  // waits for value to not be zero
  while( (*(gpio + 13) & (1 << (button & 31))) == 0);
}

/* ======================================================= */
/* SECTION: game logic                                     */
/* ------------------------------------------------------- */
/* AUX fcts of the game logic */

/* ********************************************************** */
/* COMPLETE the code for all of the functions in this SECTION */
/* Implement these as C functions in this file                */
/* ********************************************************** */

/* initialise the secret sequence; by default it should be a random sequence */
void initSeq() {
  /* ***  COMPLETE the code here  ***  */
  srand(time(NULL));
  //initialise tempseq
  int tempseq = 0;
  //get random sequence
  for(int i = 0; i < 3; i++){
    tempseq *= 10;
    tempseq += rand() % 3 + 1;
  }
  //allocate meory and assign
  theSeq = (int*)malloc(seqlen*sizeof(int));
  *theSeq = tempseq;
}

// helper method for characters
char * getLetter(int num){
  switch(num){
    case 1:
      return "R";
      break;
    
    case 2:
      return "G";
      break;

    case 3: 
      return "B";
      break;
  }
}

/* display the sequence on the terminal window, using the format from the sample run in the spec */
void showSeq(int *seq) {
  /* ***  COMPLETE the code here  ***  */
  int firstnum = *seq/100;
  int secondnum = *seq/10 - firstnum * 10;
  int thirdnum = *seq - firstnum * 100 - secondnum * 10;

  char * firstLetter = getLetter(firstnum);
  char * secondLetter = getLetter(secondnum);
  char * thirdLetter = getLetter(thirdnum);

  printf("Secret: %s %s %s\n", firstLetter, secondLetter, thirdLetter);
}

#define NAN1 8
#define NAN2 9

/* counts how many entries in seq2 match entries in seq1 */
/* returns exact and approximate matches, either both encoded in one value, */
/* or as a pointer to a pair of values */
int /* or int* */ countMatches(int *seq1, int *seq2) {
  /* ***  COMPLETE the code here  ***  */
  int exact = 0;
  int approx = 0;

  // f means first 
  // s means second
  // t means third

  //taking apart first sequence 
  int fnum1 = *seq1/100;
  int snum1 = *seq1/10 - fnum1*10;
  int tnum1 = *seq1 - fnum1 * 100 - snum1 * 10;

  //taking apart second sequence
  int fnum2 = *seq2/100;
  int snum2 = *seq2/10 - fnum2*10;
  int tnum2 = *seq2 - fnum2 * 100 - snum2 * 10;

  //converting seq1 to letters
  char* flet1 = getLetter(fnum1);
  char* slet1 = getLetter(snum1);
  char* tlet1 = getLetter(tnum1);

  //converting seq2 to letters
  char* flet2 = getLetter(fnum2);
  char* slet2 = getLetter(snum2);
  char* tlet2 = getLetter(tnum2);

  //array of first and second sequence 
  char* aseq1[] = {flet1,slet1,tlet1};
  char* aseq2[] = {flet2,slet2,tlet2};

  //getting matches
  for (int i = 0; i < seqlen; i++){
    if (aseq1[i] == aseq2[i])
      exact++;//increment exact
    else if (aseq1[i] != aseq2[i]){
      for (int j = 0; j < seqlen; j++){
        // indexes j and i must not be the same 
        // char at index j of seq1 must be equal to chat at index i of second sequence 
        // index j must not be an exact match
        if (j != i && aseq1[j] == aseq2[i] && aseq1[j] != aseq2[j]){
          approx++;//increment approximate
          break;//break there can only be one approximate
        }
      }
    }
  }

  //output array
  int output = (exact*10) + approx;

  //return output
  return output;
}

/* show the results from calling countMatches on seq1 and seq1 */
void showMatches(int /* or int* */ code, /* only for debugging */ int *seq1, int *seq2, /* optional, to control layout */ int lcd_format) {
  /* ***  COMPLETE the code here  ***  */
  int exact = code/10;//separate exact 
  int approx = code - exact*10;// get approximate
  printf("%d exact\n", exact);//print exact
  printf("%d approximate\n", approx);//print approximate
}

/* parse an integer value as a list of digits, and put them into @seq@ */
/* needed for processing command-line with options -s or -u            */
void readSeq(int *seq, int val) {
  /* ***  COMPLETE the code here  ***  */
  *seq = val;//simply assign val to seq
}

/* read a guess sequence fron stdin and store the values in arr */
/* only needed for testing the game logic, without button input */
int readNum(int max) {
  /* ***  COMPLETE the code here  ***  */
  //didnt use it so idk
  return max;
}

/* ======================================================= */
/* SECTION: TIMER code                                     */
/* ------------------------------------------------------- */
/* TIMER code */

/* timestamps needed to implement a time-out mechanism */
static uint64_t startT, stopT;

/* you may need this function in timer_handler() below  */
/* use the libc fct gettimeofday() to implement it      */
uint64_t timeInMicroseconds(){
  /* ***  COMPLETE the code here  ***  */
  struct timeval tempclock;
  gettimeofday(&tempclock, NULL);
  return tempclock.tv_usec;
}


/* this should be the callback, triggered via an interval timer, */
/* that is set-up through a call to sigaction() in the main fct. */
void timer_handler (int signum) {
  /* ***  COMPLETE the code here  ***  */
}

/* initialise time-stamps, setup an interval timer, and install the timer_handler callback */
void initITimer(uint64_t timeout){
  /* ***  COMPLETE the code here  ***  */
}

/* ======================================================= */
/* SECTION: Aux function                                   */
/* ------------------------------------------------------- */
/* misc aux functions */

int failure (int fatal, const char *message, ...)
{
  va_list argp ;
  char buffer [1024] ;

  if (!fatal) //  && wiringPiReturnCodes)
    return -1 ;

  va_start (argp, message) ;
  vsnprintf (buffer, 1023, message, argp) ;
  va_end (argp) ;

  fprintf (stderr, "%s", buffer) ;
  exit (EXIT_FAILURE) ;

  return 0 ;
}

/*
 * waitForEnter:
 *********************************************************************************
 */

void waitForEnter (void)
{
  printf ("Press ENTER to continue: ") ;
  (void)fgetc (stdin) ;
}

/*
 * delay:
 *	Wait for some number of milliseconds
 *********************************************************************************
 */

void delay (unsigned int howLong)
{
  struct timespec sleeper, dummy ;

  sleeper.tv_sec  = (time_t)(howLong / 1000) ;
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;

  nanosleep (&sleeper, &dummy) ;
}

/* From wiringPi code; comment by Gordon Henderson
 * delayMicroseconds:
 *	This is somewhat intersting. It seems that on the Pi, a single call
 *	to nanosleep takes some 80 to 130 microseconds anyway, so while
 *	obeying the standards (may take longer), it's not always what we
 *	want!
 *
 *	So what I'll do now is if the delay is less than 100uS we'll do it
 *	in a hard loop, watching a built-in counter on the ARM chip. This is
 *	somewhat sub-optimal in that it uses 100% CPU, something not an issue
 *	in a microcontroller, but under a multi-tasking, multi-user OS, it's
 *	wastefull, however we've no real choice )-:
 *
 *      Plan B: It seems all might not be well with that plan, so changing it
 *      to use gettimeofday () and poll on that instead...
 *********************************************************************************
 */

void delayMicroseconds (unsigned int howLong)
{
  struct timespec sleeper ;
  unsigned int uSecs = howLong % 1000000 ;
  unsigned int wSecs = howLong / 1000000 ;

  /**/ if (howLong ==   0)
    return ;
#if 0
  else if (howLong  < 100)
    delayMicrosecondsHard (howLong) ;
#endif
  else
  {
    sleeper.tv_sec  = wSecs ;
    sleeper.tv_nsec = (long)(uSecs * 1000L) ;
    nanosleep (&sleeper, NULL) ;
  }
}

/* ======================================================= */
/* SECTION: aux functions for game logic                   */
/* ------------------------------------------------------- */

/* ********************************************************** */
/* COMPLETE the code for all of the functions in this SECTION */
/* Implement these as C functions in this file                */
/* ********************************************************** */

/* --------------------------------------------------------------------------- */
/* interface on top of the low-level pin I/O code */

/* blink the led on pin @led@, @c@ times */
void blinkN(uint32_t *gpio, int led, int c) { 
  /* ***  COMPLETE the code here  ***  */
  //for loop writes high then low with delays
    for(int i = 0; i < c ; i++){
    writeLED(gpio,led,HIGH);
    delay(DELAY);
    writeLED(gpio,led,LOW);
    delay(DELAY);
  }
}

/* ======================================================= */
/* SECTION: main fct                                       */
/* ------------------------------------------------------- */

int main (int argc, char *argv[])
{ // this is just a suggestion of some variable that you may want to use
  struct lcdDataStruct *lcd ;
  int bits, rows, cols ;
  unsigned char func ;

  int found = 0, attempts = 0, i, j, code;
  int c, d, buttonPressed, rel, foo;
  int *attSeq;

  int GLED = LED, RLED = LED2, Button = BUTTON;
  int fSel, shift, pin,  clrOff, setOff, off, res;
  int fd ;

  int  exact, approx;
  char str1[32];
  char str2[32];
  
  struct timeval t1, t2 ;
  int t ;

  char buf [32] ;

  //to check for mode to run the program in
  char str_in[20], str[20] = "some text";
  int verbose = 0, debug = 0, help = 0, opt_m = 0, opt_n = 0, opt_s = 0, unit_test = 0, res_matches = 0;

  //setting command line mode variables
  { // see the CW spec for the intended meaning of these options
    int opt;
    while ((opt = getopt(argc, argv, "hvdus:")) != -1) {
      switch (opt) {
      case 'v':
    verbose = 1;
    break;
      case 'h':
    help = 1;
    break;
      case 'd':
    debug = 1;
    break;
      case 'u':
    unit_test = 1;
    break;
      case 's':
    opt_s = atoi(optarg); 
    break;
      default: /* '?' */
    fprintf(stderr, "Usage: %s [-h] [-v] [-d] [-u <seq1> <seq2>] [-s <secret seq>]  \n", argv[0]);
    exit(EXIT_FAILURE);
      }
    }
  }
  //if help chosen in command line
  if (help) {
    fprintf(stderr, "MasterMind program, running on a Raspberry Pi, with connected LED, button and LCD display\n"); 
    fprintf(stderr, "Use the button for input of numbers. The LCD display will show the matches with the secret sequence.\n"); 
    fprintf(stderr, "For full specification of the program see: https://www.macs.hw.ac.uk/~hwloidl/Courses/F28HS/F28HS_CW2_2022.pdf\n"); 
    fprintf(stderr, "Usage: %s [-h] [-v] [-d] [-u <seq1> <seq2>] [-s <secret seq>]  \n", argv[0]);
    exit(EXIT_SUCCESS);
  }

  //if unit test with missing argument chosen in command line
  if (unit_test && optind >= argc-1) {
    fprintf(stderr, "Expected 2 arguments after option -u\n");
    exit(EXIT_FAILURE);
  }

  //  if verbose with unit test is selected
  if (verbose && unit_test) {
    printf("1st argument = %s\n", argv[optind]);
    printf("2nd argument = %s\n", argv[optind+1]);
  }

  // if verbose is seected
  if (verbose) {
    fprintf(stdout, "Settings for running the program\n");
    fprintf(stdout, "Verbose is %s\n", (verbose ? "ON" : "OFF"));
    fprintf(stdout, "Debug is %s\n", (debug ? "ON" : "OFF"));
    fprintf(stdout, "Unittest is %s\n", (unit_test ? "ON" : "OFF"));
    if (opt_s)  fprintf(stdout, "Secret sequence set to %d\n", opt_s);
  }
  //allocationg memory to store the seq and cpy
  seq1 = (int*)malloc(seqlen*sizeof(int));
  seq2 = (int*)malloc(seqlen*sizeof(int));
  cpy1 = (int*)malloc(seqlen*sizeof(int));
  cpy2 = (int*)malloc(seqlen*sizeof(int));

  // check for -u option, and if so run a unit test on the matching function
  if (unit_test && argc > optind+1) { // more arguments to process; only needed with -u 
    strcpy(str_in, argv[optind]);
    //turning code to int and storing it
    opt_m = atoi(str_in);
    strcpy(str_in, argv[optind+1]);
    //turning user input ti int and storing it
    opt_n = atoi(str_in);
    //printf("opt_m =%d opt_n = %d \n",opt_m, opt_n );
    // CALL a test-matches function; see testm.c for an example implementation
    readSeq(seq1, opt_m); // turn the integer number into a sequence of numbers and store in global variable
    readSeq(seq2, opt_n); // turn the integer number into a sequence of numbers and store in global variable
    //if it is being tested with verbose
    if (verbose)
      fprintf(stdout, "Testing matches function with sequences %d and %d\n", opt_m, opt_n);
    res_matches = countMatches(seq1, seq2);
    showMatches(res_matches, seq1, seq2, 1);
    exit(EXIT_SUCCESS);
  } else {
    /* nothing to do here; just continue with the rest of the main fct */
  }

    //if s was choosen
   if (opt_s) { // if -s option is given, use the sequence as secret sequence
    if (theSeq==NULL)
      theSeq = (int*)malloc(seqlen*sizeof(int));
    readSeq(theSeq, opt_s);
    showSeq(theSeq);
    if (verbose) {
      fprintf(stderr, "Running program with secret sequence:\n");
      showSeq(theSeq);
    }
  }
  //if sudo not entered
   if (geteuid () != 0)
    fprintf (stderr, "setup: Must be root. (Did you forget sudo?)\n") ;

  // init of guess sequence, and copies (for use in countMatches)
  attSeq = (int*) malloc(seqlen*sizeof(int));
  cpy1 = (int*)malloc(seqlen*sizeof(int));
  cpy2 = (int*)malloc(seqlen*sizeof(int));

      // constants for RPi2
  gpiobase = 0x3F200000 ;

  // -----------------------------------------------------------------------------
  // memory mapping 
  // Open the master /dev/memory device

  if ((fd = open ("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC) ) < 0)
    return failure (FALSE, "setup: Unable to open /dev/mem: %s\n", strerror (errno)) ;

  // GPIO:
  gpio = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, gpiobase) ;
  if ((int32_t)gpio == -1)
    return failure (FALSE, "setup: mmap (GPIO) failed: %s\n", strerror (errno)) ;

  // -------------------------------------------------------
  // Configuration of LED and BUTTON
  pinMode(gpio,GLED,1);
  pinMode(gpio,RLED,1);
  pinMode(gpio,Button,0);
  /* ***  COMPLETE the code here  ***  */
//-------------------------------------------------------------------------------------  
     // Start of game
  fprintf(stderr,"WELCOME TO MASTERMIND\n");
  /* ***  COMPLETE the code here  ***  */

  /* initialise the secret sequence */
  if (!opt_s)
    initSeq();
  if (debug)
    showSeq(theSeq);
  // optionally one of these 2 calls:
  // waitForEnter () ; 
  // waitForButton (gpio, pinButton) ;
  fprintf(stderr,"=====================\n");
  // -----------------------------------------------------------------------------
  // +++++ main loop
  while (!found) {
    attempts++;
    *attSeq = 0;

    //the red control LED should blink three times to indicate the start of a new round.
    blinkN(gpio,LED2,3);
    delay(DELAY);

    /* ******************************************************* */
    /* ***  COMPLETE the code here  ***                        */
    /* this needs to implement the main loop of the game:      */
    /* check for button presses and count them                 */
    /* store the input numbers in the sequence @attSeq@        */
    /* compute the match with the secret sequence, and         */
    /* show the result                                         */
    /* see CW spec for details                                 */
    /* ******************************************************* */
    //command terminal helper 
    printf(" Enter your guess (attempt:%d)\n\n",attempts);

    //for loop for 3 inputs
    for(int i = 0; i<seqlen; i++){
      printf("  color number %d\n",i+1);//terminal helper
      *attSeq *= 10;
      int acc = 0;//acumulator
      int press;//variable for readButton
      int currentVal = HIGH;// a little helper to deal with button holding time (human slowness)
      //timer to count timeout
      clock_t timer = clock();

      //maximum of 3 presses 
      while(acc<seqlen){

        press = readButton(gpio,Button);
        //if button pressed
        if(press!=0 && currentVal == HIGH){
          currentVal = LOW;
          acc++;
          timer = clock();
          printf("   Button Pressed\n");
        }
        //if not pressed
        else if(press == 0 && currentVal == LOW){
          currentVal = HIGH;
        }
        //if timed out 
        if(clock() > timer+TIMEOUT) break;
      }

      printf("  color number %d is %d\n\n",i+1,acc);//terminal helper
      *attSeq += acc;//add to attempt sequence
      //blink acknowledgements 
      blinkN(gpio, RLED, 1);
      blinkN(gpio, GLED, acc);

    }

    delay(DELAY);

    printf(" end of guess  \n your guess is %d\n", *attSeq);//terminal helper
    //blink end of guess
    blinkN(gpio, RLED, 2);

    delay(DELAY);

    //get matches
    res_matches = countMatches(theSeq, attSeq);
    showMatches(res_matches,attSeq,theSeq,1);
    exact = res_matches/10;
    approx = res_matches%10;

    printf("\n");

    //blink mathces
    blinkN(gpio, GLED, exact);
    blinkN(gpio, RLED, 1);
    blinkN(gpio, GLED, approx);

    delay(DELAY);

    //check exact matches 
    if(exact == 3) found = 1;
    //check attempts
    if(attempts==9) break;
  }
  if (found) {
    /* ***  COMPLETE the code here  ***  */
    fprintf(stderr,"Game finished in %d moves!\n",attempts);
    //green LED should blink three times while the red LED is turned on
    writeLED(gpio, RLED, 1);
    blinkN(gpio, GLED, 3);
    writeLED(gpio, RLED, 0);
  } else {
    fprintf(stdout, "Sequence not found\n");
  }
  return 0;
}
