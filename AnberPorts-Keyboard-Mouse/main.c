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

/* Set up a fake keyboard device */
static int uinp_fd = -1;
struct uinput_user_dev uidev;
struct input_event ev;

#define select 311
#define start 310
#define a 304
#define b 305
#define up 17
#define down 17
#define left 16
#define right 16

void send_key(int key) {
	// Report BUTTON CLICK - PRESS event
	memset(&ev, 0, sizeof(ev));
	gettimeofday(&ev.time, NULL);
	ev.type = EV_KEY;
	ev.code = key; 
	ev.value = 1;
	write(uinp_fd, &ev, sizeof(ev));

	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	write(uinp_fd, &ev, sizeof(ev));

	// Report BUTTON CLICK - RELEASE event
	memset(&ev, 0, sizeof(ev));
	gettimeofday(&ev.time, NULL);
	ev.type = EV_KEY;
	ev.code = key;
	ev.value = 0;
	write(uinp_fd, &ev, sizeof(ev));

	ev.type = EV_SYN;
	ev.code = SYN_REPORT;
	ev.value = 0;
	write(uinp_fd, &ev, sizeof(ev));
}

void handle_event(int type, int code, int value) {
	if (type == 1) {
		if (code == start && (value == 1 || value == 2)) {
			send_key(KEY_ENTER);
		}
		else if (code == select && (value == 1 || value == 2)) {
			send_key(KEY_ESC);
		}
		else if (code == a && (value == 1 || value == 2)) { // crouch
			send_key(KEY_ENTER);
		}		
		else if (code == b && (value == 1 || value == 2)) { // jump
			send_key(KEY_ESC);
		}
	}

	// d-pad
	if (type == 3) {
		if (code == up && value == -1) {
			send_key(KEY_UP);
		}
		else if (code == down && value == 1) {
			send_key(KEY_DOWN);	
		}
		else if (code == left && value == -1) {
			send_key(KEY_LEFT);
		}
		else if (code == right && value == 1) {
			send_key(KEY_RIGHT);
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
	uidev.id.version = 4;
	uidev.id.bustype = BUS_USB;

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

		usleep(500);
	} while (rc_joypad == LIBEVDEV_READ_STATUS_SYNC || rc_joypad == LIBEVDEV_READ_STATUS_SUCCESS || rc_joypad == -EAGAIN);

	libevdev_free(dev_joypad);
	close(fd_ev_joypad);

	/* Clean up */
	ioctl(uinp_fd, UI_DEV_DESTROY);
	close(uinp_fd);

	return 0;
}
