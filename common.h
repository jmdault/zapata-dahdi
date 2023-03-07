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

#define FS	8000

#define NBW	8
#define BWLIST	{45,75,100,150,200,300,400,800}

#define NSBUF	1024
#define NCOLA	0x4000

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

