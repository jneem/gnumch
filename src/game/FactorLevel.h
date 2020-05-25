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
#ifndef FACTORLEVEL_H
#define FACTORLEVEL_H

#include <Level.h>
#include "menus/Menus.h"

class FactorConfig: public LevelConfig {
    public:
        FactorConfig();
        virtual ~FactorConfig();

        virtual Level *makeLevel();
        virtual Container *makeConfigDialog();

        int min_num_factors; // only allow levels with at least this many factors

    protected:
        bool sanityCheck();
        void readConfig();
        void writeConfig();
};

class FactorGUI: public LevelGUI {
    public:
        FactorGUI(FactorConfig*);

        static void applyChanges(Clickable*, FactorGUI*);

    private:
        Label min_factors_label;
        Spinner<int> min_factors_spin;
};

/** A Level consisting of factors. */
class FactorLevel: public ListedNumberLevel {
    public:
        FactorLevel(const FactorConfig*);

        static const char *getTitle() {return _("Factors");}
        static const char *getDescription() {return _(
            "Munch all the factors of the number at the top of\n"
            "the screen. A factor of a number is another number\n"
            "that divides it evenly. For example, the factors of\n"
            "8 are 1, 2, 4, and 8.");}

        virtual const char *getError(const Number *num);
        virtual void nextLevel();

    protected:
        void getMultiples(vector<int>*, int);
        int numFactors(int n);
};

#endif
