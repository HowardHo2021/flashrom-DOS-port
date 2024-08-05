/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2000 Silicon Integrated System Corporation
 * Copyright (C) 2009,2010 Carl-Daniel Hailfinger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <limits.h>
#include "flash.h"
#include "programmer.h"

/* loops per microsecond */
static unsigned long micro = 1;

__attribute__ ((noinline)) static void myusec_delay(unsigned int usecs)
{
	unsigned long i;
	for (i = 0; i < usecs * micro; i++) {
		/* Make sure the compiler doesn't optimize the loop away. */
		__asm__ volatile ("" : : "rm" (i) );
	}
}

static unsigned long measure_os_delay_resolution(void)
{
	unsigned long timeusec;
	struct timeval start, end;
	unsigned long counter = 0;

	gettimeofday(&start, NULL);
	timeusec = 0;

	while (!timeusec && (++counter < 1000000000)) {
		gettimeofday(&end, NULL);
		timeusec = 1000000 * (end.tv_sec - start.tv_sec) +
			   (end.tv_usec - start.tv_usec);
		/* Protect against time going forward too much. */
		if ((end.tv_sec > start.tv_sec) &&
		    ((end.tv_sec - start.tv_sec) >= LONG_MAX / 1000000 - 1))
			timeusec = 0;
		/* Protect against time going backwards during leap seconds. */
		if ((end.tv_sec < start.tv_sec) || (timeusec > LONG_MAX))
			timeusec = 0;
	}
	return timeusec;
}

static unsigned long measure_delay(unsigned int usecs)
{
	unsigned long timeusec;
	struct timeval start, end;

	gettimeofday(&start, NULL);
	myusec_delay(usecs);
	gettimeofday(&end, NULL);
	timeusec = 1000000 * (end.tv_sec - start.tv_sec) +
		   (end.tv_usec - start.tv_usec);
	/* Protect against time going forward too much. */
	if ((end.tv_sec > start.tv_sec) &&
	    ((end.tv_sec - start.tv_sec) >= LONG_MAX / 1000000 - 1))
		timeusec = LONG_MAX;
	/* Protect against time going backwards during leap seconds. */
	if ((end.tv_sec < start.tv_sec) || (timeusec > LONG_MAX))
		timeusec = 1;

	return timeusec;
}

static void myusec_calibrate_delay(void)
{
	unsigned long count = 1000;
	unsigned long timeusec, resolution;
	int i, tries = 0;

	msg_pinfo("Calibrating delay loop... ");
	/* Timing resolution on DJGPP is about 50ms, but measure it precisely. */
	resolution = measure_os_delay_resolution();
	if (resolution) {
		msg_pdbg("OS timer resolution is %lu usecs, ", resolution);
	} else {
		msg_pinfo("OS timer resolution is unusable. ");
	}

recalibrate:
	count = 1000;
	while (1) {
		timeusec = measure_delay(count);
		if (timeusec > 1000000 / 4)
			break;
		if (count >= ULONG_MAX / 2) {
			msg_pinfo("timer loop overflow, reduced precision. ");
			break;
		}
		count *= 2;
	}
	tries ++;

	/* Avoid division by zero, but in that case the loop is shot anyway. */
	if (!timeusec)
		timeusec = 1;

	/* Compute rounded up number of loops per microsecond. */
	micro = (count * micro) / timeusec + 1;
	msg_pdbg("%luM loops per second, ", micro);

	/* Did we try to recalibrate less than 5 times? */
	if (tries < 5) {
		/* Recheck our timing to make sure we weren't just hitting
		 * a scheduler delay or something similar.
		 */
		for (i = 0; i < 4; i++) {
			timeusec = measure_delay(resolution * 10) *
					100 / (resolution * 10);

			if (timeusec < 90) {
				msg_pdbg("delay more than 10%% too short (got "
					 "%lu%% of expected delay), "
					 "recalculating... ", timeusec);
				goto recalibrate;
			}
		}
	} else {
		msg_perr("delay loop is unreliable, trying to continue ");
	}

	/* We're interested in the actual precision. */
	timeusec = measure_delay(resolution * 4);
	msg_pdbg("%ld myus = %ld us, ", resolution * 4, timeusec);

	msg_pinfo("OK.\n");
}

/* Not very precise sleep. */
void internal_sleep(unsigned int usecs)
{
	sleep(usecs / 1000000);
	usleep(usecs % 1000000);
}

static const unsigned min_sleep = CONFIG_DELAY_MINIMUM_SLEEP_US;

/* Precise delay. */
void default_delay(unsigned int usecs)
{
	static bool calibrated = false;

	if (usecs < min_sleep) {
		if (!calibrated) {
			myusec_calibrate_delay();
			calibrated = true;
		}
		myusec_delay(usecs);
	} else {
		internal_sleep(usecs);
	}
}
