TARGETS = autoscan-devices loadcpufreq rotdash waitforX

PREFIX = /usr/local
INSTDIR = $(DESTDIR)/$(PREFIX)/bin

CFLAGS += -Os -s -Wall -Wextra
CFLAGS += -march=i486 -mtune=i686 # Comment out for other arches
CFLAGS += -ffunction-sections -fdata-sections

LDFLAGS += -Wl,-O1 -Wl,-gc-sections

.PHONY: all clean install

all: $(TARGETS)
	sstrip $(TARGETS)

waitforX: LDFLAGS += -lX11

clean:
	rm -f $(TARGETS) *.o

install: all
	mkdir -p $(INSTDIR)
	cp -a $(TARGETS) $(INSTDIR)
