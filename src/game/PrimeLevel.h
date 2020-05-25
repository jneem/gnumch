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
#ifndef PRIMELEVEL_H
#define PRIMELEVEL_H

#include <Level.h>
#include "menus/Menus.h"

class PrimeConfig: public LevelConfig {
    public:
        PrimeConfig();
        virtual ~PrimeConfig();
        virtual Level *makeLevel();
};

/** A Level consisting of prime numbers. */
class PrimeLevel: public ListedNumberLevel {
    public:
        PrimeLevel(const PrimeConfig*);
        virtual ~PrimeLevel();

        static const char *getTitle() {return _("Prime Numbers");}
        static const char *getDescription() {return _(
            "Munch all the prime numbers on the board.\n"
            "A prime number is a number that has no factors\n"
            "except for itself and 1. Also, 1 is not prime.");}

        virtual const char *getError(const Number *num);
        virtual void nextLevel();

    protected:
        int max_listed;

        bool isPrime(int);
};

#endif
