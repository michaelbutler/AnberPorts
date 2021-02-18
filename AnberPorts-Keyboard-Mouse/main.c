/* Copyright (c) 2021
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
#
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
#
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
#
* Authored by: Kris Henriksen <krishenriksen.work@gmail.com>
#
* AnberPorts-Keyboard-Mouse
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

struct libevdev* dev_joypad;
int fd_ev_joypad;
int rc_joypad;
struct input_event ev_joypad;

static int uinp_fd = -1;
struct uinput_user_dev uidev;

#define select 311
#define start 310
#define a 304
#define b 305
#define up 17
#define down 17
#define left 16
#define right 16

void emit(int type, int code, int val) {
   struct input_event ev;

   ev.type = type;
   ev.code = code;
   ev.value = val;
   /* timestamp values below are ignored */
   ev.time.tv_sec = 0;
   ev.time.tv_usec = 0;

   write(uinp_fd, &ev, sizeof(ev));
}

void handle_event(int type, int code, int value) {
	if (type == 1) {
		if (code == start && (value == 1 || value == 2)) {
			emit(EV_KEY, KEY_ENTER, 1);
			emit(EV_SYN, SYN_REPORT, 0);
		}
		else if (code == select && (value == 1 || value == 2)) {
			emit(EV_KEY, KEY_ESC, 1);
			emit(EV_SYN, SYN_REPORT, 0);
		}
		else if (code == a && (value == 1 || value == 2)) {
			emit(EV_KEY, KEY_ENTER, 1);
			emit(EV_SYN, SYN_REPORT, 0);
		}
		else if (code == b && (value == 1 || value == 2)) {
			emit(EV_KEY, KEY_ESC, 1);
			emit(EV_SYN, SYN_REPORT, 0);
		}
		else {
			// reset press down
			emit(EV_KEY, KEY_ENTER, 0);
			emit(EV_SYN, SYN_REPORT, 0);

			emit(EV_KEY, KEY_ESC, 0);
			emit(EV_SYN, SYN_REPORT, 0);
		}
	}

	// d-pad
	if (type == 3) {
		if (code == up && value == -1) {
			emit(EV_KEY, KEY_UP, 1);
			emit(EV_SYN, SYN_REPORT, 0);
		}
		else if (code == up && value == 0) {
			emit(EV_KEY, KEY_UP, 0);
			emit(EV_SYN, SYN_REPORT, 0);
		}

		if (code == down && value == 1) {
			emit(EV_KEY, KEY_DOWN, 1);
			emit(EV_SYN, SYN_REPORT, 0);

		}
		else if (code == down && value == 0) {
			emit(EV_KEY, KEY_DOWN, 0);
			emit(EV_SYN, SYN_REPORT, 0);
		}

		if (code == left && value == -1) {
			emit(EV_KEY, KEY_LEFT, 1);
			emit(EV_SYN, SYN_REPORT, 0);
		}
		else if (code == left && value == 0) {
			emit(EV_KEY, KEY_LEFT, 0);
			emit(EV_SYN, SYN_REPORT, 0);
		}

		if (code == right && value == 1) {
			emit(EV_KEY, KEY_RIGHT, 1);
			emit(EV_SYN, SYN_REPORT, 0);
		}
		else if (code == right && value == 0) {
			emit(EV_KEY, KEY_RIGHT, 0);
			emit(EV_SYN, SYN_REPORT, 0);
		}
	}
}

int main () {
	int i = 0;

	uinp_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (uinp_fd < 0) {
		printf("Unable to open /dev/uinput\n");
		return -1;
	}

	// Intialize the uInput device to NULL
	memset(&uidev, 0, sizeof(uidev));

	strncpy(uidev.name, "AnberPorts Keyboard", UINPUT_MAX_NAME_SIZE);
	uidev.id.version = 1;
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor = 0x1234; /* sample vendor */
	uidev.id.product = 0x5678; /* sample product */

	for (i = 0; i < 256; i++) {
		ioctl(uinp_fd, UI_SET_KEYBIT, i);
	}

	// Keys or Buttons
	ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);

	// Create input device into input sub-system
	write(uinp_fd, &uidev, sizeof(uidev));

	if (ioctl(uinp_fd, UI_DEV_CREATE)) {
		printf("Unable to create UINPUT device.");
		return -1;
	}

	fd_ev_joypad = open("/dev/input/event2", O_RDONLY|O_NONBLOCK);
	rc_joypad = libevdev_new_from_fd(fd_ev_joypad, &dev_joypad);

	do {
		rc_joypad = libevdev_next_event(dev_joypad, LIBEVDEV_READ_FLAG_NORMAL, &ev_joypad);

		if (rc_joypad == LIBEVDEV_READ_STATUS_SUCCESS) {
			handle_event(ev_joypad.type, ev_joypad.code, ev_joypad.value);
		}

		usleep(100);
	} while (rc_joypad == LIBEVDEV_READ_STATUS_SYNC || rc_joypad == LIBEVDEV_READ_STATUS_SUCCESS || rc_joypad == -EAGAIN);

	libevdev_free(dev_joypad);
	close(fd_ev_joypad);

	/*
	* Give userspace some time to read the events before we destroy the
	* device with UI_DEV_DESTROY.
	*/
	sleep(1);

	/* Clean up */
	ioctl(uinp_fd, UI_DEV_DESTROY);
	close(uinp_fd);

	return 0;
}