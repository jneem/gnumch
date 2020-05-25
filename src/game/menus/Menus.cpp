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
#include <libGui.h>
#include "../Game.h"
#include "Menus.h"

static vector<Container*> menu_stack;

Game::GameSettings game_settings;
Menu::VideoSettings video_settings;
Menu::SoundSettings sound_settings;
KeyBindings bindings1;
KeyBindings bindings2;
int trog_mask;

void quitGame(Clickable *ho, void *hum)
{
    cout << _("Thank you for playing Gnumch.\nMake sure to check for updates regularly at http://spuzz.net\n");

    /* by deleting everything in the menu stack, we ensure that all settings will be
     * written out */
    vector<Container*>::iterator i;
    for (i = menu_stack.begin(); i != menu_stack.end(); i++)
        delete *i;

    Game::writeSettings(game_settings, video_settings, sound_settings, bindings1, bindings2);
    TroggleGame::writeSettings(trog_mask);
    exit(0);
}

void pushMenu(Container *menu)
{
    menu_stack.push_back(menu);
    Menu::setRoot(menu);
}

void popMenu(Clickable *blah, void *bleh)
{
    popMenuNoRedraw();
}

void popMenuNoRedraw(Clickable *blah, void *bleh)
{
    menu_stack.pop_back();

    Container *wid = menu_stack.back();
    Menu::setRoot(wid, true);
}

void discardTopMenu()
{
    assert(!menu_stack.empty());
    delete menu_stack.back();
    menu_stack.pop_back();
}
