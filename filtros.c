/* 
 *   Digital Filter module, part of:
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

/* Coeficientes para filtros de entrada					*/
/* Tabla de coeficientes, generada a partir del programa "mkfilter"	*/
/* Formato: coef[IDX_FREC][IDX_BW][IDX_COEF]				*/
/* IDX_COEF=0	=>	1/GAIN						*/
/* IDX_COEF=1-6	=>	Coeficientes y[n]				*/

double coef_in[NF][NBW][8]={
#include "coef_in.h"
};

/* Coeficientes para filtro de salida					*/
/* Tabla de coeficientes, generada a partir del programa "mkfilter"	*/
/* Formato: coef[IDX_BW][IDX_COEF]					*/
/* IDX_COEF=0	=>	1/GAIN						*/
/* IDX_COEF=1-6	=>	Coeficientes y[n]				*/

double coef_out[NBW][8]={
#include "coef_out.h"
};


/* Filtro pasa-banda para frecuencia de MARCA */
float filtroM(ZAP *zap,float in)
{
	int i,j;
	double s;
	double *pc;
	
	pc=&coef_in[zap->fskd.f_mark_idx][zap->fskd.bw][0];
	zap->fmxv[(zap->fmp+6)&7]=in*(*pc++);
	
	s=(zap->fmxv[(zap->fmp+6)&7] - zap->fmxv[zap->fmp]) + 3 * (zap->fmxv[(zap->fmp+2)&7] - zap->fmxv[(zap->fmp+4)&7]);
	for (i=0,j=zap->fmp;i<6;i++,j++) s+=zap->fmyv[j&7]*(*pc++);
	zap->fmyv[j&7]=s;
	zap->fmp++; zap->fmp&=7;
	return s;
}

/* Filtro pasa-banda para frecuencia de ESPACIO */
float filtroS(ZAP *zap,float in)
{
	int i,j;
	double s;
	double *pc;
	
	pc=&coef_in[zap->fskd.f_space_idx][zap->fskd.bw][0];
	zap->fsxv[(zap->fsp+6)&7]=in*(*pc++);
	
	s=(zap->fsxv[(zap->fsp+6)&7] - zap->fsxv[zap->fsp]) + 3 * (zap->fsxv[(zap->fsp+2)&7] - zap->fsxv[(zap->fsp+4)&7]);
	for (i=0,j=zap->fsp;i<6;i++,j++) s+=zap->fsyv[j&7]*(*pc++);
	zap->fsyv[j&7]=s;
	zap->fsp++; zap->fsp&=7;
	return s;
}

/* Filtro pasa-bajos para datos demodulados */
float filtroL(ZAP *zap,float in)
{
	int i,j;
	double s;
	double *pc;
	
	pc=&coef_out[zap->fskd.bw][0];
	zap->flxv[(zap->flp + 6) & 7]=in * (*pc++); 
	
	s=     (zap->flxv[zap->flp]       + zap->flxv[(zap->flp+6)&7]) +
	  6  * (zap->flxv[(zap->flp+1)&7] + zap->flxv[(zap->flp+5)&7]) +
	  15 * (zap->flxv[(zap->flp+2)&7] + zap->flxv[(zap->flp+4)&7]) +
	  20 *  zap->flxv[(zap->flp+3)&7]; 
	
	for (i=0,j=zap->flp;i<6;i++,j++) s+=zap->flyv[j&7]*(*pc++);
	zap->flyv[j&7]=s;
	zap->flp++; zap->flp&=7;
	return s;
}

