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
#ifndef MULTIPLELEVEL_H
#define MULTIPLELEVEL_H

#include <Level.h>
#include "menus/Menus.h"

class MultipleConfig: public LevelConfig {
    public:
        MultipleConfig();
        virtual ~MultipleConfig();
        virtual Level *makeLevel();
        virtual Container *makeConfigDialog();

        int max_multiplier;

    protected:
        bool sanityCheck();
        void readConfig();
        void writeConfig();
};

class MultipleGUI: public LevelGUI {
    public:
        MultipleGUI(MultipleConfig*);
        static void applyChanges(Clickable*, MultipleGUI*);

    private:
        Spinner<int> mult_spin;
        Label mult_label;
};

class MultipleLevel: public ListedNumberLevel {
    public:
        MultipleLevel(const MultipleConfig*);

        static const char *getTitle() {return _("Multiples");}
        static const char *getDescription() {return _(
            "Munch all the multiples of the number at the top\n"
            "of the screen.\n"
            "Examples:\n"
            "The multiples of 2 include 2, 4, 6 and 8.\n"
            "The multiples of 5 include 5, 10 and 15.");}

        const char *getError(const Number *num);
        Container *getConfigDialog();
        void nextLevel();
};
#endif
