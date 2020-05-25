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
#ifndef TROGGLE_H
#define TROGGLE_H

#include <Player.h>

class Troggle: public Player {
    typedef void (*TroggleAction)(Troggle*);

    public:
        Troggle();

        void die(int time);
        void queueSpawn();
        void spawn(int time);
        void update();

        void stopCallback() { if(onStop) (*onStop)(this); }
        void moveCallback() { if(onMove) (*onMove)(this); }

        virtual bool isMuncher() {return 0;}
        virtual bool isTroggle() {return 1;}

    protected:
        void calculateMove();
        void calculateSpawn();
        TroggleAction getMove;
        TroggleAction onMove;
        TroggleAction onStop;

    // TroggleActions need to access x and y
    friend void getMove_straight(Troggle*);
    friend void getMove_random(Troggle*);
    friend void getMove_chase(Troggle*);
    friend void getMove_run(Troggle*);
    friend void onMove_create(Troggle*);
    friend void onStop_munch(Troggle*);
};

typedef void (*TroggleAction)(Troggle*);

typedef struct {
    char *name;
    TroggleAction action;
} TrogActionDef;

class TroggleDef {
    public:
        TroggleDef(Animation*, TroggleAction, TroggleAction, TroggleAction);
        Animation *anim;
        TroggleAction getMove;
        TroggleAction onMove;
        TroggleAction onStop;
};

TroggleAction getAction(const char*);

/* TroggleAction prototypes */
void getMove_straight(Troggle*);
void getMove_random(Troggle*);
void getMove_chase(Troggle*);
void getMove_run(Troggle*);
void onMove_create(Troggle*);
void onStop_munch(Troggle*);

#endif

