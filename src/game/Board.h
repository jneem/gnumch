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
#ifndef BOARD_H
#define BOARD_H

#include <Gnumch.h>
//#include <Player.h>
//#include <Muncher.h>
//#include <Level.h>
class Number;
class Player;
class Muncher;

class Square {
    public:
        Square();
        ~Square();

        bool    filled();
        bool    good();
        void    setNum(Number*);
        Number *getNum();
        SDL_Surface *getTextPic();

    protected:
        Number *num;
};

class Board {
    public:
        Board(int, int, const vector<Player*>&);
        ~Board();

        void    setDirty(int, int);
        void    redraw();
        void    reset();
        void    unset();
        void    update();
        void    pause();
        bool    isPaused() {return paused;}
        void    resume();
        int     getWidth();
        int     getHeight();
        int     munch(int, int);
        bool    contains(int x, int y) 
                { return x>=0 && x<width && y>=0 && y<height; }
        vector<Player*> getPlayersAt(int, int);
        vector<Player*> getPlayersNear(int, int);

        // get info from a particular square
        bool    filled(int, int);
        bool    good(int, int);
        void    setNum(int, int, Number*, bool wincheck=true);
        Number *getNum(int, int);
        SDL_Surface  *getTextPic(int, int);

    protected:
        bool **dirty;
        Square ***square;
        int width, height;

        vector<Player*> players;

        /* the time at which the game was paused */
        int paused;

        /* the number of good Number*s on the board */
        int goodies;
};

#endif
