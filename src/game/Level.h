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
#ifndef LEVEL_H
#define LEVEL_H

#include <Gnumch.h>
#include <libGui.h>

/** This class represents a number on the game board. It consists of UTF-8
 *  encoded text and a "goodness" boolean.
 */
class Number {
    public:
        /** Create a new Number.
         *  @param text The text to display on the game board.
         *  @param good If true, this number is safe to eat.
         *  @param value The integer value of this Number. Since many Numbers
         *               represent integers, this is an effective shortcut.
         */
        Number(const string& text, bool good, int value=0);
        ~Number();

        /** Render this Number's text onto an internal SDL_Surface. This must
         *  be done before the Number can be rendered on-screen */
        void render();

        /** Free the internal SDL_Surface */
        void unrender();

        /** Return the internal SDL_Surface. Used for drawing the number. */
        SDL_Surface *getPic();

        /** Query method for the goodness of this Number. */
        bool good() {return isgood;}

        /** Query method for the text of this Number. */
        const char *getText() const {return text.c_str();}

        /** Query method for the value of this Number. */
        int getValue() const {return value;}

    protected:
        string text;
        vector<string> error;    /* the list of items that gets displayed if this number is eaten erroneously */
        SDL_Surface *pic;
        bool isgood;
        int  value;
};

class ltnum {
    public:
        /** Compare two numbers by comparing their strings */
        bool operator()(Number *n1, Number *n2) const
        {
            return strcmp(n1->getText(), n2->getText()) < 0;
        }
};

typedef enum {
    ADD=0,
    SUB,
    MUL,
    DIV
} Op_t;

class Level;
class ConfigFile;

class LevelConfig {
    public:
        LevelConfig(const char *s="level");
        virtual ~LevelConfig() {}

        Clickable::Callback apply_fn;
        Clickable::Callback cancel_fn;
        int abs_maxlevel, def_maxlevel;
        int abs_minlevel, def_minlevel;
        int minlevel;
        int maxlevel;

        const char *title;
        const char *description;

        virtual Level *makeLevel() = 0;
        virtual Container *makeConfigDialog();

    protected:
        const char *section;
        ConfigFile *conf;

        bool sanityCheck();
        void readConfig();
        void writeConfig();
};

class ExpressionLevelConfig: public LevelConfig {
    public:
        ExpressionLevelConfig(const char *s="expressionLevel");

        int maxsub;
        int maxdiv;
        bool op[4];

    protected:
        bool sanityCheck();
        void readConfig();
        void writeConfig();
};

class LevelGUI: public VBox {
    public:
        // LevelConfig must be a valid address for the entire life of the LevelGUI
        LevelGUI(LevelConfig*);
        static void applyChanges(Clickable*, LevelGUI*);

    protected:
        Spinner<int> min_spin, max_spin;
        VBox top, bottom_l, bottom_r;
        HBox bottom;
        Column2 options;
        TextButton apply, cancel;
        Label min_label, max_label;

        LevelConfig *conf;
};

/** An abstract class representing the interface for a Level. */
class Level {
    public:
        Level(const LevelConfig*);
        virtual ~Level();

        /** Return a random Number belonging to this Level */
        virtual Number  *randomNumber() = 0;

        /** Force all the numbers in this level to redraw their internal
         *  pictures. This is used if, for example, the game font changes.
         */
        void reRender();

        /** Notify the level that this number is not being used any more.
         *  This is used to implement a reference-counting GC for the numbers.
         */
        void releaseNumber(Number *num);

        /** Return the title of the current level being played */
        const char *getLevelTitle() {return title.c_str();}

        /** Return a formatted error message for when a Muncher eats the
         *  wrong number.
         *  @param num the number that was erroneously eaten.
         *  @return The correct error message for the given number.
         */
        virtual const char *getError(const Number *num) = 0;

        /** Go to the next level. */
        virtual void nextLevel() = 0;

        /** Return a list of available title/description/Level* tuples */
        static void getLevelList(vector<LevelConfig*>*);

    protected:
        const LevelConfig *conf;
        int curlevel;

        /* the int in this map is the reference count */
        map<Number*, int, ltnum> numbers;
        string title;
        string error;

        string translateList(const vector<int>&);
        Number *addNumber(const char*, bool, int=0);

        // helpful math functions
        static void getFactors(vector<int> *factors, int n);
};

class ListedNumberLevel: public Level {
    public:
        ListedNumberLevel(const LevelConfig *c): Level(c) {}
        ~ListedNumberLevel() {}

        Number *randomNumber();

    protected:
        vector<int> good_num, bad_num;
};

class ExpressionLevelGUI: public LevelGUI {
    public:
        ExpressionLevelGUI(ExpressionLevelConfig*);
        static void applyChanges(Clickable*, ExpressionLevelGUI*);

    protected:
        Spinner<int> maxsub_spin, maxdiv_spin;
        TextButton tog_op[4];
        Label lab_maxsub, lab_maxdiv, lab_op[4];

        /* callback for the toggle buttons */
        static void toggleOps(TextButton*, ExpressionLevelGUI*);
};

#define EXPR_MAX_SUBTRACTOR 20
#define EXPR_MIN_SUBTRACTOR 2
#define EXPR_MAX_DIVISOR 8
#define EXPR_MIN_DIVISOR 3
class ExpressionLevel: public Level {
    public:

        ExpressionLevel(const ExpressionLevelConfig*);
        ~ExpressionLevel();

    protected:
        void getExpr(int val);
        string expr;

    private:
        typedef void (*GetExpr)(string*, int, const ExpressionLevel*);
        static void getAddExpr(string *, int val, const ExpressionLevel*);
        static void getSubExpr(string *, int val, const ExpressionLevel*);
        static void getMulExpr(string *, int val, const ExpressionLevel*);
        static void getDivExpr(string *, int val, const ExpressionLevel*);

        const static GetExpr getExprFns[];
};

class PrimeLevel;
class FactorLevel;
class MultipleLevel;
class EqualityLevel;
class InequalityLevel;

/* a useful set of callbacks for min/max spinner pairs */
void minSpinCallback(Spinner<int>* smaller, Spinner<int> *bigger);
void maxSpinCallback(Spinner<int>* bigger, Spinner<int>* smaller);

#endif
