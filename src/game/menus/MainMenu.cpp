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
#include "CreditsMenu.h"
#include "InGameMenu.h"
#include "MainMenu.h"
#include "MultiPlayMenu.h"
#include "OptionsMenu.h"
#include "ScoresMenu.h"
#include "SinglePlayMenu.h"

extern Game::GameSettings game_settings;
extern Menu::VideoSettings video_settings;
extern Menu::SoundSettings sound_settings;
extern KeyBindings bindings1;
extern KeyBindings bindings2;
extern int trog_mask;

static void pushSinglePlay(Clickable *a, void *b)
{
    Container *c = new SinglePlayMenu();
    pushMenu(c);
}

static void pushMultiPlay(Clickable *a, void *b)
{
    pushMenu(new MultiPlayMenu());
}

static void pushOptions(Clickable *a, void *b)
{
    Container *c = new OptionsMenu();
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

MainMenu::MainMenu():
    play    (_("Single Player")),
    mplay   (_("Multi Player")),
    options (_("Options")),
    credits (_("Credits")),
    scores  (_("High Scores")),
    quit    (_("Quit"))
{
    Game::readSettings(&game_settings, &video_settings, &sound_settings, &bindings1, &bindings2);
    TroggleGame::readSettings(&trog_mask);

    centred.add(&play);
    centred.add(&mplay);
    centred.add(&options);
    centred.add(&credits);
    centred.add(&scores);
    centred.add(&quit);
    centred.setAlign(Container::ALIGN_JUSTIFIED);

    hcentred.add(&centred);
    this->add(&hcentred);
    this->setPack(Container::PACK_MAX);

    play.setCallback(pushSinglePlay, NULL);
    mplay.setCallback(pushMultiPlay, NULL);
    options.setCallback(pushOptions, NULL);
    credits.setCallback(pushCredits, NULL);
    scores.setCallback(pushScores, NULL);
    quit.setCallback(quitGame, NULL);
}

MainMenu::~MainMenu()
{
}
