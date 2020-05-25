#!/bin/sh
autoheader || { echo "autoheader failed"; exit 1; }
aclocal || { echo "aclocal failed"; exit 1; }
automake || { echo "automake failed"; exit 1; }
autoconf || { echo "autoconf failed"; exit 1; }
./configure "$@"

