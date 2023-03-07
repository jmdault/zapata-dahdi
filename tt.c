/* 
 *   Tone Test Program -- Part of:
 *      BSD Telephony Of Mexico "Zapata" Telecom Library, version 1.10 7/13/01
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
 */
#include <stdio.h>
#include "zap.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>


#define	NDIGS 50
#define	SENDLEN 60

char	buf1[100];
char *dtmfstr = "0123456789ABCD*"; 
char *mfstr = "0123456789ABC*"; 
extern int kludgefd;

/* do the actual test */
int dott(float rxgain,char *dstr)
{
ZAP	*zap;
int	i;

	i = fork();
	if (i < 0)
	   {
		perror("fork");
		exit(255);
	   }
	if (!i) /* if sender task */
	   {
		  /* open sender channel */
		zap = zap_open("/dev/dahdi/channel/001/001", 0);
		if (!zap)
		   {
			perror("send:open");
			exit(255);
		   }
		  /* set to a tx gain of +9 dbm so that the
			tones will actually be at +0dbm on span */
		if (zap_gains(zap,0.0,9.0) == -1)
		   {
			perror("send:gains");
			exit(255);
		   }
		usleep(25000);  /* wait for rx task to startup */
		if (dstr == mfstr) /* if MF mode */
		   {
			  /* put into MF mode */
			if (zap_dial(zap,"M",0) == -1)
			   {
				perror("send:dial");
				exit(255);
			   }
		   }
		  /* send digit string */
		if (zap_dial(zap,buf1,SENDLEN) == -1)
		   {
			perror("send:dial");
			exit(255);
		   }
		exit(0);
	   }
	  /* open receiver channel */
	zap = zap_open("/dev/dahdi/001/002", 0);
	if (!zap)
	   {
		perror("recv:open");
		exit(255);
	   }
	  /* set to specified gain */
	if (zap_gains(zap,rxgain,0.0) == -1)
	   {
		perror("recv:gains");
		exit(255);
	   }
	if (dstr == mfstr)  /* if MF mode */
	   {
		  /* put into MF digit mode */
		if (zap_digitmode(zap,ZAP_MF) == -1)
		   {
			perror("zap_digitmode");
			exit(255);
		   }
	   }
	  /* attempt to receive digits */
	i = zap_getdtmf(zap,55,"#",' ',25000,0,0);
	if (i == -2) /* if hangup */
	   {
		printf("Got hangup\n");
		exit(1);
	   }
	if (i == -1) /* if error */
	   {
		perror("recv:getdtmf");
		exit(255);
	   }
	if (i != strlen(buf1)) /* if buffer wrong length */
	   {
		printf("Got wrong len (%d) buf:        %s\n",i,zap_dtmfbuf(zap));
		printf("Should have been len (%lu) buf: %s\n",strlen(buf1),buf1);
		exit(1);
	   }
	if (strcmp(zap_dtmfbuf(zap),buf1)) /* if they dont match */
	   {
		printf("Got:             %s\n",zap_dtmfbuf(zap));
		printf("Should have got: %s\n",buf1);
		exit(1);
	   }
	printf("Success at Rx level of %0.1f dbm !!\n",rxgain);
	wait(NULL); /* wait for completion of TX process */
	zap_close(zap);
	return(0);
}

int main()
{
int	i,j,k;

	printf("tt - tone test (or torture test :-)) for Zapata technology\n");
	printf("\"Tormenta\" telephony card, version 1.0 1/1/01\n\n");
	printf("The technologies, software, hardware, designs, drawings,\n");
	printf("schematics, board layouts and/or artwork, concepts, methodologies\n");
	printf("(including the use of all of these, and that which is derived\n");
	printf("from the use of all of these), all other intellectual properties\n");
	printf("contained herein, and all intellectual property rights have been\n");
	printf("and shall continue to be expressly for the benefit of all mankind,\n");
	printf("and are perpetually placed in the public domain, and may be used,\n");
	printf("copied, and/or modified by anyone, in any manner, for any legal\n");
	printf("purpose, without restriction.\n\n");


	  /* start with quality randomness :-) */
	srandom(time(NULL) ^ getpid());
	  /* generate a random DTMF string */
	for(i = 0; i < NDIGS; i++)
	   {
		buf1[i] = dtmfstr[random() % strlen(dtmfstr)];
	   }
	buf1[i++] = '#'; /* terminated with a pound */
	buf1[i] = 0;
	
	printf("Testing DTMF..\n");
	dott(1.5,dtmfstr);  
	dott(0.0,dtmfstr); 
	dott(-6.0,dtmfstr);
	dott(-12.0,dtmfstr);
	dott(-15.0,dtmfstr);
	dott(-18.0,dtmfstr);
	dott(-20.5,dtmfstr);

	  /* generate a random MF string */
	for(i = 0; i < NDIGS; i++)
	   {
		buf1[i] = mfstr[random() % strlen(mfstr)];
	   }
	buf1[i++] = '#';  /* terminated with an ST */
	buf1[i] = 0;
	printf("Testing MF..\n");
	dott(1.5,mfstr);  
	dott(0.0,mfstr); 
	dott(-6.0,mfstr);
	dott(-12.0,mfstr);
	dott(-15.0,mfstr);
	dott(-18.0,mfstr);
	dott(-20.5,mfstr);
	exit(0);
}



