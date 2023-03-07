/*
 * Header file for:
 *     BSD Telephony Of Mexico "Zapata" Telecom Library, version 1.10  12/9/01
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

#ifndef _ZAP_H
#define _ZAP_H

#ifndef	ELAST
#define	ELAST 500
#endif

#define	LINEAR2XLAW(s,law) ((law == 1) ? linear2ulaw(s) : linear2alaw(s)) 
#define	XLAW(c,law) ((law == 1) ? mulaw[c] : alaw[c])
#define	FULLXLAW(c,law) ((law == 1) ? fullmulaw[c] : fullalaw[c])

struct zap;
typedef struct zap ZAP;

typedef struct zapio  /* I/O element */
{
	void	*p;		/* file pointer or memory location (0 for last entry) */
	int	length;		/* length of entry (0 = play until EOF) */
	int	offset;		/* offset into file (or -1 for memory location) */
} ZAPIO;

ZAP *zap_open(char *devname, int nonblocking);		/* open a telecom channel (nonblocking if specified) */
int zap_close(ZAP *zap);			/* close a telecom channel */
int zap_clrdtmf(ZAP *zap);		/* clear DTMF buffer */
int zap_clrdtmfn(ZAP *zap);		/* clear DTMF buffer */
int zap_getdtmf(ZAP *zap, int max, char *term, 
	char erase, int idtimeout, int timeout, int flags);  /* get DTMF */
int zap_digitmode(ZAP *zap, int mode);  /* set digit mode */
int zap_play(ZAP *zap, ZAPIO *ioblock, int flags);  /* play an io block */
int zap_playf(ZAP *zap, char *fname, int flags);  /* play a file */
int zap_rec(ZAP *zap, ZAPIO *ioblock, int flags); /* record into io block */
int zap_recf(ZAP *zap, char *fname, int maxlen, int flags); /* record into file */
int zap_sethook(ZAP *zap, int hookstate);	/* set hookswitch state */
int zap_getevent(ZAP *zap);  /* get event */
int zap_waitevent(ZAP *zap);  /* wait for event */
int zap_flushevent(ZAP *zap);  /* flush all events */
int zap_waitcall(ZAP *zap, int nrings, int hookstate, int timeout); /* wait for call, then ans. */
int zap_dial(ZAP *zap, char *dialstring, int length);  /* dial a number */
int zap_wink(ZAP *zap);  /* output a wink */
int zap_start(ZAP *zap);  /* start the line */
int zap_flash(ZAP *zap);  /* output a flash */
int zap_ring(ZAP *zap, int rings); /* ring a phone/station (FXO interface only) */
int zap_getparams(ZAP *zap, void *params);  /* get channel parameters */
int zap_setparams(ZAP *zap, void *params);  /* set channel parameters */
int zap_gettdd(ZAP *zap);  /* get a TDD char */
int zap_sendtdd(ZAP *zap,char *string);  /* send a string to TDD */
int zap_disatone(ZAP *zap,int flag);  /* send 2100hz echo can disable tone */
int zap_conf(ZAP *zap, int chan, int confno, int confmode);  /* set conference mode */
int zap_getconf(ZAP *zap, int chan, int *confnop, int *confmodep); /* get conf mode */
int zap_clearlinks(ZAP *zap);  /* clear conf links */
int zap_makelink(ZAP *zap, int dstconfno, int srcconfno); /* make a conference link */
int zap_breaklink(ZAP *zap, int dstconfno, int srcconfno); /* break a conference link */
int zap_gains(ZAP *zap,float rxgain,float txgain); /* set gains in db */
int zap_channel(ZAP *zap); /* get channel number */
int zap_silence(ZAP *zap,int length,int flag); /* send silence, length in ms */
int zap_arbtone(ZAP *zap, float freq, int length, int flag); /* send arbitrary tone */
int zap_arbtones(ZAP *zap,float freq1, float freq2, int length,int flag); /* send
	arbitrary tone pair */
int zap_clid(ZAP *zap, char *number, char *name); /* get Caller*ID */
int zap_ringclid(ZAP *zap, char *number, char *name); /* send ring w/Caller*ID */
int zap_finish_audio(ZAP *zap, int flag); /* finish sending audio */
char *zap_dtmfbuf(ZAP *zap);	/* Return dtmf buffer */
int zap_dtmfms(ZAP *zap);  /* Return last dtmf time in ms */
int zap_fd(ZAP *zap);		/* Return file descriptor */
int zap_nxfer(ZAP *zap);	/* Return number of samples xferred */
int zap_playchunk(ZAP *zap, void *buf, int len, int flags); /* Write a buffer of a given length */
int zap_recchunk(ZAP *zap, void *buf, int len, int flags); /* Record a buffer of a given length */
int zap_recpurge(ZAP *zap);											/* Clear record memory */
int zap_dtmfwaiting(ZAP *zap);	/* Returns non-zero if DTMF stuff is waiting for us */
int zap_setlinear(ZAP *zap, int linear); /* Set channel into linear mode */
int zap_setlaw(ZAP *zap, int law);
int zap_setradio(ZAP *zap, int flag); /* Set/Clear "relaxed DTMF" mode for radio use */
/* flags to various functions */
#define	ZAP_DTMFINT	1		/* interrupt on DTMF */
#define	ZAP_BEEPTONE	2		/* play beep tone before rec. */
#define	ZAP_HOOKEXIT	4		/* exit on hook events */
#define	ZAP_TIMEOUTOK	8		/* timeout ok term for DTMF */
#define	ZAP_SILENCEINT	16		/* interrupt on silence for rec. */

/* hookswitch states */
#define	ZAP_ONHOOK	0		/* on hook */
#define	ZAP_OFFHOOK	1		/* off hook */

/* Events that can be received */
#define	ZAP_EVENT_NOEVENT	0	/* no event */
#define	ZAP_EVENT_ONHOOK	1	/* went on hook */
#define	ZAP_EVENT_RINGANSWER	2	/* got ring (incomming) or answered (outgoing) */
#define	ZAP_EVENT_WINKFLASH	3	/* got wink */
#define	ZAP_EVENT_ALARM		4	/* got alarm */
#define	ZAP_EVENT_NOALARM	5	/* alarm went away */
#define	ZAP_EVENT_ABORT		6	/* HDLC abort */
#define	ZAP_EVENT_OVERRUN	7	/* HDLC overrun */
#define	ZAP_EVENT_BADFCS	8	/* HDLC bad FCS */
#define	ZAP_EVENT_DIALCOMPLETE	9	/* HDLC dial complete */
#define	ZAP_EVENT_RINGERON	10	/* ringer on */
#define	ZAP_EVENT_RINGEROFF	11	/* ringer off */
#define	ZAP_EVENT_HOOKCOMPLETE	12	/* hook change complete */

/* Conference values and masks */
#define	ZAP_NORMAL	0		/* normal mode */
#define	ZAP_MONITOR 1			/* monitor mode (rx of other chan) */
#define	ZAP_MONITORTX 2			/* monitor mode (tx of other chan) */
#define	ZAP_MONITORBOTH 3		/* monitor mode (rx & tx of other chan) */
#define	ZAP_CONF 4			/* conference mode */
#define	ZAP_CONFANN 5			/* conference announce mode */
#define	ZAP_CONFMON 6			/* conference monitor mode */
#define	ZAP_CONFANNMON 7		/* conference announce/monitor mode */
#define	ZAP_CONF_LISTENER 0x100		/* is a listener on the conference */
#define	ZAP_CONF_TALKER 0x200		/* is a talker on the conference */

/* digit modes */
#define	ZAP_DTMF 0			/* receive DTMF digits */
#define	ZAP_MF 1			/* receive MF digits */
#define	ZAP_MUTECONF 2			/* mute on conf */
#define	ZAP_MUTEMAX 4			/* mute and delay */

#define	ZAP_DEFAULT_MFLENGTH 60	/* default tone length -- 60 ms */
#define	ZAP_DEFAULT_DTMFLENGTH 100 /* default tone length -- 100 ms */

#endif
