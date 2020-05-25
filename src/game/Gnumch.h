/*
 *  Copyright (C) 2005 Joe Neeman <spuzzzzzzz@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef GNUMCH_H
#define GNUMCH_H

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_framerate.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_rotozoom.h>
#include <SDL_mixer.h>
/*#include <SDL_net.h>*/
#include <config.h>

#ifdef HAVE_LOCALE_H
#   include <locale.h>
#endif

#ifdef ENABLE_NLS
#   include <libintl.h>
#   include <langinfo.h>
#   define _(String) gettext(String)
#else
#   define _(String) (String)
#   define ngettext(a,b,c) ((c)>1 ? (b) : (a))
#endif

#define RAND(a) ( (int) ((float)(a)*rand() / (RAND_MAX + 1.0)) )
#define RANDLE(a) ( (int)(rand()*(a+1.0) / (RAND_MAX+1.0)) )
#define RANDLT(a) RANDLE(a-1)


using namespace std;

class Point {
    public:
    Point(int a, int b) {x=a; y=b;}
    int x;
    int y;

    bool operator == (Point);
};

void printMsg(int, const char*, ...);
void printWarning(const char *, ...);
void printError(const char *, ...);

int menuEventFilter(const SDL_Event*);
int gameEventFilter(const SDL_Event*);

SDL_Surface *renderString(TTF_Font*, const char*, SDL_Color);
void stringSize(TTF_Font*, const char*, int *, int*);

string itostr(int i, int w=0);
void sprintf(string*, const char*, ...);

/* used for hash_maps and maps */
class eqstr
{
    public:
        bool operator()(char* s1, char* s2) const
        {
            return strcmp(s1, s2) == 0;
        }
};

class ltstr
{
    public:
        bool operator()(const string& s1, const string& s2) const
        {
            return s1 < s2;
        }
};

#endif
