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

#include "MultiPlayMenu.h"
#include "../MultiPlayerGame.h"
#include "../FileSys.h"
#include "../Level.h"
#include "Menus.h"

extern Game::GameSettings game_settings;
extern Menu::VideoSettings video_settings;
extern KeyBindings bindings1;
extern KeyBindings bindings2;
extern int trog_mask;
extern Game *game;

MultiPlayMenu::MultiPlayMenu():
    target_spin(1000),
    target_score(_("Target score"))
{
    play.setCallback(runGame, this);

    for (int i=990; i>=100; i -= 10) {
        target_spin.addItem(i);
    }
    target_spin.setItem(80);

    score_box.add(&target_score);
    score_box.add(&target_spin);
    top.insert(&score_box, &level_spin);
}

MultiPlayMenu::~MultiPlayMenu()
{
}

void MultiPlayMenu::runGame(Clickable *blah, void *playmenu)
{
    MultiPlayMenu *p = static_cast<MultiPlayMenu*>(playmenu);

    if (p->level_spin.getItem() == _("Choose a level")) return;

    Level *l = p->level_list.at( p->level_spin.getIndex()-1 )->makeLevel();
    int target = p->target_spin.getItem();

    game = new MultiPlayerGame( game_settings, video_settings,
                                bindings1, bindings2, target );
    static_cast<TroggleGame*>(game)->setTrogMask(trog_mask);
    SDL_SetEventFilter(gameEventFilter);
    game->run(l, SDL_GetVideoSurface());

    /* when the game is over */
    popMenu();
    delete l;
    delete game;
    game = NULL;

    Menu::redrawAll();
    Menu::resume();
}
