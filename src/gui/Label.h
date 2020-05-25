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
#ifndef LABEL_H
#define LABEL_H

#include "Widget.h"

class Label: public Widget, public TextWidget {
    public:
        Label(const string &s="");

        virtual void    draw        (bool update=0);
        virtual void    getMinSize  (int *w, int *h);
        void            setText     (const string&);

    protected:
};

class PicLabel: public Widget {
    public:
        PicLabel(string, int w=-1, int h=-1);
        PicLabel(SDL_Surface*);
        PicLabel();
        virtual ~PicLabel();

        void draw           (bool update=0);
        void getMinSize     (int *w, int *h);
        void setPic         (SDL_Surface*);
        void setPic         (string, int w=-1, int h=-1);

    protected:
        SDL_Surface *pic;
        bool private_surface;
};

#endif
