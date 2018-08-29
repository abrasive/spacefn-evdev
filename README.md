spacefn-evdev
=============

This is a little tool to implement
[the SpaceFn keyboard layout](https://geekhack.org/index.php?topic=51069.0)
on Linux.

I wanted to try SpaceFn on my laptop, obviously with a built-in keyboard.
The only previous Linux implementation I could find
[requires recompiling the Xorg input driver](http://www.ljosa.com/~ljosa/software/spacefn-xorg/),
which is an impressive effort but is tricky to compile and means restarting my X server every time I want to make a change.


## Requirements


- libevdev
    and its headers or -dev packages on some systems
- uinput
    `/dev/uinput` must be present and you must have permission to read and write it

You also need permission to read `/dev/input/eventXX`.

On my system all the requisite permissions are granted by making myself a member of the `input` group.
You can also just run the program as root.

## Compiling

Run `make`.

## Running

Find your keyboard in `/dev/input/`.
The easiest way is to look in `/dev/input/by-id`;
for example, my laptop keyboard is
`/dev/input/usb-Apple_Inc._Apple_Internal_Keyboard___Trackpad_XXXXXXXXXXXXXXXXXXXX-if01-event-kbd`.

Then run the program:
```
./spacefn /dev/input/usb-Apple_Inc._Apple_Internal_Keyboard___Trackpad_XXXXXXXXXXXXXXXXXXXX-if01-event-kbd
```

## Customising

The key map is in the function `key_map()`.
