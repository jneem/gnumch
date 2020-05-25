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
#include <Game.h>
#include <FileSys.h>
#include <Menu.h>
#include <ConfigFile.h>
#include <Board.h>
#include <Level.h>
#include <Animation.h>

#include <Troggle.h>
#include <Muncher.h>

/* all routines in SDL_gfx expect the color to be 0xRRGGBBAA */
Uint32 white = 0xffffffff;
Uint32 black = 0x000000ff;

extern Container *enter_score_menu;

/* divide the screen width by these amounts to get the size of the desired font.
 */
#define FONT_TITLE_DIV 42
#define FONT_LIVES_DIV 42
#define FONT_SCORE_DIV 42
#define FONT_MESSAGE_DIV 45
#define FONT_SQUARE_DIV 55

SDL_Color color_black = {0, 0, 0};
extern FileSys *fs;

/* check if two TTF_Fonts are of the same size */

Game::Game(const GameSettings &set, const Menu::VideoSettings &video,
           const KeyBindings &k1, const KeyBindings &k2)
{
  /* muncher/troggle behaviour settings */
  muncher_name    = "muncher";
  troggle_file    = "troggles.cfg";
  trog_spawn_min  = 3000;
  trog_spawn_max  = 7000;
  trog_warn_time  = 1000;

  /* graphics settings */
  numdirty = 0;

  trogwarning_file = "trogwarning.png";
  gameover_file    = "gameover.png";
  trogwarning      = NULL;
  background_file  = "menu_back.png";
  background       = NULL;
  board_bg         = NULL;
  message          = NULL;
  message_button   = NULL;
  message_stop     = 0;

  board = NULL;
  level = NULL;

  /* font settings */
  fontname[FONT_SQUARE] = fontname[FONT_MESSAGE] = "URWPalladioL-Roma.ttf";
  fontname[FONT_TITLE] = fontname[FONT_LIVES] = fontname[FONT_SCORE]
    = "URWGothicL-Demi.ttf";
  for(int i=0; i<FONT_NUM; i++)
    {
      font[i] = NULL;
    }

  this->set   = set;
  // make sure the current (garbage) settings aren't the same as the desired
  // settings
  this->video.bpp = video.bpp - 1;
  this->changeVideoSettings(video);
  this->k1 = new KeyBindings(k1);
  this->k2 = new KeyBindings(k2);

  int nj = SDL_NumJoysticks();
  printMsg(1, "found %d joysticks\n", nj);
  if (nj) {
    for (int i=0; i<nj; i++) {
      SDL_Joystick *j = SDL_JoystickOpen(i);
      if (!j) {
	printWarning("failed to open joystick %d: %s\n", i, SDL_GetError());
      } else {
	this->jstick.push_back(j);
      }
    }
    SDL_JoystickEventState(SDL_ENABLE);
  }
}

void Game::readSettings(GameSettings *set,
                        Menu::VideoSettings *video,
                        Menu::SoundSettings *s,
                        KeyBindings *k1,
                        KeyBindings *k2)
{
  *k1 = KeyBindings("bindings1");
  *k2 = KeyBindings("bindings2");

  for (int i=0; i<KEY_NUM; i++) {
    if (!k1->hasEvent((enum Key)i)) {
      k1->addEvent((enum Key)i, Event(default_keys1[i]));
    }
    if (!k2->hasEvent((enum Key)i)) {
      k2->addEvent((enum Key)i, Event(default_keys2[i]));
    }
  }

  video->flags= SDL_SRCALPHA;

  /* read in overriding values from the config file */
  ConfigFile *config = fs->openConfig("gnumch.cfg");
  bool fulls = config->readInt("global", "fullscreen", 0);
  if (fulls) video->flags |= SDL_FULLSCREEN;
  video->w = config->readInt("global", "width", 800);
  video->h = config->readInt("global", "height", 600);
  video->bpp = config->readInt("global", "bpp", 32);
  s->frequency = config->readInt("global", "frequency", 44100);
  s->channels = config->readInt("global", "channels", 2);
  s->fx_volume = config->readInt("global", "fx_volume", 50);
  s->music_volume = config->readInt("global", "music_volume", 50);
  set->trog_number = config->readInt("global", "trog_number", 3);
  set->trog_wait = config->readInt("global", "trog_wait", 1000);
  set->change_time = config->readInt("global", "change_time", 300);
  set->eat_time = config->readInt("global", "eat_time", 500);
  set->width = config->readInt("global", "board_width", 6);
  set->height = config->readInt("global", "board_height", 6);
  delete config;
}

Game::~Game()
{
  delete k1;
  delete k2;
}

int Game::getChangeTime()
{
  return set.change_time;
}

int Game::getEatTime()
{
  return set.eat_time;
}

/* return a random number between trog_spawn_min and trog_spawn_max */
int Game::getTrogSpawnDelay()
{
  int delay = (int) 
    (((double)rand()/(RAND_MAX+1.0)) * (trog_spawn_max - trog_spawn_min)
     +  trog_spawn_min);

  printMsg(1, "setting trogSpawnDelay = %d\n", delay);
  return delay;
}

int Game::getTrogWarnTime()
{
  return trog_warn_time;
}

int Game::getTrogWait()
{
  return set.trog_wait;
}

int Game::getSquareWidth()
{
  return square_width;
}

int Game::getSquareHeight()
{
  return square_height;
}

void Game::changeGameSettings(const GameSettings &gset)
{
  set = gset;
}

void Game::changeKeyBindings(const KeyBindings &k1, const KeyBindings &k2)
{
  *(this->k1) = KeyBindings(k1);
  *(this->k2) = KeyBindings(k2);
}

const Game::GameSettings &Game::getGameSettings()
{
  return set;
}

void Game::getKeyBindings(KeyBindings *k1, KeyBindings *k2)
{
  *k1 = KeyBindings(*this->k1);
  *k2 = KeyBindings(*this->k2);
}

void Game::showTrogWarning()
{
  if( !warning_on )
    {
      SDL_Rect rect = {(video.w - right) + (right - trogwarning->w)/2,
		       top,
		       trogwarning->w,
		       trogwarning->h};

      SDL_BlitSurface(trogwarning, NULL, screen, &rect);
      refreshRect(&rect);
      warning_on = true;
    }
}

void Game::hideTrogWarning()
{
  if( warning_on )
    {
      Rectangle rect ((video.w - right) + (right - trogwarning->w)/2, top,
		      trogwarning->w, trogwarning->h);
      SDL_Rect sdl_rect = rect.toSDL();

      printMsg(1, "hiding troggle warning\n");
      drawBackground(rect);
      refreshRect(&sdl_rect);
      warning_on = false;
    }
}

void Game::showMessage(const char *text, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  if(message) hideMessage();

  message = Menu::renderString(text, font[FONT_MESSAGE]);
  message_button = Menu::createButton(message->w + message->h, 2*message->h, Menu::mapRGBA(r, g, b, a));

  SDL_Rect rect = {(video.w - message_button->w)/2, (video.w - message_button->h)/2, message_button->w, message_button->h};
  redrawRect(&rect);
  message_stop = 0;
}

void Game::showMessageTimed(const char *text, int ms, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  showMessage(text, r, g, b, a);
  message_stop = SDL_GetTicks() + ms;
}

void Game::hideMessage()
{
  if(message) {
    SDL_Rect rect = {(video.w - message_button->w)/2, 
		     (video.h - message_button->h)/2, message_button->w, message_button->h};
    redrawRect(&rect);
    SDL_FreeSurface(message);
    SDL_FreeSurface(message_button);
    message = NULL;
    message_button = NULL;
    message_stop = 0;
  }
}

bool Game::running()
{
  return board && !lost;
}

/* blit the correct picture onto a square. Draws the background, then draws the
 * text and munchers, if necessary */
void Game::redrawSquare(int x, int y)
{
  if(x<0 || x>=set.width || y<0 || y>=set.height) {
    printWarning("tried to draw a square off the board\n");
    return;
  }

  printMsg(2, "redrawing square %d,%d\n", x, y);
  SDL_Rect dest = {left + square_width*x, top + square_height*y,
		   square_width,         square_height};
  SDL_Surface *text       = board->getTextPic(x, y);

  /* draw the background */
  drawSquareBg(x, y);
  refreshRect(&dest);

  drawPlayersAt(x, y);

  /* draw the text */
  if(text) {
    dest.x = square_width*x  + left + (square_width  - text->w)/2;
    dest.y = square_height*y + top  + (square_height - text->h)/2;
    SDL_BlitSurface(text, NULL, screen, &dest);
  }

  /* draw a portion of the message */
  if(message) {
    if (message_stop && message_stop < (int)SDL_GetTicks()) {
      hideMessage();
    } else {
      SDL_Rect m_pos = {(video.w - message->w)/2, (video.h - message->h)/2,
			message->w, message->h};
      SDL_Rect m_pos2 = {(video.w - message_button->w)/2, (video.h - message_button->h)/2,
			 message_button->w, message_button->h};
      SDL_Rect sq_pos = {square_width*x + left, square_height*y + top,
			 square_width, square_height};
      SDL_Rect sq_pos2 = sq_pos;

      if (splitRectangles(&m_pos2, &sq_pos2)) {
	SDL_BlitSurface(message_button, &m_pos2, screen, &sq_pos2);
      }
      if (splitRectangles(&m_pos, &sq_pos)) {
	SDL_BlitSurface(message, &m_pos, screen, &sq_pos);
      }
    }
  }
}

void Game::drawPlayersAt(int x, int y)
{
  vector<Player*> players = board->getPlayersNear(x, y);
  SDL_Rect src = {0, 0, square_width, square_height};
  SDL_Rect dest = {left + square_width*x, top + square_height*y,
		   square_width,         square_height};

  /* draw the players */
  for(int i=0; i<(int)players.size(); i++) {
    SDL_Surface *pic = players[i]->getPic(x, y, square_width, square_height,
					  &src, &dest);
    dest.x += left + square_width*x;
    dest.y += top  + square_height*y;
    SDL_BlitSurface(pic, &src, screen, &dest);
  }

}

bool Game::splitRectangles(SDL_Rect *r1, SDL_Rect *r2)
{
  int x_min = max(r1->x, r2->x);
  int x_max = min(r1->x + r1->w, r2->x + r2->w);
  int y_min = max(r1->y, r2->y);
  int y_max = min(r1->y + r1->h, r2->y + r2->h);

  if (x_max > x_min && y_max > y_min) {
    SDL_Rect new_r1 = {
      max(x_min - r1->x, 0),
      max(y_min - r1->y, 0),
      x_max - x_min,
      y_max - y_min};
    r2->x = r1->x + new_r1.x;
    r2->y = r1->y + new_r1.y;
    *r1 = new_r1;
    return true;
  }
  return false;
}

void Game::setDirty(int x, int y)
{
  board->setDirty(x, y);
}

void Game::pause()
{
  board->pause();
}

void Game::resume()
{
  board->resume();
}

/* calls SDL_UpdateRect on the areas that need to be updated */
void Game::refresh()
{
  if(numdirty) {
    SDL_UpdateRects(screen, numdirty, dirty);
    numdirty = 0;
  }
}

/* redraw the whole screen (all squares, title, lives, etc) */
void Game::redrawAll()
{
  int i, j;
  SDL_Rect dest = {left, top};

  drawBackground(Rectangle(0, 0, video.w, video.h));
  SDL_BlitSurface(board_bg, NULL, screen, &dest);
  for(i=0; i<set.height; i++) {
    for(j=0; j<set.width; j++) {
      SDL_Surface *text = board->getTextPic(j, i);
      drawPlayersAt(j, i);
      if (text) {
	dest.x = square_width*j  + left + (square_width  - text->w)/2;
	dest.y = square_height*i + top  + (square_height - text->h)/2;
	SDL_BlitSurface(text, NULL, screen, &dest);
      }
    }
  }

  if (message) {
    SDL_Rect mess_dest = {(video.w - message->w) / 2,
			  (video.h - message->h) / 2};
    SDL_Rect butt_dest = {(video.w - message_button->w) / 2,
			  (video.h - message_button->h) / 2};
    SDL_BlitSurface(message_button, NULL, screen, &butt_dest);
    SDL_BlitSurface(message, NULL, screen, &mess_dest);
  }

  string title = level->getLevelTitle();
  Rectangle rect (0, 0, video.w, top);
  drawTextBox(title.c_str(), FONT_TITLE, Menu::getBlack(), Menu::getWhite(),
	      rect);
}

void Game::redrawSpiral(int ms_)
{
  int ms = (int) ((double)ms_ / (set.width * set.height));

  for (int i=0; i <= set.height/2; i++) {
    for (int j = i; j < set.width - i - 1; j++) {
      redrawSquare(j, i);
      refresh();
      SDL_Delay(ms);
    }
    for (int j = i; j < set.height - i - 1; j++) {
      redrawSquare(set.width - i - 1, j);
      refresh();
      SDL_Delay(ms);
    }
    for (int j = set.width - i - 1; j >= i; j--) {
      redrawSquare(j, set.height - i - 1);
      refresh();
      SDL_Delay(ms);
    }
    for (int j = set.height - i - 1; j > i; j--) {
      redrawSquare(i, j);
      refresh();
      SDL_Delay(ms);
    }
  }
}

void Game::changeVideoSettings(const Menu::VideoSettings &settings)
{
  if (video == settings)
    return;

  video = settings;

  square_width    = video.w / (set.width+2);
  square_height   = video.h / (set.height+2);
  left            = square_width;
  top             = square_height;
  right           = video.w - (square_width  * set.width)  - left - 1;
  bottom          = video.h - (square_height * set.height) - top - 1;
  screen = SDL_GetVideoSurface();

  /* resize the fonts */
  fontsize[FONT_SQUARE]  = video.w/FONT_SQUARE_DIV;
  fontsize[FONT_TITLE]   = video.w/FONT_TITLE_DIV;
  fontsize[FONT_LIVES]   = video.w/FONT_LIVES_DIV;
  fontsize[FONT_SCORE]   = video.w/FONT_SCORE_DIV;
  fontsize[FONT_MESSAGE] = video.w/FONT_MESSAGE_DIV;
  for(int i=0; i<FONT_NUM; i++) {
    if(font[i]) { // if !font[i], the game hasn't started yet
      TTF_CloseFont(font[i]);
      font[i] = fs->openFont(fontname[i], fontsize[i]);
    }
  }
  if (level) {
    level->reRender();
  }

  if(background) {
    SDL_FreeSurface(background);
    background = fs->openPic(background_file, video.w, video.h);
    drawBoardBg();
  }
  if(trogwarning) {
    SDL_FreeSurface(trogwarning);
    trogwarning = fs->openPic(trogwarning_file, right, -1);
  }

  vector<Animation*>::iterator an, an_end = anim.end();
  for(an=anim.begin(); an<an_end; an++)
    (*an)->reloadPics();
}

void Game::changeSoundSettings(const Menu::SoundSettings &s)
{
  vector<Animation*>::iterator an, an_end = anim.end();
  for(an=anim.begin(); an<an_end; an++)
    (*an)->reloadSounds();
}

void Game::drawBoardBg()
{
  if (board_bg) {
    SDL_FreeSurface(board_bg);
  }
  board_bg = Menu::createSurfaceNoAlpha(set.width * square_width + 1,
					set.height * square_height + 1);

  SDL_Rect src = {left, top, board_bg->w, board_bg->h};
  SDL_BlitSurface(background, &src, board_bg, NULL);
  boxColor(board_bg, 0, 0, board_bg->w, board_bg->h, Menu::mapRGBA(255, 255, 255, 192));

  for (int i=0; i<set.height; i++) {
    for (int j=0; j<set.width; j++) {
      rectangleColor(board_bg, j*square_width, i*square_height, 
		     (j+1)*square_width, (i+1)*square_height, black);
    }
  }
}

void Game::writeSettings(const GameSettings &g,
                         const Menu::VideoSettings &v,
                         const Menu::SoundSettings &s,
                         const KeyBindings &k1,
                         const KeyBindings &k2)
{
  ConfigFile *config = fs->openConfig("gnumch.cfg", true);
  config->writeInt("global", "fullscreen", (v.flags & SDL_FULLSCREEN)?1:0);
  config->writeInt("global", "width", v.w);
  config->writeInt("global", "height", v.h);
  config->writeInt("global", "bpp", v.bpp);
  config->writeInt("global", "frequency", s.frequency);
  config->writeInt("global", "fx_volume", s.fx_volume);
  config->writeInt("global", "music_volume", s.music_volume);
  config->writeInt("global", "channels", s.channels);
  config->writeInt("global", "trog_number", g.trog_number);
  config->writeInt("global", "trog_wait", g.trog_wait);
  config->writeInt("global", "change_time", g.change_time);
  config->writeInt("global", "eat_time", g.eat_time);
  config->writeInt("global", "board_width", g.width);
  config->writeInt("global", "board_height", g.height);
  delete config;

  k1.writeSettings();
  k2.writeSettings();
}

string Game::getError(int x, int y)
{
  return level->getError(board->getNum(x, y));
}

/* return a randomly chosen Number* from the LevelSet */
Number *Game::randomNumber()
{
  return level->randomNumber();
}

void Game::drawSquareBg(int x, int y)
{
  Rectangle dest (left+x*square_width, top+y*square_height,
		  square_width,        square_height);

  if (background) {
    SDL_Rect src = {x*square_width, y*square_height, square_width, square_height};
    SDL_Rect sdl_dest = dest.toSDL();
    SDL_BlitSurface(board_bg, &src, screen, &sdl_dest);
  } else {
    Menu::drawBox(dest, Menu::getWhite());
    rectangleColor(screen, dest.x, dest.y, 
		   dest.x+square_width, dest.y+square_height, black);
  }
}

void Game::drawBackground(const Rectangle &rect)
{
  SDL_Rect rec = rect.toSDL();

  if (background) {
    SDL_BlitSurface(background, &rec, screen, &rec);
  } else {
    SDL_FillRect(screen, &rec, Menu::getBlack());
  }
}

SDL_Surface *Game::renderText(const char *text, FontType type)
{
  SDL_Surface *ret;

  ret = renderString(font[type], text, color_black);
  if(ret->w > square_width || ret->h > square_height) {
    double zoomx, zoomy, zoom;
    SDL_Surface *newret;

    zoomx = (double)ret->w / square_width;
    zoomy = (double)ret->h / square_height;
    zoom  = (zoomx > zoomy) ? zoomx : zoomy;
    newret = zoomSurface(ret, zoom, zoom, 1);
    SDL_FreeSurface(ret);
    ret = newret;
    printWarning("textbox overflow; suggest decreasing text size\n");
  }
  return ret;
}

void Game::playerMove(Player *p, int old_x, int old_y, int x, int y, int time)
{
  printMsg(1, "player %p moving (%d,%d)-(%d-%d) at t=%d\n", p, old_x, old_y, x, y, time);
  /* if the player is a Troggle and has an onMove callback, only do the
   * callback if the Troggle is already on the board */
  if (p->isTroggle() && board->contains(old_x, old_y)) {
    ((Troggle*)p)->moveCallback();
  }

  p->move( old_x, old_y, x, y, time );    
}

void Game::playerStop(Player *p, int x, int y, int time)
{
  printMsg(1, "player %x stopping at (%d,%d) at t=%d\n", p, x, y, time);
  /* if the Player has walked off the board, kill it */
  if( x < 0 || x >= set.width || y < 0 || y >= set.height )
    {
      p->die( time );
      return;
    }

  vector<Player*>::iterator i;
  for (i=players.begin(); i<players.end(); i++) {
    if ( (*i) != p && (*i)->isAt(x, y) ) {
      int n = p->attack(*i);
      if (n == 1) {
	if ((*i)->isMuncher()) {
	  handleMuncherEaten((Muncher*)(*i), p);
	} else {
	  (*i)->die(time);
	}
	p->munch(time);
      } else if (n == -1) {
	if (p->isMuncher()) {
	  handleMuncherEaten((Muncher*)p, *i);
	} else {
	  p->die(time);
	}
	(*i)->munch(time);
      }
    }
  }

  /* if the player is a Troggle and has an onStop callback */
  if( p->isTroggle() )
    {
      ((Troggle*)p)->stopCallback();
    }
}

void Game::playerMunch(Player *p, int x, int y, int time, bool im)
{
  int n = board->munch(x, y);
  if( n == 1 || (n == -1 && im == true) )
    {
      if( p->isMuncher() )
        {
	  ((Muncher*)p)->givePoints(1);
        }
      setNum( x, y, NULL, true );
      p->munch( time );
    }
  else if( n == -1 )
    {
      if( p->isMuncher() ) {
	handleMuncherIndigestion((Muncher*)p);
      } else {
	p->die( time );
      }
      setNum( x, y, NULL, true );
    }
}

void Game::playerSpawn(Player *p, int time)
{
  p->spawn( time );
}

void Game::playerDie(Player *p, int time)
{
  p->die( time );
}

void Game::setNum(int x, int y, Number *n, bool wincheck)
{
  assert(x >= 0 && x < set.width && y >= 0 && y < set.height);
  Number *old = board->getNum(x, y);

  board->setNum(x, y, n, wincheck);
  if (old) level->releaseNumber(old);
}

/*_________________________________protected__________________________________*/

void Game::refreshRect(SDL_Rect *rect)
{
  if (rect->x < 0 || rect->y < 0
      || rect->x + rect->w > video.w
      || rect->y + rect->h > video.h) {
    printWarning("Tried to refresh invalid rectangle at (%d,%d)-(%d,%d)\n",
		 rect->x, rect->y, rect->x+rect->w, rect->y+rect->h);
    return;
  }

  if(numdirty > 19) {
    refresh();
  }
  dirty[numdirty++] = *rect;
};

void Game::drawTextBox(const char *string, FontType type, Uint32 fg, Uint32 bg,
                       const Rectangle &bound)
{
  SDL_Surface *text;
  SDL_Color color = {fg >> 24, (fg >> 16) & 0xff, (fg >> 8) & 0xff};

  text = renderString(font[type], string, color);
  if(text->w > bound.w || text->h > bound.h) {
    double zoomx, zoomy, zoom;
    SDL_Surface *newtext;

    zoomx = (double)bound.w / text->w;
    zoomy = (double)bound.h / text->h;
    zoom  = min(zoomx, zoomy);
    newtext = zoomSurface(text, zoom, zoom, 1);
    SDL_FreeSurface(text);
    text = newtext;
    printWarning("textbox overflow; suggest decreasing text size\n");
  }

  Rectangle dest (bound.x + (bound.w - text->w)/2 - text->h/2, 
		  bound.y + (bound.h - text->h)/2 - text->h/2,
		  text->w + text->h, text->h + text->h);
  SDL_Rect text_dest = {dest.x + text->h/2, dest.y + text->h/2};
  Menu::drawButton(dest, ((bg & 0xff) / 4 * 3) | (bg & ~0xff) );
  SDL_BlitSurface(text, NULL, screen, &text_dest);
  SDL_FreeSurface(text);

  SDL_Rect tmp = bound.toSDL();
  refreshRect(&tmp);
}

void Game::redrawRect(SDL_Rect *rect)
{
  int px_l, px_t, px_r, px_b;
  int sq_l, sq_t, sq_r, sq_b;
  int i, j;

  /* work out the area in pixels that needs to be redrawn */
  px_l = (video.w - rect->w)/2;
  px_t = (video.h - rect->h)/2;
  px_r = px_l + rect->w;
  px_b = px_t + rect->h;

  /* convert the coordinates to squares */
  sq_l = (px_l - left) / square_width;
  sq_t = (px_t - top ) / square_height;
  sq_r = (px_r - left) / square_width + 1;
  sq_b = (px_b - top ) / square_height + 1;

  /* set the squares as dirty */
  for(i=sq_l; i<=sq_r; i++) {
    for(j=sq_t; j<=sq_b; j++) {
      board->setDirty(i, j);
    }
  }
}

void Game::queuePlayerSpawn(Player *p, int x, int y)
{
  spawning_players.push_back(p);
  player_spawning_points.push_back(Point(x, y));
}

void Game::tryPlayerSpawn()
{
  int now = SDL_GetTicks();

  for (size_t i=0; i<spawning_players.size(); i++) {
    Point p = player_spawning_points[i];
    bool can_spawn = true;

    for (vector<Player*>::iterator j=players.begin(); j<players.end(); j++) {
      if ((*j)->isNear(p.x, p.y) && (*j)->isTroggle()) {
	can_spawn = false;
	break;
      }
    }
    if (can_spawn) {
      spawning_players[i]->spawn(now);
      spawning_players[i]->move(p.x, p.y, p.x, p.y, now);
      spawning_players.erase(spawning_players.begin() + i);
      player_spawning_points.erase(player_spawning_points.begin() + i);
      i--;
    }
  }
}

void Game::clearPlayerSpawn()
{
  spawning_players.clear();
  player_spawning_points.clear();
}

void Game::prepareGame()
{
  SDL_Surface *screen = SDL_GetVideoSurface();

  video.flags  = screen->flags;
  video.bpp    = screen->format->BitsPerPixel;
  numdirty = 0;

  /* set size parameters */
  video.w           = screen->w;
  video.h           = screen->h;
  square_width    = video.w / (set.width+2);
  square_height   = video.h / (set.height+2);
  left            = square_width;
  top             = square_height;
  right           = video.w - (square_width  * set.width)  - left - 1;
  bottom          = video.h - (square_height * set.height) - top - 1;

  /* open fonts */
  for(int i=0; i<FONT_NUM; i++) {
    font[i] = fs->openFont(fontname[i], fontsize[i]);
  }

  background = fs->openPic(background_file, video.w, video.h);
  SDL_EnableKeyRepeat(1, set.change_time);
  drawBoardBg();
}

void Game::freeGame()
{
  vector<Animation*>::iterator an;
  vector<Player*>::iterator p;

  for(an=anim.begin(); an<anim.end(); an++) {
    delete *an;
  }
  for (p=players.begin(); p<players.end(); p++) {
    delete *p;
  }

  if(background) SDL_FreeSurface(background), background=NULL;
  if(board_bg) SDL_FreeSurface(board_bg), board_bg=NULL;
  for(int i=0; i<FONT_NUM; i++) {
    TTF_CloseFont(font[i]);
    font[i] = NULL;
  }

  background  = NULL;
}

void Game::zoomPlayer(Player *p, int frames)
{
  if (!p->exist()) {
    return;
  }

  SDL_Rect rect;
  SDL_Surface *pic = p->getWholePic(square_width, square_height, &rect);
  double total_xzoom = (double) video.w / pic->w;
  double total_yzoom = (double) video.h / pic->h;
  Point pos = p->getPos();

  rect.x += left + pos.x * square_width;
  rect.y += top + pos.y * square_height;
  for (int i=1; i<=frames; i++) {
    SDL_Rect dest = { (rect.x*(frames-i)) / frames,
		      (rect.y*(frames-i)) / frames };
    double zoomx = total_xzoom * i / frames;
    double zoomy = total_yzoom * i / frames;

    SDL_Surface *tmp = zoomSurface(pic, zoomx, zoomy, 1);
    SDL_BlitSurface(tmp, NULL, screen, &dest);
    SDL_UpdateRect(screen, dest.x, dest.y, tmp->w, tmp->h);
    SDL_FreeSurface(tmp);
    SDL_Delay(30);
  }
}

TroggleGame::TroggleGame(const GameSettings &g, const Menu::VideoSettings &v,
                         const KeyBindings &k1, const KeyBindings &k2):
  Game(g, v, k1, k2)
{
}

TroggleGame::~TroggleGame()
{
}

void TroggleGame::troggleNextSpawn( Troggle *t, int time )
{
  trog_dead.push_back(t);

  /* have to make sure the list remains in order */
  int delay = getTrogSpawnDelay();
  if( trog_warning_times.empty() )
    {
      trog_warning_times.push_back( time + delay );
    }
  else
    {
      trog_warning_times.push_back( trog_warning_times.back() 
                                    + delay/troggles.size() );
    }
}

void TroggleGame::handleTrogSpawns()
{
  int now = SDL_GetTicks();

  assert (trog_warning_times.size() == trog_dead.size()
	  && trog_spawning_times.size() == trog_spawning.size());
  while ( !trog_warning_times.empty() && trog_warning_times.front() <= now ) {
    Troggle *t = trog_dead.front();
    int time = trog_warning_times.front();
    trog_dead.pop_front();
    trog_warning_times.pop_front();

    t->queueSpawn();
    trog_spawning.push_back(t);
    trog_spawning_times.push_back(time + trog_warn_time);
  }
  if (!trog_spawning_times.empty()) {
    showTrogWarning();
  } else {
    hideTrogWarning();
  }
  while ( !trog_spawning_times.empty() && trog_spawning_times.front() <= now ) {
    Troggle *t = trog_spawning.front();
    int time = trog_spawning_times.front();
    trog_spawning.pop_front();
    trog_spawning_times.pop_front();

    playerSpawn( t, time );
  }
}

void TroggleGame::pause()
{
  Game::pause();
  paused_time = SDL_GetTicks();
}

void TroggleGame::resume()
{
  Game::resume();
  int delay = SDL_GetTicks() - paused_time;

  size_t i;
  for (i=0; i<trog_warning_times.size(); i++) {
    trog_warning_times.push_back(trog_warning_times.front()+delay);
    trog_warning_times.pop_front();
  }
  for (i=0; i<trog_spawning_times.size(); i++) {
    trog_spawning_times.push_back(trog_spawning_times.front()+delay);
    trog_spawning_times.pop_front();
  }
}

void TroggleGame::setupTroggles()
{
  FILE *cfg = fs->openCfg(troggle_file);
  char trogname[20];
  char getmove_name[20], onmove_name[20], onstop_name[20];
  int i = 0;

  while(fscanf(cfg, " %19s", trogname) == 1) {
    if(     fscanf(cfg, " %19s", getmove_name) != 1
            ||  fscanf(cfg, " %19s", onmove_name ) != 1
            ||  fscanf(cfg, " %19s", onstop_name ) != 1) {
      printError("malformed troggle config file\n");
    }
    if (trog_mask & (1<<i)) {
      anim.push_back(new Animation(trogname));
      trogdef.push_back(new TroggleDef(anim.back(),
				       getAction(getmove_name),
				       getAction(onmove_name),
				       getAction(onstop_name)));
    }
    i++;
  }
  fclose(cfg);

  trogwarning = fs->openPic(trogwarning_file, right, -1);

  for( int i=0; i<set.trog_number; i++ ) {
    troggles.push_back( new Troggle() );
    players.push_back( troggles.back() );
  }

  cur_num_trog = cur_trog_type = 0;
}

void TroggleGame::freeTroggles()
{
  vector<TroggleDef*>::iterator td;

  for (td=trogdef.begin(); td<trogdef.end(); td++) {
    delete *td;
  }
  if (trogwarning) {
    SDL_FreeSurface(trogwarning);
    trogwarning=NULL;
  }

  /* no need to free the troggles or the animations; they are taken care of elsewhere */
}

TroggleDef *TroggleGame::randomTroggle()
{
  if (cur_trog_type == 0) {
    return trogdef.at(0);
  }
  return trogdef.at((int)((double)cur_trog_type*rand() / (RAND_MAX + 1.0)));
}

void TroggleGame::nextTrogLevel()
{
  if (cur_trog_type < (int)trogdef.size()) {
    cur_trog_type++;
  }
  if (cur_num_trog < set.trog_number) {
    cur_num_trog++;
  }
}

int TroggleGame::getTrogMask()
{
  return trog_mask;
}

void TroggleGame::setTrogMask(int mask)
{
  assert(mask & 0x1f);
  trog_mask = mask;
}

void TroggleGame::writeSettings(int trog_mask)
{
  ConfigFile *conf = fs->openConfig("gnumch.cfg", true);
  conf->writeInt("troggleGame", "trog_mask", trog_mask);
  delete conf;
}

void TroggleGame::readSettings(int *trog_mask)
{
  ConfigFile *conf = fs->openConfig("gnumch.cfg");
  *trog_mask = conf->readInt("troggleGame", "trog_mask", 0x1f);
  delete conf;
}

void TroggleGame::resetTroggles()
{
  trog_spawning.clear();
  trog_spawning_times.clear();
  trog_dead.clear();
  trog_warning_times.clear();

  warning_on = 1;
  hideTrogWarning();

  for(int i=0; i<cur_num_trog; i++)
    {
      troggles[i]->die( SDL_GetTicks() );
    }
}
