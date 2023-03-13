/* Code to test dahdi FXS interface *
 * Copyright: M5 Technologies, 2023
 * Author: Jean-Michel Dault <jmdault@media5corp.com>
 * License: GNU General Public License, version 2 http://www.gnu.org/copyleft/gpl.html
 * Based on Bob Smith's Answering Machine http://www.linuxtoys.org/answer/answering_machine.html
*/

/* Requirements:
 * Telephony (FXS) hardware supported by the DAHDI driver interface
 * Install DAHDI from https://github.com/asterisk/dahdi-linux
*/ 

/* To build:
gcc -o zaptest-cw -I. zaptest-cw.c ./libzap.a -lm
*/

#include <stdio.h>
#include <zap.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <math.h> /* pow, cos */
#include <unistd.h> /* sleep */


        /* The time (ms) to wait for a "1" from the caller */
#define TM_OUT  (9000)

        /* The handle to the open telephone interface */
ZAP     *zp;

void err_if(int res, char operator, int errval, int linenum);


int main()
{

  char       *cidnumber = "8195551212"; /* Caller-ID phone number */
  char       *cidname = "Dahdi test";   /* Caller-ID name */
  char       *cidnumber2 = "8001234567"; /* Caller-ID phone number */
  char       *cidname2 = "Company inc.";   /* Caller-ID name */
  char       *pDigits;                  /* points to entered DTMF digits */
  int	     res,i;
  float	     busy1=480,busy2=620;

  zp = zap_open("/dev/dahdi/chan/001/001", 0);
  if (!zp) {
    printf("Unable to open /dev/dahdi/chan/001/001.  Please verify that the\n");
    printf("dahdi modules are installed, and that your\n");
    printf("hardware is installed and working correctly.\n");
    exit(1);
  }
  
  printf("Ringing...\n");
  res = zap_ringclid(zp, cidnumber, cidname);
  if(res==-1) {
    printf("Error!\n");
    exit(1);
  }
  if(res==1) {
    printf("Answered after CLID\n");
  }
  if(res==2) {
    printf("Answered before CLID\n");
  }
  if(res==0) {
    printf("Not answered.. continuing for 5 rings...\n");
    res = zap_ring(zp,5);
    if(res==0) {
        printf("No answer\n");
        exit(0);
    } else {
        printf("Answered.\n");
    } 
  }

  res = zap_playf(zp, "sounds/synth.ulaw", 0);

  res = zap_cwclid(zp,cidnumber2, cidname2);

  res = zap_playf(zp, "sounds/synth.ulaw", 0);

  zap_sethook(zp, ZAP_ONHOOK);

  exit(0);
}

/* err_if() -- Exit the program if the result is not correct.
 * Inputs are the result, the comparison operator, the value
 * to match, and the line number */
void err_if(int res, char operator, int errval, int linenum)
{
  if (((operator == '=') && (res == errval)) ||
      ((operator == '!') && (res != errval))) {
    printf("Error code %d at line = %d, with errno = %d.   Bye...\n",
           res, linenum, errno);

    (void) zap_sethook(zp, ZAP_ONHOOK);
    exit(1);
  }
}
