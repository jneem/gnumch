aclocal19 -I /usr/local/share/aclocal || die "aclocal failed"
automake19 || die "automake failed"
autoheader259 || die "autoheader failed"
autoconf259 || die "autoconf failed"

./configure "$@"
