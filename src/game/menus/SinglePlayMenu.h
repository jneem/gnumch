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
#ifndef GMENUS_PLAYMENU_H
#define GMENUS_PLAYMENU_H

#include <Level.h>

template<class T> class Spinner;
class Clickable;
class TextButton;
class TextField;
class Label;
class Container;

//void showDesc(Spinner<string>, void*);

class SinglePlayMenu: public VBox {
    public:
        SinglePlayMenu();
        virtual ~SinglePlayMenu();

    protected:
        static void showDesc(Spinner<string> *spin, void *playmenu);
        static void runGame(Clickable *blah, void *bleh);
        static void configLevel(Clickable *blah, SinglePlayMenu*);

        VBox top, top_mid, bottom_l, bottom_m, bottom_r;
        HBox bottom;
        TextButton back, play, conf;
        Spinner<string> level_spin;
        Label description;

        vector<LevelConfig*>  level_list;
        vector<string>        level_name_list;
        vector<string>        level_desc_list;
};

class EnterScoreMenu: public VBox {
    public:
        EnterScoreMenu(int score);
        virtual ~EnterScoreMenu();

    private:
        Label msg;
        TextField name;
        TextButton accept;
        VBox top, bottom;
        int score;

        static void acceptName(Clickable *c, EnterScoreMenu*);
        static void focusTextField(Container *c, TextField *f);
};
#endif
