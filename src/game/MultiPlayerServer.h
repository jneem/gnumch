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
#ifndef MPSERVER_H
#define MPSERVER_H

#include "Gnumch.h"
#include "Board.h"
#include "Troggle.h"
#include "Level.h"
#include "Menu.h"
#include "Game.h"

#define SERVER_PORT 7529

typedef struct {
    int id;             /* unique ID number */
    int lives;
    int score;
    string name;
} PlayerDetails;

class ServerDetails {
    public:
        ServerDetails(const UDPpacket *p);
        ~ServerDetails();

        string name;
        IPaddress address;
        int numplayers;
        int maxplayers;
        int ping;
        int gametime; /* 0 means the game isn't running */
        vector<PlayerDetails*> player_details;
};

class MultiPlayerServer: public Game {
    public:
        MultiPlayerServer(const GameSettings&, const Menu::VideoSettings&,
                          int numlocal);
        virtual ~MultiPlayerServer();

        virtual void    updateLives(Player*, int);
        virtual void    updateScore(Player*, int);
        virtual void    end();
        virtual void    win();
        virtual void    run(Level*, SDL_Surface*);

        virtual void    playerMove( Player*, int, int, int, int, int );
        virtual void    playerStop( Player*, int, int, int );
        virtual void    playerMunch( Player*, int, int, int, bool=0 );
        virtual void    playerSpawn( Player*, int );
        virtual void    playerDie( Player*, int );
        virtual void    troggleNextSpawn( Troggle*, int );
        virtual void    setNum(int x, int y, Number *n) {board->setNum(x,y,n);}

        virtual Player *getNearestMuncher(int, int);

        /** Try to add a new player to the server.
         *   @param name The name of the player to add.
         *   @return the new player id, or -1 if the server is full. */
        virtual int     addPlayer(const string &name);

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
