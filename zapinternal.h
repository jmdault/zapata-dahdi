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

#ifndef _ZAP_INTERNAL_H
#define _ZAP_INTERNAL_H

#include <stdint.h>
#include "tone_detect.h"

#define FS	8000 

#define NBW	2
#define BWLIST	{75,800}
#define	NF	4
#define	FLIST {1400,1800,1200,2200}

#define NSBUF	1024
#define NCOLA	0x4000

#ifndef	ELAST
#define	ELAST 500
#endif

/* used internally, but needs to be here so that ZAP structure will be valid */
typedef struct {
	float spb;	/* Samples / Bit */
	int nbit;	/* Número de bits de datos (5,7,8) */
	float nstop;	/* Número de bits de stop 1,1.5,2  */
	int paridad;	/* Bit de paridad 0=nada 1=Par 2=Impar */
	int hdlc;	/* Modo Packet */

	int bw;		/* Ancho de Banda */

	int f_mark_idx;	/* Indice de frecuencia de marca (f_M-500)/5 */
	int f_space_idx;/* Indice de frecuencia de espacio (f_S-500)/5 */
	
	int pcola;	/* Puntero de las colas de datos */
	float cola_in[NCOLA];		/* Cola de muestras de entrada */
	float cola_filtro[NCOLA];	/* Cola de muestras tras filtros */
	float cola_demod[NCOLA];	/* Cola de muestras demoduladas */
} fsk_data;

#define	ZAP_MAXDTMF 64
#define	ZAP_BLOCKSIZE 204	/* perfict for goertzel */

struct zap  /* channel context structure */
{
	int	fd;		/* channel Unix file descriptor */
	int blocking;
	int radio;		/* flag to be set for radio use to relax DTMF restrictions */
	char	dtmfbuf[ZAP_MAXDTMF + 1];  /* DTMF buffer */
	int	dtmfn;		/* number of chars in DTMF buffer */
	char	dtmf_heard;	/* dtmf heard (when not in getdtmf) */
	char	goertzel_last;  /* 'last' flag for goertzel */
	int	lastgretval;	/* last return value for goertzel */
	int	digit_mode;	/* rx mode: 0 for DTMF, 1 for MF */
	int	dial_mode;	/* tx mode: 0 for DTMF, 1 for MF */
	fsk_data  fskd;		/* fsk data */
	float	ci;		/* ci for fsk tone generation */
	float	cr;		/* cr for fsk tone generation */
	float	myci;		/* ci for local (non-fsk) tone generation */
	float	mycr;		/* cr for local (non-fsk) tone generation */
	float	myci1;		/* ci1 for local (non-fsk) tone generation */
	float	mycr1;		/* cr1 for local (non-fsk) tone generation */
	int	bp;		/* fsk read buffer pointer */
	int	ns;		/* fsk read silly thingy */
	float	x0,cont;	/* stuff for dpll */
	int	mode;		/* fsk transmit mode */
	int	modo;		/* fsk receive mode */
	unsigned char audio_buf[ZAP_BLOCKSIZE];  /* fsk read audio buffer */
	int	p;		/* fsk transmit buffer pointer */
	unsigned char buf[ZAP_BLOCKSIZE]; /* fsk write audio buffer */
	short dbuf[ZAP_BLOCKSIZE]; /* dtmf read delay buffer */
	double fmxv[8],fmyv[8];	/* filter stuff for M filter */
	int	fmp;		/* pointer for M filter */
	double fsxv[8],fsyv[8];	/* filter stuff for S filter */
	int	fsp;		/* pointer for S filter */
	double flxv[8],flyv[8];	/* filter stuff for L filter */
	int	flp;		/* pointer for L filter */
	float lxv[13],lyv[13];	/* filter stuff for low group DTMF filter */
	float hxv[9],hyv[9];	/* filter stuff for high group DTMF filter */
	float	rxgain;		/* rx gain in db */
	float	txgain;		/* tx gain in db */
	int	nxfer;		/* number of bytes transfered last */
	unsigned int nsilence;	/* number of contiguous silence samples */
	int	mflevel;	/* accumulator for MF det. level */
	float	scont;		/* stuff for CLID sending */
	unsigned short recbuf[ZAP_BLOCKSIZE * 2];
	int recsamps;
	dtmf_detect_state_t udtmf;
	int dtmfms;
	int law;		/* 1 for mu-law, 2 for a-law */
	int linear;		/* 0 for x-law, 1 for linear */
};

#endif
