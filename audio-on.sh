#!/bin/sh
/usr/local/bin/audiostreamer -a 192.168.5.92 &
echo -n "Sending..." > /tmp/ptt_in
aplay beep.wav
exit 0

