/*
 * MasterMind: a template structure for the master-mind game logic (purely C) 

Sample run:
Contents of the sequence (of length 3):  2 1 1
Input seq (len 3): 1 2 3
0 2
Input seq (len 3): 3 2 1
1 1
Input seq (len 3): 2 1 1
3 0
SUCCESS after 3 iterations; secret sequence is  2 1 1

 * Compile:    gcc -o cw2  master-mind-terminal.c
 * Run:        ./cw2

 */

/* --------------------------------------------------------------------------- */

/* Library headers from libc functions */
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* Constants */
#define  COL  3
#define  LEN  3

/* Global variables */

static const int colors = COL;
static const int seqlen = LEN;

static char* color_names[] = { "red", "green", "blue" };

static int* theSeq = NULL;

/* Aux functions */

/* initialise the secret sequence; by default it should be a random sequence, with -s use that sequence */
void initSeq() {
  /* COMPLETE the code here */
}

/* display the sequence on the terminal window, using the format from the sample run above */
void showSeq(int *seq) {
  /* COMPLETE the code here */
}

/* counts how many entries in seq2 match entries in seq1 */
/* returns exact and approximate matches  */
int countMatches(int *seq1, int *seq2) {
  /* COMPLETE the code here */
}

/* show the results from calling countMatches on seq1 and seq1 */
void showMatches(int code, /* only for debugging */ int * seq1, int *seq2) {
  /* COMPLETE the code here */
}

/* read a guess sequence fron stdin and store the values in arr */
void readString(int *arr) {
  /* COMPLETE the code here */
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int main(int argc, char **argv){
  /* DEFINE your variables here */
  int found = 0, attempts = 0;
  /* for getopts() command line processing */
  int verbose = 0, help = 0, unittest = 0, debug = 0;
  char *sseq = NULL;

  // see: man 3 getopt for docu and an example of command line parsing
  // Use this template to process command line options and to store the input
  {
    int opt;
    while ((opt = getopt(argc, argv, "vuds:")) != -1) {
      switch (opt) {
      case 'v':
	verbose = 1;
	break;
      case 'u':
	unittest = 1;
	break;
      case 'd':
	debug = 1;
	break;
      case 's':
	sseq = (char *)malloc(LEN*sizeof(char));
	strcpy(sseq,optarg);
	break;
      default: /* '?' */
	fprintf(stderr, "Usage: %s [-v] [-d] [-s] <secret sequence> [-u] <secret sequence> <guess sequence> \n", argv[0]);
	exit(EXIT_FAILURE);
      }
    }
    if (unittest && optind >= argc) {
      fprintf(stderr, "Expected argument after options\n");
      exit(EXIT_FAILURE);
    }

    if (verbose && unittest) {
      printf("1st argument = %s\n", argv[optind]);
      printf("2nd argument = %s\n", argv[optind+1]);
    }
  }

  if (verbose) {
    fprintf(stdout, "Settings for running the program\n");
    fprintf(stdout, "Verbose is %s\n", (verbose ? "ON" : "OFF"));
    fprintf(stdout, "Debug is %s\n", (debug ? "ON" : "OFF"));
    fprintf(stdout, "Unittest is %s\n", (unittest ? "ON" : "OFF"));
    if (sseq)  fprintf(stdout, "Secret sequence set to %s\n", sseq);
  }

  if (sseq) { // explicitly setting secret sequence
    /* SET the secret sequence here */
  }    
  if (unittest) {
    /* SET secret and guess sequence here */
    /* then run the countMatches function and show the result */

    /* for now just terminate; DELETE these two lines, once you have an implementation for -u in here */
    fprintf(stdout, "INCOMPLETE implementation, terminating program\n");
    return EXIT_FAILURE;
  }

  // -----------------------------------------------------------------------------

  // +++++ main loop
  while (!found) {
    attempts++;
    /* IMPLEMENT the main game logic here */
  }

  if (found) {
    /* print SUCCESS and the number of iterations */
  } else {
    /* print something else */
  }
  return EXIT_SUCCESS;
}
  
