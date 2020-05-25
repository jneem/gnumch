dnl Process this file with autoconf to produce a configure script.
AC_INIT(gnumch, 0.2.0)
AC_CONFIG_SRCDIR(src/game)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
AM_GNU_GETTEXT([external])
dnl AM_PO_SUBDIRS

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_INTLTOOL

dnl check for dependencies
CXXFLAGS="-Wall"
LIBS=""

if ! which sdl-config; then
    if which sdl11-config; then
        SDL_CONFIG=`which sdl11-config`
    fi
fi

AM_PATH_SDL(1.2.0,
    CXXFLAGS="$CXXFLAGS $SDL_CXXFLAGS $SDL_CFLAGS" LIBS="$LIBS $SDL_LIBS",
    echo "SDL not found. Is sdl-config in your PATH?"; exit 1)

AC_CHECK_LIB(SDL_image, IMG_Load,
    [ LIBS="$LIBS -lSDL_image" ],
    [ echo "SDL_image not found"; exit 1 ])

AC_CHECK_LIB(SDL_ttf, TTF_OpenFont,
    [ LIBS="$LIBS -lSDL_ttf" ],
    [ echo "SDL_ttf not found"; exit 1 ])

AC_CHECK_LIB(SDL_gfx, SDL_initFramerate,
    [ LIBS="$LIBS -lSDL_gfx" ],
    [ echo "SDL_gfx not found"; exit 1 ])

AC_CHECK_LIB(SDL_gfx, filledpieColor,
    [ AC_DEFINE( filledPieColor, filledpieColor, "older versions of SDL_gfx have a typo" )])

AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio,
    [ LIBS="$LIBS -lSDL_mixer" ],
    [ echo "SDL_mixer not found"; exit 1 ])

dnl AC_CHECK_LIB(SDL_net, SDLNet_Init,
dnl     [ LIBS="$LIBS -lSDL_net" ],
dnl     [ echo "SDL_net not found"; exit 1 ])

dnl enable args
AC_ARG_ENABLE(debug, 
    [  --enable-debug       Enable debugging information],
    [if test "x$enable_debug" = "xyes"; then 
        CXXFLAGS="$CXXFLAGS -Werror -g"
    else 
        CXXFLAGS="$CXXFLAGS -O2"
    fi],
    [CXXFLAGS="$CXXFLAGS -O2"])

AC_ARG_ENABLE(profile,
    [  --enable-profile     Enable profiling information],
    [if test "x$enable_debug" = "xyes"; then
        CXXFLAGS="$CXXFLAGS -pg"
    fi])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(locale.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T

dnl Checks for library functions.

CPPFLAGS="${CPPFLAGS} -DDATADIR=\\\"${datadir}\\\""
dnl LIBS="$LIBS -lefence"
AC_SUBST(CXXFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LIBS)
AC_OUTPUT( po/Makefile.in m4/Makefile 
        Makefile
        src/Makefile
        src/game/Makefile
        src/game/menus/Makefile
        src/gui/Makefile
        data/Makefile
        data/animation/Makefile
        data/animation/pics/Makefile
        data/animation/sounds/Makefile
        data/fonts/Makefile
        data/pics/Makefile
        data/sounds/Makefile)
