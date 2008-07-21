CPPFLAGS = `pkg-config --cflags dbus-1` -D_GNU_SOURCE
CFLAGS = -g -Wall
LDFLAGS = `pkg-config --libs dbus-1`

BINARIES = dbus-wait

all: $(BINARIES)

clean:
	rm -f $(BINARIES)
