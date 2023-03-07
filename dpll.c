/* 
 *   Digital PLL module, part of:
 *      BSD Telephony Of Mexico "Zapata" Telecom Library, version 1.10  12/9/01
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

int get_bit_raw(ZAP *zap)
{
	/* Esta funcion implementa un DPLL para sincronizarse con los bits */
	float x,spb,spb2,ds;
	int f;

	spb=zap->fskd.spb; 
	if (zap->fskd.spb == 7) spb = 8000.0 / 1200.0;
	ds=spb/32.;
	spb2=spb/2.;

	for (f=0;;){
		if (demodulador(zap,&x)) return(-1);
		if ((x*zap->x0)<0) {	/* Transicion */
			if (!f) {
				if (zap->cont<(spb2)) zap->cont+=ds; else zap->cont-=ds;
				f=1;
			}
		}
		zap->x0=x;
		zap->cont+=1.;
		if (zap->cont>spb) {
			zap->cont-=spb;
			break;
		}
	}
	f=(x>0)?0x80:0;
	return(f);
}
