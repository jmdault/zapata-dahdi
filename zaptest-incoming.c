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
gcc -o zaptest-incoming -I. zaptest-incoming.c ./libzap.a -lm
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
    }
  }

  /* Init the digit buffer and tell Zapata that we want DTMF digits */
  (void) zap_clrdtmfn(zp);
  res = zap_digitmode(zp, ZAP_DTMF) && zap_clrdtmf(zp);
  err_if(res, '!', 0, __LINE__);    /* error if not zero */

  sleep(1);

  /* Play our outgoing message.  Abort on DTMF or hang-up. */
  res = zap_playf(zp, "sounds/screen-callee-options.ulaw", ZAP_DTMFINT | ZAP_HOOKEXIT);
  if (res == -2) {                  /* == -2 on caller hang-up */
    printf("Hungup during message\n");
    zap_sethook(zp, ZAP_ONHOOK);
    exit(0);
  }

  /* Wait up to TM_OUT ms for the caller to enter a DTMF digit */
  res = zap_getdtmf(zp, 1, (char *)0, 0, 0, TM_OUT, ZAP_HOOKEXIT);
  err_if(res, '=', -1, __LINE__);   /* error if -1 */

  /* Hang-up if time-out and no DTMF digit.  (Bulk dialer?) */
  if (res != 1) {
    printf("\n");
    zap_sethook(zp, ZAP_ONHOOK);
    err_if(res, '=', -1, __LINE__); /* error if -1 */
    exit(0);
  }

  /* Get the DTMF digit the caller entered. */
  pDigits = zap_dtmfbuf(zp);
  printf("Pressed: %s\n",pDigits);

  res = zap_playf(zp, "sounds/goodbye.ulaw", ZAP_DTMFINT | ZAP_HOOKEXIT);

  for(i=0;i<10;i++) {
    zap_arbtones(zp, busy1, busy2, 500, 0);
    zap_silence(zp,500,0);
  }

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
