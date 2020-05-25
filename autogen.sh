#!/bin/sh
autoheader || { echo "autoheader failed"; exit 1; }
aclocal || { echo "aclocal failed"; exit 1; }
automake --add-missing || { echo "automake failed"; exit 1; }
autoconf || { echo "autoconf failed"; exit 1; }
./configure "$@"

