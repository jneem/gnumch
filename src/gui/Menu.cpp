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
#include "Menu.h"
#include <FileSys.h>
#include <signal.h>

#define CLICK_REPEAT_PAUSE 500
#define CLICK_REPEAT_INTERVAL 100 // milliseconds

#define ROUND_CORNER_RATIO 64 // divide the screen height by this to determine
                              // the radius of the rounded corners on buttons

extern FileSys *fs;

Menu::VideoSettings Menu::vset;
Menu::SoundSettings Menu::sset;
Uint32 Menu::rmask;
Uint32 Menu::gmask;
Uint32 Menu::bmask;
Uint32 Menu::amask;
bool Menu::running;
SDL_Surface *Menu::screen, *Menu::alphasurface;
SDL_Surface *Menu::background;
string Menu::background_file;
SDL_PixelFormat *Menu::format, *Menu::afmt;
Container *Menu::root;
Container *Menu::pending_root;
Container *Menu::pending_root_delete;
TTF_Font *Menu::font_button;
Mix_Chunk *Menu::click_sound;

map<const Menu::ColorSize, SDL_Surface*, Menu::ColorSize> Menu::circle_cache;

SDL_Rect Menu::update_rects[10];
int Menu::num_update_rects;

int Menu::click_repeat_time;
int Menu::click_repeat_state;
Widget *Menu::mid_click_widget;
int Menu::mid_click_button;

Widget *Menu::focused_widget;
Widget *Menu::keyboard_focused_widget;

const static Uint32 gfx_black = 0x000000FF;

SDL_Rect default_modes[] = {
  {0, 0, 1600, 1200},
  {0, 0, 1280, 960},
  {0, 0, 1152, 864},
  {0, 0, 1024, 768},
  {0, 0, 800, 600},
  {0, 0, 640, 480},
};

int intdescend(const void *a, const void *b)
{
  if(*(int*)a < *(int*)b) return 1;
  if(*(int*)a > *(int*)b) return -1;
  return 0;
}

Menu::ColorSize::ColorSize()
{
  color = 0xFFFFFFFF;
  radius = 10;
}

Menu::ColorSize::ColorSize(Uint32 c, int rad)
{
  color = c;
  radius = rad;
}

Menu::ColorSize::ColorSize(Uint8 r, Uint8 g, Uint8 b, Uint8 a, int rad)
{
  color = (r << 24) | (g << 16) | (b << 8) | a;
  radius = rad;
}

void Menu::init(SDL_Surface *screen_)
{
  screen = screen_;
  vset.w = screen->w;
  vset.h = screen->h;
  vset.flags = screen->flags;
  format = screen->format;
  vset.bpp = format->BitsPerPixel;
  rmask  = format->Rmask;
  gmask  = format->Gmask;
  bmask  = format->Bmask;
  amask  = format->Amask;

  SDL_Surface *tmp;
  tmp = SDL_CreateRGBSurface(0, 1, 1, vset.bpp, rmask, gmask, bmask, amask);
  alphasurface = SDL_DisplayFormatAlpha(tmp);
  SDL_FreeSurface(tmp);

  afmt = alphasurface->format;

  font_button = fs->openFont("URWGothicL-Demi.ttf", vset.w/50);

  click_sound = fs->openSound("menu_click.ogg");
}

Uint32 Menu::getFlags() { return vset.flags; }
Uint32 Menu::getRmask() { return rmask; }
Uint32 Menu::getGmask() { return gmask; }
Uint32 Menu::getBmask() { return bmask; }
Uint32 Menu::getAmask() { return amask; }
Uint32 Menu::getWhite() { return mapRGBA(255, 255, 255); }
Uint32 Menu::getGray()  { return mapRGBA(170, 170, 170); }
Uint32 Menu::getBlack() { return mapRGBA(0,   0,   0  ); }

Uint32 Menu::getButtonColor(bool foc, bool key_foc, bool highlight)
{
  int r, g, b;
  if (foc) {
    if (!key_foc) {
      r = g = b = 170;
    } else {
      r = 0;
      g = 128;
      b = 0;
    }
  } else {
    if (!key_foc) {
      r = g = b = 255;
    } else {
      r = 0;
      g = 192;
      b = 32;
    }
  }
  if (highlight) {
    /* alpha blend (0, 32, 192) with (r,g,b) */
    r = r/2;
    g = g/2 + 16;
    b = b/2 + 96;
  }
  return mapRGBA(r, g, b);
}

Uint32 Menu::mapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  return r << 24 | g << 16 | b << 8 | a;
}

int Menu::getBpp() { return vset.bpp; }

int Menu::getNavKey(const SDL_keysym &k)
{
  if (k.sym == SDLK_UP || k.sym == SDLK_LEFT)
    return -1;
  else if (k.sym == SDLK_DOWN || k.sym == SDLK_RIGHT || k.sym == SDLK_TAB)
    return 1;
  return 0;
}

void Menu::setRoot(Container *cont, bool delete_old)
{
  pending_root = cont;
  if (delete_old) {
    pending_root_delete = root;
  }
  if (!running) {
    setRootForReal();
  }
}

void Menu::setRootForReal() {
  root = pending_root;
  root->unfocus();
  root->keyboardUnfocus();
  root->setSize(-1, -1);  // force the whole widget tree to resize
  root->setSize(vset.w, vset.h);
  root->setX(0);
  root->setY(0);

  int x, y;
  SDL_GetMouseState(&x, &y);
  focused_widget = root->handleFocus(x, y);
  focused_widget->focus();
  keyboard_focused_widget = NULL;

  click_repeat_state = CLICK_REPEAT_IDLE;
  root->callCallback();
  if (background) {
    root->setBackground(background);
  }

  if (pending_root_delete) {
    delete pending_root_delete;
    pending_root_delete = NULL;
  }
  pending_root = NULL;
}

void Menu::setBackground(const char *name)
{
  if (background) {
    SDL_FreeSurface(background);
  }
  try {
    background = fs->openPic(name, vset.w, vset.h);
    background_file = name;
    if (root) {
      root->setBackground(background);
    }
  } catch (int i) {
    printWarning("Couldn't open background pic: %s\n", strerror(i));
    if (root) {
      root->setBackground(NULL);
    }
    background = NULL;
    background_file.clear();
  }
}

void Menu::setBackground(SDL_Surface *s)
{
  if (background) {
    SDL_FreeSurface(background);
    background_file.clear();
  }
  background = s;
  if (root) {
    root->setBackground(s);
  }
}

void Menu::run()
{
  SDL_Event event;
  int last = SDL_GetTicks();
  int now;

  if (!root) {
    printError("Cannot run a menu without a root widget\n");
  }
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  SDL_ShowCursor(1);
  root->draw();
  SDL_Flip(screen);
  running = 1;
  while(running) {
    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT) {
	exit(0);
      } else if(event.type == SDL_MOUSEMOTION
		|| event.type == SDL_MOUSEBUTTONDOWN
		|| event.type == SDL_MOUSEBUTTONUP) {
	if (focused_widget && focused_widget->getRawEvents()
	    && focused_widget->handleEvent(&event)) {
	  /* nothing */
	} else if (event.type == SDL_MOUSEMOTION) {
	  updateFocus(event);
	} else {
	  handleClick(event);
	}
      } else if(event.type == SDL_KEYDOWN) {
	if (!keyboard_focused_widget) {
	  keyboard_focused_widget = root;
	}
	Widget *foc = keyboard_focused_widget;
	int direction = getNavKey(event.key.keysym);

	if (foc->getRawKeyEvents() && foc->handleRawKey(&event)) {
	  /* nothing */
	} else if (direction) {
	  if (foc->hasKeyboardFocus()) {
	    foc = foc->handleKeyFocus(direction, foc);
	  } else {
	    foc = foc->handleKeyFocus(direction, NULL);
	  }
	  if (foc != keyboard_focused_widget) {
	    keyboard_focused_widget->keyboardUnfocus(true);
	    foc->keyboardFocus(true);
	    keyboard_focused_widget = foc;
	  }
	} else {
	  foc->handleKey(&event);
	}
      }

      if (pending_root) {
	break;
      }
    }

    /* it's possible that an event has caused Menu::stop() to be called */
    if (!running) {
      if (pending_root) {
	setRootForReal();
      }
      break;
    }
    now = SDL_GetTicks();

    /* send repeated clicks if they are holding the mouse button down */
    if(     (click_repeat_state == CLICK_REPEAT_ACTIVE &&
	     click_repeat_time + CLICK_REPEAT_INTERVAL < now)
            ||  (click_repeat_state == CLICK_REPEAT_PAUSED &&
                 click_repeat_time + CLICK_REPEAT_PAUSE < now)) {
      if (focused_widget != mid_click_widget) {
	click_repeat_state = CLICK_REPEAT_IDLE;
	click_repeat_time = 0;
      } else if (focused_widget && focused_widget->getRepeating()) {
	Widget::MouseEvent_t m = { 1, Widget::CLICK };
	focused_widget->handleClick(m);
	click_repeat_state = CLICK_REPEAT_ACTIVE;
	click_repeat_time = now;
      } else {
	printWarning(
		     "tried to repeat a click to a non-repeating widget\n");
	click_repeat_state = CLICK_REPEAT_IDLE;
      }
    }
    Menu::refresh();
    if (pending_root) {
      setRootForReal();
      redrawAll();
    }
    if(now < last + 20) {
      SDL_Delay(last + 20 - now);
    }
    last = now;
  }
}

void Menu::stop()
{
  running = 0;
}

void Menu::resume()
{
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  running = 1;
}

void Menu::update(const Rectangle &r)
{
  if (num_update_rects == 10) doUpdate();

  if( r.w <= 0 || r.h <= 0 || !(r <= Rectangle(0, 0, vset.w, vset.h)) ) {
    printWarning("tried to update rectangle at (%d,%d)-(%d,%d)\n",
		 r.x, r.y, r.x+r.w, r.y+r.h);
    return;
  }

  update_rects[num_update_rects++] = r.toSDL();
}

void Menu::doUpdate()
{
  if (num_update_rects) {
    SDL_UpdateRects(screen, num_update_rects, update_rects);
  }
  num_update_rects = 0;
}

void Menu::drawBox(const Rectangle &rect, int r, int g, int b)
{
  if(! (rect <= Rectangle(0, 0, vset.w, vset.h)) ) {
    printWarning("tried to draw box at (%d,%d)-(%d,%d)\n",
		 rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
    return;
  }

  SDL_Rect sdl_rect = rect.toSDL();
  SDL_FillRect(screen, &sdl_rect, SDL_MapRGB(screen->format, r, g, b));
}

void Menu::drawBox(const Rectangle &rect, Uint32 color)
{
  if(! (rect <= Rectangle(0, 0, vset.w, vset.h)) ) {
    printWarning("tried to draw box at (%d,%d)-(%d,%d)\n",
		 rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
    return;
  }

  boxColor(screen, rect.x, rect.y, rect.x+rect.w, rect.y+rect.h, color);
}

void Menu::drawPicClipped(SDL_Surface *s,
                          const Rectangle &clip,
                          const Rectangle &dest,
                          bool update)
{
  if(! (dest <= Rectangle(0, 0, vset.w, vset.h)) ) {
    printWarning("tried to draw clipped pic at (%d,%d)-(%d,%d)\n", dest.x,
		 dest.y, dest.x+dest.w, dest.y+dest.h);
    return;
  }

  SDL_Rect c = clip.toSDL();
  SDL_Rect d = dest.toSDL();

  if (d.x + c.w > vset.w) c.w -= vset.w - (d.x + c.w);
  if (d.y + c.h > vset.h) c.h -= vset.h - (d.y + c.h);

  SDL_BlitSurface(s, &c, screen, &d);
  if(update) Menu::update(Rectangle(d.x, d.y, d.w, d.h));
}

void Menu::drawPic(SDL_Surface *surface, const Rectangle &rect, bool update)
{
  if(! (rect <= Rectangle(0, 0, vset.w, vset.h)) ) {
    printWarning("tried to draw pic at (%d,%d)-(%d,%d)\n",
		 rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
    return;
  }

  SDL_Rect r = rect.toSDL();
  SDL_BlitSurface(surface, NULL, screen, &r);
  if(update) Menu::update(rect);
}

void Menu::drawString(const string &text, const Rectangle &rect_,
                      bool update, TTF_Font *font)
{
  SDL_Surface *pic = renderString(text, font);
  Rectangle rect = rect_;
  rect.w = pic->w; rect.h = pic->h;
  SDL_Rect sr = rect.toSDL();

  SDL_BlitSurface(pic, NULL, screen, &sr);

  if(! (rect <= Rectangle(0, 0, vset.w, vset.h)) ) {
    printWarning("tried to draw text %s at (%d,%d)-(%d,%d)\n", text.c_str(),
		 rect.x, rect.y, rect.x+rect.w, rect.y+rect.h);
    return;
  }
  if(update) {
    Menu::update(rect);
  }
  SDL_FreeSurface(pic);
}

void Menu::drawButton(const Rectangle &rect, Uint32 color, bool update)
{
  if(! (rect <= Rectangle(0, 0, vset.w, vset.h)) ) {
    printWarning("tried to draw button at (%d,%d)-(%d,%d)\n", rect.x,
		 rect.y, rect.x+rect.w, rect.y+rect.h);
    return;
  }

  drawButtonAlpha(screen, rect, color);
  if (update) {
    Menu::update(rect);
  }
}

void Menu::drawButtonAlpha(SDL_Surface *s, const Rectangle &rect, Uint32 color)
{
  Uint8 real_a = color & 0xff;
  SDL_Rect dest = rect.toSDL();

  color |= 0xff;
  if (real_a < 255) {
    dest.x = dest.y = 0;
    SDL_Surface *tmp = createSurfaceNoAlpha(rect.w, rect.h);

    // choose a colorkey that won't appear in the image
    Uint32 r, g, b;
    r = color >> 24;
    g = (color >> 16) & 0xff;
    b = (color >> 8) & 0xff;
    SDL_SetColorKey(tmp, SDL_SRCCOLORKEY, SDL_MapRGB(tmp->format, r, g, b+50) );
    SDL_FillRect(tmp, &dest, SDL_MapRGB(tmp->format, r, g, b+50) );
    SDL_SetAlpha(tmp, SDL_SRCALPHA | SDL_SRCCOLORKEY, real_a);

    drawButton(tmp, Rectangle(0, 0, rect.w, rect.h), color);

    dest = rect.toSDL();
    int ret = SDL_BlitSurface(tmp, NULL, s, &dest);
    if (ret == -1) {
      printWarning("%s\n", SDL_GetError());
    }
    SDL_FreeSurface(tmp);
  } else {
    drawButton(s, rect, color);
  }
}

void Menu::drawButton(SDL_Surface *screen, const Rectangle &dest, Uint32 color)
{
  int rad, x1, x2, y1, y2;

  rad = min(dest.h/2 - 1, vset.h / ROUND_CORNER_RATIO);

  /* Each pair (xn, yn) is at the centre of one or the corner arcs */
  x1 = dest.x + rad - 1;
  x2 = dest.x + dest.w - rad;
  y1 = dest.y + rad - 1;
  y2 = dest.y + dest.h - rad;

  /* blit the four corners of a circle onto the corners of our button */
  SDL_Surface *circ = makeButtonCircle(color, rad);
  SDL_Rect d = {dest.x, dest.y, 0, 0};
  SDL_Rect s = {0, 0, rad, rad};
  SDL_BlitSurface(circ, &s, screen, &d);
  s.y = rad+1;
  d.y = y2;
  SDL_BlitSurface(circ, &s, screen, &d);
  s.x = rad+1;
  d.x = x2;
  SDL_BlitSurface(circ, &s, screen, &d);
  s.y = 0;
  d.y = dest.y;
  SDL_BlitSurface(circ, &s, screen, &d);

  boxColor(screen, x1+1, dest.y+1, x2-1, dest.y + dest.h - 2, color);
  boxColor(screen, dest.x+1, y1+1, x1, y2-1, color);
  boxColor(screen, x2, y1+1, dest.x + dest.w - 2, y2-1, color);

  /* the rest of the border */
  lineColor(screen, dest.x, y1+1, dest.x, y2-1, gfx_black);
  lineColor(screen, dest.x+dest.w-1, y1+1, dest.x+dest.w-1, y2-1, gfx_black);
  lineColor(screen, x1+1, dest.y, x2-1, dest.y, gfx_black);
  lineColor(screen, x1+1, dest.y+dest.h-1, x2-1, dest.y+dest.h-1, gfx_black);
}

SDL_Surface *Menu::makeButtonCircle(Uint32 color, int r)
{
  map<const ColorSize, SDL_Surface*, ColorSize>::iterator i;
  i = circle_cache.find( ColorSize(color, r) );
  if (i != circle_cache.end()) {
    return i->second;
  }

  SDL_Surface *s = createSurface(2*r + 1, 2*r + 1);

  filledCircleColor(s, r, r, r, color);
  circleColor(s, r, r, r, 0x000000FF);
  circle_cache[ColorSize(color, r)] = s;
  return s;
}

void Menu::splitString(const string &text, vector<string> *split)
{
  int last = 0;
  int newline = 0;
  split->clear();

  while( (newline = text.find('\n', last)) != (int)string::npos ) {
    string line(text, last, newline-last);
    split->push_back(line);
    last = newline + 1;
  }
  string line(text, last, text.size() - last);
  split->push_back(line);
}

void Menu::getStringSize(const string &text, int *w, int *h,
                         TTF_Font *font)
{
  vector<string> lines;
  splitString(text, &lines);

  int w_text, h_text;
  *w = *h = 0;
  vector<string>::iterator i, end=lines.end();
  for(i=lines.begin(); i<end; i++) {
    stringSize(font, i->c_str(), &w_text, &h_text);
    *w = max(w_text, *w);
    *h += h_text;
  }
}

SDL_Surface *Menu::renderString(const string &text, TTF_Font *font)
{
  vector<string> lines;
  int w, h;

  getStringSize(text, &w, &h, font);
  splitString(text, &lines);
  SDL_Color black = {0, 0, 0};
  SDL_Surface *cur, *ret;

  ret = createSurface(w, h);
  SDL_FillRect(ret, NULL, ~ret->format->Amask);

  int i, end=lines.size();
  int y_cur;
  y_cur = 0;
  for(i=0; i<end; i++) {
    if(lines[i].empty()) {
      y_cur += TTF_FontLineSkip(font);
    } else {
      cur = TTF_RenderUTF8_Blended(font, lines[i].c_str(), black);
      combineAlphaSurfaces(cur, ret, (ret->w - cur->w)/2, y_cur);
      y_cur += cur->h;
      SDL_FreeSurface(cur);
    }
  }
  return ret;
}

void Menu::refresh()
{
  root->draw(1);
  doUpdate();
  //SDL_Flip(screen);
}

void Menu::redrawAll()
{
  root->setAllChanged(1);
  SDL_FillRect(screen, NULL, amask);
  root->draw(1);
  SDL_Flip(screen);
  num_update_rects = 0;
}

SDL_Surface *Menu::createSurface(int w, int h)
{
  SDL_Surface *ret;
  ret = SDL_CreateRGBSurface(vset.flags, w, h, afmt->BitsPerPixel, afmt->Rmask, afmt->Gmask, afmt->Bmask, afmt->Amask);
  SDL_FillRect(ret, NULL, ~ret->format->Amask);
  return ret;
}

SDL_Surface *Menu::createSurfaceNoAlpha(int w, int h)
{
  return SDL_CreateRGBSurface(vset.flags, w, h, afmt->BitsPerPixel, afmt->Rmask, afmt->Gmask, afmt->Bmask, 0);
}

SDL_Surface *Menu::createButton(int w, int h, Uint32 color)
{
  SDL_Surface *ret = createSurfaceNoAlpha(w, h);

  /* choose a colour key that will not be part of the button */
  Uint32 r, g, b;
  r = color >> 24;
  g = (color >> 16) & 0xff;
  b = (color >> 8) & 0xff;
  SDL_SetColorKey(ret, SDL_SRCCOLORKEY, SDL_MapRGB(ret->format, r, g, b+50) );
  SDL_FillRect(ret, NULL, SDL_MapRGB(ret->format, r, g, b+50) );

  drawButton(ret, Rectangle(0, 0, w, h), color | 0xff);
  SDL_SetAlpha(ret, SDL_SRCALPHA, color & 0xff);
  return ret;
}

TTF_Font *Menu::getButtonFont()
{
  return font_button;
}

const Menu::VideoSettings &Menu::getVideoSettings()
{
  return vset;
}

void Menu::changeVideoSettings(const VideoSettings &newset)
{
  if (vset == newset)
    return;

  int w_old = vset.w, h_old = vset.h;

  vset = newset;
  screen = SDL_SetVideoMode(vset.w, vset.h, vset.bpp, vset.flags);
  if (font_button)
    TTF_CloseFont(font_button);
  init(screen);

  if (background) {
    if (background_file.empty()) {
      double zoomx = (double) newset.w / w_old;
      double zoomy = (double) newset.h / h_old;
      setBackground( zoomSurface(background, zoomx, zoomy, true) );
    } else {
      setBackground( background_file.c_str() );
    }
  }
}

const Menu::SoundSettings &Menu::getSoundSettings()
{
  return sset;
}

void Menu::changeSoundSettings(const SoundSettings &newset)
{
  if (!SDL_WasInit (SDL_INIT_AUDIO))
    {
      if (SDL_InitSubSystem (SDL_INIT_AUDIO) == -1)
	{
	  printWarning ("couldn't initialise audio: %s\n", SDL_GetError ());
	  return;
	}
      atexit (Mix_CloseAudio);
    }
  else if (sset == newset)
    return;
  else
    Mix_CloseAudio ();
  
  sset = newset;

  Mix_CloseAudio();
  if(Mix_OpenAudio(newset.frequency, AUDIO_S16SYS, newset.channels, 1024) == -1) {
    printWarning("couldn't initialise SDL_Mixer: %s\n", Mix_GetError());
    SDL_QuitSubSystem (SDL_INIT_AUDIO);
  }
  // for some reason, it will play different volumes on different channels
  // without this
  else
    Mix_Volume (-1, MIX_MAX_VOLUME);
}

int Menu::getFXVolume()
{
    return (sset.fx_volume * MIX_MAX_VOLUME) / 100;
}

int Menu::getMusicVolume()
{
    return (sset.music_volume * MIX_MAX_VOLUME) / 100;
}

void Menu::setKeyboardFocus(Widget *newfoc)
{
    assert(newfoc->getAllowKeyFocus());

    if (keyboard_focused_widget) {
        keyboard_focused_widget->keyboardUnfocus();
    }
    newfoc->keyboardFocus();
    keyboard_focused_widget = newfoc;
}

void Menu::getModes(vector<SDL_Rect> *list, int *cur, const SDL_Rect &res, int flags)
{
    SDL_Rect **modes = SDL_ListModes(NULL, flags);
    int i;

    list->clear();
    if(!modes) {
        cout << "Error: no video modes available\n";
        exit(1);
    } else if(modes == (SDL_Rect**)-1) {
        unsigned i, j;

        /* If no mode restrictions are imposed, add all the modes that
         * are either in the fullscreen modes list or have both of the
         * following properties:
         * 1) they are in our default_modes list
         * 2) they are smaller than the maximum fullscreen mode
         */

        /* copy over the fullscreen modes */
        modes = SDL_ListModes(NULL, flags | SDL_FULLSCREEN);
        list->push_back(*modes[0]);
        for (i=1; modes[i]; i++) {
            if(modes[i]->w != modes[i-1]->w || modes[i]->h != modes[i-1]->h) {
                if(modes[i]->w >= 512)
                    list->push_back(*modes[i]);
            }
        }

        /* merge in the default modes */
        j = 1;
        i = 0;
        while (i<sizeof(default_modes)/sizeof(SDL_Rect)) {
            if (default_modes[i].w > modes[0]->w && default_modes[i].h > modes[0]->h) {
                i++;
            } else if (j == list->size()) {
                list->push_back(default_modes[i]);
                j++; i++;
            } else if (default_modes[i].w < (*list)[j].w
                    || default_modes[i].h < (*list)[j].h) {
                j++;
            } else if (default_modes[i].w > (*list)[j].w
                    || default_modes[i].h > (*list)[j].h) {
                list->insert(list->begin() + j, default_modes[i]);
                i++;
                j += 2;
            } else {
                i++;    /* default_modes[i] is a dupe, skip it */
            }
        }
    } else {
        list->push_back(*modes[0]);
        for(i=1; modes[i]; i++) {
            if(modes[i]->w != modes[i-1]->w || modes[i]->h != modes[i-1]->h) {
                if(modes[i]->w >= 512)
                    list->push_back(*modes[i]);
            }
        }
    }

    for(i=0; i<(int)list->size(); i++) {
        /* The screen sizes are in descending order. So if we find a size that
         * is smaller than out current resolution, our current resolution isn't
         * supported with these flags. */
        if(list->at(i).w <= res.w && list->at(i).h <= res.h) {
            *cur = i;
            return;
        }
    }
    *cur = 0;
}

void Menu::combineAlphaSurfaces(SDL_Surface *src, SDL_Surface *dest, int x, int y) {
    SDL_Rect dest_r = {x, y};
    SDL_SetAlpha(src, 0, 255);
    SDL_BlitSurface(src, NULL, dest, &dest_r);
}

void Menu::updateFocus(const SDL_Event &event)
{
    Widget *oldfoc = focused_widget;

    focused_widget = root->handleFocus(event.motion.x, event.motion.y);
    if (oldfoc && focused_widget != oldfoc) {
        oldfoc->unfocus(true);
    }
    if (focused_widget) {
        focused_widget->focus(true);
    }
}

void Menu::handleClick(const SDL_Event &event)
{
    if (!focused_widget) {
        return;
    }

    Widget::MouseEvent_t m;
    m.button = event.button.button;
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        m.type = Widget::PRESS;
        if (focused_widget->getRepeating()) {
            click_repeat_state = CLICK_REPEAT_PAUSED;
            click_repeat_time = SDL_GetTicks();
        }
        mid_click_widget = focused_widget;
        mid_click_button = m.button;
    } else {
        m.type = Widget::RELEASE;
        if (mid_click_widget == focused_widget
                && mid_click_button == m.button
                && click_repeat_state != CLICK_REPEAT_ACTIVE) {
            Widget::MouseEvent_t m2 = { event.button.button, Widget::CLICK };
            focused_widget->handleClick(m2);
        }
        mid_click_widget = NULL;
        mid_click_button = 0;
        click_repeat_state = CLICK_REPEAT_IDLE;
    }
    focused_widget->handleClick(m);
}

void Menu::playClick()
{
    if (click_sound) {
        Mix_VolumeChunk(click_sound, getFXVolume());
        if (Mix_PlayChannel(-1, click_sound, 0) == -1) {
            printWarning("couldn't play sound: %s\n", Mix_GetError());
        }
    }
}
