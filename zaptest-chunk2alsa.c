/*
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2 as published by the
 * Free Software Foundation. See the LICENSE file included with
 * this program for more details.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "zap.h"
#include <dahdi/user.h>
#include <time.h>

int main(int argc, char *argv[])
{
	ZAP *z;
	struct dahdi_params p;
	char tmp[1024];
	int len;
	int res;
	int firstpass=1;
	int linear=0;
	FILE *fp1;
	FILE *fp2;
	clock_t t;
	char *cmd;
	int result;
	char *device = "/dev/dahdi/chan/001/001";

	z = zap_open(device, 0);
	if (!z) {
		fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
		exit(1);
	}


	if (linear) {
                printf("Going linear!\n");
                zap_setlinear(z, linear);
        }

        if(linear) {
		cmd = "aplay -v -D plughw:0 -f S16_LE -c 1 -r 8000 -t raw -q -";
		fp1 = popen(cmd, "w");
		//FILE *fp1 = fopen("test.sln", "w");
		cmd = "arecord -D plughw:0 -f S16_LE -c 1 -r 8000 -t raw -q -";
		fp2 = popen(cmd, "r");
	} else {
		cmd = "aplay -v -D plughw:0 -f MU_LAW -c 1 -r 8000 -t raw -q -";
		fp1 = popen(cmd, "w");
		//FILE *fp1 = fopen("test.sln", "w");
		cmd = "arecord  -D plughw:0 -f MU_LAW -c 1 -r 8000 -t raw -q -";
		fp2 = popen(cmd, "r");
	}

	len = 204;
	for (;;) {
		/* Record, play, and check for events */
		t = clock();
		res = zap_recchunk(z, tmp, len, ZAP_DTMFINT);
		t = clock() - t;
		double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
		//printf("took %f seconds to execute \n", time_taken);
		if (res == len) {
			result = fwrite(tmp, 1, len, fp1);
			//printf("write %d bytes\n", result);
			//res = zap_playchunk(z, tmp, len, 0);
			result = fread(tmp, 1, len, fp2);
			zap_playchunk(z, tmp, len, 0);
		}
		if (res < len) {
			res = zap_getevent(z);
			if (zap_dtmfwaiting(z)) {
				zap_getdtmf(z, 1, NULL, 0, 1, 1, 0);                    
				printf("Got DTMF: %s\n", zap_dtmfbuf(z));
				zap_clrdtmfn(z);
			} else if (res) {
				switch(res) {
				case ZAP_EVENT_ONHOOK:
					printf("On Hook!\n");
					break;
				case ZAP_EVENT_RINGANSWER:
					printf("Off hook!\n");
					break;
				case ZAP_EVENT_WINKFLASH:
					printf("Flash!\n");
					break;
				default:
					printf("Unknown event %d\n", res);
				}
			}
		}
	}
	pclose(fp1);
//	fclose(fp1);
	return 0;
}
