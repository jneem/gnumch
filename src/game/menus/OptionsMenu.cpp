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
#include "OptionsMenu.h"
#include "../FileSys.h"
#include "../Event.h"
#include "../Animation.h"
#include "Menus.h"

/* translate between speed (0 fastest - 7 slowest) and troggle/game speed */
#define SPD2TS(i) ((i)*300 + 100)
#define SPD2GS(i) ((i)*50  + 150)
#define TS2SPD(s) (((s)-100)/300)
#define GS2SPD(s) (((s)-150)/50)

extern Game::GameSettings game_settings;
extern Menu::VideoSettings video_settings;
extern Menu::SoundSettings sound_settings;
extern KeyBindings bindings1;
extern KeyBindings bindings2;
extern int trog_mask;
extern FileSys *fs;
extern Game *game;

static char *key_names[KEY_NUM] = {"Up", "Down", "Left", "Right", "Munch",
                                   "Respawn", "Exit"};

InputOptionsPane::InputOptionsPane():
    b1(bindings1),
    b2(bindings2),
    player1(_("Player 1")),
    player2(_("Player 2"))
{
    for (int i=0; i<KEY_NUM; i++) {
        label1[i].setText( _(key_names[i]) );
        label2[i].setText( _(key_names[i]) );
        key1[i].setText( b1.getName((enum Key)i) );
        key2[i].setText( b2.getName((enum Key)i) );
        key1[i].setCallback((Clickable::Callback)changeKey, this);
        key2[i].setCallback((Clickable::Callback)changeKey, this);

        col1.add(&label1[i], &key1[i]);
        col2.add(&label2[i], &key2[i]);
    }

    add(&list1);
    add(&list2);
    setPack(Container::PACK_MAX);
    setPadding(14);

    list1.add(&player1);
    list1.add(&col1);
    list2.add(&player2);
    list2.add(&col2);

    list1.setAlign(Container::ALIGN_JUSTIFIED);
    list2.setAlign(Container::ALIGN_JUSTIFIED);
    col1.setAlign(Container::ALIGN_JUSTIFIED);
    col2.setAlign(Container::ALIGN_JUSTIFIED);
}

InputOptionsPane::~InputOptionsPane()
{
}

void InputOptionsPane::applyChanges(InputOptionsPane *in)
{
    bindings1 = in->b1;
    bindings2 = in->b2;

    if (game)
        game->changeKeyBindings(bindings1, bindings2);
}

void InputOptionsPane::changeKey(TextButton *button, InputOptionsPane *in)
{
    int j;
    KeyBindings *b = NULL;

    /* find which index button was pushed */
    for (j=0; j<KEY_NUM; j++) {
        if (button == &in->key1[j]) {
            b = &in->b1;
            break;
        } else if (button == &in->key2[j]) {
            b = &in->b2;
            break;
        }
    }

    button->setText(_("Press a key"));

    Event event;
    enum Key k;
    int nj = SDL_NumJoysticks();
    vector<SDL_Joystick*> jstick;

    printMsg(1, "found %d joysticks\n", nj);
    if (nj) {
        for (int i=0; i<nj; i++) {
            SDL_Joystick *j = SDL_JoystickOpen(i);
            if (!j) {
                printWarning("failed to open joystick %d: %s\n", i, SDL_GetError());
            } else {
                jstick.push_back(j);
            }
        }
        SDL_JoystickEventState(SDL_ENABLE);
    }

    Menu::refresh();
    SDL_SetEventFilter(gameEventFilter);
    event.waitEvent();
    if ( (k = in->b1.getKey(event)) != KEY_NUM ) {
        in->b1.deleteEvent(event);
        in->key1[k].setText( in->b1.getName(k) );
    }
    if ( (k = in->b2.getKey(event)) != KEY_NUM ) {
        in->b2.deleteEvent(event);
        in->key2[k].setText( in->b2.getName(k) );
    }
    b->addEvent((enum Key)j, event);
    button->setText( b->getName((enum Key)j) );

    SDL_SetEventFilter(menuEventFilter);
    for (int i=0; i<(int)jstick.size(); i++) {
        SDL_JoystickClose(jstick[i]);
    }
}

TroggleOptionsPane::TroggleOptionsPane()
{
    string trog_pic;

    cur_mask = trog_mask;
    for (int i=0; i<5; i++) {
        sprintf(&trog_pic, "trogpic%d.png", i);
        trog[i].setPic(trog_pic.c_str(), -1, video_settings.h / 8);
        toggle[i].setText( (trog_mask & (1<<i)) ? _("On") : _("Off") );
        toggle[i].setCallback((Clickable::Callback)toggleButton, this);
        this->add(&trog[i], &toggle[i]);
    }
}

TroggleOptionsPane::~TroggleOptionsPane()
{
}

void TroggleOptionsPane::applyChanges(TroggleOptionsPane *trog)
{
    trog_mask = trog->cur_mask;
    /* no need to worry about in-game changes because this isn't in the
     * in-game menu */
}

void TroggleOptionsPane::toggleButton(TextButton *b, TroggleOptionsPane *trog)
{
    int i;

    for (i=0; i<5; i++) {
        if (&trog->toggle[i] == b) break;
    }
    assert(i < 5);

    if ((trog->cur_mask ^ (1<<i)) == 0) {
        return; /* don't allow them to turn off the last troggle */
    }
    trog->cur_mask ^= 1<<i;
    b->setText( (trog->cur_mask & (1<<i)) ? _("On") : _("Off") );
}

void GraphicsOptionsPane::applyChanges(GraphicsOptionsPane *gp)
{
    Menu::VideoSettings settings = {
        strcmp(gp->fullscreen_toggle.getText(), _("On")) ? 0: SDL_FULLSCREEN,
        gp->bpp_spin.getItem(),
        gp->size_spin.getItem().w,
        gp->size_spin.getItem().h
    };
    Menu::changeVideoSettings(settings);
    if(game)
        game->changeVideoSettings(settings);
    video_settings = settings;
}

void GraphicsOptionsPane::toggleFS(Clickable *button, GraphicsOptionsPane *g)
{
    string old = g->fullscreen_toggle.getText();
    string newtext;
    int graphicsflags = 0;

    if(old == _("On")) {
        newtext = _("Off");
    } else {
        newtext = _("On");
        graphicsflags = SDL_FULLSCREEN;
    }
    g->fullscreen_toggle.setText(newtext);

    vector<SDL_Rect> new_modes;
    int curmode;
    Menu::getModes(&new_modes, &curmode, g->size_spin.getItem(), graphicsflags);
    g->size_spin.reset(new_modes, curmode);
}

SoundOptionsPane::SoundOptionsPane():
    fx_spin(100),
    music_spin(100),
    freq_spin(48000),
    stereo_toggle(sound_settings.channels == 2 ? _("Stereo") : _("Mono")),
    fx(_("Sound effects volume")),
    music(_("Music volume")),
    freq(_("Sample rate")),
    stereo(_("# Channels")),
    channels(sound_settings.channels)
{
    for (int i=95; i >= 0; i -= 5) {
        fx_spin.addItem(i);
        music_spin.addItem(i);
    }
    fx_spin.setItem( 20 - max(min(sound_settings.fx_volume / 5, 20), 0) );
    music_spin.setItem( 20 - max(min(sound_settings.music_volume / 5, 20), 0) );

    freq_spin.addItem(44100);
    freq_spin.addItem(22050);
    freq_spin.setItem(freqToIndex(sound_settings.frequency));

    stereo_toggle.setCallback((Clickable::Callback)toggleStereo, this);

    add(&fx, &fx_spin);
    add(&music, &music_spin);
    add(&freq, &freq_spin);
    add(&stereo, &stereo_toggle);
    setAlign(Container::ALIGN_JUSTIFIED);
}

SoundOptionsPane::~SoundOptionsPane()
{
}

void SoundOptionsPane::toggleStereo(Clickable *b, SoundOptionsPane *p)
{
    p->channels = p->channels == 1 ? 2 : 1;
    p->stereo_toggle.setText(p->channels == 2 ? _("Stereo") : _("Mono"));
}

int SoundOptionsPane::freqToIndex(int f)
{
    if (f == 48000) return 0;
    if (f == 44100) return 1;
    return 2;
}

void SoundOptionsPane::applyChanges(SoundOptionsPane *p)
{
    sound_settings.fx_volume = p->fx_spin.getItem();
    sound_settings.music_volume = p->music_spin.getItem();
    sound_settings.frequency = p->freq_spin.getItem();
    sound_settings.channels = p->channels;

    Menu::changeSoundSettings(sound_settings);
    if (game)
        game->changeSoundSettings(sound_settings);
}

GraphicsOptionsPane::GraphicsOptionsPane():
    bpp_spin(32),
    size_spin((SDL_Rect){0, 0, 0, 0}),
    size(_("Screen resolution")),
    bpp(_("Colour depth")),
    fullscreen(_("Fullscreen")),
    fullscreen_toggle((video_settings.flags & SDL_FULLSCREEN)? _("On"):_("Off"))
{
    vector<SDL_Rect> modes;
    int curmode;
    SDL_Rect cur_res = {0, 0, video_settings.w, video_settings.h};

    Menu::getModes(&modes, &curmode, cur_res, video_settings.flags);

    size_spin.reset(modes, curmode);

    bpp_spin.addItem(24);
    bpp_spin.addItem(16);
    bpp_spin.addItem(8);
    bpp_spin.setItem((32 - video_settings.bpp)/8);

    fullscreen_toggle.setCallback((Clickable::Callback)toggleFS, this);

    add(&size, &size_spin);
    add(&bpp, &bpp_spin);
    add(&fullscreen, &fullscreen_toggle);
    setAlign(Container::ALIGN_JUSTIFIED);
}

GraphicsOptionsPane::~GraphicsOptionsPane()
{
}

GameOptionsPane::GameOptionsPane():
    trognum(_("Number of Troggles")),
    trogspeed(_("Troggle speed")),
    gamespeed(_("Game speed")),
    boardwidth(_("Board width")),
    boardheight(_("Board height")),
    trognum_spin(6),
    boardwidth_spin(10),
    boardheight_spin(10),
    trogspeed_spin(""),
    gamespeed_spin("")
{
    speeds.push_back(_("Turbo"));
    speeds.push_back(_("Faster"));
    speeds.push_back(_("Fast"));
    speeds.push_back(_("Normal"));
    speeds.push_back(_("Slow"));
    speeds.push_back(_("Slower"));
    speeds.push_back(_("Sluggish"));

    gamespeed_spin.reset(speeds, GS2SPD(game_settings.change_time));
    trogspeed_spin.reset(speeds, TS2SPD(game_settings.trog_wait));
    for(int i=5; i>=0; i--) trognum_spin.addItem(i);
    trognum_spin.setItem(6 - game_settings.trog_number);

    for(int i=9; i>=4; i--) {
        boardheight_spin.addItem(i);
        boardwidth_spin.addItem(i);
    }
    boardheight_spin.setItem(10 - game_settings.height);
    boardwidth_spin.setItem(10 - game_settings.width);

    add(&trognum, &trognum_spin);
    add(&trogspeed, &trogspeed_spin);
    add(&gamespeed, &gamespeed_spin);
    add(&boardwidth, &boardwidth_spin);
    add(&boardheight, &boardheight_spin);
    setAlign(Container::ALIGN_JUSTIFIED);
}

GameOptionsPane::~GameOptionsPane()
{
}

void GameOptionsPane::applyChanges(GameOptionsPane *gp)
{
    int movetime=0, trogtime=0;

    string trogspeed = gp->trogspeed_spin.getItem();
    string gamespeed = gp->gamespeed_spin.getItem();
    int i;
    for(i=0; i<(int)gp->speeds.size(); i++) {
        if(trogspeed == gp->speeds[i]) trogtime = SPD2TS(i);
        if(gamespeed == gp->speeds[i]) movetime = SPD2GS(i);
    }

    Game::GameSettings gsettings = {
        gp->trognum_spin.getItem(),
        trogtime,
        movetime,
        300,
        gp->boardwidth_spin.getItem(),
        gp->boardheight_spin.getItem()
    };

    if(game)
        game->changeGameSettings(gsettings);
    game_settings = gsettings;
}

OptionsMenu::OptionsMenu():
    choose_game(_("Gameplay")),
    choose_trog(_("Troggles"))
{
    choose_pane.add(&choose_game, (Clickable::Callback)showGamePane, this);
    choose_pane.add(&choose_trog, (Clickable::Callback)showTrogPane, this);

    apply.setCallback((Clickable::Callback)applyChanges,  this);
}

OptionsMenu::~OptionsMenu()
{
}

void OptionsMenu::showGamePane(Clickable *blah, OptionsMenu *m)
{
    m->top_mid.setPack(Container::PACK_MIN_CENTER);
    m->top_mid.clear();
    m->top_mid.add(&m->game_pane);
}

void OptionsMenu::showTrogPane(Clickable *blah, OptionsMenu *m)
{
    m->top_mid.setPack(Container::PACK_MIN_CENTER);
    m->top_mid.clear();
    m->top_mid.add(&m->trog_pane);
}

void OptionsMenu::applyChanges(Clickable *a, OptionsMenu *m)
{
    GameOptionsPane::applyChanges(&m->game_pane);
    TroggleOptionsPane::applyChanges(&m->trog_pane);
    InGameOptionsMenu::applyChanges(a, m);
}

InGameOptionsMenu::InGameOptionsMenu():
    choose_pane(false),
    apply(_("Apply")),
    cancel(_("Cancel")),
    choose_graphics(_("Graphics")),
    choose_sound(_("Sound")),
    choose_input(_("Input"))
{
    choose_pane.add(&choose_graphics, (Clickable::Callback)showGraphicsPane, this);
    choose_pane.add(&choose_sound, (Clickable::Callback)showSoundPane, this);
    choose_pane.add(&choose_input, (Clickable::Callback)showInputPane, this);

    apply.setCallback((Clickable::Callback)applyChanges,  this);
    cancel.setCallback(popMenu, NULL);

    this->add(&top);
    this->add(&bottom);
    top .add(&choose_pane);
    top .add(&top_mid);
    top_mid.add(&graphics_pane);

    bottom  .add(&bottom_l);
    bottom  .add(&bottom_r);
    bottom_l.add(&cancel);
    bottom_r.add(&apply);

    bottom.setPack(Container::PACK_MAX);
    bottom.setAlign(Container::ALIGN_BOTTOM);
    bottom_l.setAlign(Container::ALIGN_LEFT);
    bottom_r.setAlign(Container::ALIGN_RIGHT);

    top.setAlign(Container::ALIGN_JUSTIFIED);
    top.setPack(Container::PACK_TRAILING);

    top_mid.setAlign(Container::ALIGN_CENTER);
    top_mid.setPack(Container::PACK_MIN_CENTER);

    choose_pane.setPack(Container::PACK_MIN_CENTER);

    this->setAlign(Container::ALIGN_JUSTIFIED);
    this->setPack(Container::PACK_LEADING);
}

InGameOptionsMenu::~InGameOptionsMenu()
{
}

void InGameOptionsMenu::showGraphicsPane(Clickable *blah, InGameOptionsMenu *m)
{
    m->top_mid.setPack(Container::PACK_MIN_CENTER);
    m->top_mid.clear();
    m->top_mid.add(&m->graphics_pane);
}

void InGameOptionsMenu::showSoundPane(Clickable *blah, InGameOptionsMenu *m)
{
    m->top_mid.setPack(Container::PACK_MIN_CENTER);
    m->top_mid.clear();
    m->top_mid.add(&m->sound_pane);
}

void InGameOptionsMenu::showInputPane(Clickable *blah, InGameOptionsMenu *m)
{
    m->top_mid.setPack(Container::PACK_MAX);
    m->top_mid.clear();
    m->top_mid.add(&m->input_pane);
}

void InGameOptionsMenu::applyChanges(Clickable *a, InGameOptionsMenu *m)
{
    GraphicsOptionsPane::applyChanges(&m->graphics_pane);
    SoundOptionsPane::applyChanges(&m->sound_pane);
    InputOptionsPane::applyChanges(&m->input_pane);
    popMenu();
}

