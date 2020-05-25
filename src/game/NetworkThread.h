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
#ifndef NETWORKTHREAD_H
#define NETWORKTHREAD_H

#include "Gnumch.h"
#include "MultiPlayerServer.h"
#include "MultiPlayerClient.h"

#define SERVER_PORT 7529

typedef struct {
    int id;
    int channel;
    int clockskew;
    string name;
} playerInfo_t;

class NetThread {
    public:
        enum NetEvent {
            PLAYER_MOVE,
            PLAYER_STOP,
            PLAYER_EAT,
            PLAYER_DIE,
            PLAYER_SPAWN,
            PLAYER_SCORE,
            GAME_NEXTLEVEL,
            GAME_TROGWARN
        };

        enum PacketType {
            PT_PING = 0,
            PT_ACK,
            PT_REG,
            PT_EVENT,
            PT_DETAILS
        };

        NetThread();
        virtual ~NetThread();
        virtual void start();

    protected:
        UDPsocket sock;
        queue<UDPpacket*> send_queue;
        SDL_mutex *send_mutex;
        vector<UDPpacket*> sent; // sent but unAcked/unReplied packets
        int packet_id;           // the id number of the next packet to send
        SDL_mutex *packet_id_mutex;

        vector<playerInfo_t*> active_players;
        SDL_mutex *active_player_mutex;

        UDPpacket *createPingPacket();
        UDPpacket *createAckPacket(UDPpacket*);
        UDPpacket *createRegisterPacket(const string &name);
        UDPpacket *createEventPacket(enum NetEvent, ...);
        UDPpacket *createDetailsPacket();
        UDPpacket *createGenericPacket();

        virtual void handlePacket(UDPpacket*);
        virtual void handleEvent(UDPpacket*) = 0;
        virtual void handlePing(UDPpacket*) = 0;
        virtual void handleAck(UDPpacket*);
        virtual void handleRegister(UDPpacket*) = 0;
        void queueSend(UDPpacket*);

        virtual void cancelThreads();
        SDL_Thread *loop_thread;
        friend int netThreadLoop(void *net);
};

class ClientNetThread: public NetThread {
    public:
        ClientNetThread();
        virtual ~ClientNetThread();

        virtual void findServers();
        virtual void joinServer(IPaddress *a);
        virtual void cancelThreads();

    private:
        virtual void handleEvent(UDPpacket*);
        virtual void handlePing(UDPpacket*);
        virtual void handleRegister(UDPpacket*);

        friend int findServersThread(void *cNetThread);
        friend int joinServerThread(void *cNetThread);

        SDL_Thread *find_servers;
        SDL_Thread *join_servers;
        MultiPlayerClient *mpgame;
};

class ServerNetThread: public NetThread {
    public:
        ServerNetThread();
        virtual ~ServerNetThread();

    private:
        virtual void handleEvent(UDPpacket*);
        virtual void handlePing(UDPpacket*);
        virtual void handleRegister(UDPpacket*);

        MultiPlayerServer *mpgame;
};

extern SDL_mutex *game_mutex;

#define PACKET_RETRYNO(p) (p->data[0])
#define PACKET_TYPE(p) (p->data[1])
#define PACKET_ID(p) (p->data[2])
#define PACKET_TIME(p) (p->data[6])
#define PACKET_DATA(p) (&p->data[10])

#define SERVER_NAME_LEN 16
#define PLAYER_NAME_LEN 16

#endif
