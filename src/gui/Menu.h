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
#ifndef MENU_H
#define MENU_H

#include <src/game/Gnumch.h>
#include <iostream>
#include <map>
#include "Widget.h"
#include "Container.h"

/* used with qsort to sort integers in descending order */
int intdescend(const void*, const void*);

class Menu {
 public:
  class VideoSettings {
  public:
    Uint32  flags;
    int     bpp;
    int     w;
    int     h;
    bool operator== (const VideoSettings &arg) {
      return flags == arg.flags
      && bpp == arg.bpp
      && w == arg.w
      && h == arg.h;
    }
    bool operator!= (const VideoSettings &arg) {
      return ! (*this == arg);
    }
  };

  class SoundSettings {
  public:
    int channels;
    int frequency;
    int fx_volume;
    int music_volume;
    bool operator== (const SoundSettings &arg) {
      return channels == arg.channels
      && frequency == arg.frequency
      && fx_volume == arg.fx_volume
      && music_volume == arg.music_volume;
    }
    bool operator!= (const SoundSettings &arg) {
      return ! (*this == arg);
    }
  };

  static void init(SDL_Surface *screen);

  static Uint32 getFlags();
  static Uint32 getRmask();
  static Uint32 getGmask();
  static Uint32 getBmask();
  static Uint32 getAmask();
  static Uint32 getWhite();
  static Uint32 getGray();
  static Uint32 getBlack();
  static Uint32 mapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a=255);
  static Uint32 getButtonColor(bool foc, bool key_foc, bool highlight);
  static int getBpp();

  static void stop();
  static void resume();

  static int getNavKey(const SDL_keysym&);
  static void setRoot(Container*, bool delete_old=0);
  static void run();
  static void update(const Rectangle&);
  static void drawBox(const Rectangle&, int r=0, int g=0, int b=0);
  static void drawBox(const Rectangle&, Uint32 color);
  static void drawPic(SDL_Surface*, const Rectangle&, bool update=0);
  static void drawPicClipped(SDL_Surface *s,
			     const Rectangle &clip,
			     const Rectangle &dest,
			     bool update=0);
  static void drawString(const string&, const Rectangle&,
			 bool update=0, TTF_Font *font=font_button);
  static void drawButton(const Rectangle&, Uint32, bool update=0);
  static void drawButtonBorder(const Rectangle&, bool update=0);
  static void drawMouseFocRectangle(const Rectangle&, bool=0);
  static void drawKeyFocRectangle(const Rectangle&, bool=0);
  static void refresh();
  static void redrawAll();
  static SDL_Surface *createSurface(int w, int h);
  static SDL_Surface *createSurfaceNoAlpha(int w, int h);
  static SDL_Surface *createButton(int w, int h, Uint32 color);
  static void playClick();

  /* text rendering */
  static SDL_Surface *renderString(const string&,
				   TTF_Font *font=font_button);
  static void splitString(const string&, vector<string>*);
  static void getStringSize(const string&, int*, int*,
			    TTF_Font *font=font_button);

  static TTF_Font *getButtonFont();

  /* flags here can be 0 or SDL_FULLSCREEN */
  static void getModes(vector<SDL_Rect>*, int*, const SDL_Rect &res, int flags);
  static void setBackground(const char *filename);
  static void setBackground(SDL_Surface *s);
  static const VideoSettings &getVideoSettings();
  static void changeVideoSettings(const VideoSettings&);
  static const SoundSettings &getSoundSettings();
  static void changeSoundSettings(const SoundSettings&);

  static int getFXVolume();
  static int getMusicVolume();

  static void setKeyboardFocus(Widget*);

 protected:
  static VideoSettings vset;
  static SoundSettings sset;
  static Uint32 rmask;
  static Uint32 gmask;
  static Uint32 bmask;
  static Uint32 amask;
  static bool running;
  static SDL_Surface *screen, *alphasurface;
  static SDL_PixelFormat *format, *afmt;
  static Container *root;
  static Widget *focused_widget;
  static Widget *keyboard_focused_widget;
  static Mix_Chunk *click_sound;

  /* rather than redraw the button corners every time, we store a circle for
   * each colour/size we need, then chop it into quarters when it comes time to
   * draw the button */
  struct ColorSize {
    ColorSize();
    ColorSize(Uint32 c, int rad);
    ColorSize(Uint8 r, Uint8 g, Uint8 b, Uint8 a, int rad);
    Uint32 color;
    int radius;

    bool operator()(const ColorSize &c1, const ColorSize &c2) const
    {
      return c1.color < c2.color
	|| (c1.color == c2.color && c1.radius < c2.radius);
    }
  };
  static map<const ColorSize, SDL_Surface*, ColorSize> circle_cache;

  static TTF_Font *font_button;

  enum {
    CLICK_REPEAT_IDLE = 0,
    CLICK_REPEAT_PAUSED,
    CLICK_REPEAT_ACTIVE
  };
  static int click_repeat_state;
  static int click_repeat_time;

  /* the widget that was focused when a button was last pressed */
  static Widget *mid_click_widget;
  /* the last button number that was pressed */
  static int     mid_click_button;

  static void combineAlphaSurfaces(SDL_Surface *src, SDL_Surface *dest, int, int);

  /// Given a MOUSEMOTION event, check for a change in widget focus.
  static void updateFocus(const SDL_Event &event);

  /// Given a MOUSEBUTTONDOWN or MOUSEBUTTONUP event, send the
  //  appropriate signal to the focused widget.

  /** Also check whether to send a CLICKED signal.
   */
  static void handleClick(const SDL_Event &event);

  /// calling setRoot doesn't actually change the root widget. This
  /// function runs at the end of the drawing loop (so as not to
  /// interfere with pending operations) and changes the root widget if
  /// there is a pending change.
  static void setRootForReal();
  static Container *pending_root, *pending_root_delete;

  /// calling Menu::update just adds the rectangle to the update list.
  /// The actual updating is done in his function, once per timestep
  /// (or when the array of SDL_Rects is full)
  static void doUpdate();
  static SDL_Rect update_rects[10];
  static int num_update_rects;

  static string background_file;
  static SDL_Surface *background;

  static void drawButtonAlpha(SDL_Surface*, const Rectangle&, Uint32);

  /* this assumes that the colour is purely opaque */
  static void drawButton(SDL_Surface*, const Rectangle&, Uint32);

  static SDL_Surface *makeButtonCircle(Uint32 color, int radius);
};

#endif
