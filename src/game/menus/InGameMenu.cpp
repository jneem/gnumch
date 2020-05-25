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
#include "Menus.h"
#include "../Game.h"
#include "CreditsMenu.h"
#include "InGameMenu.h"
#include "MainMenu.h"
#include "MultiPlayMenu.h"
#include "OptionsMenu.h"
#include "ScoresMenu.h"
#include "SinglePlayMenu.h"

extern Game *game;

static void resumeGame(Clickable *a, void *b)
{
    Menu::stop();
    Menu::setBackground("menu_back.png");
    popMenuNoRedraw();
    SDL_SetEventFilter(gameEventFilter);
    SDL_ShowCursor(0);

    if (game->running()) {
        game->resume();
        game->redrawAll();
        SDL_Flip(SDL_GetVideoSurface());
    }
}

static void stopGame(Clickable *a, void *b)
{
    game->end();
    resumeGame(NULL, NULL);
}

static void pushOptions(Clickable *a, void *b)
{
    Container *c = new InGameOptionsMenu();
    pushMenu(c);
}

static void pushCredits(Clickable *a, void *b)
{
    Container *c = new CreditsMenu();
    pushMenu(c);
}

static void pushScores(Clickable *a, void *b)
{
    Container *c = new ScoresMenu();
    pushMenu(c);
}

InGameMenu::InGameMenu():
    resume(_("Resume Game")),
    options(_("Options")),
    credits(_("Credits")),
    scores(_("High Scores")),
    stop(_("Stop Game")),
    quit(_("Quit"))
{
    centred.add(&resume);
    centred.add(&options);
    centred.add(&credits);
    centred.add(&scores);
    centred.add(&stop);
    centred.add(&quit);
    centred.setAlign(Container::ALIGN_JUSTIFIED);

    hcentred.add(&centred);
    this->add(&hcentred);
    this->setPack(Container::PACK_MAX);

    Menu::VideoSettings vset = Menu::getVideoSettings();
    background = Menu::createSurface(vset.w, vset.h);
    SDL_BlitSurface(SDL_GetVideoSurface(), NULL, background, NULL);

    /* tint the background grey */
    boxRGBA(background, 0, 0, vset.w, vset.h, 128, 128, 128, 64);

    Menu::setBackground(background);

    resume.setCallback(resumeGame, NULL);
    options.setCallback(pushOptions, NULL);
    credits.setCallback(pushCredits, NULL);
    scores.setCallback(pushScores, NULL);
    stop.setCallback(stopGame, NULL);
    quit.setCallback(quitGame, NULL);
}

InGameMenu::~InGameMenu()
{
}
