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
#include <PrimeLevel.h>
#include <FileSys.h>
#include <ConfigFile.h>

#define MIN_LEVEL 3
#define MAX_LEVEL 200
#define DEF_MIN_LEVEL 3
#define DEF_MAX_LEVEL 50

#define ITEM_NO(a) ( MAX_LEVEL - (a) )

extern char *number_error_fmt[];
extern SDL_Surface *logo;
extern FileSys *fs;

PrimeLevel::PrimeLevel(const PrimeConfig *c):
    ListedNumberLevel(c)
{
    max_listed = 1;
}

PrimeLevel::~PrimeLevel()
{
}

const char *PrimeLevel::getError(const Number *num)
{
    vector<int> factors;
    getFactors(&factors, num->getValue());
    factors.erase(factors.begin());
    factors.erase(factors.end()-1);

    if (num->getValue() == 1) {
        return _("Oops! 1 is not a prime number.");
    }

    /* We only have translations for lists of <= 10 elements */
    if (factors.size() > 10) {
        factors.erase(factors.begin()+10, factors.end());
    }

    error = _("Oops! %d is not a prime number.\n");
    error += translateList(factors);
    error += ngettext(" is a factor of %d", " are factors of %d.", factors.size());

    size_t n = 0;
    while ( (n=error.find("%d", n)) != string::npos ) {
        error.replace(n, 2, num->getText(), strlen(num->getText()));
    }
    return error.c_str();
}

void PrimeLevel::nextLevel()
{
    int old = curlevel;

    /* find the smallest prime number greater than curlevel */
    for( curlevel++; !isPrime(curlevel); curlevel++ )
        ;
    if (curlevel > conf->maxlevel && conf->maxlevel > old) {
        curlevel = conf->maxlevel;
    }

    if (curlevel > conf->maxlevel) {
        curlevel = conf->minlevel;
        max_listed = 1;
        good_num.clear();
        bad_num.clear();
    }

    for ( ; max_listed<=curlevel; max_listed++ ) {
        ( isPrime(max_listed)? good_num : bad_num ).push_back(max_listed);
    }

    sprintf(&title, _("Primes less than %d"), curlevel+1);
}

bool PrimeLevel::isPrime(int n)
{
    int max = (int)sqrt((float)n) + 1;

    if (n == 1) return false;
    for (int i=2; i<max; i++) {
        if (n%i == 0) {
            return false;
        }
    }
    return true;
}

PrimeConfig::PrimeConfig():
    LevelConfig("primeLevel")
{
    title = PrimeLevel::getTitle();
    description = PrimeLevel::getDescription();
    abs_maxlevel = MAX_LEVEL;
    abs_minlevel = MIN_LEVEL;
    def_maxlevel = DEF_MAX_LEVEL;
    def_minlevel = DEF_MIN_LEVEL;

    conf = fs->openConfig("gnumch.cfg");
    LevelConfig::readConfig();
    delete conf;
    conf = NULL;
}

PrimeConfig::~PrimeConfig()
{
    conf = fs->openConfig("gnumch.cfg", true);
    LevelConfig::writeConfig();
    delete conf;
    conf = NULL;
}

Level *PrimeConfig::makeLevel()
{
    return new PrimeLevel(this);
}
