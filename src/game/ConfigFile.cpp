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
#include "ConfigFile.h"

const static char white[] = "\t\v\f\n ";

ConfigFile::ConfigFile(const string &name, bool write)
{
    this->write = write;
    this->name = name;

    file.open(name.c_str(), ios::in);

    int line = 0;
    map <string, string, ltstr>* curtable = NULL;
    while (file.peek() != EOF) {
        readLine();
        line++;

        size_t first_non_white = curline.find_first_not_of(white);
        if (first_non_white == string::npos
                || curline[first_non_white] == '#') {
            continue;
        }
        curline.erase(0, first_non_white);
        if (curline[0] == '[') {
            size_t secname_start, secname_end;

            secname_start = curline.find_first_not_of(white, 1);
            secname_end = curline.find(']', 1);

            if (secname_start >= secname_end) {
                printError(
                        "syntax error in line %d of %s: invalid section name\n",
                        line, name.c_str());
            }
            // make sure there are no trailing characters
            if (curline.find_first_not_of(white, secname_end+1)!=string::npos) {
                printError(
                        "syntax error in line %d of %s: trailing characters\n",
                        line, name.c_str());
            }
            secname_end = curline.find_last_not_of(white, secname_end-1);
            assert(secname_end >= secname_start);

            curline.erase(secname_end+1, curline.size()-secname_end-1);
            curline.erase(0, secname_start);

            curtable = new map<string, string, ltstr>();
            section_map[curline] = curtable;
        } else {
            if (!curtable) {
                printError(
                        "syntax error in line %d of %s: assignment must be "
                        "within a section.\n", line, name.c_str());
            }
            // parse a name/value pair
            size_t name_end, val_start, val_end, eq_pos;

            eq_pos = curline.find('=');
            if (eq_pos == string::npos) {
                printError(
                        "syntax error in line %d of %s: missing '='.\n",
                        line, name.c_str());
            }
            name_end = curline.find_last_not_of(white, eq_pos-1);
            val_start = curline.find_first_not_of(white, eq_pos+1);
            val_end = curline.find_last_not_of(white);

            if (name_end == string::npos || val_start == string::npos) {
                printError(
                        "syntax error in line %d of %s: expected name/value "
                        "pair.", line, name.c_str());
            }
            string name(curline, 0, name_end+1);
            string val (curline, val_start, val_end-val_start+1);
            (*curtable)[name] = val;
        }
    }
    file.close();
}

ConfigFile::~ConfigFile()
{
    if (write) flush();

    /* free all the section/key/value strings */
    map<string, map<string, string, ltstr>*, ltstr>::iterator i;
    map<string, string, ltstr>::iterator j;
    for (i=section_map.begin(); i!=section_map.end(); i++) {
        delete i->second;
    }
}

int ConfigFile::readInt(const char *section, const char *key, int def)
{
    const char *val = lookup(section, key);
    if (!val) return def;
    return atoi(val);
}

const char *ConfigFile::readString(const char *section, const char *key,
                                   const char *def)
{
    const char *val = lookup(section, key);
    if (!val) return def;
    return val;
}

void ConfigFile::writeInt(const char *section, const char *k, int v)
{
    map< string, map<string, string, ltstr>*, ltstr >::iterator i;

    string sec(section);
    i = section_map.find(sec);
    if (i == section_map.end()) {
        section_map[sec] = new map< string, string, ltstr >();
        i = section_map.find(sec);
    }
    (*i->second)[string(k)] = itostr(v);
}

void ConfigFile::writeString(const char *section, const char *k, const char *v)
{
    map< string, map<string, string, ltstr>*, ltstr >::iterator i;

    string sec(section);
    i = section_map.find(sec);
    if (i == section_map.end()) {
        section_map[sec] = new map< string, string, ltstr >();
        i = section_map.find(sec);
    }
    (*i->second)[string(k)] = string(v);
}

void ConfigFile::flush()
{
    assert(write);

    file.open(name.c_str(), ios::out | ios::trunc);
    if (file.bad() || file.fail()) {
        /*
        printWarning("couldn't open file %s for writing.\n", name.c_str());
        file.close();
        return;
        */
    }
    file.clear();

    map< string, map<string, string, ltstr>*, ltstr >::iterator i;
    map< string, string, ltstr >::iterator j;
    file <<
"# Warning: this configuration file is automatically generated. Any changes you\n"
"# make to it might not be preserved. If editing this file causes gnumch to\n"
"# abort the next time you run it, delete this file to start again.\n";

    for (i = section_map.begin(); i != section_map.end(); i++) {
        file << endl;
        file << "[" << i->first << "]" << endl;
        for (j = i->second->begin(); j != i->second->end(); j++) {
            file << j->first << " = " << j->second << endl;
        }
    }
    file.close();
}

const char *ConfigFile::lookup(const char *section, const char *key)
{
    map<string, map<string, string, ltstr>*, ltstr>::iterator table;
    map<string, string, ltstr>::iterator value;

    table = section_map.find(section);
    if (table == section_map.end()) {
        return NULL;
    }
    value = table->second->find(key);
    if (value == table->second->end()) {
        return NULL;
    }
    return value->second.c_str();
}

void ConfigFile::readLine()
{
    char tmp[40];
    int c;

    curline.clear();

    while ( (c=file.peek()) != EOF && c != '\n' ) {
        file.get(tmp, 40, '\n');
        curline += tmp;
    }
    file.ignore(1); // remove the \n
}

#if 0
void printError(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, _("Fatal error: "));
    vfprintf(stderr, _(fmt), ap);
    va_end(ap);
    fprintf(stderr, _("Gnumch has encountered a fatal error and needs to close. If you don't know what caused this error, please submit a bug report to spuzzzzzzz@gmail.com\n"));
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printError("need 2 args\n");
    }
    ConfigFile *f = new ConfigFile(argv[1]);
    map< string, map<string, string, ltstr>*, ltstr >::iterator i;
    map< string, string, ltstr >::iterator j;
    for (i = f->section_map.begin(); i != f->section_map.end(); i++) {
        printf("Section %s\n", i->first.c_str());
        for (j = i->second->begin(); j != i->second->end(); j++) {
            printf("\t%s = %s\n", j->first.c_str(), j->second.c_str());
        }
    }
    return 0;
}
#endif
