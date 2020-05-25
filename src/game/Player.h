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
#ifndef PLAYER_H
#define PLAYER_H

#include <Gnumch.h>
#include <Animation.h>

class Player {
    public:
        Player();
        virtual ~Player();

        // returns 1 if the attacker wins, 0 for a tie and -1 if the defender
        // wins.
        virtual int attack(Player *p) { return p->attacked(eat_offense); }
        virtual int attacked(int attacker_offense);
        virtual void update() = 0;

        // signals sent by the Game
        virtual void move(int old_x, int old_y, int x, int y, int time);
        virtual void spawn(int time);
        virtual void munch(int time);
        virtual void die(int time);

        virtual bool isMuncher() = 0;
        virtual bool isTroggle() = 0;

        virtual void delay(int delay);
        bool isAt(int, int);
        bool isNear(int, int);
        bool isIdle();
        Point        getPos();
        SDL_Surface  *getPic(int, int, int, int, SDL_Rect*, SDL_Rect*);
        SDL_Surface  *getWholePic(int w, int h, SDL_Rect*);
        bool         exist();

    protected:
        int x, y;           // current position
        int old_x, old_y;   // old position
        bool moving;         // is it moving between squares?
        bool appearing;
        bool disappearing;
        int action_start;   // when it started its current action
        int exists;         // is it on the board?

        /* food chain status (ie. what happens when this Player and another
         * Player end up on the same square */
        int eat_offense;
        int eat_defense_live;
        int eat_defense_eatback;

        /* contains all the animation info */
        AnimationState anim;

        void updatePos();
};

#endif
