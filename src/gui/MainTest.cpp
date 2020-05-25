#include "libGui.h"
#include "../game/FileSys.h"
#include "../game/Game.h"

FileSys *fs;
Game *game; /* this remains NULL: just to shut up linking errors with FileSys */

void printStuff(void*, void*)
{
    printf("Hello\n");
}

int main(int argc, char **argv)
{
    Container *top;
    if (argc != 2) {
        printError("Please type the name of a widget\n");
    }

    if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) == -1) {
        printError("couldn't init SDL: %s\n", SDL_GetError());
    }
    if(TTF_Init() == -1) {
        printError("couldn't init TTF: %s\n", TTF_GetError());
    }
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    fs = new FileSys();
    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 32, 0);
    if(!screen) printError("couldn't get a screen: %s\n", SDL_GetError());
    Menu::init(screen);

    if (!strcmp(argv[1], "spinner")) {
        Spinner<int> *spin;
        vector<int> v;
        for (int i=0; i<100; i++) {
            v.push_back(i);
        }

        spin = new Spinner<int>(v, 0);
        top = new VBox();

        top->setAlign(Container::ALIGN_JUSTIFIED);
        top->add(spin);
    } else if (!strcmp(argv[1], "button")) {
        TextButton *b = new TextButton("Click me!");
        b->setCallback((Clickable::Callback)printStuff, NULL);
        top = new HBox();
        top->setPack(Container::PACK_MAX);
        top->add(b);
    } else if (!strcmp(argv[1], "textfield")) {
        top = new VBox();
        TextField *f = new TextField(15);
        top->add(f);
        f = new TextField(15);
        top->add(f);
    } else if (!strcmp(argv[1], "rectangles")) {
        Rectangle r1(1, 1, 1, 1),
                  r2(0, 0, 2, 2),
                  r3(0, 0, 3, 3);
        printf("(1,1)-(2,2) <= (0,0,2,2): %d\n", r1 <= r2);
        printf("(1,1)-(2,2) <  (0,0,2,2): %d\n", r1 < r2);
        printf("(1,1)-(2,2) <  (0,0,3,3): %d\n", r1 < r3);
        return 0;
    }
    Menu::setRoot(top);
    Menu::run();
}

bool Point::operator== (Point param)
{
    return x == param.x && y == param.y;
}

SDL_Surface *renderString(TTF_Font *f, const char *s, SDL_Color c)
{
    return TTF_RenderUTF8_Blended(f, s, c);
}

void stringSize(TTF_Font *f, const char *s, int *w, int *h)
{
    TTF_SizeUTF8(f, s, w, h);
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
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
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
    exit(1);
}

