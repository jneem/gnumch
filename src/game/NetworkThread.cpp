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

/* Packet formats are as follows:
 * All packets begin with the following 10 bytes:
 * ---------------------------------------------------------------------------
 * |Retry # (8)  |Packet Type (8)| Packet ID(32)     |   Timestamp (32)      |
 * ---------------------------------------------------------------------------
 *
 * The retry number specifies how many times this packet has failed (a packet
 * fails if a response is not had withing a reasonable amount of tile). If a
 * packet fails too many times, the recipient of the failed packet will be
 * disconnected.
 *
 * The packet type specifies the format of the packet. The formats are
 * described below.
 *
 * The packet id is a unique 32-bit identifier for the packet. It is used for
 * replying to packets. (It is only unique for packets originating from any
 * given computer. That is, a server and a client can both send a packet of id
 * 763, but each server/client will only send ONE packet of id 763.
 *
 * The timestamp from a client packet is the game time of that client. The
 * timestamp from a server packet is the game time that the server thinks the
 * recipient is at. That is, the server is responsible for checking the clock
 * skew of a client and adjusting for it.
 *
 *
 *
 * Packet format 0 (used for pinging):
 * Length: 10
 * ------
 *  ... |
 * ------
 *
 * Packet format 1 (used for ack):
 * Length: 14
 * ------------------------
 *  ... |   Reply-to ID (32)  |
 * ------------------------
 *
 * Reply-to ID is the unique identifier of the packet that this packet is
 * replying to.
 *
 *
 * Packet format 2 (used for requesting/granting player IDs):
 * Length: 32
 * ---------------------------------------------------
 *  ... |   Reply-to ID (32)  |  ID (8)  | name (17) |
 * ---------------------------------------------------
 *
 * Reply-to ID is -1 for requesting a player ID. If this packet is granting
 * a player ID, reply-to ID is the same as in packet format 1.
 *
 * ID is the player ID that was granted. If this packet is requesting an ID,
 * ID is -1
 *
 * Name is the player name to associate with the granted ID. It is
 * null-terminated.
 *
 * Packet format 3 (used for NetEvents):
 * Length: >=32
 * ----------------------
 *  ... | Event Type(8) |
 * ----------------------
 *
 * The subsequent elements of this packet depend on the Event Type. They are
 * defined in NetThread.h
 *
 * Packet format 4 (request server details):
 * Length: 23 + 23*numplayers
 * -----------------------------------------------------------------------------
 *  ... | Server name(17) | # Players(1) | max players(1) |   runtime(4)   | ...
 * -----------------------------------------------------------------------------
 * ----------------------------------------------------------------------------
 *  ... | Player id(1) | Player name(17) | Player score(4) | Player Lives(1)...
 * ----------------------------------------------------------------------------
 */
#define PACKET_RETRYNO(p) (p->data[0])
#define PACKET_TYPE(p) (p->data[1])
#define PACKET_ID(p) (p->data[2])
#define PACKET_TIME(p) (p->data[6])
#define PACKET_DATA(p) (&p->data[10])

#define NETASSERT_S(n,s) \
    if(!n) { \
        printWarning("NetAssert failed: %s\n", s); \
    }

#define NETASSERT(n) NETASSERT_S(n,"Packet sanity check failed")

extern Game *game;

NetThread::NetThread()
{
    if(SDLNet_Init()==-1) {
        printError("SDLNet_Init failed: %s\n", SDLNet_GetError());
    }
    packet_id_mutex = SDL_CreateMutex();
}

NetThread::~NetThread()
{
    if(listen)
        SDLNet_UDP_Close(listen);
}

void NetThread::start()
{
    SDL_CreateThread(netThreadLoop, this);
}

ClientNetThread::ClientNetThread()
{
    listen = SDLNet_UDP_Open(0);
}

ClientNetThread::~ClientNetThread()
{
}

ServerNetThread::ServerNetThread()
{
    listen = SDLNet_UDP_Open(SERVER_PORT);
}

ServerNetThread::~ServerNetThread()
{
}

int netThreadLoop(void *net)
{
    NetThread *n = (NetThread*)net;
    UDPpacket *p = n->createGenericPacket();

    while( SDLNet_UDP_Recv(n->listen, p) != -1 ) {
        n->handlePacket(p);

        SDL_mutexP(n->send_mutex);

        while( !n->send_queue.empty() ) {
            UDPpacket *tmp = n->send_queue.front();

            if(tmp->channel == -1) {
                for(int i=0; i<(int)n->active_players.size(); i++) {
                    SDLNet_UDP_Send(n->listen, n->active_players[i]->channel,
                                                                        tmp);
                }
            } else {
                SDLNet_UDP_Send(n->listen, tmp->channel, tmp);
            }

            SDLNet_FreePacket(tmp);
            n->send_queue.pop();
        };

        SDL_mutexV(n->send_mutex);

        SDL_Delay(5);
    }

    printError("SDLNet_UDP_Recv returned error: %s\n", SDLNet_GetError());
    return 0;
}

UDPpacket *NetThread::createGenericPacket()
{
    UDPpacket *ret = SDLNet_AllocPacket(256);
    ret->channel = -1;
    ret->len = 10;
    PACKET_RETRYNO(ret) = 0;
    PACKET_TYPE(ret) = 0;

    SDL_mutexP(packet_id_mutex);
    SDLNet_Write32( packet_id++, &ret->data[2] );
    SDL_mutexV(packet_id_mutex);

    SDLNet_Write32( SDL_GetTicks(), &ret->data[6] );

    return ret;
}

UDPpacket *NetThread::createPingPacket()
{
    return createGenericPacket();
}

UDPpacket *NetThread::createAckPacket(UDPpacket *orig)
{
    UDPpacket *ret = createGenericPacket();
    ret->len = 14;
    PACKET_TYPE(ret) = 4;
    SDLNet_Write32( PACKET_ID(orig), PACKET_DATA(ret) );
    return ret;
}

UDPpacket *NetThread::createRegisterPacket( const string &name )
{
    UDPpacket *ret = createGenericPacket();
    ret->len = 178;
    PACKET_TYPE(ret) = 5;
    SDLNet_Write32(0, PACKET_DATA(ret));
    PACKET_DATA(ret)[4] = (char)-1;
    strncpy( (char*)&PACKET_DATA(ret)[5], name.c_str(), 16 );
    PACKET_DATA(ret)[21] = '\0';
    return ret;
}

UDPpacket *NetThread::createEventPacket(enum NetEvent e, ...)
{
    UDPpacket *ret = createGenericPacket();
    ret->len = 11;
    PACKET_TYPE(ret) = 1;
    PACKET_DATA(ret)[0] = e;
    return ret;
}

UDPpacket *NetThread::createDetailsPacket()
{
    UDPpacket *ret = createGenericPacket();
    ret->len = 10;
    PACKET_TYPE(ret) = PT_DETAILS;
    return ret;
}

void NetThread::queueSend(UDPpacket *p)
{
    SDL_mutexP(send_mutex);
    send_queue.push(p);
    SDL_mutexV(send_mutex);
}

void NetThread::handlePacket(UDPpacket *p)
{
    switch( PACKET_TYPE(p) ) {
        case PT_PING:
            handlePing(p);
            break;
        case PT_EVENT0:
        case PT_EVENT3:
        case PT_EVENT5:
            handleEvent(p);
        case PT_REG:
            handleRegister(p);
        case PT_ACK:
            handleAck(p);
    }
}

void NetThread::handleAck(UDPpacket *p)
{
    vector<UDPpacket*>::iterator i;
    int id1, id2;

    id1 = SDLNet_Read32( &PACKET_ID(p) );
    for( i=sent.begin(); i<sent.end(); i++) {
        id2 = SDLNet_Read32( &PACKET_ID(p) );
        if( id1 == id2 ) {
            sent.erase(i);
            break;
        }
    }
}


