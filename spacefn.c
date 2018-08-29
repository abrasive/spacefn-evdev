#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

struct libevdev *idev;
struct libevdev_uinput *odev;

static void send_key(unsigned int code, int value) {
    printf("send: %d %d\n", code, value);
    libevdev_uinput_write_event(odev, EV_KEY, code, value);
    libevdev_uinput_write_event(odev, EV_SYN, SYN_REPORT, 0);
}

static void send_press(unsigned int code) {
    send_key(code, 1);
}

static void send_release(unsigned int code) {
    send_key(code, 0);
}

static void send_repeat(unsigned int code) {
    send_key(code, 2);
}

unsigned int key_map(unsigned int code) {
    switch (code) {
        case KEY_BRIGHTNESSDOWN:    // my magical escape button
            exit(0);

        case KEY_J:
            return KEY_LEFT;
        case KEY_K:
            return KEY_DOWN;
        case KEY_L:
            return KEY_RIGHT;
        case KEY_I:
            return KEY_UP;

        case KEY_U:
            return KEY_HOME;
        case KEY_O:
            return KEY_END;

        case KEY_H:
            return KEY_PAGEUP;
        case KEY_N:
            return KEY_PAGEDOWN;

        case KEY_B:
            return KEY_SPACE;
    }
    return 0;
}

int space_pressed = 0;

#define MAX_KEYS 8
int n_shifted = 0;
unsigned int shifted[MAX_KEYS];
int any_shifted;

// If key was shifted, remove it and return true.
// If was not shifted, return false.
static int pop_shifted_key(unsigned int code) {
    for (int i=0; i<n_shifted; i++)
        if (shifted[i] == code) {
            shifted[i] = shifted[--n_shifted];
            return 1;
        }
    return 0;
}

static int is_shifted_key(unsigned int code) {
    for (int i=0; i<n_shifted; i++)
        if (shifted[i] == code)
            return 1;
    return 0;
}

static void debug_shift(void) {
    printf("n%d a%d", n_shifted, any_shifted);
    for (int i=0; i<n_shifted; i++)
        printf(" %x", shifted[i]);
    printf("\n");
}

static void handle_press(unsigned int code) {
    if (code == KEY_SPACE) {
        space_pressed = 1;
        return;
    }

    if (space_pressed && key_map(code) && n_shifted < MAX_KEYS) {
        shifted[n_shifted++] = code;
        any_shifted = 1;
        code = key_map(code);
    }

    send_press(code);
}

static void handle_release(unsigned int code) {
    if (code == KEY_SPACE) {
        space_pressed = 0;
        if (n_shifted) {
            // release all keys
            for (int i=0; i<n_shifted; i++)
                send_release(shifted[i]);
            n_shifted = 0;
        }

        if (!any_shifted) {
            send_press(KEY_SPACE);
            send_release(KEY_SPACE);
        }

        any_shifted = 0;

        return;
    }

    if (pop_shifted_key(code))
        code = key_map(code);

    send_release(code);
}

static void handle_repeat(unsigned int code) {
    if (code == KEY_SPACE)
        return;

    if (space_pressed && is_shifted_key(code))
        code = key_map(code);

    send_repeat(code);
}

static void do_one_event(void) {
    struct input_event ev;
    int err = libevdev_next_event(idev, LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING, &ev);
    if (err) {
        fprintf(stderr, "Failed: (%d) %s\n", -err, strerror(err));
        return;
    }

    if (ev.type != EV_KEY)
        return;

    if (ev.value == 0)
        handle_release(ev.code);
    else if (ev.value == 1)
        handle_press(ev.code);
    else if (ev.value == 2)
        handle_repeat(ev.code);

    debug_shift();
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s /dev/input/...", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open input");
        return 1;
    }

    int err = libevdev_new_from_fd(fd, &idev);
    if (err) {
        fprintf(stderr, "Failed: (%d) %s\n", -err, strerror(err));
        return 1;
    }

    int uifd = open("/dev/uinput", O_RDWR);
    if (uifd < 0) {
        perror("open /dev/uinput");
        return 1;
    }

    err = libevdev_uinput_create_from_device(idev, uifd, &odev);
    if (err) {
        fprintf(stderr, "Failed: (%d) %s\n", -err, strerror(err));
        return 1;
    }

    err = libevdev_grab(idev, LIBEVDEV_GRAB);
    if (err) {
        fprintf(stderr, "Failed: (%d) %s\n", -err, strerror(err));
        return 1;
    }

    for (;;)
        do_one_event();
}
