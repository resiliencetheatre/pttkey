CC=gcc

EXTRA_WARNINGS = -Wall 

CFLAGS=$(EXTRA_WARNINGS) -lrt

BINS=pttkey

all: pttkey 

pttkey:	pttkey.c  log.c ini.c
	$(CC) $+ $(CFLAGS) $(GST_CFLAGS) $(GST_LIBS) -o $@ -I.

install:
	cp pttkey /usr/local/bin/
	mkdir /opt/pttkey
	cp pttkey.ini /opt/pttkey
	cp audio-on.sh /opt/pttkey
	cp audio-off.sh /opt/pttkey
	cp beep.wav /opt/pttkey
	cp end.wav /opt/pttkey
	
clean:
	rm -rf $(BINS)


	

