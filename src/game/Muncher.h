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
#ifndef MUNCHER_H
#define MUNCHER_H

#include <Player.h>
#include <Event.h>

class Muncher: public Player {
    public:
        Muncher(Animation*);
        virtual ~Muncher();

        void spawn(int time);
        void update();
        void handleKey(enum Key);
        void givePoints(int);
        int getScore() { return score; };

        virtual bool isMuncher() {return 1;}
        virtual bool isTroggle() {return 0;}

    protected:
        int score;

        /* sound info */
        Mix_Chunk *good;
        Mix_Chunk *bad;

        deque<enum Key> key_queue;
};

#endif
