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
#include <Gnumch.h>
#include <Game.h>
#include <FileSys.h>
#include "menus/MainMenu.h"
#include "menus/Menus.h"
#include <signal.h>

#define JAXIS_THRESHOLD 3

static int verbosity;

Game *game;
FileSys *fs;
Menu *menu;

// render in utf8 or text?
bool utf8 = true;

/* CLI parsing stuff */
const static char *optstring = "hvV";
extern char *optarg;
extern int optind, opterr;

bool Point::operator== (Point param)
{
    return x == param.x && y == param.y;
}

SDL_Surface *renderString(TTF_Font *f, const char *s, SDL_Color c)
{
    if(utf8) return TTF_RenderUTF8_Blended(f, s, c);
    return TTF_RenderText_Blended(f, s, c);
}

void stringSize(TTF_Font *f, const char *s, int *w, int *h)
{
    if(utf8) TTF_SizeUTF8(f, s, w, h);
    else     TTF_SizeText(f, s, w, h);
}

string itostr(int i, int w)
{
    ostringstream os;
    if(w>0) {
        os.fill('0');
        os.width(w);
        os.setf(ios_base::right, ios_base::adjustfield);
    }
    os << i;
    return os.str();
}

void sprintf(string *str, const char *fmt, ...)
{
    size_t n;
    va_list ap;

    va_start(ap, fmt);
    str->assign(fmt, strlen(fmt));
    n = 0;
    while ( (n=str->find("%d", n)) != string::npos ) {
        string repl = itostr( va_arg(ap, int) );
        str->replace(n, 2, repl.c_str(), repl.size());
    }
    va_end(ap);
}

void printMsg(int v, const char *fmt, ...)
{
    if(verbosity >= v) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
}

void printWarning(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Warning: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

void printError(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, _("Fatal error: "));
    vfprintf(stderr, _(fmt), ap);
    va_end(ap);
    fprintf(stderr, _("Gnumch has encountered a fatal error and needs to close. If you don't know what caused this error, please submit a bug report to spuzzzzzzz@gmail.com\n"));
    raise(SIGABRT);
}

void version()
{
    printf("This is %s, version %s\n", PACKAGE, VERSION);
}

void usage(const char *name)
{
    version();
    printf("Usage: %s [OPTIONS]\n\
\n\
Options:\n\
  -h                Display this help message\n\
  -v                Verbose output\n\
  -V                Display version information\n", name);
}

/* drop all events except MOUSEMOTION, MOUSEBUTTONDOWN and QUIT */
int menuEventFilter(const SDL_Event *event)
{
    if(event->type == SDL_MOUSEMOTION || event->type == SDL_MOUSEBUTTONDOWN
                                      || event->type == SDL_MOUSEBUTTONUP
                                      || event->type == SDL_KEYDOWN)
        return 1;
    if(event->type == SDL_QUIT) {
        printMsg(1, "quit request received while in menu mode\n");
        exit(0);
    }
    return 0;
}

int gameEventFilter(const SDL_Event *event)
{
    if(event->type == SDL_KEYDOWN || event->type == SDL_JOYBUTTONDOWN) {
        return 1;
    } else if (event->type == SDL_JOYHATMOTION) {
        /* only allow pure directions (no diagonal) */
        return    (event->jhat.value == SDL_HAT_UP
                || event->jhat.value == SDL_HAT_DOWN
                || event->jhat.value == SDL_HAT_LEFT
                || event->jhat.value == SDL_HAT_RIGHT);
    } else if (event->type == SDL_JOYAXISMOTION) {
        /* we want to avoid repeating events if the axis/direction doesn't change
         * but the value does */
        static int last_axis = 0;
        static int last_value = 0;
        static int last_time = 0;

        if (abs(event->jaxis.value) <= JAXIS_THRESHOLD) {
            last_value = 0;
            return 0;
        }
        if (event->jaxis.axis == last_axis &&
                (event->jaxis.value > JAXIS_THRESHOLD && last_axis > JAXIS_THRESHOLD
              || event->jaxis.value < -JAXIS_THRESHOLD && last_axis < -JAXIS_THRESHOLD
              )) {
            Game::GameSettings gset = game->getGameSettings();
            if ((int)SDL_GetTicks() < last_time + gset.change_time) {
                return 0;
            }
        }
        last_axis = event->jaxis.axis;
        last_value = event->jaxis.value;
        last_time = SDL_GetTicks();
        return 1;
    } else if(event->type == SDL_QUIT) {
        printMsg(1, "quit request received while in game mode\n");
        exit(0);
    }
    return 0;
}

int main(int argc, char **argv)
{
    int c;

    if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) == -1) {
        printError("couldn't init SDL: %s\n", SDL_GetError());
    }
    if(TTF_Init() == -1) {
        printError("couldn't init TTF: %s\n", TTF_GetError());
    }
    atexit (SDL_Quit);

#ifdef HAVE_LOCALE_H
#ifdef ENABLE_NLS
    if(!setlocale(LC_MESSAGES, "") || !setlocale(LC_CTYPE, ""))
        printWarning("couldn't set your locale. Make sure it is installed correctly\n");

    /* try to switch to utf8 */
    if( !bind_textdomain_codeset(PACKAGE, "UTF-8") ) {
        printWarning("couldn't switch to UTF-8\n");
        utf8 = false;
    }
    const char *tmp = bindtextdomain(PACKAGE, LOCALEDIR);
    printMsg(1, "after bindtextdomain(), domain is %s\n", tmp);
    /* textdomain(PACKAGE); */
#endif
#endif
    SDL_EnableUNICODE(1);

    fs   = new FileSys();
    while((c=getopt(argc, argv, optstring)) >= 0) {
        switch(c) {
            case 'h':
            case '?':
                usage(argv[0]);
                exit(0);
                break;
            case 'v':
                verbosity++;
                break;
            case 'V':
                version();
                exit(0);
                break;
            case ':':
                fprintf(stderr, "you need to specify an argument to -%c\n", c);
                exit(1);
                break;
        }
    }

    Game::GameSettings gset;
    Menu::VideoSettings vset;
    Menu::SoundSettings sset;
    KeyBindings dummy;

    Game::readSettings( &gset, &vset, &sset, &dummy, &dummy );
    SDL_SetEventFilter(menuEventFilter);
    Menu::changeSoundSettings(sset);
    Menu::changeVideoSettings(vset);
    Menu::setBackground("menu_back.png");
    pushMenu(new MainMenu());
    Menu::run();
    return 1;
}

