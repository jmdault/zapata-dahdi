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
gcc -o zaptest-outgoing -I. zaptest-outgoing.c ./libzap.a -lm
*/

#include <stdio.h>
#include <stdbool.h>
#include <zap.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <math.h> /* pow, cos */
#include <unistd.h> /* sleep */
#include <dahdi/user.h>
#include <sys/ioctl.h>
#include <string.h>


        /* The time (ms) to wait for a "1" from the caller */
#define TM_OUT  (20000)

        /* The handle to the open telephone interface */
ZAP     *zp;

void err_if(int ret, char operator, int errval, int linenum);


int main()
{

  char       *cidnumber = "8195551212"; /* Caller-ID phone number */
  char       *cidname = "Dahdi test";   /* Caller-ID name */
  char       *pDigits;                  /* points to entered DTMF digits */
  int	     res,i;
  float	     busy1=480,busy2=620;
  float      dial1=350,dial2=440;
  float      ring1=440,ring2=480;
  struct dahdi_params params;
  bool isoffhook = false;
  char       *dtmf;
  char       *termdigits = "#";

  zp = zap_open("/dev/dahdi/chan/001/001", 0);
  if (!zp) {
    printf("Unable to open /dev/dahdi/chan/001/001.  Please verify that the\n");
    printf("dahdi modules are installed, and that your\n");
    printf("hardware is installed and working correctly.\n");
    exit(1);
  }

  printf("Waiting for off-hook...\n");
  sleep(1);

  /* Initial state */
  res = ioctl(zap_fd(zp), DAHDI_GET_PARAMS, &params);
  if(params.rxisoffhook) {
    isoffhook = true;
    printf("OK!\n");
  }

  if(!isoffhook) {
    /* Wait for event */
    for(;;) {
      res = zap_waitevent(zp);
      if(res==ZAP_EVENT_RINGANSWER) {
        break;
        printf("OK!\n");
      }
      sleep(0.2);
    }
  }

  /* Init the digit buffer and tell Zapata that we want DTMF digits */
  (void) zap_clrdtmfn(zp);
  res = zap_digitmode(zp, ZAP_DTMF) && zap_clrdtmf(zp);
  err_if(res, '!', 0, __LINE__);    /* error if not zero */

  zap_arbtones(zp, dial1, dial2, 20000, ZAP_DTMFINT | ZAP_HOOKEXIT);
  
  /* Wait up to TM_OUT ms for the caller to enter a DTMF digit */
  res = zap_getdtmf(zp, 20, termdigits, 0, 0, TM_OUT, ZAP_HOOKEXIT | ZAP_TIMEOUTOK);
  err_if(res, '=', -1, __LINE__);   /* error if -1 */


  /* Get the DTMF digit the caller entered. */
  dtmf = zap_dtmfbuf(zp);
  printf("Pressed: %s\n",dtmf);

  if(strlen(dtmf)<3) { 
    for(i=0;i<10;i++) {
      zap_arbtones(zp, busy1, busy2, 500, 0);
      zap_silence(zp,500,0);
    }
    zap_sethook(zp, ZAP_ONHOOK);
    exit(1);
  }

  res = zap_playf(zp, "sounds/transfer.ulaw", ZAP_DTMFINT | ZAP_HOOKEXIT);

  sleep(0.5);
  zap_dial(zp, dtmf, 20);
  sleep(0.5);

  for(i=0;i<3;i++) {
    zap_arbtones(zp,ring1, ring2, 2000, 0);
    zap_silence(zp,4000,0);
  }

  res = zap_playf(zp, "sounds/tt-monkeys.ulaw", ZAP_DTMFINT | ZAP_HOOKEXIT);

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
