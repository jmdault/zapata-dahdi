/* 
 *   FSK Demodulator module, part of:
 *      BSD Telephony Of Mexico "Zapata" Telecom Library, version 1.10 12/9/01
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
 *   This module mainly written by Jesus Arias.
 */

#include "zap.h"
#include "zapinternal.h"
#include "tdd.h"

int demodulador(ZAP *zap, float *retval)
{
	float x,xS,xM;

	if (get_audio_sample(zap,&x)) return(-1);
	zap->fskd.cola_in[zap->fskd.pcola]=x;
	
	xS=filtroS(zap,x);
	xM=filtroM(zap,x);

	zap->fskd.cola_filtro[zap->fskd.pcola]=xM-xS;

	x=filtroL(zap,xM*xM - xS*xS);
	
	zap->fskd.cola_demod[zap->fskd.pcola++]=x;
	zap->fskd.pcola &= (NCOLA-1);

	*retval = x;
	return(0);
}

