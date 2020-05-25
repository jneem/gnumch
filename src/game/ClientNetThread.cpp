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
#include "MultiPlayerServer.h"
#include "MultiPlayerClient.h"

void ClientNetThread::handleEvent(UDPpacket *p)
{
    int ts = SDLNet_Read32( &PACKET_TIME(p) );
    switch( PACKET_DATA(p)[0] ) {
        case PLAYER_MOVE:
            NETASSERT( PACKET_TYPE(p) == PT_EVENT5 );
            mpgame->doPlayerMove( PACKET_DATA(p)[1],
                                  PACKET_DATA(p)[2],
                                  PACKET_DATA(p)[3],
                                  PACKET_DATA(p)[4],
                                  PACKET_DATA(p)[5], ts);
            break;
        case PLAYER_STOP:
            NETASSERT( PACKET_TYPE(p) == PT_EVENT3 );
            mpgame->doPlayerStop( PACKET_DATA(p)[1],
                                  PACKET_DATA(p)[2],
                                  PACKET_DATA(p)[3], ts);
            break;
        case PLAYER_EAT:
            NETASSERT( PACKET_TYPE(p) == PT_EVENT3 );
            mpgame->doPlayerMunch( PACKET_DATA(p)[1],
                                   PACKET_DATA(p)[2],
                                   PACKET_DATA(p)[3], ts);
            break;
        case PLAYER_DIE:
            NETASSERT( PACKET_TYPE(p) == PT_EVENT0 );
            mpgame->doPlayerDie( PACKET_DATA(p)[1], ts );
            break;
        case PLAYER_SPAWN:
            NETASSERT( PACKET_TYPE(p) == PT_EVENT0 );
            mpgame->doPlayerSpawn( PACKET_DATA(p)[1], ts );
            break;
        case PLAYER_SCORE:
            NETASSERT( PACKET_TYPE(p) == PT_EVENT4 );
            mpgame->doUpdateScore( SDLNet_Read32(PACKET_DATA(p)+1), ts );
            break;
        case GAME_NEXTLEVEL:
            NETASSERT( PACKET_TYPE(p) == PT_EVENT0 );
            mpgame->doNextLevel( ts );
            break;
        case GAME_TROGWARN:
            NETASSERT( PACKET_TYPE(p) == PT_EVENT0 );
            mpgame->showTrogWarning();
        default:
            printWarning("Unknown NetEvent\n");
    }
}

void ClientNetThread::handlePing(UDPpacket *p)
{
    queueSend( createAckPacket(p) );
}

void ClientNetThread::handleRegister(UDPpacket *p)
{
    printWarning("warning: client got unexpected register packet\n");
}

void ClientNetThread::findServers()
{
    find_servers = SDL_CreateThread(findServersThread, this);
}

void ClientNetThread::joinServer(IPaddress *a)
{
    if( SDLNet_UDP_Bind(listen, 1, a) != 1 ) {
        printWarning("Couldn't bind server address to socket\n");
    } else {
        join_servers = SDL_CreateThread(joinServerThread, this);
    }
}

int findServersThread(void *c)
{
    ClientNetThread *cnet = (ClientNetThread*)c;
    UDPpacket *sendpacket = cnet->createDetailsPacket();
    UDPpacket *recvpacket = cnet->createGenericPacket();

    /* wait 5 seconds in 1/4 second blocks */
    for( int i=0; i<20; i++ ) {
        SDLNet_UDP_Send(cnet->listen, -1, sendpacket);
        SDL_Delay(250);

        while( SDLNet_UDP_Recv( cnet->listen, recvpacket ) == 1 ) {
            if( PACKET_TYPE(recvpacket) != PT_DETAILS ) {
                printMsg(0, "unexpected packet type\n"); 
                continue;
            }

            mpgame->addServer(new ServerDetails(recvpacket));
        }
    }
    return 0;
}

int joinServerThread(void *c)
{
    return 0;
}
