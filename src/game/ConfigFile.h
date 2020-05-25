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
#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <Gnumch.h>

class ConfigFile {
    public:
        ConfigFile(const string&, bool write=0);
        ~ConfigFile();

        int readInt(const char *section, const char *key, int def);
        const char* readString(const char *section, const char *key,
                               const char *def);

        void writeInt(const char *section, const char *key, int val);
        void writeString(const char *section, const char *key, const char *val);

        void flush();

    protected:
        fstream file;
        string name;
        bool write;

        string curline;
        void readLine();

        map< string, map<string, string, ltstr>*, ltstr > section_map;
        const char *lookup(const char *section, const char *key);

        friend int main(int argc, char **argv);
};

#endif
