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
#ifndef CONTAINER_H
#define CONTAINER_H

#include "Widget.h"
#include "Label.h"
#include "Button.h"

/// An abstract container class.

/** The children of this class are more interesting.
 */
class Container: public Widget {
    public:
        typedef void (*Callback)(Container*, void*);
        typedef enum {
            ALIGN_LEFT,
            ALIGN_CENTER,
            ALIGN_RIGHT,
            ALIGN_TOP,
            ALIGN_BOTTOM,
            ALIGN_JUSTIFIED
        } Align;

        typedef enum {
            PACK_MIN,
            PACK_MIN_CENTER,
            PACK_EQUAL,
            PACK_MAX,
            PACK_LEADING,   /* make all but the first element their minimum
                             * size. The first element is then given the rest
                             * of the space. */
            PACK_TRAILING   /* s/first/last/g */
        } PackMethod;

        Container();
        virtual ~Container();

        void setCallback(Callback, void *data);
        void callCallback();

        virtual void    add(Widget*);
        virtual void    remove(Widget*);
        virtual void    insert(Widget*, Widget*);
        virtual void    clear();
        virtual void    setAlign(Align);
        virtual void    setPack(PackMethod);
        virtual void    setPadding(int);
        virtual void    setBackground(SDL_Surface *bg);

        virtual void    getMinSize  (int *w, int *h)    = 0;

        virtual Widget* handleFocus (int, int);
        virtual Widget* handleKeyFocus(int direction, Widget *from);
        virtual void    handleClick (const MouseEvent_t &);

        virtual void    unfocus(bool=0);
        virtual void    keyboardUnfocus(bool=0);
        void            setWidth(int);
        void            setHeight(int);

        /** draw the portion of the background of this container that
         * covers the given rectangle.
         */
        virtual void    drawBackground(const Rectangle&);
        virtual void    setAllChanged(bool);

        /* return true if at least one child allows keyboard focus */
        virtual bool    getAllowKeyFocus();

    protected:
        vector<Widget*> children;
        SDL_Surface *background;
        bool need_bg_redraw;

        Align align;
        PackMethod pack;
        int padding;

        Callback callback;
        void *data;

        int packMax(int total, int *size, int num);
        int childPos(int pos, int dim, int child_dim);

        vector<Widget*>::iterator
                nextKeyFocChild(vector<Widget*>::iterator start, int inc);

        /* packing utility functions */
        void makeChildrenRects(vector<Rectangle> &dest, bool vertical);
        void packChildren(vector<Rectangle> &dest, bool vertical);

        static void packChildrenMin(vector<Rectangle> &dest, const Rectangle &self, int padding);
        static void packChildrenMinCenter(vector<Rectangle> &dest, const Rectangle &self, int padding);
        static void packChildrenEqual(vector<Rectangle> &dest, const Rectangle &self, int padding);
        static void packChildrenMax(vector<Rectangle> &dest, const Rectangle &self, int padding);
        static void packChildrenLeading(vector<Rectangle> &dest, const Rectangle &self, int padding);
        static void packChildrenTrailing(vector<Rectangle> &dest, const Rectangle &self, int padding);

        void alignChildren(vector<Rectangle> &dest, bool vertical);
        static void alignChild(int x, int w, int *x_aligned, int *w_aligned, Align align);

        void sizeChildren(const vector<Rectangle> &dest, Rectangle &result, bool vertical);

        static void sizeChildrenMin(const vector<Rectangle> &dest, Rectangle &result, int padding);
        static void sizeChildrenEqual(const vector<Rectangle> &dest, Rectangle &result, int padding);

        static void swapOrientation(vector<Rectangle> &dest);

        bool commonDraw();
        void simpleDraw(bool vertical, bool update);
};

/// A vertically stacked container class.

/** Widgets packed into a VBox are stacked vertically, with the first item on
 *  top.
 */

class VBox: public Container {
    public:
        void draw(bool update=0);
        void getMinSize (int *w, int *h);

        friend class Spinner<int>;
};

/// A horizontally stacked container class.

/** Widgets packed into an HBox are stacked horizontally, with the first item
 *  on the left.
 */
class HBox: public Container {
    public:
        void draw(bool update=0);
        void getMinSize (int *w, int *h);
};

/// A vertically stacked container class with two equal-width columns.

/** Two widgets in the same position but different columns will have the same
 *  height and y-position.
 */
class Column2: public Container {
    public:
        void add(Widget*, Widget*);

        void draw(bool update=0);
        void getMinSize(int *w, int *h);
};

/// Pack Buttons into radio groups.

/** This widget forces one (and only one) button in the group to be focused
 *  at any time.
 */
class ButtonGroup: public Container {
    public:
        /// Construct a new ButtonGroup.

        /** \param vertical If true, pack the buttons vertically. Otherwise,
         *  pack them horizontally.
         */
        ButtonGroup(bool vertical);
        virtual ~ButtonGroup();

        virtual void add(Widget*);  /* this function causes an error message */
        virtual void add(Clickable*, Clickable::Callback call, void *data);

        virtual void setAlign(Align a)      { box->setAlign(a); }
        virtual void setPack(PackMethod p)  { box->setPack(p); }
        virtual void setPadding(int p)      { box->setPadding(p); }

        virtual void setX(int x)           { pos.x = x; box->setX(x); }
        virtual void setY(int y)           { pos.y = y; box->setY(y); }
        virtual void setWidth(int);
        virtual void setHeight(int);

        virtual void draw(bool update=0);
        virtual void getMinSize(int*, int*);

    protected:
        Widget *foc;
        Widget *selected;
        Container *box;
        vector<Clickable*> buttons;
        vector<Clickable::Callback> callbacks;
        vector<void*> data;

        static void buttonCallback(Clickable*, ButtonGroup*);
};

#endif
