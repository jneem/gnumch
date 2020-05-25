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

#ifndef EVENT_H
#define EVENT_H

#include <Gnumch.h>

enum Key {
    KEY_UP = 0,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_MUNCH,
    KEY_SPAWN,
    KEY_MENU,
    KEY_NUM     // the number of different Keys
};

extern SDLKey default_keys1[KEY_NUM];
extern SDLKey default_keys2[KEY_NUM];

/* a simple wrapper around SDL_Event that supports conversion to a
 * human-readable name and conversion to and from a unique key-specifying string
 * (for the config file)
 */
class Event {
    public:
        Event(const char *ev_string);
        Event(const SDL_Event&);
        Event(const Event&);
        Event(SDLKey);
        Event();
        ~Event();

        const char *toString() const;
        const char *toLongString() const;
        bool pollEvent();
        void waitEvent();
        bool empty() const;

        bool operator == (const Event&) const;

    private:
        mutable string str;
        mutable string long_str;
        SDL_Event ev_data;

        void parseStr();
        void makeStr() const;
        void makeLongStr() const;
};

class KeyBindings {
    public:
        KeyBindings(const char *section);
        KeyBindings(const KeyBindings&);
        KeyBindings();
        ~KeyBindings();

        enum Key getKey(const Event&) const;
        bool hasEvent(enum Key) const;
        const Event &getPrimaryEvent(enum Key) const;
        const Event &getSecondaryEvent(enum Key) const;
        const char *getName(enum Key) const;

        void addEvent(enum Key, const Event&);
        void deleteEvent(const Event&);
        void writeSettings() const;

    private:
        Event primary[KEY_NUM];
        Event secondary[KEY_NUM];
        string name[KEY_NUM];
        string section;

        void generateName(int i);
};

#endif
