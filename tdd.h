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

int get_audio_sample(ZAP *,float *);
float filtroS(ZAP *,float);
float filtroM(ZAP *,float);
float filtroL(ZAP *,float);
int demodulador(ZAP *,float *);
int serie(ZAP *);
int baudot(ZAP *zap,unsigned char);
int hdlc(char *);
int get_bit_raw(ZAP *);
void hdump(unsigned char *,int);
int check_crc_ccitt(const unsigned char *, int);
