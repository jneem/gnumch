#!/bin/sh

for FILE in $1*.svg; do
	echo $FILE
	if echo $FILE |grep _right_; then
		ln -s $FILE ${FILE/_right_/_left_}
	fi
done
