#!/bin/sh
killall audiostreamer
echo -n "Ready" > /tmp/ptt_in
aplay end.wav
exit 0
