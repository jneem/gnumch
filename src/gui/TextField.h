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
#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include "Label.h"

class TextField: public Label {
    public:
        TextField(int);
        virtual ~TextField();

        void handleClick(const SDL_Event*);
        bool handleRawKey(const SDL_Event*);
        virtual void getMinSize(int *w, int *h);
        virtual void draw(bool update=0);

    protected:
        int cursor;
        int maxsize;

        void getCursorPos(Rectangle*);
};

#endif
