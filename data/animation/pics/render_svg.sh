#!/bin/sh

PNG=$1
SVG=`echo $PNG |sed 's/.png/.svg/'`

rsvg-convert -w 128 -h 96 $SVG > $PNG
if echo $PNG |grep _left_; then
    mogrify -flop $PNG
fi
