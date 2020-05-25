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
#include "Spinner.h"

/* specialisations for toString */
template<> string toString<int>(int i)
{
    return itostr(i);
}

template<> string toString<SDL_Rect>(SDL_Rect rect)
{
    char w_text[5], h_text[5];
    sprintf(w_text, "%-4d", rect.w);
    sprintf(h_text, "%-4d", rect.h);
    return (string)w_text + "x" + h_text;
}

template<> string toString<string>(string s)
{
    return s;
}

#if 0
Spinner<int>::Spinner<int>(int min, int max, int initial)
{
    cur = max - initial;
    data = NULL;
    callback = NULL;

    for (int i=max; i>=min; i++) {
        values.push_back(i);
    }
    initChildren();
}
#endif
