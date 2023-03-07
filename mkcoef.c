/* 
 *   FSK Filter generator, part of:
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

#include <stdlib.h>
#include <stdio.h>
#include "zapinternal.h"

/* Coeficientes para filtros de entrada					*/

double coef_in[NF][NBW][8];

/* Coeficientes para filtro de salida					*/

double coef_out[NBW][8];

float 	bwi[NBW]=BWLIST;
int	freqs[NF]=FLIST;

void get_coef(double *pd,int nc)
{
	int i;
	char buf[256];
	FILE *fp;
	double g;
		
	if ((fp=fopen("p_tmp_","r"))==NULL) exit(-1);
	fgets(buf,256, fp);
	fgets(buf,256, fp);
	g=atof(&buf[4]);
	*pd++=1./g;
	do {
		fgets(buf,256, fp);
	} while (buf[0]!='N' || buf[1]!='P');
	for (i=0;i<nc-2;i++) {
		fgets(buf,256, fp);
		g=atof(buf);
		*pd++=g;
	}
	fclose(fp);
	*pd=0.0;
}

main()
{
	int i,j,k;
	double f,f1,f2,bw,g;
	char buf[256];
	FILE *fp;
	
	for (i=0;i<NF;i++){
	    f=freqs[i];
	    printf("Frec.=%f\n",f); fflush(stdout);
	    for (j=0;j<NBW;j++) {
		bw=bwi[j];
		f1=f-bw;	f2=f+bw;
	    	f1/=FS;	f2/=FS;
		if (f1 < 0.001) f1 = 0.001;
	    	sprintf(buf,"mkfilter/mkfilter -Bu -Bp -o 3 -a %g %g -l >p_tmp_",f1,f2);
	    	system(buf);
	    	get_coef(&coef_in[i][j][0],8);
	    }
	}

	for(j=0;j<NBW;j++) {
	    bw=bwi[j];
	    bw/=FS;
	    sprintf(buf,"mkfilter/mkfilter -Be -Lp -o 6 -a %g -l >p_tmp_",bw);
	    system(buf);
	    get_coef(&coef_out[j][0],8);
	}

	if ((fp=fopen("coef_in.h","w"))==NULL) exit(-1);
	for (i=0;i<NF;i++){
	    fprintf(fp, " { ");
	    for (j=0;j<NBW;j++) {
		fprintf(fp, " { ");
	    	for (k=0;k<8;k++) {
		    fprintf(fp,"%1.10e%c",coef_in[i][j][k],(i!=399 || j!=(NBW-1) || k!=7)?',':' ');
		}
		fprintf(fp,"\n");
		fprintf(fp, " }, ");
	    }
	    fprintf(fp, " }, ");
	}
	fclose(fp);

	if ((fp=fopen("coef_out.h","w"))==NULL) exit(-1);
	    for(j=0;j<NBW;j++) {
		fprintf(fp, " { ");
	    	for (k=0;k<8;k++) {
		    fprintf(fp,"%1.10e%c",coef_out[j][k],(j!=(NBW-1) || k!=7)?',':' ');
		}
		fprintf(fp,"\n");
		fprintf(fp, " }, ");
	    }
	fclose(fp);
}

