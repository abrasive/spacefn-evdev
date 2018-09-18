spacefn: spacefn.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

CFLAGS := `pkg-config --cflags libevdev`
LDFLAGS := `pkg-config --libs libevdev`
