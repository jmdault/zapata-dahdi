/* 
 *   FSK Receive module, part of:
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


int serie(ZAP *zap)
{
	int a;
	float x1,x2;
	int i,j,n1,r;
	
	/* Esperamos bit de start	*/
	do {
/* this was jesus's nice, reasonable, working (at least with RTTY) code
to look for the beginning of the start bit. Unfortunately, since TTY/TDD's
just start sending a start bit with nothing preceding it at the beginning
of a transmission (what a LOSING design), we cant do it this elegantly */
/*
		if (demodulador(zap,&x1)) return(-1);
		for(;;) {
			if (demodulador(zap,&x2)) return(-1);
			if (x1>0 && x2<0) break;
			x1=x2;
		}
*/
/* this is now the imprecise, losing, but functional code to detect the
beginning of a start bit in the TDD sceanario. It just looks for sufficient
level to maybe, perhaps, guess, maybe that its maybe the beginning of
a start bit, perhaps. This whole thing stinks! */
		if (demodulador(zap,&x1)) return(-1);
		for(;;)
		   {
			if (demodulador(zap,&x2)) return(-1);
			if (x2 < -0.5) break; 
		   }
		/* Esperamos 0.5 bits antes de usar DPLL */
		i=zap->fskd.spb/2;
		for(;i;i--) if (demodulador(zap,&x1)) return(-1);	

		/* x1 debe ser negativo (confirmación del bit de start) */

	} while (x1>0);
	
	/* Leemos ahora los bits de datos */
	j=zap->fskd.nbit;
	for (a=n1=0;j;j--) {
		i=get_bit_raw(zap);
		if (i == -1) return(-1);
		if (i) n1++;
		a>>=1; a|=i;
	}
	j=8-zap->fskd.nbit;
	a>>=j;

	/* Leemos bit de paridad (si existe) y la comprobamos */
	if (zap->fskd.paridad) {
		i=get_bit_raw(zap); 
		if (i == -1) return(-1);
		if (i) n1++;
		if (zap->fskd.paridad==1) {	/* paridad=1 (par) */
			if (n1&1) a|=0x100;		/* error */
		} else {			/* paridad=2 (impar) */
			if (!(n1&1)) a|=0x100;	/* error */
		}
	}
	
	/* Leemos bits de STOP. Todos deben ser 1 */
	
	for (j=zap->fskd.nstop;j;j--) {
		r = get_bit_raw(zap);
		if (r == -1) return(-1);
		if (!r) a|=0x200;
	}

	/* Por fin retornamos  */
	/* Bit 8 : Error de paridad */
	/* Bit 9 : Error de Framming */
	
	return a;
}
