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
#ifndef MULTIGAME_H
#define MULTIGAME_H

#include <Gnumch.h>
#include <Board.h>
#include <Troggle.h>
#include <Level.h>
#include <Menu.h>
#include <Game.h>

class MultiPlayerGame: public TroggleGame {
    public:
        MultiPlayerGame(const GameSettings&, const Menu::VideoSettings&,
                        const KeyBindings&, const KeyBindings&, int target);
        virtual ~MultiPlayerGame();

        // retrieve and set various settings
        virtual void    updateLives(Player*, int);
        virtual void    updateScore(Player*, int);
        virtual void    end();
        virtual void    win();
        virtual void    run(Level*, SDL_Surface*);

        virtual void    showMessage(const char*, Uint8=255, Uint8=255, Uint8=255, Uint8=128);

        virtual void    playerSpawn( Player*, int );

        virtual void    redrawAll();
        virtual Player  *getNearestMuncher(int, int);

    protected:
        virtual void    runLevel();
        virtual void    handleEvent(const Event&);

        Muncher *muncher[2];
        int score[2];

        int message_display_time;

        virtual void handleMuncherEaten(Muncher *m, Player *eater);
        virtual void handleMuncherIndigestion(Muncher *m);

        int target_score;
};

#endif
