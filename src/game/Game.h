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
#ifndef GAME_H
#define GAME_H

#include <Gnumch.h>
#include <Menu.h>
#include <Event.h>

class Player;
class Troggle;
class TroggleDef;
class Muncher;
class Board;
class Level;
class Number;
class Animation;
class AnimationState;

typedef enum {
  FONT_SQUARE,
  FONT_TITLE,
  FONT_LIVES,
  FONT_SCORE,
  FONT_MESSAGE,
  FONT_NUM
} FontType;

class Game {
 public:
  typedef struct {
    int trog_number;
    int trog_wait;
    int change_time;
    int eat_time;
    int width;
    int height;
  } GameSettings;

  Game(const Game::GameSettings&, const Menu::VideoSettings&,
       const KeyBindings&, const KeyBindings&);
  virtual ~Game();

  // retrieve and set various settings
  static void     readSettings(GameSettings*,
			       Menu::VideoSettings*,
			       Menu::SoundSettings*,
			       KeyBindings*,
			       KeyBindings*);
  static void     writeSettings(const GameSettings&,
				const Menu::VideoSettings&,
				const Menu::SoundSettings&,
				const KeyBindings&,
				const KeyBindings&);

  virtual int     getChangeTime();
  virtual int     getEatTime();
  virtual int     getTrogSpawnDelay();
  virtual int     getTrogWarnTime();
  virtual int     getTrogWait();
  virtual int     getSquareWidth();
  virtual int     getSquareHeight();
  int             getWidth()  { return set.width; }
  int             getHeight() { return set.height;}
  virtual const GameSettings &getGameSettings();
  virtual void    getKeyBindings(KeyBindings*, KeyBindings*);

  // signals that are sent by Players
  virtual void    playerMove(Player*, int old_x, int old_y, 
			     int x, int y, int time);
  virtual void    playerStop(Player*, int x, int y, int time);
  virtual void    playerMunch(Player*, int x, int y, int time,
			      bool immune=0);
  virtual void    playerSpawn(Player*, int time);
  virtual void    playerDie(Player*, int time);
  virtual void    troggleNextSpawn(Troggle*, int time) = 0;
  virtual void    setNum(int, int, Number*, bool win=1);

  virtual void    showTrogWarning();
  virtual void    hideTrogWarning();
  virtual void    showMessage(const char*, Uint8 r=255, Uint8 g=255, Uint8 b=255, Uint8 a=128);
  virtual void    showMessageTimed(const char*, int ms, Uint8 r=255, Uint8 g=255, Uint8 b=255, Uint8 a=128);
  virtual void    hideMessage();
  virtual void    updateLives(Player*, int) = 0;
  virtual void    updateScore(Player*, int) = 0;
  virtual void    end() = 0;
  virtual void    win() = 0;
  virtual bool    running();
  virtual void    redrawSquare(int,int);
  virtual void    setDirty(int x, int y);
  virtual void    refresh();
  virtual void    redrawAll();
  virtual void    redrawSpiral(int ms);
  virtual void    resume();
  virtual void    pause();
  virtual void    run(Level*, SDL_Surface*) = 0;
  virtual void    changeVideoSettings(const Menu::VideoSettings&);
  virtual void    changeSoundSettings(const Menu::SoundSettings&);
  virtual void    changeGameSettings(const GameSettings&);
  virtual void    changeKeyBindings(const KeyBindings&, const KeyBindings&);
  virtual string  getError(int, int);
  virtual Number  *randomNumber();
  void            drawSquareBg(int, int);
  void            drawBackground(const Rectangle&);
  SDL_Surface *renderText(const char*, FontType);

  virtual Player *getNearestMuncher(int, int) = 0;

 protected:
  bool won;
  bool lost;
  int score;
  Board *board;
  Level *level;

  GameSettings set;
  Menu::VideoSettings video;
  KeyBindings *k1, *k2;

  /* muncher/troggle behaviour settings */
  char *muncher_name;
  char *troggle_file;
  int trog_spawn_min;
  int trog_spawn_max;
  int trog_warn_time;
  vector<Animation*> anim;

  /* size settings */
  int square_width;
  int square_height;
  int top;
  int bottom;
  int left;
  int right;

  /* graphics settings */
  SDL_Surface *screen;
  SDL_Rect dirty[20];
  int numdirty;

  /* the troggle warning */
  char *trogwarning_file;
  SDL_Surface *trogwarning;
  bool warning_on;

  /* the message */
  SDL_Surface *message;
  SDL_Surface *message_button;
  int message_stop;

  /* gameboard background */
  char *background_file;
  SDL_Surface *background;
  SDL_Surface *board_bg;
  void drawBoardBg();

  /* game over filename */
  char *gameover_file;

  /* fonts */
  char *fontname[FONT_NUM];
  int fontsize[FONT_NUM];
  TTF_Font *font[FONT_NUM];

  void drawTextBox(const char*, FontType, Uint32 fg, Uint32 bg,
		   const Rectangle&);
  void redrawRect(SDL_Rect*);
  void refreshRect(SDL_Rect*);
  void drawPlayersAt(int x, int y);

  static bool splitRectangles(SDL_Rect*, SDL_Rect*);

  // for handling players. This allows more of the game logic to go in
  // the parent game; these functions are for child-specific actions.
  virtual void handleMuncherEaten(Muncher *m, Player *eater) = 0;
  virtual void handleMuncherIndigestion(Muncher *m) = 0;

  // queue a player to spawn at the specified square. The player will spawn as soon
  // as the square is empty.
  void queuePlayerSpawn(Player *p, int x, int y);
  void tryPlayerSpawn();  // spawn any queued players whose spawn point is free
  void clearPlayerSpawn();
  vector<Player*> spawning_players;
  vector<Point> player_spawning_points;

  // set all the sizing information, open the fonts and the background picture
  void prepareGame();
  void freeGame();    // free the fonts, the players and the background picture


  // if the player exist(), zoom it to the middle of the screen in the given number of frames.
  void zoomPlayer(Player *p, int frames);

  vector<Player*> players;
  vector<SDL_Joystick*> jstick;
};

class TroggleGame: public Game {
 public:
  TroggleGame(const Game::GameSettings&, const Menu::VideoSettings&,
	      const KeyBindings&, const KeyBindings&);
  virtual ~TroggleGame();

  virtual void troggleNextSpawn( Troggle*, int );
  virtual TroggleDef *randomTroggle();
  virtual void pause();
  virtual void resume();
  virtual void nextTrogLevel();

  virtual int getTrogMask();
  virtual void setTrogMask(int);

  static void writeSettings(int);
  static void readSettings(int*);

 protected:
  vector<Troggle*> troggles;
  deque<Troggle*>  trog_dead;
  deque<int>       trog_warning_times;
  deque<Troggle*>  trog_spawning;
  deque<int>       trog_spawning_times;
  vector<TroggleDef*> trogdef;

  int paused_time;

  int cur_num_trog;
  int cur_trog_type;
  int trog_mask;

  void setupTroggles();
  void freeTroggles();

  void resetTroggles();
  void handleTrogSpawns();
};

#endif
