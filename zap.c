/* 
 * BSD Telephony Of Mexico "Zapata" Telecom Library, version 1.10  12/10/01
 *
 *   Part of the "Zapata" Computer Telephony Technology.
 *
 *   See http://www.bsdtelephony.com.mx
 *
 *
 *  The technologies, software, hardware, designs, drawings, scheumatics, board
 *  layouts and/or artwork, concepts, methodologies (including the use of all
 *  of these, and that which is derived from the use of all of these), all other
 *  intellectual properties contained herein, and all intellectual property
 *  rights have been and shall continue to be expressly for the benefit of all
 *  mankind, and are perpetually placed in the public domain, and may be used,
 *  copied, and/or modified by anyone, in any manner, for any legal purpose,
 *  without restriction. 
 *
 *   TTY/TDD functionality heavily based upon 'rtty' program by Jesus Arias.
 */

/* Library Documentation

The "Zapata" library implements function calls allowing the user easy
access to the telephony functionality.

Each of the library functions use a pointer to the type "ZAP", which is
a struct that contains open channel information and context.

An application using this library must include <zap.h> in the C language
source, and reference -lzap and -lm in the link command when building. Both of
these files reside in the standard place (/usr/include and /usr/lib).

In all cases (except zap_open, which returns NULL on error), a function
returns -1 on error, and has the error in errno (standard Unix error
methodology).

GENERAL EVENT NOTIFICATION

All functions that perform Channel Input and/or output either directly
or indirectly have the characteristic that if a channel-related event occurrs,
such as the user hanging up, or the associated span going into alarm state,
etc., the function will be interrupted (without the loss of data), and
return -2.

Some of these functions, such as zap_play, zap_record, etc. require enabling
of this feature with the ZAP_HOOKEXIT flag. Others, such as zap_sendtdd do
it unconditionally.

THE "ZAPIO" STRUCTURE

The "ZAPIO" structure is provided for the zap_play and zap_rec functions.
This allows multiple entities (either open files or memory buffers) to be
played or recorded into within the context of a single function call.
Each ZAPIO entry contains the following:

	void *p;  --  Pointer to memory buffer or FILE * (obtained from fopen())
	int length;  --  Amount of bytes to play/record (0 for play until EOF)
	int offset;  --  Offset into file, or -1 to indicate memory location

The zap_play and zap_record functions take a pointer to an array of ZAPIO
structures. The list is terminated by an array element (ZAP structure)
with the p = 0 (NULL).


CLOSING OF CHANNEL / APPLICATION TERMINATION

It is a good idea for an application to zap_close() before exiting (which
will stop all I/O, and disconnect and initialize the channel), but
is not necessary. When the physical file channel for the device is closed
(by virture of the program exiting, and releasing the file descriptor)
the channel will be shut down and disconnected.


FUNCTION DESCRIPTIONS


CHANNEL OPEN/CLOSE FUNCTIONS

Open a Zapata device channel
    ZAP *zap_open(char *devname, int nonblocking);
	devname is the ascii name of the device to be opened
	 (such as "/dev/dahdi/1"), or a positive integer in ascii, (such
	 as "123") for opening channels > 253.  Set nonblocking to non-zero
	 in order to open with O_NONBLOCK (async mode). 
    Returns NULL if error, with error in errno.

Close a Zapata device channel
    int zap_close(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Get raw file descriptor
	int zap_fd(ZAP *zap);
	zap is a pointer to an open Zapata device channel
	Returns the file descriptor for the open zapata channel or
    Returns -1 if error, with error in errno.

DTMF-RELATED FUNCTIONS

Clear DTMF buffer
    int zap_clrdtmf(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.
	
Get DTMF string
    int zap_getdtmf(ZAP *zap, int max, char *term,
      char erase, int idtimeout, int timeout, int flags);
	zap is a pointer to an open Zapata device channel
	max is maximum number of digits to get
	term is pointer to ascii string containing all digits that 
	    are to serve as input termination, or NULL for none
	erase is the character to serve as input erase (discard buffer
	    and start again), or NULL for none
	idtimeout is the inter-digit timeout in milliseconds (0 = no timeout)
	timeout is the full function timeout in milliseconds (0 = no timeout)
	flags is the flag mask (logical OR of the following) as follows:
	    ZAP_HOOKEXIT -- Abort function and return on user hangup
	    ZAP_TIMEOUTOK -- If timeout occurrs, save digits received (if any
		before timeout. Normally digits received before timeout
		are discarded.
    Upon successful completion, returns number of DTMF digits received.
    Received DTMF digits may be accessed via the zap_dtmfbuf() call, which
	returns a pointer to the buffer containing the digits.
    Returns -1 if error, with error in errno. If ZAP_HOOKEXIT is set,
	returns -2 if user disconnect.
NOTE: The zap_getdtmf function is designed to be able to be called several
    times in a row on the same buffer (each call to meet a series of
    conditions etc., and yielding one, single, complete buffer). Therefore,
    before its use, a zap_clrdtmf() call is necessary to clear the buffer
    and internal registers for a fresh, clean tone decode. If preceeded by
    a play or record function that is interruptable by tone input, the
    zap_clrdtmf() should be done before the play or record, because if it
    is done after, the digit that was decoded (that interrupted the play or
    record function) will get lost. Also, if your application uses several
    zap_getdtmf() functions in a row on a continuous string of digits, but
    uses the output of each zap_getdtmf() call separately, you must at least
    initialize the dtmf buffer by calling zap_clrdtmfn(). Otherwise, it
    just adds to the buffer, rather then replaces it every time the
    zap_getdtmf() call is made. Also, some user-side devices may have poor
    transhybrid-loss characteristics, inherent echo, or other issues which
    could potentially cause a line in a conference to respond to DTMF tones
    from another line in the same conference. This can be eliminated by
    setting the ZAP_CONFMUTE mode in the zap_digitmode() function (see below).
    Also, this is a nice thing to do for multi-party conferences, since it
    mutes the tone on the conference bridge immediately upon detection, thus
    minimizing annoyance to other callers.

Get DTMF buffer
	int zap_dtmfbuf(ZAP *zap);
	zap is a pointer to an open Zapata device channel
	Returns a pointer to the dtmf buffer of the Zapata channel structure

Get Length of last DTMF tone decoded (in ms)
	int zap_dtmfms(ZAP *zap);
	zap is a pointer to an open Zapata device channel
	Returns a value in milliseconds of length of last DTMF tone decoded.

Set Digit Receive mode (DTMF or MF, and CONFMUTE mode)
    int zap_digitmode(ZAP *zap, int mode);
	zap is a pointer to an open Zapata device channel
	mode is an integer, one of the following: ZAP_DTMF or ZAP_MF or'ed
	    with ZAP_CONFMUTE if mode desired where call is taken off conference
	    bridge (if on one) when tone detected, to verify positively that
	    it was not tone leakage from some other channel in conference.
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Clear DTMF buffer pointer (but not parameters)
    int zap_clrdtmfn(ZAP *zap);
	zap is a poitner to an open Zapata device channel
	Clears the dtmf buffer without reseting the DTMF state machine.
	Always returns zero.
NOTE: This is used for making several calls to zap_getdtmf() in a row
and extracting the buffer each time its called, without upsetting the state
of the rest of the DTMF decoding subsystem.

AUDIO INPUT/OUTPUT FUNCTIONS

TRADITONAL/BLOCK-RELATED CALLS

Play 8khz mu-law audio on channel
int zap_play(ZAP *zap, ZAPIO *ioblock, int flags);
	zap is a pointer to an open Zapata device channel
	ioblock is a pointer to an array of ZAPIO structures (see ZAPIO 
	    structure documentation above)
	flags is the flag mask (logical OR of the following) as follows:
	    ZAP_HOOKEXIT -- Abort function and return on user hangup
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
    Upon successful completion, returns 0, unless ZAP_DTMFINT set and
	DTMF tone received, in which case will return 1.
    Returns -1 if error, with error in errno. If ZAP_HOOKEXIT is set,
	returns -2 if user disconnect.

Play individual file containing 8khz mu-law audio on channel
int zap_playf(ZAP *zap, char *fname, int flags);
	zap is a pointer to an open Zapata device channel
	fname is a poitner to ascii filename to play
	flags is the flag mask (logical OR of the following) as follows:
	    ZAP_HOOKEXIT -- Abort function and return on user hangup
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
    Upon successful completion, returns 0, unless ZAP_DTMFINT set and
	DTMF tone received, in which case will return 1.
    Returns -1 if error, with error in errno. If ZAP_HOOKEXIT is set,
	returns -2 if user disconnect.
Note: This is just a wrapper function for zap_play.

Record 8khz mu-law audio on channel
int zap_rec(ZAP *zap, ZAPIO *ioblock, int flags);
	zap is a pointer to an open Zapata device channel
	ioblock is a pointer to an array of ZAPIO structures (see ZAPIO 
	    structure documentation above)
	flags is the flag mask (logical OR of the following) as follows:
	    ZAP_HOOKEXIT -- Abort function and return on user hangup
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
	    ZAP_BEEPTONE -- Play beep tone before recording
	    ZAP_SILENCEINT -- Abort function and return 2 if more then 5
		seconds of silence detected.
    Upon successful completion, returns 0, unless ZAP_DTMFINT set and
	DTMF tone received, in which case will return 1, or ZAP_SILENCEINT
	set and 5 seconds of silence, in which case will return 2.
    Returns -1 if error, with error in errno. If ZAP_HOOKEXIT is set,
	returns -2 if user disconnect.

Record individual file to contain 8khz mu-law audio on channel
int zap_recf(ZAP *zap, char *fname, int flags);
	zap is a pointer to an open Zapata device channel
	fname is a poitner to ascii filename to play
	flags is the flag mask (logical OR of the following) as follows:
	    ZAP_HOOKEXIT -- Abort function and return on user hangup
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
	    ZAP_BEEPTONE -- Play beep tone before recording
	    ZAP_SILENCEINT -- Abort function and return 2 if more then 5
		seconds of silence detected.
    Upon successful completion, returns 0, unless ZAP_DTMFINT set and
	DTMF tone received, in which case will return 1, or ZAP_SILENCEINT
	set and 5 seconds of silence, in which case will return 2.
    Returns -1 if error, with error in errno. If ZAP_HOOKEXIT is set,
	returns -2 if user disconnect.
Note: This is just a wrapper function for zap_rec.

BUFFERED/CHUNK-RELATED/NON-SYNCRONOUS CALLS

Play an arbitrary chunk of mu-law 8khz audio
int zap_playchunk(ZAP *zap, void *buf, int len, int flags);
	zap is a pointer to an open Zapata device channel
	buf is a pointer to mu-law 8khz audio of arbitrary length
	len is an integer specifying length in bytes of buffer to play
	flags is the flag mask (logical OR of the following as follows:
		ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
	Upon successful completion returns the number of bytes written on
	the channel.  On error, -1 will be returned, and if an event is
	received (or a DTMF character) then the number of bytes that were
	written before the event was received will be returned.  No flushing
	(waiting for the transmit to complete before returning) is done
        (unlike zap_play), meaning this mode is more suited for applications
        where the buffer sizes are small, or for streaming applications.

Record an arbitrary chunk of mulaw or linear 8khz audio
int zap_recchunk(ZAP *zap, void *buf, int len, int flags);
	zap is a pointer to an open Zapata device channel
	buf is a pointer to space for storing recorded mu-law 8khz audio
	len is an integer specifying *number of samples* to record
	flags is the flag mask (logical OR of the following as follows:
		ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
	Upon successful completion returns the number of bytes read from
	the channel.  On error, -1 will be returned, and if an event is
	received (or a DTMF character) then the number of bytes that were
	read before the event was received will be returned.  Extra samples
	are carried over into the next buffer in this mode, meaning this mode 
	is more suited for applications where the buffer sizes are small,
	or for streaming applications.

Purge recorder memory
int zap_recpurge(ZAP *zap);
	zap is a pointer to an open Zapata device channel
	Clears the "carry" into the next record call.
	Always returns zero.


HOOKSWITCH MANIPULATION FUNCTIONS

Take a channel off hook, or put channel on hook
int zap_sethook(ZAP *zap,int hookstate);
	zap is a pointer to an open Zapata device channel
	hookstate is the hook state as follows:
	    ZAP_OFFHOOK --  Answer a call (take offhook)
	    ZAP_ONHOOK --  Disconnect a call (put onhook)
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Send a wink on a channel
int zap_wink(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Start the dialtone on a channel
int zap_start(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Flash hookswitch on a channel
int zap_flash(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Ring station on a channel
int zap_ring(ZAP *zap,int nrings);
	zap is a pointer to an open Zapata device channel
	nrings is number of rings maximum, 0 for infinite.
    Upon answer, returns 1, or 0 if went through all rings with no answer.
    Returns -1 if error, with error in errno.

Wait for a call, then optionally answer it (convenience function)
int zap_waitcall(ZAP *zap, int nrings, int hookstate);
	zap is a pointer to an open Zapata device channel
	nrings in integer number of rings to wait (use 1 for E&M).
	hookstate is hook state to be set after rings received, one of the
	  following:
	    ZAP_OFFHOOK --  Take offhook
	    ZAP_ONHOOK --  Leave onhook (use for E&M, often followed by
				a zap_wink() call)
    NOTE: If nrings > 1, this functions uses SIGALRM, and when finishes, sets
      it back to SIG_DFL. Make sure your program set it to something appropriate
      after using this call, if necessary.
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno. An errno of EINTR means function
      timed out (multiple ring case only).


CHANNEL PARAMETER FUNCTIONS

Get channel parameters
int zap_getparams(ZAP *zap, void *params);
	zap is a pointer to an open Zapata device channel
	params is a pointer to a channel parameter structure appropriate for
	    the given device (such as dahdi_params)
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Set channel parameters
int zap_setparams(ZAP *zap, void *params);
	zap is a pointer to an open Zapata device channel
	params is a pointer to a channel parameter structure appropriate for
	    the given device (such as dahdi_params)
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Get channel number
int zap_channel(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns channel number.
    Returns -1 if error, with error in errno.

Set channel receive and transmit gains
int zap_gains(ZAP *zap, float rxgain, float txgain);
	zap is a pointer to an open Zapata device channel
	rxgain is a float containing receive gain in db.
	txgain is a float containing transmit gain in db.
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.


TTY/TDD INPUT/OUTPUT FUNCTIONS
(The input and output functions convert to & from Baudot to ASCII)

Get an ASCII character from a TTY/TDD device
int zap_gettdd(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns character received in ASCII.
    Returns -1 if error, with error in errno, or -2 if user disconnected.

Put an ASCII string to a TTY/TDD device
int zap_sendtdd(ZAP *zap, char *string)
	zap is a pointer to an open Zapata device channel
	string is a pointer to ASCII string to be sent to TTY/TDD.
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno, or -2 if user disconnected.
Note: You must call zap_finish_audio() to finish sending audio data in the
send audio buffer when through making calls to zap_sendtdd().

Send the Echo Canceller Disable Tone (2100 HZ)
int zap_disatone(ZAP *zap,int flag);
	zap is a pointer to an open Zapata device channel
	flag is the flag mask (or 0) as follows:
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno, or -2 if the user disconnected.


EVENT HANDLING FUNCTIONS

Get An Event from Event Buffer (if any)
int zap_getevent(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns event in buffer, or 0 if none.
    Returns -1 if error, with error in errno.

Wait for Event in Event Buffer
int zap_waitevent(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns event in buffer.
    Returns -1 if error, with error in errno.

Flush ALL Events in Event Buffer
int zap_flushevent(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    Upon Successful completion, returns event in buffer.
    Returns -1 if error, with error in errno.


SWITCHING/CONFERENCING FUNCTIONS

int zap_conf(ZAP *zap, int chan, int confno, int confmode);
	zap is a pointer to an open Zapata device channel
	chan is channel number (0 for current)
	confno is integer conference number
	confmode is the integer mode/flag mask as follows:
	    ZAP_NONE --  No switching or conferencing, normal play/record.
	    ZAP_MONITOR --  Monitor Mode
	    ZAP_MONITORTX --  Monitor Tx Mode
	    ZAP_MONITORBOTH --  Monitor Both Mode
	    ZAP_CONF --  Normal Conference Mode
	    ZAP_CONFANN --  Conference/Announce Mode
	    ZAP_CONFMON --  Conference/Monitor Mode
	    ZAP_CONFANNMON --  Conference/Monitor/Announce Mode
	    ZAP_CONF_LISTENER --  Is listener on this conference (flag to be OR'ed)
	    ZAP_CONF_TALKER --  Is talker on this conference (flag to be OR'ed)
	  See conference architecture documentation about conference modes.
    Upon successful completion, returns 0.
    Returns -1 if error, with error in errno.

Get Conference information
int zap_getconf(ZAP *zap, int chan, int *confnop, int *confmodep);
	zap is a pointer to an open Zapata device channel
	chan is channel number (0 for current)
	confnop is pointer to returned integer conference number 
	confmodep is pointer to returned integer mode/flag mask as follows:
	    ZAP_NONE --  No switching or conferencing, normal play/record.
	    ZAP_MONITOR --  Monitor Mode
	    ZAP_MONITORTX --  Monitor Tx Mode
	    ZAP_MONITORBOTH --  Monitor Both Mode
	    ZAP_CONF --  Normal Conference Mode
	    ZAP_CONFANN --  Conference/Announce Mode
	    ZAP_CONFMON --  Conference/Monitor Mode
	    ZAP_CONFANNMON --  Conference/Monitor/Announce Mode
	    ZAP_CONF_LISTENER --  Is listener on this conference (flag to be OR'ed)
	    ZAP_CONF_TALKER --  Is talker on this conference (flag to be OR'ed)
	  See conference architecture documentation about conference modes.
    Upon successful completion, returns actual channel number.
    Returns -1 if error, with error in errno.

Clear all conference links
int zap_clearlinks(ZAP *zap);
	zap is a pointer to an open Zapata device channel
    See switching/conferencing archetecture documentation for information on
      conference links.
    Upon Successful completion, returns event in buffer.
    Returns -1 if error, with error in errno.

Make a conference link (cause 1 conference to listen to another)
int zap_makelink(ZAP *zap, int dstconfno, int srcconfno);
	zap is a pointer to an open Zapata device channel
	dstconfno is conference number listening to other conference
	srcconfno is conference number being listened to
    See switching/conferencing archetecture documentation for information on
      conference links.
    Upon Successful completion, returns event in buffer.
    Returns -1 if error, with error in errno.

Break a conference link (cause 1 conference to stop listening to another)
int zap_makelink(ZAP *zap, int dstconfno, int srcconfno);
	zap is a pointer to an open Zapata device channel
	dstconfno is conference number listening to other conference
	srcconfno is conference number being listened to
    See switching/conferencing archetecture documentation for information on
      conference links.
    Upon Successful completion, returns event in buffer.
    Returns -1 if error, with error in errno.


TONE GENERATION FUNCTIONS

Dial digits
int zap_dial(ZAP *zap, char *string, int length);
	zap is a pointer to an open Zapata device channel
	string is a pointer to an ascii string containing digits to dial,
		plus 'M' for MF, and 'D' for DTMF.
	length is length of tones in milliseconds (or 0 for default).
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Send silence for a specified period
int zap_silence(ZAP *zap, int length, int flag);
	zap is a pointer to an open Zapata device channel
	length is length of silence in milliseconds (or 0 for default).
	flag is the flag mask (or 0) as follows:
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
    Upon Successful completion, returns 0, or 1 if DTMF tone received.
    Returns -1 if error, with error in errno, or -2 if user disconnected.

Send a single arbitrary tone for a specified period
int zap_arbtone(ZAP *zap, float freq, int length, int flag);
	zap is a pointer to an open Zapata device channel
	freq is a float containing desired frequency in hertz.
	length is length of silence in milliseconds (or 0 for default).
	flag is the flag mask (or 0) as follows:
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
    Upon Successful completion, returns 0, or 1 if DTMF tone received.
    Returns -1 if error, with error in errno, or -2 if user disconnected.

Send a pair of arbitrary tones for a specified period
int zap_arbtones(ZAP *zap, float freq1, float freq2, int length, int flag);
	zap is a pointer to an open Zapata device channel
	freq1,freq2 are floats containing desired frequencies in hertz.
	length is length of silence in milliseconds (or 0 for default).
	flag is the flag mask (or 0) as follows:
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
    Upon Successful completion, returns 0, or 1 if DTMF tone received.
    Returns -1 if error, with error in errno, or -2 if user disconnected.

Finish sending audio buffer
int zap_finish_audio(ZAP *zap,int flag)
	zap is a pointer to an open Zapata device channel
	flag is the flag mask (or 0) as follows:
	    ZAP_DTMFINT -- Abort function and return 1 if DTMF tone received.
    Upon Successful completion, returns 0, or 1 if DTMF tone received.
    Returns -1 if error, with error in errno, or -2 if user disconnected.
Note: This function must be called after the last tone generation
function (any of above) is called to finish sending what's in the audio
buffer.

CALLER*ID RECEPTION / GENERATION

Wait for ring, and then receive and parse Caller*ID
int zap_clid(ZAP *zap, char *number, char *name);
	zap is a pointer to an open Zapata device channel
	number is a pointer to a char buffer in which to receive caller's number
	name is a pointer to a char buffer in which to receive caller's name
NOTE: above-referenced buffers should be at least 64 chars in length.
    Upon Successful completion, returns 0.
    Returns -1 if error, with error in errno.

Send a Ring Followed by a Caller*ID Spill (occupies 6 seconds of time, or less)
(only works on FXO channels, that, obviously, are connected to an FXS device)
int zap_ringclid(ZAP *zap, char *number, char *name);
	zap is a pointer to an open Zapata device channel
	number is a pointer to a char buffer containing caller's number, or NULL
	    max number buffer length 10 chars. Should be 10 digits, or 'P'
	    to indicate 'private' (blocked), or 'O' to indicate out of area.
	name is a pointer to a char buffer containing caller's name, or NULL
	    max name buffer length 16 chars. Should be 16 alphanumerics, or 'P'
	    to indicate 'private' (blocked), or 'O' to indicate out of area.
    Upon Successful completion, returns 0. If answered after CLID is received,
	returns 1, and if answered before CLID is received returns 2.
    Returns -1 if error, with error in errno.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <sys/ioctl.h>
#include <dahdi/user.h>
#include <dahdi/wctdm_user.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "zap.h"
#include "zapinternal.h"
#include "tdd.h"

/* Global variables set up for the fsk decoding / encoding */
int spb,spb1;  /* these dont change, so they are okay as globals */
float dr[4],di[4]; /* these too */

/* Functions we get from goertzel.c */
extern void zap_goertzel_init(ZAP *zap);
extern int zap_goertzel_link(ZAP *zap,int inf, void *readbuffer, int mode, char *last,int *nsamp, int tedge);

/* Dial frequency tables */
typedef struct
{
char	chr;	/* character representation */
float	f1;	/* first freq */
float	f2;	/* second freq */
} ZAP_DIAL;

ZAP_DIAL dtmf_dial[] = {
{ '1',697.0,1209.0 },
{ '4',770.0,1209.0 },
{ '7',852.0,1209.0 },
{ '*',941.0,1209.0 },
{ '2',697.0,1336.0 },
{ '5',770.0,1336.0 },
{ '8',852.0,1336.0 },
{ '0',941.0,1336.0 },
{ '3',697.0,1477.0 },
{ '6',770.0,1477.0 },
{ '9',852.0,1477.0 },
{ '#',941.0,1477.0 },
{ 'A',697.0,1633.0 },
{ 'B',770.0,1633.0 },
{ 'C',852.0,1633.0 },
{ 'D',941.0,1633.0 },
{ 0,0,0 }
} ;

ZAP_DIAL mf_dial[] = {
{ '1',700.0,900.0 },
{ '2',700.0,1100.0 },
{ '3',900.0,1100.0 },
{ '4',700.0,1300.0 },
{ '5',900.0,1300.0 },
{ '6',1100.0,1300.0 },
{ '7',700.0,1500.0 },
{ '8',900.0,1500.0 },
{ '9',1100.0,1500.0 },
{ '0',1300.0,1500.0 },
{ '*',1100.0,1700.0 }, /* KP */
{ '#',1500.0,1700.0 }, /* ST */
{ 'A',900.0,1700.0 }, /* ST' */
{ 'B',1300.0,1700.0}, /* ST'' */
{ 'C',700.0,1700.0}, /* ST''' */
{ 0,0,0 }
} ;

/* signal handler for timeout */
static void zaptmo(int sig)
{
	return;
}

/* User-interface calls */

/* open a device channel */
ZAP *zap_open(char *devname /* device name (in ascii) */, int nonblock)
{
int fd,bs;
ZAP *zap;
struct dahdi_params  parms;
float	f0,f1,f2,f3;
int bf=0;
int chan = 0, x, isnum;

	if (nonblock)  /* if in non-block mode, set it that way */
		bf = O_NONBLOCK;
	isnum = 1; /* initialize to assume its a number */
	  /* go through device name string, and see if it all numeric */
	for (x=0;x<strlen(devname);x++) 
		if (!isdigit(devname[x]))  /* if this one not numeric */
		   {		   
			isnum =0; /* clear flag */
			break;
		   }
	if (isnum)  /* if it was just a number */
	   {
		chan = atoi(devname);  /* get channel number from string */
		if (chan < 1) /* if not valid, return as such */
		   {
			errno = -EINVAL;
			return NULL;
		   }
		devname="/dev/zap/channel";  /* use this for actual name in open call */
	   }
	  /* open up the device channel */
	fd = open(devname,O_RDWR | bf);
	  /* if unable to open, return with error */
	if (fd == -1) return(NULL);
	if (chan)  /* if was a numeric specification */
	   {
		  /* specify channel number on open fd */
		if (ioctl(fd, DAHDI_SPECIFY, &chan)) 
		   { /* if error, bail here */
			x = errno; /* save errno from ioctl call */
			close(fd);
			errno = x;
			return NULL;
		   }
	   }
	bs = ZAP_BLOCKSIZE;  /* set to our blocksize */
	   /* if unable to set blocksize, return with error */
	if (ioctl(fd,DAHDI_SET_BLOCKSIZE,&bs) == -1) return(NULL);
	  /* allocate memory for the ZAP structure */
	zap = (ZAP *)malloc(sizeof(ZAP));
	  /* if unable to allocate memory, return with error */
	if (zap == (ZAP *)NULL) return(NULL);
	  /* get channel parameters */
	if (ioctl(fd,DAHDI_GET_PARAMS,&parms) == -1) return(NULL);
	  /* zap the zap :-) (initialize structure) */
	memset(zap,0,sizeof(ZAP));
	memset(zap->dbuf,0x7f,ZAP_BLOCKSIZE);
	zap->fd = fd;		/* set the descriptor */
	zap->cr = 1.0;		/* initialize cr value for fsk tone genration */
	zap->ci = 0.0;		/* initialize ci value for fsk tone generation */
	zap->mycr = 1.0;	/* initialize cr value for local tone genration */
	zap->myci = 0.0;	/* initialize ci value for local tone generation */
	zap->mycr1 = 1.0;	/* initialize cr value for local tone genration */
	zap->myci1 = 0.0;	/* initialize ci value for local tone generation */
	zap->bp = 0;		/* fsk rx buffer pointer */
	zap->ns = 0;		/* silly fsk read buffer size */
	zap->x0 = 0.0;		/* stuff for dpll */
	zap->cont = 0.0;	/* stuff for dpll */
	zap->scont = 0.0;	/* stuff for clid sending */
	if (nonblock)
		zap->blocking = 0;
	else
		zap->blocking = 1;
	zap_goertzel_init(zap);	/* initialize goertzel stuff */
	  /* init tdd sending stuff */
	f0=1800.0;   /* space tone for TDD generation */
	f1=1400.0;   /* mark tone for TDD generation */
	f2=2200.0;   /* space tone for CLID generation */
	f3=1200.0;   /* mark tone for CLID generation */
	spb = 176;
	spb1 = 264;
	dr[0]=cos(f0*2.0*M_PI/8000.0);
	di[0]=sin(f0*2.0*M_PI/8000.0);
	dr[1]=cos(f1*2.0*M_PI/8000.0);
	di[1]=sin(f1*2.0*M_PI/8000.0);
	dr[2]=cos(f2*2.0*M_PI/8000.0);
	di[2]=sin(f2*2.0*M_PI/8000.0);
	dr[3]=cos(f3*2.0*M_PI/8000.0);
	di[3]=sin(f3*2.0*M_PI/8000.0);
	/* TDD read values setup */
	/* Valores por defecto */
	zap->fskd.spb=176;	/* 45.5 baudios */
	zap->fskd.hdlc=0;		/* Modo asíncrono */
	zap->fskd.nbit=5;		/* Baudot */
	zap->fskd.nstop=1.5;	/* Bits de Stop */
	zap->fskd.paridad=0;	/* Paridad ninguna */
	zap->fskd.bw=0;		/* Filtro 75 Hz */
	zap->fskd.f_mark_idx=0;	/* f_MARK = 1400 Hz */
	zap->fskd.f_space_idx=1;	/* f_SPACE = 1800 Hz */
	zap->fskd.pcola=0;		/* Colas vacías */
	zap->law = parms.curlaw;	/* get current law type */
	/* reuturn here, since everything else in structure is zero */
	return(zap);
}

/* close a device channel */
int zap_close(ZAP *zap /* pointer to channel structure */)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	  /* close the channel */
	close(zap->fd);
	  /* free the allocated memory */
	free(zap);
	/* return ok */
	return(0);
}

/* clear DTMF buffer */
int zap_clrdtmf(ZAP *zap /* pointer to channel structure */)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	   /* clear the DTMF buffer */
	memset(zap->dtmfbuf,0,ZAP_MAXDTMF);
	   /* initialize the counter */
	zap->dtmfn = 0;
	   /* initialize the goertzel state */
	zap->goertzel_last = 0;
	   /* clear this silly one too */
	zap->dtmf_heard = 0;
	  /* clear MF level detect accumulator */
	zap->mflevel = 0;
	  /* clear the Steve DTMF stuff too */
	zap_dtmf_detect_init(&zap->udtmf);
	return(0);
}

/* clear DTMF buffer pointer */
int zap_clrdtmfn(ZAP *zap /* pointer to channel structure */)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	zap->dtmfn = 0;
	memset(zap->dtmfbuf, 0, ZAP_MAXDTMF);
	return(0);
}

/* set digit receive mode */
int zap_digitmode(ZAP *zap /* pointer to channel structure */,
int mode /* new digit mode */)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	   /* initialize the goertzel state */
	zap->goertzel_last = 0;
	  /* set the mode */
	zap->digit_mode = mode;
	return(0);
}

/* Lets us know if there is DTMF outstanding */

int zap_dtmfwaiting(ZAP *zap)
{
	return (strlen(zap->dtmfbuf) || zap->dtmf_heard);
}

/* Get DTMF chararcter(s). If complete, returns number of characters received,
otherwise returns 0 to indicate timeout, -1 for error, or -2 if hangup. */
int zap_getdtmf(ZAP *zap /* pointer to channel structure */,
	int max  /* max # of digits */, 
	char *term /* string containing termination chars (if any) */,
	char erase /* erase char (if any, otherwise ' ' or 0) */,
	int idtimeout /* inter-digit timeout in milliseconds, 0 for infinite */,
	int timeout /* function timeout in milliseconds, 0 for infinite */,
	int flags /* flags for getdtmf: ZAP_HOOKEXIT, ZAP_TIMEOUTOK */ )
{
int	c,i,samps,samplast;
char	readbuffer[ZAP_BLOCKSIZE];

	if ((!zap) || (max < 1) || (max > ZAP_MAXDTMF)) /* if bad arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	   /* if there is already a full buffer, our work is done */
	if (zap->dtmfn >= ZAP_MAXDTMF) return(zap->dtmfn);
	samps = 0;  /* initialize sample counter */
	samplast = 0;  /* initialize last tone hit sample number */
	/* Put in blocking mode for the time being */
	for(;;)  /* stay in loop */
	   {
		if (!zap->dtmf_heard)
		   {
			  /* if function timeout */
			if ((timeout && (samps > (timeout * 8))) ||
			  /* or interdigit timeout */		    
			    (idtimeout && samplast && 
				(samps > (samplast + (idtimeout * 8)))))
			   {
				  /* if timeout not okay, return here */
				if (!(flags & ZAP_TIMEOUTOK)) 
				   {
					  /* clear buffer count */
					zap->dtmfn = 0;
				   }
				  /* terminate buffer */
				zap->dtmfbuf[zap->dtmfn] = 0;
				return(zap->dtmfn); /* return count */
			   }
			   /* Wait for something to be ready to be read */
			   /* read a block, and goertzelize it if appropriate */
			c = zap_goertzel_link(zap,zap->fd,readbuffer,
				zap->digit_mode & ZAP_MF,&zap->goertzel_last,&samps,1);
			  /* if nothing pressed, do it again */
			if (c == 0) continue;
			if (c < 0)  /* if error returned from read */
			   {
				   /* if not an event, return with error */
				if (errno != ELAST) return(-1);
				   /* if error getting event, return with error */
				if (ioctl(zap->fd,DAHDI_GETEVENT,&i) == -1) return(-1);
				   /* if we dont care about */
				if (!(flags & ZAP_HOOKEXIT)) continue;
					/* Continue if it's not a hangup */
				if (i != DAHDI_EVENT_ONHOOK) continue;
				return(-2);  /* return indicating hangup */
			   }
		   }  /* if zap->dtmf_heard */
		else
		   {  /* simulate tone being pressed */
			c = zap->dtmf_heard;
			zap->dtmf_heard = 0;
		   }
		   /* save sample count here (tone hit) */
		samplast = samps;
		/* okay, it must be a valid DTMF character, then */
		  /* if to erase */
		if (erase && (c == erase))
		   {
			   /* clear dtmf buffer */
			zap->dtmfbuf[0] = 0;
			   /* clear counter */
			zap->dtmfn = 0;
			continue; /* keep running loop */
		   }
		   /* store character */
		zap->dtmfbuf[zap->dtmfn++] = c;
		   /* terminate buffer */
		zap->dtmfbuf[zap->dtmfn] = 0;
		   /* if MAX chars, return here */
		if (zap->dtmfn >= max) return(zap->dtmfn);
		  /* if to term, return */
		if (term && strchr(term,c)) return(zap->dtmfn);
	}
}

int zap_recpurge(ZAP *zap)
{
	zap->recsamps = 0;
	return 0;
}

/* Record a number of bytes into 'buf'.  Return # of bytes recorded, can be less
   than requested if an event occurs */

int zap_recchunk(ZAP *zap, void *buf, int len, int flags)
{
	int offset = 0;
	int size;
	int j,k;
	void *where;
	while(len) {
		size = len;
		if (size > ZAP_BLOCKSIZE)
			size = ZAP_BLOCKSIZE;
		if (zap->recsamps < size) {
			if (!zap->blocking) {
				j = DAHDI_IOMUX_READ | DAHDI_IOMUX_SIGEVENT;
				if (ioctl(zap->fd, DAHDI_IOMUX, &j) == -1) {
					fprintf(stderr, "DAHDI_MUX failed\n");
					return -1;
				}
				/* Return immediately if there is a pending event */
				if (j & DAHDI_IOMUX_SIGEVENT) {
					return offset;
				}
			}
			if (zap->linear)
				where = zap->recbuf + zap->recsamps;
			else
				where = ((unsigned char *)zap->recbuf) + zap->recsamps;
			if (flags & ZAP_DTMFINT) {
				k = zap_goertzel_link(zap, zap->fd, where, zap->digit_mode & ZAP_MF, &zap->goertzel_last, NULL, 2);
				zap->recsamps += ZAP_BLOCKSIZE;
				if (k > 0) {
					/* Return of DTMF was heard */
					zap->dtmf_heard = k;
					return offset;
				}
			} else {
				k = read(zap->fd, where, zap->linear ? ZAP_BLOCKSIZE * 2 : ZAP_BLOCKSIZE);
				zap->recsamps += ZAP_BLOCKSIZE;
			}

			if (k < 0) {
				if (errno == ELAST) return offset;
				return -1;
			}
		}
		memcpy(buf, zap->recbuf, zap->linear ? size * 2 : size);
		zap->recsamps -= size;
		buf += zap->linear ? size * 2 : size;
		offset += size;
		len -= size;
		/* Move the rest to the front if there is any */
		if (zap->recsamps)
			memmove(zap->recbuf, zap->recbuf + size, zap->linear ? zap->recsamps * 2 : zap->recsamps);
	}
	return offset;
}

/* Returns number of bytes sent, not like dahdi_play.  Can return < len if an event takes place.  Does not
   honor ZAP_HOOKEXIT */
int zap_playchunk(ZAP *zap, void *buf, int len, int flags)
{
	int offset = 0; 
	int size;
	int j,k;
	short readbuffer[ZAP_BLOCKSIZE];
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	zap->goertzel_last = 0;
	zap->dtmf_heard = 0;
	zap->nxfer = 0;
	while(len) {
		size = len;
		if (size > ZAP_BLOCKSIZE)
			size = ZAP_BLOCKSIZE;
		j = DAHDI_IOMUX_WRITE | DAHDI_IOMUX_SIGEVENT;
		if (flags & ZAP_DTMFINT) j |= DAHDI_IOMUX_READ;

		if (ioctl(zap->fd,DAHDI_IOMUX,&j) == -1) return(-1);

		if (j & DAHDI_IOMUX_SIGEVENT) 
			return offset;

		if (j & DAHDI_IOMUX_READ) {
			/* get the DTMF */
			k = zap_goertzel_link(zap,zap->fd,(void *)readbuffer,
					    zap->digit_mode & ZAP_MF,&zap->goertzel_last,NULL,1);
		    /* if got a DTMF, return as such */
			if (k > 0) {
				  /* save the DTMF info */
				zap->dtmf_heard = k;
				return offset;
			}
			if (k < 0) {
						   /* if error, return as such */
				if (errno != ELAST) return(-1);
				return offset;
			}
		}
		if (j & DAHDI_IOMUX_WRITE) {
			 /* do the write */
			k = write(zap->fd,buf,zap->linear ? size * 2 : size);
			if (k < 0) {
				if (errno == ELAST)
					return offset;
				else
					return -1;
			}
			buf += k;
			if (zap->linear)
				k >>= 1;
			  /* update total xfer count */
			zap->nxfer += k;
			len -= k;
			offset += k;
		}
	}
	return offset;
}

int zap_play(ZAP *zap /* pointer to channel structure */,
	ZAPIO *iop /* pointer to io block */,
	int flags /* flags: ZAP_DTMFINT, ZAP_HOOKEXIT */ )
{
int	i,j,k,n,nbytes;
char	*cp,buf[ZAP_BLOCKSIZE],readbuffer[ZAP_BLOCKSIZE];

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	zap->goertzel_last = 0;
	zap->dtmf_heard = 0;
	zap->nxfer = 0;
	for(n = 0;iop[n].p;n++) /* loop thru IO block */
	   {
		  /* clear byte counter */
		nbytes = 0;
		   /* if a file, set to correct beginning */
		if (iop[n].offset != -1)
		   {
			  /* if cant seek to desired loc., ret. with error */
			if (fseek((FILE *) iop[n].p, iop[n].offset, 
				SEEK_SET) == -1) return(-1);
		   }
		for(;;) /* loop thru recording */
		   {
			if (iop[n].offset == -1) /* if memory */
			   {
				   /* point to memory location to start at */
				cp = (char *)iop[n].p + nbytes;
				   /* normal amount to output */
				i = ZAP_BLOCKSIZE;
			   }
			else
			   {
				cp = buf; /* set it to our buf */
				   /* read from the file */
				i = fread(buf,1,ZAP_BLOCKSIZE,(FILE *) iop[n].p);
				  /* return error if read error */
				if (i < 0) return(-1);
			   }
			  /* if max specified, and past it */
			if (iop[n].length && ((nbytes + i) > iop[n].length))
			   {
				i = iop[n].length - nbytes;  /* do only whats requested */
			   }
			if (!i) break; /* if no more, do next one */
			/* cp now points to a block of memory to output i bytes long */
			for(;;) /* iomux loop */
			   {
				   /* set bits of interest */
				j = DAHDI_IOMUX_WRITE | DAHDI_IOMUX_SIGEVENT;
				   /* if can interrupt, wanna do read too */
				if (flags & ZAP_DTMFINT) j |= DAHDI_IOMUX_READ;
				    /* wait for some happening */
				if (ioctl(zap->fd,DAHDI_IOMUX,&j) == -1) return(-1);
				   /* if was a signalling event */
				if (j & DAHDI_IOMUX_SIGEVENT)
				   {
					   /* get event */
					if (ioctl(zap->fd,DAHDI_GETEVENT,&k) == -1)
						return(-1);
					  /* if to return on exit, do so */
					if (flags & ZAP_HOOKEXIT) return(-2);
				   }
				if (j & DAHDI_IOMUX_READ)  /* if can read */
				   {
					  /* get the DTMF */
					k = zap_goertzel_link(zap,zap->fd,readbuffer,
					    zap->digit_mode & ZAP_MF,&zap->goertzel_last,NULL,1);
					   /* if got a DTMF, return as such */
					if (k > 0)
					   {
						  /* save the DTMF info */
						zap->dtmf_heard = k;
						return(1);
					   }
					if (k < 0)
					   {
						   /* if error, return as such */
						if (errno != ELAST) return(-1);
						/* was an event */
						   /* get event */
						if (ioctl(zap->fd,DAHDI_GETEVENT,&k) == -1)
							return(-1);
						  /* if to return on exit, do so */
						if (flags & ZAP_HOOKEXIT) return(-2);
					   }
				   }
				if (j & DAHDI_IOMUX_WRITE)  /* if can write */
				   {
					   /* do the write */
					k = write(zap->fd,cp,i);
					  /* update total xfer count */
					if (k > 0) zap->nxfer += k;
					   /* if successful, get outa loop */
					if (k == i) break;
					   /* if error, return as such */
					if (errno != ELAST) return(-1);
					/* was an event */
					   /* get event */
					if (ioctl(zap->fd,DAHDI_GETEVENT,&k) == -1)
						return(-1);
					  /* if to return on exit, do so */
					if (flags & ZAP_HOOKEXIT) return(-2);
				   }
			   }  /* end of iomux loop */
			   /* bump bytes count */
			nbytes += i;
			   /* if short block, this was end */
			if (i < ZAP_BLOCKSIZE) break;
		   } /* end of current recording */
	   } /* end of IO item loop */
	for(;;) /* iomux loop */
	   {
		   /* set bits of interest */
		j = DAHDI_IOMUX_WRITEEMPTY | DAHDI_IOMUX_SIGEVENT;
		   /* if can interrupt, wanna do read too */
		if (flags & ZAP_DTMFINT) j |= DAHDI_IOMUX_READ;
		    /* wait for some happening */
		if (ioctl(zap->fd,DAHDI_IOMUX,&j) == -1) return(-1);
		   /* if was a signalling event */
		if (j & DAHDI_IOMUX_SIGEVENT)
		   {
			   /* get event */
			if (ioctl(zap->fd,DAHDI_GETEVENT,&k) == -1)
				return(-1);
			  /* if to return on exit, do so */
			if (flags & ZAP_HOOKEXIT) return(-2);
		   }
		if (j & DAHDI_IOMUX_READ)  /* if can read */
		   {
			  /* get the DTMF */
			k = zap_goertzel_link(zap,zap->fd,readbuffer,
			    zap->digit_mode & ZAP_MF,&zap->goertzel_last,NULL,1);
			   /* if got a DTMF, return as such */
			if (k > 0)
			   {
				  /* save the DTMF info */
				zap->dtmf_heard = k;
				return(1);
			   }
			if (k < 0)
			   {
				   /* if error, return as such */
				if (errno != ELAST) return(-1);
				/* was an event */
				   /* get event */
				if (ioctl(zap->fd,DAHDI_GETEVENT,&k) == -1)
					return(-1);
				  /* if to return on exit, do so */
				if (flags & ZAP_HOOKEXIT) return(-2);
			   }
		   }
		if (j & DAHDI_IOMUX_WRITEEMPTY) break;
	   } /* end of iomux loop */
	return(0);
}

/* Convienience function for playing just one named file */

int zap_playf(ZAP *zap /* pointer to channel structure */,
	char *fname /* filename to play (in ascii) */,
	int flags /* flags: ZAP_DTMFINT, ZAP_HOOKEXIT */ )
{
ZAPIO	iop[2];
int	r;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	   /* clear struct */
	memset(iop,0,sizeof(iop));
	   /* attempt to open the file and point the iop pointer at it */
	iop[0].p = (void *) fopen(fname,"r");
	   /* if error, return as such */
	if (!iop[0].p) return(-1);
	/* everything else is set correctly in the iop structure */
	r = zap_play(zap,iop,flags);  /* do the play */
	fclose(iop[0].p);  /* close the file */
	return(r);  /* return value returned by play (above) */
}

int zap_setlinear(ZAP *zap, int linear)
{
	int res;
	res = ioctl(zap->fd, DAHDI_SETLINEAR, &linear);
	if (res)
		return res;
	zap->linear = linear;
	zap->recsamps = 0;
	return 0;
}

int zap_setlaw(ZAP *zap, int law)
{
	int res;
	res = ioctl(zap->fd, DAHDI_SETLAW, &law);
	if (res)
		return res;
	zap->law = law;
	zap->recsamps = 0;
	return 0;
}

int zap_rectone(ZAP *zap, int flag);
/* Record audio into specified location(s) */
int zap_rec(ZAP *zap /* pointer to channel structure */,
	ZAPIO *iop /* pointer to io block */,
	int flags /* flags: ZAP_DTMFINT, ZAP_HOOKEXIT, ZAP_BEEPTONE */ )
{
int	i,j,k,n,nbytes,thislength;
char	readbuffer[ZAP_BLOCKSIZE];

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	if (flags & ZAP_BEEPTONE)  /* if to beep before recording */
	   {
		i = zap_rectone(zap, 0);  /* output beep */
		if (i) return(i);
		  /* wait for beep to finish output */
		/* if (ioctl(zap->fd,DAHDI_SYNC,&i) == -1) return(-1); */ 
		i = DAHDI_FLUSH_READ; /* flush the input buffer */
		if (ioctl(zap->fd,DAHDI_FLUSH,&i) == -1) return(-1);
		  /* suck in and discard 4 blocks (50ms) to get rid of echo of
			beep tone in beginning of recording */
		for(n = 0; n < 4; n++)
		   {
			if (read(zap->fd,readbuffer,sizeof(readbuffer)) 
				!= sizeof(readbuffer)) return(-1);
		   }
	   }
	zap->goertzel_last = 0;
	zap->dtmf_heard = 0;
	zap->nxfer = 0;
	zap->nsilence = 0;
	for(n = 0;iop[n].p;n++) /* loop thru IO block */
	   {
		thislength = iop[n].length;
		iop[n].length = 0;
		  /* clear byte counter */
		nbytes = 0;
		   /* if a file, set to correct beginning */
		if (iop[n].offset != -1)
		   {
			  /* if cant seek to desired loc., ret. with error */
			if (fseek((FILE *) iop[n].p, iop[n].offset, 
				SEEK_SET) == -1) return(-1);
		   }
		for(;;) /* loop thru recording */
		   {
			i = ZAP_BLOCKSIZE;
			  /* if max specified, and past it */
			if (thislength && ((nbytes + i) > thislength))
			   {
				i = thislength - nbytes;  /* do only whats requested */
			   }
			if (!i) break; /* if no more, do next one */
			/* i now has the number of bytes to write this time */
			for(;;) /* iomux loop */
			   {
				   /* set bits of interest */
				j = DAHDI_IOMUX_READ | DAHDI_IOMUX_SIGEVENT;
				    /* wait for some happening */
				if (ioctl(zap->fd,DAHDI_IOMUX,&j) == -1) return(-1);
				   /* if was a signalling event */
				if (j & DAHDI_IOMUX_SIGEVENT)
				   {
					   /* get event */
					if (ioctl(zap->fd,DAHDI_GETEVENT,&k) == -1)
						return(-1);
					  /* if to return on exit, do so */
					if (flags & ZAP_HOOKEXIT) return(-2);
				   }
				if (j & DAHDI_IOMUX_READ)  /* if can read */
				   {
					  /* get the DTMF */
					k = zap_goertzel_link(zap,zap->fd,readbuffer,
					    zap->digit_mode & ZAP_MF,&zap->goertzel_last,NULL,1);
					if (k < 0) /* if was an error */
					   {
						   /* if error, return as such */
						if (errno != ELAST) return(-1);
						/* was an event */
						   /* get event */
						if (ioctl(zap->fd,DAHDI_GETEVENT,&k) == -1)
							return(-1);
						  /* if to return on exit, do so */
						if (flags & ZAP_HOOKEXIT) return(-2);
					   }
					/* have the return here, so if there
					   is a beep, you dont get it in the 
						recording */
					   /* if got a DTMF, return as such */
					if (k > 0)
					   {
						if (nbytes >= ZAP_BLOCKSIZE)
						   {
							nbytes -= ZAP_BLOCKSIZE;
						   }
					        iop[n].length = nbytes;
						  /* if file, truncate it */
						if (iop[n].offset != -1)
						   {
						      /* put file pointer in 
							reasonable place */	
						    fseek((FILE *) iop[n].p,
							iop[n].offset + (nbytes - 1),SEEK_SET);
						      /* truncate file to 1 
							block less */
						    if (ftruncate(fileno((FILE *
						      ) iop[n].p),nbytes +
							iop[n].offset) == -1)
							return(-1);
						   }
						   /* zero rest of lengths */
						for(n++; iop[n].p; n++)
						     iop[n].length = 0;
						  /* save the DTMF info */
						zap->dtmf_heard = k;
						return(1);
					   }
					/* write out the recorded block */
					if (iop[n].offset == -1) /* if memory */
					   {
						  /* copy this block into memory */
						memcpy((char *)iop[n].p + 
							nbytes, readbuffer,i);
					   }
					else /* is a file */
					   {
						  /* if cant write, ret. error */
						if (fwrite(readbuffer,1,i,
						    (FILE *)iop[n].p) != i)
							return(-1);
					   }
					  /* if silence detected, return if appropriate */
					if ((flags & ZAP_SILENCEINT) &&
					    (zap->nsilence > 40000)) return(2);
				   }
				break;
			   }  /* end of iomux loop */
			   /* bump bytes count */
			nbytes += i;
			   /* if short block, this was end */
			if (i < ZAP_BLOCKSIZE) break;
		   } /* end of current recording */
		  /* if memory, store length */
		if (iop[n].offset == -1) iop[n].length = nbytes;
		  /* add to total xfer count, also */
		zap->nxfer += nbytes;
	   } /* end of IO item loop */
	return(0);
}

/* Convienience function for recording just one named file */

int zap_recf(ZAP *zap /* pointer to channel structure */,
	char *fname /* filename to play (in ascii) */,
	int maxlen /* max length in bytes, 0 for no max */,
	int flags /* flags: ZAP_DTMFINT, ZAP_HOOKEXIT, ZAP_BEEPTONE */ )
{
ZAPIO	iop[2];
int	r;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	   /* clear struct */
	memset(iop,0,sizeof(iop));
	   /* attempt to open the file and point the iop pointer at it */
	iop[0].p = (void *) fopen(fname,"w");
	   /* if error, return as such */
	if (!iop[0].p) return(-1);
	iop[0].length = maxlen;
	/* everything else is set correctly in the iop structure */
	r = zap_rec(zap,iop,flags);  /* do the play */
	fclose(iop[0].p);  /* close the file */
	return(r);  /* return value returned by play (above) */
}

/* set hook switch state */
int zap_sethook(ZAP *zap /* pointer to channel structure */,
	int hookstate /* ZAP_OFHOOK or ZAP_ONHOOK */ )
{
int	i;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	i = DAHDI_ONHOOK;
	if (hookstate) i = DAHDI_OFFHOOK;
	return(ioctl(zap->fd,DAHDI_HOOK,&i));
}

/* send wink to trunk */
int zap_wink(ZAP *zap /* pointer to channel structure */)
{
int	i;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	i = DAHDI_WINK;
	return(ioctl(zap->fd,DAHDI_HOOK,&i));
}

/* start a trunk */
int zap_start(ZAP *zap /* pointer to channel structure */)
{
int	i;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	i = DAHDI_START;
	return(ioctl(zap->fd,DAHDI_HOOK,&i));
}

/* flash hookswitch */
int zap_flash(ZAP *zap /* pointer to channel structure */)
{
int	i;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	i = DAHDI_FLASH;
	return(ioctl(zap->fd,DAHDI_HOOK,&i));
}

/* ring station */
int zap_ring(ZAP *zap /* pointer to channel structure */,
int nrings /* number of rings, 0 for infinite */)
{
int	i,n;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	n = 0; /* init counter */
	i = DAHDI_RINGOFF;  /* start fresh */
	  /* this may return error if not presently in ringing state. its ok. */
	ioctl(zap->fd,DAHDI_HOOK,&i);
	i = DAHDI_RING;  /* ring phone once */
	  /* if error, return as such */
	if(ioctl(zap->fd,DAHDI_HOOK,&i) == -1) return(-1);
	for(;;) /* loop forever */
	   {
		i = zap_waitevent(zap); /* wait for event */
		if (i < 0) return(-1); /* if error */
		  /* if answered, get outa here and report as such */
		if (i == ZAP_EVENT_RINGANSWER)
		   {
			i = DAHDI_RINGOFF;  /* turn ringing seq. off */
			  /* if error, return as such */
			if(ioctl(zap->fd,DAHDI_HOOK,&i) == -1) return(-1);
			return(1);
		   }
		if (i != ZAP_EVENT_RINGEROFF) continue;
		  /* if too many rings, get outa here */
		if (nrings && (++n >= nrings)) break;
	   }
	i = DAHDI_RINGOFF;  /* turn ringing seq. off */
	  /* if error, return as such */
	if(ioctl(zap->fd,DAHDI_HOOK,&i) == -1) return(-1);
	return(0);
}

/* Get device channel params */
int zap_getparams(ZAP *zap /* pointer to channel structure */,
void *params /* pointer to device-dependent parameter block */ )
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	return(ioctl(zap->fd,DAHDI_GET_PARAMS,(struct dahdi_params *) params));
}

/* Set device channel params */
int zap_setparams(ZAP *zap /* pointer to channel structure */,
void *params /* pointer to device-dependent parameter block */ )
{

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	return(ioctl(zap->fd,DAHDI_SET_PARAMS,(struct dahdi_params *) params));
}

/* Get an inbound signalling event if there is one */
int zap_getevent(ZAP *zap /* pointer to channel structure */ )
{
int	j;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	  /* get the event info */
	if (ioctl(zap->fd,DAHDI_GETEVENT,&j) == -1) return(-1);
	switch (j)  /* attempt to translate event */
	   {
	    case DAHDI_EVENT_ONHOOK:  /* hung up */
		return ZAP_EVENT_ONHOOK;
	    case DAHDI_EVENT_RINGOFFHOOK:	  /* rang in or answered */
		return ZAP_EVENT_RINGANSWER;
	    case DAHDI_EVENT_WINKFLASH:  /* got wink */
		return ZAP_EVENT_WINKFLASH;
	    case DAHDI_EVENT_ALARM:  /* got alarm */
		return ZAP_EVENT_ALARM;
	    case DAHDI_EVENT_NOALARM: /* alarm went away */
		return ZAP_EVENT_NOALARM;
	    case DAHDI_EVENT_ABORT: /* got HDLC abort */
		return ZAP_EVENT_ABORT;
	    case DAHDI_EVENT_OVERRUN: /* got HDLC overrun */
		return ZAP_EVENT_OVERRUN;
	    case DAHDI_EVENT_BADFCS: /* got HDLC bad FCS */
		return ZAP_EVENT_BADFCS;
	    case DAHDI_EVENT_DIALCOMPLETE: /* dial complete */
		return ZAP_EVENT_DIALCOMPLETE;
	    case DAHDI_EVENT_RINGERON: /* ringer on */
		return ZAP_EVENT_RINGERON;
	    case DAHDI_EVENT_RINGEROFF: /* ringer off */
		return ZAP_EVENT_RINGEROFF;
	    case DAHDI_EVENT_HOOKCOMPLETE: /* hook change complete */
		return ZAP_EVENT_HOOKCOMPLETE;
	    default:  /* otherwise, ignore it */
		return ZAP_EVENT_NOEVENT; /* no event */
	   }
}

/* Wait for an inbound signalling event and return it */
int zap_waitevent(ZAP *zap /* pointer to channel structure */ )
{
int	j;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	for(;;)
	   {
		   /* set bits of interest */
		j = DAHDI_IOMUX_SIGEVENT;
		    /* wait for some happening */
		if (ioctl(zap->fd,DAHDI_IOMUX,&j) == -1) return(-1);
		   /* again, if we dont have it */
		if (!(j & DAHDI_IOMUX_SIGEVENT)) continue;
		  /* get the event info */
		if (ioctl(zap->fd,DAHDI_GETEVENT,&j) == -1) return(-1);
		switch (j)  /* attempt to translate event */
		   {
		    case DAHDI_EVENT_ONHOOK:  /* hung up */
			return ZAP_EVENT_ONHOOK;
		    case DAHDI_EVENT_RINGOFFHOOK:	  /* rang in or answered */
			return ZAP_EVENT_RINGANSWER;
		    case DAHDI_EVENT_WINKFLASH:  /* got wink */
			return ZAP_EVENT_WINKFLASH;
		    case DAHDI_EVENT_ALARM:  /* got alarm */
			return ZAP_EVENT_ALARM;
		    case DAHDI_EVENT_NOALARM: /* alarm went away */
			return ZAP_EVENT_NOALARM;
		    case DAHDI_EVENT_ABORT: /* got HDLC abort */
			return ZAP_EVENT_ABORT;
		    case DAHDI_EVENT_OVERRUN: /* got HDLC overrun */
			return ZAP_EVENT_OVERRUN;
		    case DAHDI_EVENT_BADFCS: /* got HDLC bad FCS */
			return ZAP_EVENT_BADFCS;
		    case DAHDI_EVENT_DIALCOMPLETE: /* dial complete */
			return ZAP_EVENT_DIALCOMPLETE;
		    case DAHDI_EVENT_RINGERON: /* ringer on */
			return ZAP_EVENT_RINGERON;
		    case DAHDI_EVENT_RINGEROFF: /* ringer off */
			return ZAP_EVENT_RINGEROFF;
		    case DAHDI_EVENT_HOOKCOMPLETE: /* hook change complete */
			return ZAP_EVENT_HOOKCOMPLETE;
		    default:  /* otherwise, ignore it */
			break;
		   }
	   }
}

/* Flush all events for this channel */
int zap_flushevent(ZAP *zap /* pointer to channel structure */ )
{
int	i;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	i = DAHDI_FLUSH_EVENT;
	return(ioctl(zap->fd,DAHDI_FLUSH,&i));
}

/* Convenience function to wait for inbound call and answer it */
int zap_waitcall(ZAP *zap /* pointer to channel structure */,
int nrings /* number of rings to wait for */,
int hookstate /* hook state to set it to when answered */,
int timeout /* timeout in seconds */)
{
int	i,j;


	  /* if bad arg(s), return as such */
	if ((!zap) || (nrings < 1))
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	for(i = 0; i < nrings; i++) /* go thru all the rings */
	   {
		do  
		   {
			if (timeout) /* if more then first ring */
			   {
				  /* set up alarm signal to something benine */
				signal(SIGALRM,zaptmo);
				  /* let it interrupt system call */
				siginterrupt(SIGALRM,1);
				sigsetmask(0); /* un-mask it */
				alarm(timeout);  /* time-out after 6 seconds */
			   }
			j = zap_waitevent(zap); /* wait for ring */
			if (timeout) /* if more then first ring */
			   {
				alarm(0); /* clear time-out */
				signal(SIGALRM,SIG_DFL); /* clear handler */
			   }
			if (j < 0) return(-1);  /* return if error */
		   } while (j != ZAP_EVENT_RINGANSWER); /* until ring */
	   }
	return(zap_sethook(zap,hookstate));
}

/* Set Conference Mode */
int zap_conf(ZAP *zap /* pointer to channel structure */,
int chan /* channel number, or 0 for current */,
int confno /* conference number */,
int confmode /* conf mode, see zap.h for values */ )
{
struct	dahdi_confinfo	c;

c.chan = chan;
c.confno = confno;
c.confmode = confmode;

	return(ioctl(zap->fd,DAHDI_SETCONF,&c));
}

/* Get Conference Mode */
int zap_getconf(ZAP *zap /* pointer to channel structure */,
int chan /* channel number, or 0 for current */,
int *confnop /* conference number */,
int *confmodep /* conf mode, see zap.h for values */ )
{
struct	dahdi_confinfo	c;

	c.chan = chan;
	if (ioctl(zap->fd,DAHDI_GETCONF,&c) == -1) return(-1);
	if (confnop) *confnop = c.confno;
	if (confmodep) *confmodep = c.confmode;
	return(c.chan);
}

/* Clear all conference links */
int zap_clearlinks(ZAP *zap /* pointer to channel structure */)
{
struct	dahdi_confinfo	c;

	c.chan = c.confno = c.confmode = 0;
	return(ioctl(zap->fd,DAHDI_CONFLINK,&c));
}

/* Make a Conference link */
int zap_makelink(ZAP *zap /* pointer to channel structure */,
int dstconfno /* destination conference number (the one to do the listening) */,
int srcconfno /* source conference number (the one to be listened to) */ )
{
struct	dahdi_confinfo	c;

	c.chan = srcconfno;
	c.confno = dstconfno;
	c.confmode = 1;  /* set to create link */
	return(ioctl(zap->fd,DAHDI_CONFLINK,&c));
}

/* Break a Conference link */
int zap_breaklink(ZAP *zap /* pointer to channel structure */,
int dstconfno /* destination conference number (the one to do the listening) */,
int srcconfno /* source conference number (the one to be listened to) */ )
{
struct	dahdi_confinfo	c;

	c.chan = srcconfno;
	c.confno = dstconfno;
	c.confmode = 0;  /* set to destroy link */
	return(ioctl(zap->fd,DAHDI_CONFLINK,&c));
}

/* Interface to TDD fsk send/receive routines */

extern int put_byte(ZAP *zap, int byte);

static int send_baudot(ZAP *zap,char ac)
{
static unsigned char lstr[31] = "\000E\nA SIU\rDRJNFCKTZLWHYPQOBG\000MXV";
static unsigned char fstr[31] = "\0003\n- \00787\r$4',!:(5\")2\0006019?&\000./;";
int	i;
char	c;

	c = toupper(ac);
	printf("%c",c); fflush(stdout);
	if (c == 0) /* send null */
	   {
		return(put_byte(zap,0));
	   }
	if (c == '\r') /* send c/r */
	   {
		return(put_byte(zap,8));
	   }
	if (c == '\n') /* send c/r and l/f */
	   {
		if (put_byte(zap,8)) return(-1);
		return(put_byte(zap,2));
	   }
	if (c == ' ') /* send space */
	   {
		return(put_byte(zap,4));
	   }
	for(i = 0; i < 31; i++)
	   {
		if (lstr[i] == c) break;
	   }
	if (i < 31) /* if we found it */
	   {
		if (zap->mode)  /* if in figs mode, change it */
		   {
			if (put_byte(zap,31)) return(-1);  /* send LTRS */
			zap->mode = 0;
		   }
		return(put_byte(zap,i));   /* send byte */
	   }
	for(i = 0; i < 31; i++)
	   {
		if (fstr[i] == c) break;
	   }
	if (i < 31) /* if we found it */
	   {
		if (!zap->mode)  /* if in ltrs mode, change it */
		   {
			if (put_byte(zap,27)) return(-1);  /* send FIGS */
			zap->mode = 1;
		   }
		return(put_byte(zap,i));   /* send byte */
	   }
	return(0);
}

/*
** This routine converts from linear to ulaw
**
** Craig Reese: IDA/Supercomputing Research Center
** Joe Campbell: Department of Defense
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) "A New Digital Technique for Implementation of Any
**     Continuous PCM Companding Law," Villeret, Michel,
**     et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
**     1973, pg. 11.12-11.17
** 3) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: Signed 16 bit linear sample
** Output: 8 bit ulaw sample
*/

#define ZEROTRAP    /* turn on the trap as per the MIL-STD */
#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

static unsigned char linear2ulaw(short sample) {
static int exp_lut[256] = {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
                             4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
  int sign, exponent, mantissa;
  unsigned char ulawbyte;

  /* Get the sample into sign-magnitude. */
  sign = (sample >> 8) & 0x80;          /* set aside the sign */
  if (sign != 0) sample = -sample;              /* get magnitude */
  if (sample > CLIP) sample = CLIP;             /* clip the magnitude */

  /* Convert from 16 bit linear to ulaw. */
  sample = sample + BIAS;
  exponent = exp_lut[(sample >> 7) & 0xFF];
  mantissa = (sample >> (exponent + 3)) & 0x0F;
  ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
  if (ulawbyte == 0) ulawbyte = 0x02;   /* optional CCITT trap */
#endif

  return(ulawbyte);
}

static inline int top_bit (unsigned int bits)
{
    int res;

    __asm__ __volatile__(" bsrl %%eax,%%eax;"
                         : "=a" (res)
                         : "a" (bits));
    return res;
}
/*- End of function --------------------------------------------------------*/ 

/*
 * A-law is basically as follows:
 *
 *      Linear Input Code        Compressed Code
 *      -----------------        ---------------
 *      0000000wxyza             000wxyz
 *      0000001wxyza             001wxyz
 *      000001wxyzab             010wxyz
 *      00001wxyzabc             011wxyz
 *      0001wxyzabcd             100wxyz
 *      001wxyzabcde             101wxyz
 *      01wxyzabcdef             110wxyz
 *      1wxyzabcdefg             111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */

#define AMI_MASK	0x55

static unsigned char linear2alaw (short linear)
{
    int mask;
    int seg;
    int pcm_val;
#if !defined(__i386__)
    static int seg_end[8] =
    {
         0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF
    };
#endif
    
    pcm_val = linear;
    if (pcm_val >= 0)
    {
        /* Sign (7th) bit = 1 */
        mask = AMI_MASK | 0x80;
    }
    else
    {
        /* Sign bit = 0 */
        mask = AMI_MASK;
        pcm_val = -pcm_val;
    }

    /* Convert the scaled magnitude to segment number. */
#if defined(__i386__)
    seg = top_bit (pcm_val | 0xFF) - 7;
#else
    for (seg = 0;  seg < 8;  seg++)
    {
        if (pcm_val <= seg_end[seg])
	    break;
    }
#endif
    /* Combine the sign, segment, and quantization bits. */
    return  ((seg << 4) | ((pcm_val >> ((seg)  ?  (seg + 3)  :  4)) & 0x0F)) ^ mask;
}
/*- End of function --------------------------------------------------------*/

/* finish writing out the audio buffer (if any). Returns 0 if not interrupted 
by DTMF, 1 if interrupted by DTMF, or -1 if error, or -2 if hung up . Also
sets zap->nxfer with total number of bytes padded out */
int zap_finish_audio(ZAP *zap,int flag)
{
int	i;
char	readbuffer[ZAP_BLOCKSIZE];

	zap->nxfer = 0;  /* clear count */
	if (zap->p)  /* if something to write */
	   {
		while(zap->p < ZAP_BLOCKSIZE)   /* pad with silence */
		   {		
			zap->buf[zap->p] = LINEAR2XLAW(0,zap->law); 
			zap->p++;
			zap->nxfer++; /* bump pad count */
		   }
		zap->p = 0;
		  /* write the block */
		if (write(zap->fd,zap->buf,ZAP_BLOCKSIZE) != ZAP_BLOCKSIZE)
		   {
			if (errno == ELAST) return(-2);
			return(-1);
		   }
	}
	/* if not in DTMF mode, ignore */
	if (!flag) return(0);
	  /* set up for IOMUX to see if read is available */
	i = DAHDI_IOMUX_READ | DAHDI_IOMUX_NOWAIT;
	  /* check to see if read is available */
	if (ioctl(zap->fd,DAHDI_IOMUX,&i) == -1) return(-1);
	  /* if read not ready, just return */
	if (!(i & DAHDI_IOMUX_READ)) return(0);
	  /* get the DTMF */
	i = zap_goertzel_link(zap,zap->fd,readbuffer,
	    zap->digit_mode & ZAP_MF,&zap->goertzel_last,NULL,0);
	if (i < 0) /* if was an error */
	   {
		   /* if error, return as such */
		if (errno == ELAST) return(-2); else return(-1);
	   }
	if (i) /* if DTMF heard */
	   {
		  /* save the DTMF info */
		zap->dtmf_heard = i;
		return(1);  /* if DTMF hit */
	   }
	return(0);
}

int put_audio_sample(ZAP *zap,float y,int flag)
{
int	i;
char	readbuffer[ZAP_BLOCKSIZE];
#if 0
	int index = (short)rint(8192.0 * y);
	printf(" %04x", index + 32768); 
#endif
	zap->buf[zap->p]=LINEAR2XLAW((short)rint(8192.0 * y),zap->law);
	zap->p++;
	if (zap->p== ZAP_BLOCKSIZE) {
		zap->p=0;
		if (write(zap->fd,zap->buf,ZAP_BLOCKSIZE) != ZAP_BLOCKSIZE)
		   {
			if (errno == ELAST) return(-2);
			return(-1);
		   }
	}
	/* if not in DTMF mode, ignore */
	if (!flag) return(0);
	  /* set up for IOMUX to see if read is available */
	i = DAHDI_IOMUX_READ | DAHDI_IOMUX_NOWAIT;
	  /* check to see if read is available */
	if (ioctl(zap->fd,DAHDI_IOMUX,&i) == -1) return(-1);
	  /* if read not ready, just return */
	if (!(i & DAHDI_IOMUX_READ)) return(0);
	  /* get the DTMF */
	i = zap_goertzel_link(zap,zap->fd,readbuffer,
	    zap->digit_mode & ZAP_MF,&zap->goertzel_last,NULL,1);
	if (i < 0) /* if was an error */
	   {
		   /* if error, return as such */
		if (errno == ELAST) return(-2); else return(-1);
	   }
	if (i) /* if DTMF heard */
	   {
		  /* save the DTMF info */
		zap->dtmf_heard = i;
		return(1);  /* if DTMF hit */
	   }
	return(0);
}

float get_carrier(ZAP *zap,int bit)
{
	float t;
	
	t =zap->cr*dr[bit]-zap->ci*di[bit];
	zap->ci=zap->cr*di[bit]+zap->ci*dr[bit];
	zap->cr=t;
	
	t=2.0-(zap->cr*zap->cr+zap->ci*zap->ci);
	zap->cr*=t;
	zap->ci*=t;
	
	return zap->cr;
};

/* send out a millisecond of CLID mark tone */
int put_clid_markms(ZAP *zap)
{
int	i,r;

	  /* do 8 samples of it */
	for(i = 0; i < 8; i++)
		if ((r = put_audio_sample(zap,get_carrier(zap,3),0))) return(r);
	return(0);
}

/* put out a single baud (bit) of FSK data (for CLID) */
static int put_clid_baud(ZAP *zap, int which)
{
int r;
float clidsb;

	clidsb = 8000.0 / 1200.0;  /* fractional baud time */
	for(;;)  /* loop until no more samples to send */
	   {
		  /* output the sample */
		if ((r = put_audio_sample(zap,get_carrier(zap,which + 2),0))) return(r);
		zap->scont += 1.0;  /* bump by 1 sample time */
		if (zap->scont > clidsb)  /* if greater then 1 baud in real time */
		   {
			zap->scont -= clidsb;  /* leave remainder around for next one */
			break;
		   }
	   }
	return(0);
}

/* Output a character of CLID FSK data */
int put_clid(ZAP *zap,int by)
{
	int j,k,r;
	  /* send start bit */
	if ((r = put_clid_baud(zap,0))) return(r);
	  /* send byte */
	for (j=0;j<8;j++) {
		k=by&1;
		by>>=1;
		if ((r = put_clid_baud(zap,k))) return(r);
	}
	  /* send stop bit */
	if ((r = put_clid_baud(zap,1))) return(r);
	return 0;
}

int put_byte(ZAP *zap,int by)
{
	int i,j,k,r;
	
	  /* send start bit */
	for (i=0;i<spb;i++) if ((r = put_audio_sample(zap,get_carrier(zap,0),0))) return(r);
	for (j=0;j<5;j++) {
		k=by&1;
		by>>=1;
		for (i=0;i<spb;i++) if ((r = put_audio_sample(zap,get_carrier(zap,k),0))) return(r);
	}
	  /* send stop bit */
	for (i=0;i<spb1;i++) if ((r = put_audio_sample(zap,get_carrier(zap,1),0))) return(r);
	return 0;
}

int zap_sendtdd(ZAP *zap /* pointer to channel structure */,
	char *string /* string in ASCII to send */)
{
	if (put_byte(zap,31)) /* force ltrs mode */
	   {
		if (errno == ELAST) return(-2);
		return(-1);
	   }
	while(*string)
	   {
		if (send_baudot(zap,*string++)) 
		   {
			if (errno == ELAST) return(-2);
			return(-1);
		   }
	   }
	return(0);
}

int zap_gettdd(ZAP *zap /* pointer to channel structure */)
{
int	b,c;

	for(;;)
	   {
		  /* get baudot char */
		b = serie(zap);
		if (b == -1)
		   {
			if (errno == ELAST) return(-2);
			return(-1);
		   }
		  /* if null, do it again */
		if (!b) continue;
		  /* if in Caller*ID mode, return with Ascii */
		if (zap->fskd.nbit == 8) return(b);
		  /* if error, do it again */
		if (b > 0x7f) continue;
		c = baudot(zap,b); /* translate to ascii */
		  /* if a valid char */
		if ((c > 0) && (c < 127)) break;
	   }
	return(c);
}

int get_audio_sample(ZAP *zap,float *retval)
{
	extern int fullmulaw[],fullalaw[];

	if (zap->bp==zap->ns) {
		zap->ns=read(zap->fd,zap->audio_buf,ZAP_BLOCKSIZE);
		zap->bp=0;
		if (zap->ns != ZAP_BLOCKSIZE)
		   {
			zap->ns = 0;
			return(-1);
		   }
	}
	*retval = (float)(FULLXLAW(zap->audio_buf[zap->bp++],zap->law) / 256);
	return(0);
}

float get_localtone(ZAP *zap,float ddr,float ddi)
{

	float t;
	
	t =zap->mycr*ddr-zap->myci*ddi;
	zap->myci=zap->mycr*ddi+zap->myci*ddr;
	zap->mycr=t;
	
	t=2.0-(zap->mycr*zap->mycr+zap->myci*zap->myci);
	zap->mycr*=t;
	zap->myci*=t;
	
	return zap->mycr;
};

float get_localtones(ZAP *zap,float ddr,float ddi,float ddr1, float ddi1)
{

	float t;
	
	t =zap->mycr*ddr-zap->myci*ddi;
	zap->myci=zap->mycr*ddi+zap->myci*ddr;
	zap->mycr=t;
	
	t=2.0-(zap->mycr*zap->mycr+zap->myci*zap->myci);
	zap->mycr*=t;
	zap->myci*=t;
	
	t =zap->mycr1*ddr1-zap->myci1*ddi1;
	zap->myci1=zap->mycr1*ddi1+zap->myci1*ddr1;
	zap->mycr1=t;
	
	t=2.0-(zap->mycr1*zap->mycr1+zap->myci1*zap->myci1);
	zap->mycr1*=t;
	zap->myci1*=t;
	
	return zap->mycr + zap->mycr1; 
};

int zap_disatone(ZAP *zap /* pointer to channel structure */,
int flag /* flag: ZAP_DTMFINT */)
{
float	f0 = 2100.0,ddr,ddi;
int	i,r;

	ddr = cos(f0*2.0*M_PI/8000.0);
	ddi = sin(f0*2.0*M_PI/8000.0);
	for(i = 0; i < (2 * FS); i++) if ((r = put_audio_sample(zap,get_localtone(zap,ddr,ddi),flag))) return(r);
	return 0;
}

int zap_rectone(ZAP *zap /* pointer to channel structure */,
int flag /* flag: ZAP_DTMFINT */)
{
float	f0 = 1000.0,ddr,ddi;
int	i,r;

	ddr = cos(f0*2.0*M_PI/8000.0);
	ddi = sin(f0*2.0*M_PI/8000.0);
	for(i = 0; i < (FS / 3); i++) if ((r = put_audio_sample(zap,get_localtone(zap,ddr,ddi),flag))) return(r);
	return 0;
}

/* send silence */
int zap_silence(ZAP *zap /* pointer to channel structure */,
int len /* length in ms */, 
int flag /* flag: ZAP_DTMFINT */)
{
int	i,r;

	for(i = 0; i < (len * 8); i++) if ((r = put_audio_sample(zap,0,flag))) return(r);
	return 0;
}

/* send arbitrary tone */
int zap_arbtone(ZAP *zap /* pointer to channel structure */,
float f0 /* tone 1 freq in hz */,
int len /* length in ms */,
int flag /* flag: ZAP_DTMFINT */)
{
float	ddr,ddi;
int	i,r;

	ddr = cos(f0*2.0*M_PI/8000.0);
	ddi = sin(f0*2.0*M_PI/8000.0);
	for(i = 0; i < (len * 8); i++) if ((r = put_audio_sample(zap,get_localtone(zap,ddr,ddi) / 2,flag))) return(r);
	return 0;
}

/* send arbitrary tone pair */
int zap_arbtones(ZAP *zap /* pointer to channel structure */,
float f0 /* tone 1 freq in hz */,
float f1 /* tone 2 freq in hz */,
int len /* length in ms */,
int flag /* flag: ZAP_DTMFINT */)
{
float	ddr,ddi,ddr1,ddi1;
int	i,r;

	ddr = cos(f0*2.0*M_PI/8000.0);
	ddi = sin(f0*2.0*M_PI/8000.0);
	ddr1 = cos(f1*2.0*M_PI/8000.0);
	ddi1 = sin(f1*2.0*M_PI/8000.0);
	for(i = 0; i < (8 * len); i++) 
	if ((r = put_audio_sample(zap,get_localtones(zap,ddr,ddi,ddr1,ddi1) / 2,flag))) return(r);
	return 0;
}

extern int fullmulaw[],fullalaw[];

static int set_actual_gain(int fd, int chan, float rxgain, float txgain, int law)
{
struct	dahdi_gains g;
	float ltxgain;
	float lrxgain;
	int j,k;
	g.chan = chan;
	  /* caluculate linear value of tx gain */
	ltxgain = pow(10.0,txgain / 20.0);
	  /* caluculate linear value of rx gain */
	lrxgain = pow(10.0,rxgain / 20.0);
	for (j=0;j<256;j++) {
		k = (int)(((float)FULLXLAW(j,law)) * lrxgain);
		if (k > 32767) k = 32767;
		if (k < -32767) k = -32767;
		g.rxgain[j] = LINEAR2XLAW(k,law);
		k = (int)(((float)FULLXLAW(j,law)) * ltxgain);
		if (k > 32767) k = 32767;
		if (k < -32767) k = -32767;
		g.txgain[j] = LINEAR2XLAW(k,law);
	}
		
	  /* set 'em */
	return(ioctl(fd,DAHDI_SETGAINS,&g));
}

/* set receive and transmit gain in db */
int zap_gains(ZAP *zap /* pointer to channel structure */,
float rxgain  /* receive gain in db */ ,
float txgain  /* transmit gain in db */ )
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	zap->txgain = txgain;
	zap->rxgain = rxgain;
	return set_actual_gain(zap->fd, 0, zap->txgain, zap->rxgain, zap->law);
}

/* get channel number */
int zap_channel(ZAP *zap /* pointer to channel structure */)
{
struct	dahdi_gains g;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	g.chan = 0;
	  /* get gains, which also returns channel */
	if (ioctl(zap->fd,DAHDI_GETGAINS,&g) == -1) return(-1);
	return(g.chan);
}

/* dial a digit string */
int zap_dial(ZAP *zap /* pointer to channel structure */,
char *digitstring /* digit string to dial in ascii */,
int length /* length of tones in ms */)
{
int	i,j,k,len,r;
ZAP_DIAL *d;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	  /* must have something to dial */
	if (!digitstring) return(0);
	  /* if not specified, make it default */
	if (!length) len = ((zap->dial_mode) ? 
		ZAP_DEFAULT_MFLENGTH : ZAP_DEFAULT_DTMFLENGTH);
	else len = length;
	for(i = 0; digitstring[i]; i++)
	   {
		if (digitstring[i] == 'T')  /* for DTMF mode */
		   {
			zap->dial_mode = ZAP_DTMF;
			  /* if not specified, make it default */
			if (!length) len = ZAP_DEFAULT_DTMFLENGTH;
			continue;
		   }
		if (digitstring[i] == 'M')  /* for MF mode */
		   {
			zap->dial_mode = ZAP_MF;
			  /* if not specified, make it default */
			if (!length) len = ZAP_DEFAULT_MFLENGTH;
			continue;
		   }
		d = ((zap->dial_mode) ? mf_dial : dtmf_dial);
		for(j = 0; d[j].chr; j++)
		   {
			if(d[j].chr == digitstring[i]) break;
		   }
		  /* if not found, just skip */
		if (!d[j].chr) continue;
		k = len;
		  /* MF KP needs to be longer */
		if ((zap->dial_mode) && (digitstring[i] == '*'))
		   {
			k = len * 5 / 3;
 		   }
		  /* send da tone */
		if ((r = zap_arbtones(zap,d[j].f1,d[j].f2,k,0))) return(r); 
		  /* silence too */
		if ((r = zap_silence(zap,len,0))) return(r);
	   }
	return(zap_finish_audio(zap,0));  /* finish writing audio */
}

/* get a Caller*ID string */
int zap_clid(ZAP *zap /* pointer to channel structure */,
char *number /* pointer to buffer in which to return caller's number */,
char *name /* pointer to buffer in which to return caller's name */)
{
int	c,cs,m,n,i,j,rv;
unsigned char msg[256];

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	
	set_actual_gain(zap->fd, 0, zap->txgain, zap->rxgain + 9.0, zap->law);
	
	do   /* wait for a ring */
	   {
		j = zap_waitevent(zap); /* wait for ring */
		if (j < 0) return(j);  /* return if error */
	   } while (j != ZAP_EVENT_RINGANSWER); /* until ring */
	/* Caller*ID read values setup */
	/* Valores por defecto */
	zap->fskd.spb=7;	/* 1200 baudios */
	zap->fskd.hdlc=0;		/* Modo asíncrono */
	zap->fskd.nbit=8;		/* Ascii */
	zap->fskd.nstop=1;	/* Bits de Stop */
	zap->fskd.paridad=0;	/* Paridad ninguna */
	zap->fskd.bw=1;		/* Filtro 800 Hz */
	zap->fskd.f_mark_idx=2;	/* f_MARK = 1200 Hz */
	zap->fskd.f_space_idx=3;	/* f_SPACE = 2200 Hz */
	zap->fskd.pcola=0;		/* Colas vacías */
	i = 0; /* reset flag-found flag */
	zap->cont = 0; /* reset DPLL thing */
	rv = 0; /* start with happy return value */
	errno = 0;
	for(;;)
	   {
		  /* clear output buffers, if specified */
		if (name) *name = 0;
		if (number) *number = 0;
		m = zap_gettdd(zap);  /* get first byte */
		if (m < 0) /* if error, return as such */
		   {
			rv = -1;
			break;
		   }
		if (m == 'U')  /* if a flag byte, set the flag */
		   {
			i = 1; /* got flag */
			continue;
		   }
		if (!i) continue; /* if no flag, go on */
		  /* if not message lead-in, go on */
		if ((m != 4) && (m != 0x80)) continue;
		n = zap_gettdd(zap); /* get message size */
		if (n < 0)  /* if error, return as such */
		   {
			rv = -1;
			break;
		   }
		j = m + n; /* initialize checksum with first 2 bytes */
		  /* collect the entire message */
		for(i = 0; i < n; i++)
		   {
			c = zap_gettdd(zap);  /* get next byte */
			if (c < 0)  /* if error, stop */
			   {
				rv = -1;
				break;
			   }
			msg[i] = c; /* save it */
			j += c;  /* add it to checksum */
		   }
		if (rv == -1) break; /* if error, return as such */
		msg[i] = 0; /* terminate the string */
		cs = zap_gettdd(zap); /* get checksum byte from message */
		if (cs < 0) /* if error, return as such */
		   {
			rv = -1;
			break;
		   }
		  /* if bad checksum, try it again */
		if (cs != 256 - (j & 255))
		   {
			i = 0; /* reset flag-found flag */
			zap->cont = 0; /* reset DPLL thing */
			continue;  /* do it again */
		   }
		if (m == 0x80) /* MDMF format */
		   {
			for(i = 0; i < n;)  /* go thru whole message */
			   {
				switch(msg[i++]) /* message type */
				   {
				    case 1: /* date */
					break;
				    case 2: /* number */
				    case 4: /* number */
					if (number) /* if number buffer specified */
					   {
						  /* copy it */
						strncpy(number,msg + i + 1,msg[i]);
						  /* terminate it */
						number[msg[i]] = 0;
					   }
					break;
				    case 7: /* name */
				    case 8: /* name */
					if (name) /* if name buffer specified */
					   {
						  /* copy it */
						strncpy(name,msg + i + 1,msg[i]);
						  /* terminate it */
						name[msg[i]] = 0;
					   }
					break;
				   }
				   /* advance index to next message element */
				i += msg[i] + 1; 
			   }
		   }
		else /* must be SDMF then */
		   {
			  /* if number buffer specified */
			if (number) strcpy(number,msg + 8); /* copy number */
		   }
		rv = 0; /* good, happy return */
		break;
	   }
	/* TDD read values setup */
	/* Valores por defecto */
	zap->fskd.spb=176;	/* 45.5 baudios */
	zap->fskd.hdlc=0;		/* Modo asíncrono */
	zap->fskd.nbit=5;		/* Baudot */
	zap->fskd.nstop=1.5;	/* Bits de Stop */
	zap->fskd.paridad=0;	/* Paridad ninguna */
	zap->fskd.bw=0;		/* Filtro 75 Hz */
	zap->fskd.f_mark_idx=0;	/* f_MARK = 1400 Hz */
	zap->fskd.f_space_idx=1;	/* f_SPACE = 1800 Hz */
	zap->fskd.pcola=0;		/* Colas vacías */
	  /* put original gains back */
	set_actual_gain(zap->fd, 0, zap->txgain, zap->rxgain, zap->law);
	  /* if hung up */
	if ((rv == -1) && (errno == ELAST)) return(-2);
	return(rv); /* return the return value */
}

/* ring the phone and send a Caller*ID string. Returns 0 if caller did not
answer during this function, 1 if caller answered after receiving CLID, and
2 if caller answered before receiving CLID */
int zap_ringclid(ZAP *zap /* pointer to channel structure */,
char *number /* pointer to buffer in which to return caller's number */,
char *name /* pointer to buffer in which to return caller's name */)
{
int	i,j,k,r;
char	msg[100];
time_t	t;
struct	tm *tm;

	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }

	zap->scont = 0.0; /* initialize fractional time accumulator */
	  /* get current time */
	time(&t);
	  /* put into usable form */
	tm = localtime(&t);
	  /* put date into message */
	sprintf(msg,"\001\010%02d%02d%02d%02d",tm->tm_mon + 1,tm->tm_mday,
		tm->tm_hour,tm->tm_min);
	   /* if number absence */
	if ((!number) || (number[0] == 'O') || (number[0] == 'P'))
	   {
		strcat(msg,"\004\001"); /* indicate number absence */
		if (number && (number[0] == 'O')) strcat(msg,"O");
			else strcat(msg,"P");
	   }
	else /* if its there */
	   {
		i = strlen(number);  /* get len of number */
		if (i > 10) i = 10;  /* limit at 10 */
		sprintf(msg + strlen(msg),"\002%c",i);  /* indicate number sequence */
		j = strlen(msg);
		  /* add number to end of message */
		for(k = 0; k < i; k++) msg[k + j] = number[k];
		msg[k + j] = 0;  /* terminate string */
	   }
	   /* if name absence */
	if ((!name) || (((name[0] == 'O') || (name[0] == 'P')) && !name[1]))
	   {
		strcat(msg,"\010\001"); /* indicate name absence */
		if (name && (name[0] == 'O')) strcat(msg,"O");
			else strcat(msg,"P");
	   }
	else /* if its there */
	   {
		i = strlen(name);  /* get len of name */
		if (i > 16) i = 16;  /* limit at 16 */
		sprintf(msg + strlen(msg),"\007%c",i);  /* indicate name sequence */
		j = strlen(msg);
		  /* add name to end of message */
		for(k = 0; k < i; k++) msg[k + j] = name[k];
		msg[k + j] = 0;  /* terminate string */
	   }
	  /* one ringy-dingy */
	i = DAHDI_RINGOFF;  /* make sure ringer is off */
	  /* do the ioctl to turn ringer off. if its already off it will ret. error. its okay. */
	ioctl(zap->fd,DAHDI_HOOK,&i);
	i = DAHDI_RING;  /* ring phone once */
	  /* if error, return as such */
	if(ioctl(zap->fd,DAHDI_HOOK,&i) == -1) return(-1);
	do /* loop until we get a ringer off or an answer */
	   {
		j = zap_waitevent(zap); /* wait for an event */
		if (j < 0) /* if error, return as such */
		   {
			i = DAHDI_RINGOFF;  /* make sure ringer is off */
			  /* do the ioctl to turn ringer off. if its already off it will ret. error. its okay. */
			ioctl(zap->fd,DAHDI_HOOK,&i);
			return(-1);
		   }			
	   }
	while ((j != ZAP_EVENT_RINGEROFF) && (j != ZAP_EVENT_RINGANSWER));
	i = DAHDI_RINGOFF;  /* turn ringer off */
	  /* if error, return as such */
	if(ioctl(zap->fd,DAHDI_HOOK,&i) == -1) return(-1);
	  /* if they interrupted the ring, return here */
	if (j == ZAP_EVENT_RINGANSWER) return(2);   
	usleep(500000);  /* sleep at least 1/2 sec */
	  /* output 30 0x55's */
	for(i = 0; i < 30; i++) 
	   {
		r = put_clid(zap,0x55);
		if (r == -1) return(-1);
		if ((r == -2) && (zap_getevent(zap) == ZAP_EVENT_RINGANSWER)) return(2);
	   }
	  /* 150ms of markage */
	for(i = 0; i < 150; i++)
	   {
		r = put_clid_markms(zap);
		if (r == -1) return(-1);
		if ((r == -2) && (zap_getevent(zap) == ZAP_EVENT_RINGANSWER)) return(2);
	   }
	  /* put out 0x80 indicting MDMF format */
	r = put_clid(zap,128);
	if (r == -1) return(-1);
	if ((r == -2) && (zap_getevent(zap) == ZAP_EVENT_RINGANSWER)) return(2);
	   /* put out length of entire message */
	r = put_clid(zap,strlen(msg));
	if (r == -1) return(-1);
	if ((r == -2) && (zap_getevent(zap) == ZAP_EVENT_RINGANSWER)) return(2);
	  /* go thru entire message */
	for(i = 0,j = 0; msg[i]; i++)
	   {
		r = put_clid(zap,msg[i]);  /* output this byte */
		if (r == -1) return(-1);
		if ((r == -2) && (zap_getevent(zap) == ZAP_EVENT_RINGANSWER)) return(2);
		j += msg[i]; /* update checksum */
	   }
	j += 128; /* add message type to checksum */
	j += strlen(msg);  /* add message length to checksum */
	r = put_clid(zap,256 - (j & 255));  /* output checksum (twos-complement) */
	if (r == -1) return(-1);
	if ((r == -2) && (zap_getevent(zap) == ZAP_EVENT_RINGANSWER)) return(2);
	  /* send marks for 50 ms to allow receiver to settle */
	for(i = 0; i < 50; i++)
	   {
		r = put_clid_markms(zap);  /* send mark for a millisecond */
		if (r == -1) return(-1); /* return if error */
		if ((r == -2) && (zap_getevent(zap) == ZAP_EVENT_RINGANSWER)) return(2);
	   }
	  /* finish writing audio */
	i = zap_finish_audio(zap,0);
	if (i == -1) return(-1); /* return if error */
	if ((i == -2) && (zap_getevent(zap) == ZAP_EVENT_RINGANSWER)) return(2);
	  /* calculate how long (in microseconds) its been since end of ringy */
	i = ((strlen(msg) + 33) * 8333) + 700000 + (zap->nxfer * 125);
	k = 4000000 - i;	
	while(k > 0)  /* do rest of ring silence period */
	   {
		k -= 125;  /* 125 usec per sample at 8000 */
		  /* output 1 sample of silence */
		i = put_audio_sample(zap,0,0);
		if (i == -1) return(-1); /* return if error */
		if (i != -2) continue;  /* if no event, continue */
		i = zap_getevent(zap); /* get the event */
		  /* if answered, get outa here and report as such */
		if (i == ZAP_EVENT_RINGANSWER) return(1);
	   }
	return(0);
}

int zap_fd(ZAP *zap)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	return zap->fd;
}

char *zap_dtmfbuf(ZAP *zap)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(NULL);  /* return with error */
	   }
	return zap->dtmfbuf;
}

int zap_dtmfms(ZAP *zap)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	return zap->dtmfms;
}

int zap_nxfer(ZAP *zap)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	return zap->nxfer;
}

int zap_setradio(ZAP *zap, int radio)
{
	if (!zap) /* if NULL arg */
	   {
		errno= EINVAL;  /* set to invalid arg */
		return(-1);  /* return with error */
	   }
	zap->radio = radio;
	return 0;
}


