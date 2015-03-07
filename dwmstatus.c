/* made by profil 2011-12-29.
**
** Compile with:
** gcc -Wall -pedantic -std=c99 -lX11 status.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

static Display *dpy;
long cpu0_work = 0;
long cpu0_total = 0;

long cpu1_work = 0;
long cpu1_total = 0;

long cpu2_work = 0;
long cpu2_total = 0;

long cpu3_work = 0;
long cpu3_total = 0;

int load0,load1,load2,load3;

void setstatus(char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

float getfreq(char *file) {
	FILE *fd;
	char *freq;
	float ret;

	freq = malloc(10);
	fd = fopen(file, "r");
	if(fd == NULL) {
		fprintf(stderr, "Cannot open '%s' for reading.\n", file);
		exit(1);
	}

	fgets(freq, 10, fd);
	fclose(fd);

	ret = atof(freq)/1000000;
	free(freq);
	return ret;
}

int readInt(char *input) {
    FILE *fd;
    int val;

    fd = fopen(input, "r");
    if (fd==NULL)
        return -1;
    fscanf(fd, "%d", &val);
    fclose(fd);
    return val;
}

int getcpu(int *cpuss) {
    FILE *fd;
    long jif1, jif2, jif3, jif4, jif5, jif6, jif7;
    long work0, total0, total1, total2, total3,  work1, work2, work3;
    //int load0, load1, load2, load3;
    int freq;
    char *color0, *color1;

    // ---- LOAD
    fd = fopen("/proc/stat", "r");
    char c;
    while (c != '\n') c = fgetc(fd);
    fscanf(fd, "cpu0 %ld %ld %ld %ld %ld %ld %ld", &jif1, &jif2, &jif3, &jif4, &jif5, &jif6, &jif7);
    work0 = jif1 + jif2 + jif3 + jif6 + jif7;
    total0 = work0 + jif4 + jif5;

    c = 0;
    while (c != '\n') c = fgetc(fd);
    fscanf(fd, "cpu1 %ld %ld %ld %ld %ld %ld %ld", &jif1, &jif2, &jif3, &jif4, &jif5, &jif6, &jif7);
    work1 = jif1 + jif2 + jif3 + jif6 + jif7;
    total1 = work1 + jif4 + jif5;

    c = 0;
    while (c != '\n') c = fgetc(fd);
    fscanf(fd, "cpu2 %ld %ld %ld %ld %ld %ld %ld", &jif1, &jif2, &jif3, &jif4, &jif5, &jif6, &jif7);
    work2 = jif1 + jif2 + jif3 + jif6 + jif7;
    total2 = work2 + jif4 + jif5;

    c = 0;
    while (c != '\n') c = fgetc(fd);
    fscanf(fd, "cpu3 %ld %ld %ld %ld %ld %ld %ld", &jif1, &jif2, &jif3, &jif4, &jif5, &jif6, &jif7);
    work3 = jif1 + jif2 + jif3 + jif6 + jif7;
    total3 = work3 + jif4 + jif5;

    fclose(fd);

    load0 = 100 * (work0 - cpu0_work) / (total0 - cpu0_total);
    load1 = 100 * (work1 - cpu1_work) / (total1 - cpu1_total);
    load2 = 100 * (work2 - cpu2_work) / (total2 - cpu2_total);
    load3 = 100 * (work3 - cpu3_work) / (total3 - cpu3_total);

    cpu0_work = work0;
    cpu0_total = total0;

    cpu1_work = work1;
    cpu1_total = total1;

    cpu2_work = work2;
    cpu2_total = total2;

    cpu3_work = work3;
    cpu3_total = total3;

    cpuss[0] = load0;
    cpuss[1] = load1;
    cpuss[2] = load2;
    cpuss[3] = load3;

    //printf(" %d %d %d %d\n",load0,load1,load2,load3);

    return 0;
}

int getmem() {
    FILE *fd;
    long total, free, available;
    int used;

    fd = fopen("/proc/meminfo", "r");
    fscanf(fd, "MemTotal: %ld kB\n", &total);
    fscanf(fd, "MemFree: %ld kB\n", &free);
    fscanf(fd, "MemAvailable: %ld kB\n", &available);
    fclose(fd);

    used = 100 * (total - available) / total;

    return used;
}

char *getdatetime() {
	char *buf;
	time_t result;
	struct tm *resulttm;

	if((buf = malloc(sizeof(char)*65)) == NULL) {
		fprintf(stderr, "Cannot allocate memory for buf.\n");
		exit(1);
	}
	result = time(NULL);
	resulttm = localtime(&result);
	if(resulttm == NULL) {
		fprintf(stderr, "Error getting localtime.\n");
		exit(1);
	}
	if(!strftime(buf, sizeof(char)*65-1, "%a %b %d %H:%M:%S", resulttm)) {
		fprintf(stderr, "strftime is 0.\n");
		exit(1);
	}

	return buf;
}

int getbattery() {
	FILE *fd;
	int energy_now, energy_full, voltage_now;

	fd = fopen("/sys/class/power_supply/BAT1/capacity", "r");
	if(fd == NULL) {
		fprintf(stderr, "Error opening energy_now.\n");
		return -1;
	}
	fscanf(fd, "%d", &energy_now);
	fclose(fd);


	return (float)energy_now;
}

int main(void) {
	char *status;
	float cpu0, cpu1;
	char *datetime;
	int bat1;
    int mem;

    int *cpuusage;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "Cannot open display.\n");
		return 1;
	}

	if((status = malloc(400)) == NULL)
		exit(1);


	for (;;sleep(1)) {
        getcpu(cpuusage);

        mem=getmem();

		datetime = getdatetime();
		bat1 = getbattery();
		snprintf(status, 400, " cpu: %2d%% %2d%% %2d%% %2d%% | mem: %d%% | bat: %d%% | %s",cpuusage[0], cpuusage[1], cpuusage[2], cpuusage[3], mem, bat1, datetime);

		free(datetime);
		setstatus(status);
	}

	free(status);
	XCloseDisplay(dpy);

	return 0;
}
