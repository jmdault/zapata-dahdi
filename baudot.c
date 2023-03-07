/* 
 *   Baudot Receive translation module, part of:
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

int baudot(ZAP *zap,unsigned char data)	/* Decodifica el código BAUDOT	*/
{
	static char letras[32]={'<','E','\n','A',' ','S','I','U',
				'\n','D','R','J','N','F','C','K',
				'T','Z','L','W','H','Y','P','Q',
				'O','B','G','^','M','X','V','^'};
	static char simbol[32]={'<','3','\n','-',' ',',','8','7',
				'\n','$','4','\'',',','·',':','(',
				'5','+',')','2','·','6','0','1',
				'9','7','·','^','.','/','=','^'};
	int d;
	d=0;			/* Retorna 0 si el código no es un caracter	*/
	switch (data) {
	case 0x1f :	zap->modo=0; break;
	case 0x1b : zap->modo=1; break;
	default:	if (zap->modo==0) d=letras[data]; else d=simbol[data]; break;
	}
	return d;
}
