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
#ifndef GMENUS_INGAMEMENU_H
#define GMENUS_INGAMEMENU_H

#include <libGui.h>
#include "Menus.h"

class InGameMenu: public VBox {
    public:
        InGameMenu();
        virtual ~InGameMenu();

    private:
        HBox hcentred;
        VBox centred;
        TextButton resume, options, credits, scores, stop, quit;
};
#endif
