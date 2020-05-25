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
#include "NetworkThread.h"

void ServerNetThread::handleEvent(UDPpacket *p)
{
}

void ServerNetThread::handlePing(UDPpacket *p)
{
    queueSend( createAckPacket(p) );
}

void ServerNetThread::handleRegister(UDPpacket *p)
{
    const char *name = (char*)&PACKET_DATA(p)[5];
    int id;

    if( (id = mpgame->addPlayer(name)) != -1 ) {
        if( SDLNet_UDP_Bind(listen, id, &p->address) != -1 ) {
            playerInfo_t *pi = (playerInfo_t*)malloc(sizeof(playerInfo_t));
            pi->id = id;
            pi->channel = id;
            pi->clockskew = SDL_GetTicks() - SDLNet_Read32( &PACKET_TIME(p) );
            pi->name.assign( name, 16 );

            SDL_mutexP(active_player_mutex);
            active_players.push_back(pi);
            SDL_mutexV(active_player_mutex);

            printMsg(0, "Server registered client %s\n", pi->name.c_str() );
        } else {
            printMsg(0, "Server failed to bind address: %s\n",
                     SDLNet_GetError());
        }
    } else {
        printMsg(0, "Server is full!\n");
    }
}

