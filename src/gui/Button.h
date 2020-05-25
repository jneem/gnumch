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
#ifndef BUTTON_H
#define BUTTON_H

#include "Widget.h"

/** \var A typedef for a function that gets called when the a button is clicked.
 *  \see Clickable::setCallback
 */

/// Abstract parent for all clickable buttons
class Clickable: public Widget {
    public:
        typedef void (*Callback)(Clickable*, void*);

        Clickable();
        virtual void    getMinSize  (int *w, int *h)    = 0;
        virtual void draw(bool update=0) = 0;

        /// Set the click-action.

        /** When the button is clicked, \a fn will be called with arguments
         *  \a this and \a data.
         *  \param fn the function to call
         *  \param data the data to pass to the function
         */
        void            setCallback (Callback fn, void *data);

        void            handleClick(const MouseEvent_t&);
        void            handleKey(const SDL_Event*);
        void            focus        (bool=0);
        void            keyboardFocus(bool=0);
        void            unfocus      (bool=0);
        void            keyboardUnfocus(bool=0);

    protected:
        Callback        clicked;
        void            *clicked_data;
};

/// A Clickable with text.
class TextButton: public Clickable, public TextWidget {
    public:
        TextButton(const string &s="");
        virtual ~TextButton();

        void draw(bool update=0);
        void getMinSize (int *w, int *h);
        void setText(const string&);

    protected:
};

/// A Clickable with a picture.
class PicButton: public Clickable {
    public:
        PicButton(const char *unfocused, const char *focused=NULL, const char *keyfoc=NULL, const char *high=NULL);
        ~PicButton();

        void draw(bool update=0);
        void getMinSize (int *w, int *h);

    protected:
        SDL_Surface *pic, *fpic, *kpic, *hpic;
};

#endif
