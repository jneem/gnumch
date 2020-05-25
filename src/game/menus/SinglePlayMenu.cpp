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
#include "SinglePlayMenu.h"
#include "Menus.h"
#include "../Game.h"
#include "../SinglePlayerGame.h"
#include "../FileSys.h"

extern Game::GameSettings game_settings;
extern Menu::VideoSettings video_settings;
extern KeyBindings bindings1;
extern KeyBindings bindings2;
extern int trog_mask;
extern Game *game;
extern FileSys *fs;

void SinglePlayMenu::showDesc(Spinner<string> *spin, void *playmenu)
{
    SinglePlayMenu *p = (SinglePlayMenu*) playmenu;
    int i = spin->getIndex();
    p->description.setText(p->level_desc_list.at(i));
    //p->top->setSize(-1, -1);
    //p->bottom->setSize(-1, -1);
}

void SinglePlayMenu::runGame(Clickable *blah, void *playmenu)
{
    SinglePlayMenu *p = (SinglePlayMenu*) playmenu;

    if(p->level_spin.getItem() == _("Choose a level")) return;

    Level *l = p->level_list.at( p->level_spin.getIndex()-1 )->makeLevel();
    game = new SinglePlayerGame( game_settings, video_settings, bindings1, bindings2);
    static_cast<TroggleGame*>(game)->setTrogMask(trog_mask);
    SDL_SetEventFilter(gameEventFilter);
    game->run(l, SDL_GetVideoSurface());

    /* when the game is over */
    popMenu();

    int score = ((SinglePlayerGame*)game)->getScore();
    if (fs->checkScore(score)) {
        pushMenu(new EnterScoreMenu(score));
    }

    delete l;
    delete game;
    game = NULL;
    Menu::redrawAll();
    Menu::resume();
}

void SinglePlayMenu::configLevel(Clickable *c, SinglePlayMenu *p)
{
    if (p->level_spin.getItem() == _("Choose a level")) return;

    Container *cont = p->level_list.at( p->level_spin.getIndex()-1 )->makeConfigDialog();
    if (cont) {
        pushMenu(cont);
    }
}

SinglePlayMenu::SinglePlayMenu():
    back(_("Back")),
    play(_("Play")),
    conf(_("Configure Level")),
    level_spin("", true),
    description(_("No description found."))
{
    Level::getLevelList(&level_list);
    for (vector<LevelConfig*>::iterator i=level_list.begin();
                                         i<level_list.end(); i++) {
        level_name_list.push_back((*i)->title);
        level_desc_list.push_back((*i)->description);
    }

    level_name_list.insert(level_name_list.begin(), _("Choose a level"));
    level_desc_list.insert(level_desc_list.begin(), _("No description found."));

    level_spin.reset(level_name_list, 0);

    level_spin.setCallback(showDesc, this);
    back.setCallback(popMenu, NULL);
    play.setCallback(runGame, this);
    conf.setCallback((Clickable::Callback)configLevel, this);

    this->add(&top);
    this->add(&bottom);
    this->setPack(Container::PACK_LEADING);
    this->setAlign(Container::ALIGN_JUSTIFIED);

    bottom.add(&bottom_l);
    bottom.add(&bottom_m);
    bottom.add(&bottom_r);
    bottom.setAlign(Container::ALIGN_BOTTOM);
    bottom.setPack(Container::PACK_MAX);

    bottom_l.add(&back);
    bottom_l.setAlign(Container::ALIGN_LEFT);
    bottom_m.add(&conf);
    bottom_r.add(&play);
    bottom_r.setAlign(Container::ALIGN_RIGHT);

    top.add(&level_spin);
    top.add(&top_mid);
    top.setPack(Container::PACK_TRAILING);

    top_mid.add(&description);
    top_mid.setPack(Container::PACK_MIN_CENTER);
}

SinglePlayMenu::~SinglePlayMenu()
{
    vector<LevelConfig*>::iterator i;
    for (i=level_list.begin(); i<level_list.end(); i++) {
        delete *i;
    }
}

EnterScoreMenu::EnterScoreMenu(int score):
    msg(""),
    name(20),
    accept(_("Accept"))
{
    string congrats;

    this->score = score;
    sprintf(&congrats, _("Congratulations! Your score of %d has\n"
                       "made the high scores list. Please enter\n"
                       "your name below."), score);

    accept.setCallback((Clickable::Callback)acceptName, this);

    msg.setText(congrats);
    top.add(&msg);
    top.add(&name);

    bottom.add(&accept);

    add(&top);
    add(&bottom);
    this->setPack(Container::PACK_LEADING);
    top.setPack(Container::PACK_MIN_CENTER);

    setCallback((Container::Callback)focusTextField, &name);
}

EnterScoreMenu::~EnterScoreMenu()
{
}

void EnterScoreMenu::acceptName(Clickable *c, EnterScoreMenu *menu)
{
    if (strlen(menu->name.getText()) > 1) {
        fs->saveScore(menu->name.getText(), menu->score);
    }
    popMenu();
}

void EnterScoreMenu::focusTextField(Container *c, TextField *f)
{
    Menu::setKeyboardFocus(f);
}
