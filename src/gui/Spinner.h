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
#ifndef SPINNER_H
#define SPINNER_H

#include "Menu.h"
#include "Container.h"
#include "Label.h"
#include "Button.h"

template<class T> class Spinner;

template<class T> void spinnerUp(Clickable*, void*);
template<class T> void spinnerDown(Clickable*, void*);
template<class T> string toString(T);

/// A spinner widget
template <class T>
class Spinner: public HBox {
    public:
        typedef void (*Callback)(Spinner*, void*);

        Spinner(T initial, bool wrap_ = false):
            up("up_arrow.png", "up_arrow_foc.png"),
            down("down_arrow.png", "down_arrow_foc.png")
        {
            cur = sticky = 0;
            data = NULL;
            callback = NULL;
            wrap = wrap_;
	    label_min_width = 0;
	    label_min_height = 0;

	    addItem (initial);

            initChildren();
        }

        Spinner(const vector<T> &copy, int cur_, bool wrap_ = false):
            up("up_arrow.png", "up_arrow_foc.png"),
            down("down_arrow.png", "down_arrow_foc.png")
        {
            cur = sticky = cur_;
            data = NULL;
            callback = NULL;
            wrap = wrap_;
	    label_min_width = 0;
	    label_min_height = 0;

	    for (int i = 0; i < (int)copy.size (); i++)
	      addItem (copy[i]);

            initChildren();
        }

        ~Spinner()
        {
        }

        /**
         * Reset this Spinner's data vector to be \p copy and its current
         * element to be \cur.
         */
        void reset(const vector<T> &copy, int cur)
        {
	    label_min_width = 0;
	    label_min_height = 0;
	    values.clear ();
	    for (int i = 0; i < (int)copy.size (); i++)
	      addItem (copy[i]);
            setItem(cur);
        }

        void draw(bool update=0)
        {
            if (!changed) return;

            /* force the label to fill all the size that isn't taken up by
             * the buttons */
            int w_buttons, h_buttons, w_label, h_label;
            label.getMinSize(&w_label, &h_label);
            buttons.getMinSize(&w_buttons, &h_buttons);

            label.setWidth(pos.w - w_buttons);
            label.setX(pos.x);
            label.setY(pos.y + (pos.h-h_label)/2);

            buttons.setX(pos.x + pos.w - w_buttons);
            buttons.setY(pos.y + (pos.h-h_buttons)/2);

            label.cleanDraw();
            buttons.cleanDraw();
            label.draw(update);
            buttons.draw(update);

            setChanged(0);
            drawn_pos = pos;
        }

        void setItem(int i)
        {
            if (i>=0 && i<(int)values.size()) {
                cur = sticky = i;
            }
            saveMinSize();
            label.setText( toString(values[cur]) );
            setChanged(1);
            resize();
        }

        void addItem(T item)
        {
	    string text = label.getText ();

            values.push_back(item);

	    int w, h;
	    label.setText( toString(item) );
	    label.getMinSize (&w, &h);
	    label_min_width = max( label_min_width, w );
	    label_min_height = max (label_min_height, h );
	    label.setText (text);
        }

        T getItem()
        {
            return values[cur];
        }

        int  getIndex()
        {
            return cur;
        }

        void setIndex(int i) {
            if (i >= 0 && i < (int)values.size()) {
                saveMinSize();
                cur = i;
                label.setText(toString(values[cur]));
                if(callback) (*callback)(this, data);
                resize();
                setChanged(1);
                setAllChanged(1);
            }
        }

        void clearItems()
        {
            values.clear();
            cur = 0;
        }

        void save()
        {
            sticky = cur;
        }

        void revert()
        {
            cur = sticky;
            saveMinSize();
            label.setText(toString(values[cur]));
            setChanged(1);
            resize();
        }

        void setCallback(Callback fn, void *fn_data)
        {
            callback = fn;
            data = fn_data;
        }

	void getMinSize(int *w, int *h)
	{
            int pics_w, pics_h;
	    buttons.getMinSize (&pics_w, &pics_h);
	    *w = pics_w + label_min_width;
	    *h = max (pics_h, label_min_height);
	}

    protected:
        vector<T> values;
	int label_min_width;
	int label_min_height;
        int cur, sticky;
        Label label;
        PicButton up, down;
        VBox buttons;
        bool wrap;  // whether to wrap the top to the bottom

        /* call this function when the spinner changes value */
        Callback callback;
        void *data;

        void initChildren()
        {
            up.setRepeating(true);
            down.setRepeating(true);

            up.setCallback(spinnerUp<T>, this);
            down.setCallback(spinnerDown<T>, this);

            label.setText(toString(values[cur]));
            buttons.add(&up);
            buttons.add(&down);

            add(&label);
            add(&buttons);
        }

        friend string toString<T> (T);
        friend void spinnerUp<T>   (Clickable*, void*);
        friend void spinnerDown<T> (Clickable*, void*);
};

template<class T> string toString(T unknown)
{
    return "unable to convert to string";
}

template<class T> void spinnerUp(Clickable *ignored, void *spinner)
{
    Spinner<T> *spin = (Spinner<T>*)spinner;

    if(spin->cur > 0) {
        spin->setIndex(spin->cur - 1);
    } else if (spin->wrap) {
        spin->setIndex(spin->values.size() - 1);
    }
}

template<class T> void spinnerDown(Clickable *ignored, void *spinner)
{
    Spinner<T> *spin = (Spinner<T>*)spinner;

    if(spin->cur + 1 < (int)spin->values.size()) {
        spin->setIndex(spin->cur + 1);
    } else if (spin->wrap) {
        spin->setIndex(0);
    }
}

/* These specialisations of toString() are defined in Spinner.cpp */
template<> string toString<int>(int);
template<> string toString<SDL_Rect>(SDL_Rect);
template<> string toString<string>(string);
#endif
