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
#include <SinglePlayerGame.h>
#include <FileSys.h>
#include <Menu.h>
#include <Event.h>
#include <Muncher.h>
#include "menus/InGameMenu.h"
#include <Animation.h>

extern Container *enter_score_menu;

/* divide the screen width by these amounts to get the size of the desired font.
 */
#define FONT_TITLE_DIV 40
#define FONT_LIVES_DIV 40
#define FONT_SCORE_DIV 40
#define FONT_MESSAGE_DIV 45
#define FONT_SQUARE_DIV 55

extern FileSys *fs;

SinglePlayerGame::SinglePlayerGame(const GameSettings &set,
                                   const Menu::VideoSettings &video,
                                   const KeyBindings &k1,
                                   const KeyBindings &k2):
    TroggleGame(set, video, k1, k2)
{
    lives = 3;
    score = 0;
}

SinglePlayerGame::~SinglePlayerGame()
{
}

void SinglePlayerGame::updateLives(Player *ignored, int lives)
{
    this->lives = lives;

    char tmp[strlen(_("Lives = ")) + 3];
    sprintf(tmp, "%s%02d", _("Lives = "), lives);

    /* draw in the right half of the bottom bar */
    Rectangle rect(video.w/2, video.h - bottom, video.w/2, bottom);
    drawBackground(rect);
    drawTextBox(tmp, FONT_LIVES, Menu::getBlack(), Menu::getWhite(), rect);
}

void SinglePlayerGame::updateScore(Player *ignored, int score)
{
    if ( (score)/50 > this->score/50 ) {
        lives++;
        updateLives( ignored, lives );
    }
    this->score = score;

    char tmp[strlen(_("Score = ")) + 7];
    sprintf(tmp, "%s%06d", _("Score = "), score);

    /* draw in the left half of the bottom bar */
    Rectangle rect(0, video.h - bottom, video.w/2, bottom);
    drawBackground(rect);
    drawTextBox(tmp, FONT_SCORE, Menu::getBlack(), Menu::getWhite(), rect);
}

void SinglePlayerGame::redrawAll()
{
    Game::redrawAll();
    if (!trog_spawning_times.empty()) {
        showTrogWarning();
    }
    updateScore(NULL, score);
    updateLives(NULL, lives);
}

/* free all the memory used in the game. Record the high score if necessary.
 * Exit gracefully to the menu */
void SinglePlayerGame::end()
{
    lost = 1;
}

/* go on to the next level */
void SinglePlayerGame::win()
{
    printMsg(1, "you beat the level\n");
    won = 1;
}

/* run a game using the selected levelset.
 * 0) set the size parameters (eg. square_width, square_height, etc)
 * 1) open the fonts and animation pictures of the required sizes
 * 2) open the bg pic and create the board
 */
void SinglePlayerGame::run(Level *level_, SDL_Surface *screen_)
{
    screen = screen_;
    level = level_;
    SDL_ShowCursor(0);

    prepareGame();

    /* open the animations */
    anim.push_back(new Animation(muncher_name));
    anim[0]->load();
    players.clear();

    players.push_back( muncher = new Muncher( anim[0] ) );
    setupTroggles();

    board = new Board(set.width, set.height, players);

    lost = 0;
    bool trog_next_level = false;
    while(!lost) {
        level->nextLevel();
        if (trog_next_level) { // bump the troggle difficulty every second level
            nextTrogLevel();
            trog_next_level = false;
        } else {
            trog_next_level = true;
        }
        runLevel();
    }

    /* free game memory */
    delete board;
    freeGame();
    freeTroggles();

    board       = NULL;
    level       = NULL;
    trogwarning = NULL;

    /* prepare for return to menu */
    SDL_ShowCursor(1);
    SDL_SetEventFilter(menuEventFilter);
}

void SinglePlayerGame::handleMuncherEaten(Muncher *m, Player *eater)
{
    showMessage( _("You were eaten by a Troggle.\nPress <Return> to continue.") );

    if (--lives >= 0) updateLives(m, lives);
    m->die( SDL_GetTicks() );
}

void SinglePlayerGame::handleMuncherIndigestion(Muncher *m)
{
    string message = getError(m->getPos().x, m->getPos().y);

    message += '\n';
    message += _("Press <Return> to continue.");
    showMessage( message.c_str() );

    if (--lives >= 0) updateLives(m, lives);
    m->die( SDL_GetTicks() );
}

void SinglePlayerGame::playerSpawn(Player *p, int time)
{
    if( p == muncher ) {
        hideMessage();
        if (lives < 0) {
            end();
        }
        queuePlayerSpawn(p, 0, 0);
    } else {
        p->spawn(time);
    }
}

Player *SinglePlayerGame::getNearestMuncher(int x, int y)
{
    if( muncher->exist() )
        return muncher;
    return NULL;
}

/*_________________________________protected__________________________________*/

/* 1) create the list of Number*s
 * 2) reset the Board
 * 3) draw the level title
 * 4) main game loop
 */
void SinglePlayerGame::runLevel()
{
    Event event;
    int now;

    resetTroggles();
    clearPlayerSpawn();

    FPSmanager manager;
    SDL_initFramerate(&manager);
    SDL_setFramerate(&manager, 50);
    won = 0;

    showMessage( _("Ready..."), 0xee, 0xee, 0, 0xaa);
    redrawAll();
    SDL_Flip(screen);
    board->reset();
    redrawSpiral(800);

    showMessageTimed( _("GO!"), 1500, 0, 0xf5, 0, 0xaa);
    while( event.pollEvent() ) handleEvent( event ); // clear event queue
    muncher->spawn( SDL_GetTicks() );
    board->update();
    refresh();

    while(!won && !lost)
    {
        now = SDL_GetTicks();
        while( event.pollEvent() )
        {
            handleEvent( event );
        }
        handleTrogSpawns();
        tryPlayerSpawn();
        board->update();
        if (!won && !lost) {
            refresh();
        }
        SDL_framerateDelay(&manager);
    }
    if (!board->isPaused()) {
        muncher->die(SDL_GetTicks());
        while (muncher->exist()) {
            board->update();
            refresh();
            SDL_framerateDelay(&manager);
        }
    }
    board->unset();
}

void SinglePlayerGame::handleEvent(const Event &event)
{
    enum Key key;

    key = k1->getKey(event);
    if (key == KEY_NUM) {
        key = k2->getKey(event);
    }
    if (key == KEY_NUM) {
        return;
    }

    if (key == KEY_MENU) {
        Mix_HaltChannel(-1);
        SDL_SetEventFilter(menuEventFilter);
        pushMenu(new InGameMenu());
        pause();
        Menu::run();
    } else {
        muncher->handleKey(key);
    }
}
