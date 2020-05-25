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
#include <MultiPlayerGame.h>
#include <FileSys.h>
#include <Menu.h>
#include "menus/InGameMenu.h"
#include <Animation.h>
#include <Muncher.h>
#include <Event.h>

extern FileSys *fs;

const int MESSAGE_DISPLAY_TIME = 3000;

MultiPlayerGame::MultiPlayerGame(const GameSettings &set,
                                 const Menu::VideoSettings &video,
                                 const KeyBindings &k1,
                                 const KeyBindings &k2,
                                 int target): TroggleGame(set, video, k1, k2)
{
    target_score = target;
    message_display_time = 0;
}

MultiPlayerGame::~MultiPlayerGame()
{
}

void MultiPlayerGame::updateLives(Player *ignored, int lives)
{
    printWarning("two player games don't deal with lives\n");
}

void MultiPlayerGame::updateScore(Player *p, int score)
{
    Muncher *m = (Muncher*)p;
    Rectangle rect;

    if (score >= target_score) {
        zoomPlayer(m, 15);
        drawTextBox( m == muncher[0] ? _("Player 1 wins! Press any key to exit.")
                                     : _("Player 2 wins! Press any key to exit."),
                     FONT_TITLE, Menu::getBlack(), Menu::getWhite(),
                     Rectangle(0, 0, video.w, video.h));
        Event e;
        while (e.pollEvent()); // flush events

        SDL_Flip(screen);

        SDL_Event dummy;
        SDL_WaitEvent(&dummy);

        end();
    }

    if (m == muncher[0]) {
        this->score[0] = score;
        rect = Rectangle(0, video.h - bottom, video.w/2, bottom);
    } else {
        this->score[1] = score;
        rect = Rectangle(video.w/2, video.h - bottom, video.w/2, bottom);
    }

    char tmp[strlen(_("Score = ")) + 7];
    sprintf(tmp, "%s%06d", _("Score = "), score);

    /* draw in the left half of the bottom bar */
    drawBackground(rect);
    drawTextBox(tmp, FONT_SCORE, Menu::getBlack(), Menu::getWhite(), rect);
}

void MultiPlayerGame::redrawAll()
{
    Game::redrawAll();
    if (!trog_spawning_times.empty()) {
        showTrogWarning();
    }
    updateScore(muncher[0], score[0]);
    updateScore(muncher[1], score[1]);
}

void MultiPlayerGame::end()
{
    lost = 1;
}

/* go on to the next level */
void MultiPlayerGame::win()
{
    printMsg(1, "you beat the level\n");
    won = 1;
}

/* run a game using the selected levelset.
 * 0) set the size parameters (eg. square_width, square_height, etc)
 * 1) open the fonts and animation pictures of the required sizes
 * 2) open the bg pic and create the board
 */
void MultiPlayerGame::run(Level *level_, SDL_Surface *screen_)
{
    screen = screen_;
    level = level_;
    SDL_ShowCursor(0);

    prepareGame();

    /* open the animations */
    anim.push_back(new Animation("muncher"));
    anim.push_back(new Animation("muncher2"));
    anim[0]->load();
    anim[1]->load();
    players.clear();

    players.push_back( muncher[0] = new Muncher( anim[0] ) );
    players.push_back( muncher[1] = new Muncher( anim[1] ) );
    score[0] = score[1] = 0;
    setupTroggles();

    board = new Board(set.width, set.height, players);

    lost = 0;
    bool trog_next_level = false;
    while(!lost) {
        level->nextLevel();
        if (trog_next_level) {
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

void MultiPlayerGame::showMessage(const char *text, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Game::showMessage(text, r, g, b, a);
    message_display_time = SDL_GetTicks();
}

void MultiPlayerGame::handleMuncherEaten(Muncher *m, Player *eater)
{
    m->givePoints(-5);
    m->die(SDL_GetTicks());
    m->handleKey(KEY_SPAWN);
}

void MultiPlayerGame::handleMuncherIndigestion(Muncher *m)
{
    string message = getError(m->getPos().x, m->getPos().y);

    if (m == muncher[0]) {
        showMessage(message.c_str(), 0, 0xf5, 0, 128);
    } else {
        showMessage(message.c_str(), 0, 0x55, 0xff, 128);
    }

    m->givePoints(-5);
}

void MultiPlayerGame::playerSpawn(Player *p, int time)
{
    if (p->isMuncher()) {
        Muncher *m = static_cast<Muncher*>(p);
        if (m == muncher[0]) {
            queuePlayerSpawn(m, 0, 0);
        } else {
            queuePlayerSpawn(m, set.width-1, set.height-1);
        }
    } else {
        p->spawn( time );
    }
}

Player *MultiPlayerGame::getNearestMuncher(int x, int y)
{
    int d0, d1, min_d;
    Point m0 = muncher[0]->getPos();
    Point m1 = muncher[1]->getPos();

    d0 = muncher[0]->exist() ? abs(m0.x - x) + abs(m0.y - y) : INT_MAX;
    d1 = muncher[1]->exist() ? abs(m1.x - x) + abs(m1.y - y) : INT_MAX;
    min_d = min(d0, d1);
    if (min_d < INT_MAX) {
        return (min_d == d0) ? muncher[0] : muncher[1];
    }
    return NULL;
}

/*_________________________________protected__________________________________*/

/* 1) create the list of Number*s
 * 2) reset the Board
 * 3) draw the level title
 * 4) main game loop
 */
void MultiPlayerGame::runLevel()
{
    Event event;
    int now;

    resetTroggles();
    clearPlayerSpawn();

    FPSmanager manager;
    SDL_initFramerate(&manager);
    SDL_setFramerate(&manager, 50);
    won = 0;

    showMessage( _("Ready..."), 0xee, 0xee, 0, 0xaa );
    redrawAll();
    SDL_Flip(screen);
    board->reset();
    redrawSpiral(800);

    showMessageTimed( _("GO!"), 1500, 0, 0xf5, 0, 0xaa);
    while( event.pollEvent() ) handleEvent( event ); // clear event queue
    muncher[0]->spawn( SDL_GetTicks() );
    muncher[1]->spawn( SDL_GetTicks() );
    muncher[1]->move( set.width-1, set.height-1, set.width-1, set.height-1, SDL_GetTicks() );
    board->update();
    refresh();

    while(!won && !lost)
    {
        now = SDL_GetTicks();
        while( event.pollEvent() )
        {
            handleEvent( event );
        }
        if (message && now >= message_display_time + MESSAGE_DISPLAY_TIME) {
            hideMessage();
        }
        handleTrogSpawns();
        tryPlayerSpawn();
        board->update();
        if (!won && !lost) {
            refresh();
        }
        SDL_framerateDelay(&manager);
    }
    if (!board->isPaused() && !lost) {
        muncher[0]->die(SDL_GetTicks());
        muncher[1]->die(SDL_GetTicks());
        while (muncher[0]->exist() || muncher[1]->exist()) {
            board->update();
            refresh();
            SDL_framerateDelay(&manager);
        }
    }
    board->unset();
}

void MultiPlayerGame::handleEvent(const Event &event)
{
    bool muncher1 = true;
    enum Key key;

    key = k1->getKey(event);
    if (key == KEY_NUM) {
        muncher1 = false;
        key = k2->getKey(event);
    }
    if (key == KEY_NUM) {
        return;
    }

    if (key == KEY_MENU) {
        SDL_SetEventFilter(menuEventFilter);
        pushMenu(new InGameMenu());
        pause();
        Menu::run();
    } else if (muncher1) {
        muncher[0]->handleKey(key);
    } else {
        muncher[1]->handleKey(key);
    }
}
