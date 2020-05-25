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
#ifndef EQUALITYLEVEL_H
#define EQUALITYLEVEL_H

#include "Level.h"

class EqualityConfig: public ExpressionLevelConfig {
    public:
        EqualityConfig(const char *s="equalityLevel");
        virtual ~EqualityConfig();
        virtual Level *makeLevel();
        virtual Container *makeConfigDialog();

        int max_error;

    protected:
        bool sanityCheck();
        void readConfig();
        void writeConfig();
};

class InequalityConfig: public EqualityConfig {
    public:
        InequalityConfig();
        virtual Level *makeLevel();
};

class EqualityGUI: public ExpressionLevelGUI {
    public:
        EqualityGUI(EqualityConfig*);
        static void applyChanges(Clickable*, EqualityGUI*);

    protected:
        Label lab_error;
        Spinner<int> spin_error;
};

class EqualityLevel: public ExpressionLevel {
    public:
        EqualityLevel(const EqualityConfig*);

        Number *randomNumber();
        static const char *getTitle() {return _("Equality");}
        static const char *getDescription() {return _(
            "Munch all the expressions that are equal to the\n"
            "number at the top of the screen.");}
        const char *getError(const Number*);
        Container *getConfigDialog();
        void nextLevel();
};

class InequalityLevel: public ExpressionLevel {
    public:
        InequalityLevel(const EqualityConfig*);

        Number *randomNumber();
        static const char *getTitle() {return _("Inequality");}
        static const char *getDescription() {return _(
            "Munch all the expressions that are not equal to\n"
            "the number at the top of the screen.");}
        const char *getError(const Number*);
        Container *getConfigDialog();
        void nextLevel();
};

#endif
