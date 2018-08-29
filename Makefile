default: spacefn

CFLAGS := `pkg-config --cflags libevdev`
LDFLAGS := `pkg-config --libs libevdev`
