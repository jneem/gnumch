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
#ifndef MULTIPLAYERCLIENT_H
#define MULTIPLAYERCLIENT_H

#include "Game.h"
#include "MultiPlayerServer.h"

class MultiPlayerClient: public Game {
    public:
        MultiPlayerClient(const GameSettings&, const Menu::VideoSettings&,
                          int numlocal);
        virtual ~MultiPlayerClient();

        virtual void    updateLives(Player*, int);
        virtual void    updateScore(Player*, int);
        virtual void    end();
        virtual void    win();
        virtual void    run(Level*, SDL_Surface*);

        /* server-querying methods */
        virtual void    playerMove( Player*, int, int, int, int, int );
        virtual void    playerStop( Player*, int, int, int );
        virtual void    playerMunch( Player*, int, int, int, bool=0 );
        virtual void    playerSpawn( Player*, int );
        virtual void    playerDie( Player*, int );

        /* the following block of functions can be called from a threaded
         * context */
        virtual void    doPlayerMove(int, int, int, int, int, int);
        virtual void    doPlayerStop(int, int, int, int);
        virtual void    doPlayerMunch(int, int, int, int);
        virtual void    doPlayerSpawn(int, int);
        virtual void    doPlayerDie(int, int);
        virtual void    doUpdateScore(int, int);
        virtual void    doNextLevel(int);
        virtual void    addServer(ServerDetails *details);

        virtual void    troggleNextSpawn( Troggle*, int );
        virtual void    setNum(int x, int y, Number *n) {board->setNum(x,y,n);}

        virtual Player *getNearestMuncher(int, int);

        /* Server handshaking functions */

        /** Add a server to the list of avilable servers. */
        void addServer( const ServerDetails& );

        /** Tell the client that a game is about to start. This doesn't take
         * care of any player/number placement; it just tells the client to
         * allocate any necessary memory, etc. */
        void startGame( void );

    protected:
        virtual void    runLevel();
        virtual void    handleEvent(const SDL_Event&);

        char *mpmuncher_file;
        int numlocal;
        int next_id;
        vector<Muncher*> munchers;
        vector<PlayerDetails*> muncher_details;
        vector<Troggle*> troggles;
        vector<Player*> remote;
        vector<PlayerDetails*> remote_details;

        vector<Player*> players;

        queue<Troggle*> trog_spawning;
        queue<int>      trog_spawning_times;
};

#endif
