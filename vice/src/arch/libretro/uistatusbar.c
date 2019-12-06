/*
 * uistatusbar.c - SDL statusbar.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Andreas Boose <viceteam@t-online.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>

#include "resources.h"
#include "types.h"
#include "ui.h"
#include "uiapi.h"
#include "uistatusbar.h"
#include "videoarch.h"
#include "keyboard.h"

#include "libretro.h"
#include "libretro-core.h"

extern retro_environment_t environ_cb;


/* ----------------------------------------------------------------- */
/* static functions/variables */

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define MAX_STATUSBAR_LEN           128
#define STATUSBAR_SPEED_POS         0
#define STATUSBAR_PAUSE_POS         4
#define STATUSBAR_DRIVE_POS         12
#define STATUSBAR_DRIVE8_TRACK_POS  14
#define STATUSBAR_DRIVE9_TRACK_POS  19
#define STATUSBAR_DRIVE10_TRACK_POS 24
#define STATUSBAR_DRIVE11_TRACK_POS 29
#define STATUSBAR_TAPE_POS          33

static char statusbar_text[MAX_STATUSBAR_LEN] = "                                       ";


static int pitch;
static int draw_offset;



static int tape_counter = 0;
static int tape_enabled = 0;
static int tape_motor = 0;
static int tape_control = 0;

static void display_tape(void)
{
	// Tell front-end
	if ( environ_cb )
	{
		if ( tape_enabled )
		{
			environ_cb( RETRO_ENVIRONMENT_SET_TAPE_COUNTER, &tape_counter );
		}
		else
		{
			int hide = -1;
			environ_cb( RETRO_ENVIRONMENT_SET_TAPE_COUNTER, &hide );
		}
	}
}

static int per = 0;
static int fps = 0;
static int warp = 0;
static int paused = 0;

static void display_speed(void)
{
    int len;
    char sep = paused ? ('P' | 0x80) : warp ? ('W' | 0x80) : '/';

    len = sprintf(&(statusbar_text[STATUSBAR_SPEED_POS]), "%3d%%%c%2dfps", per, sep, fps);
    statusbar_text[STATUSBAR_SPEED_POS + len] = ' ';
}

/* ----------------------------------------------------------------- */
/* ui.h */

void ui_display_speed(float percent, float framerate, int warp_flag)
{
    per = (int)(percent + .5);
    if (per > 999) {
        per = 999;
    }

    fps = (int)(framerate + .5);
    if (fps > 99) {
        fps = 99;
    }

    warp = warp_flag;

    display_speed();
}

void ui_display_paused(int flag)
{
    paused = flag;

    display_speed();
}

/* ----------------------------------------------------------------- */
/* uiapi.h */

/* Display a mesage without interrupting emulation */
void ui_display_statustext(const char *text, int fade_out)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: \"%s\", %i\n", __func__, text, fade_out);
#endif
}

/* Drive related UI.  */
void ui_enable_drive_status(ui_drive_enable_t state, int *drive_led_color)
{
    int drive_number;
    int drive_state = (int)state;

    for (drive_number = 0; drive_number < 4; ++drive_number) {
        if (drive_state & 1) {
            ui_display_drive_led(drive_number, 0, 0);
        } else {
            statusbar_text[STATUSBAR_DRIVE_POS + drive_number] = ' ';
        }
        drive_state >>= 1;
    }
}

void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base, unsigned int half_track_number)
{
    unsigned int track_number = half_track_number / 2;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

    switch (drive_number) {
        case 1:
            statusbar_text[STATUSBAR_DRIVE9_TRACK_POS] = (track_number / 10) + '0';
            statusbar_text[STATUSBAR_DRIVE9_TRACK_POS + 1] = (track_number % 10) + '0';
            break;
        case 2:
            statusbar_text[STATUSBAR_DRIVE10_TRACK_POS] = (track_number / 10) + '0';
            statusbar_text[STATUSBAR_DRIVE10_TRACK_POS + 1] = (track_number % 10) + '0';
            break;
        case 3:
            statusbar_text[STATUSBAR_DRIVE11_TRACK_POS] = (track_number / 10) + '0';
            statusbar_text[STATUSBAR_DRIVE11_TRACK_POS + 1] = (track_number % 10) + '0';
            break;
        default:
        case 0:
            statusbar_text[STATUSBAR_DRIVE8_TRACK_POS] = (track_number / 10) + '0';
            statusbar_text[STATUSBAR_DRIVE8_TRACK_POS + 1] = (track_number % 10) + '0';
            break;
    }
}

/* The pwm value will vary between 0 and 1000.  */
void ui_display_drive_led(int drive_number, unsigned int pwm1, unsigned int led_pwm2)
{
    char c;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s: drive %i, pwm1 = %i, led_pwm2 = %u\n", __func__, drive_number, pwm1, led_pwm2);
#endif

    c = "8901"[drive_number] | ((pwm1 > 500) ? 0x80 : 0);
    statusbar_text[STATUSBAR_DRIVE_POS + (drive_number * 5)] = c;
    statusbar_text[STATUSBAR_DRIVE_POS + (drive_number * 5) + 1] = 'T';
}

void ui_display_drive_current_image(unsigned int drive_number, const char *image)
{
printf("d%d -> %s\n",drive_number, image);
#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif
}

/* Tape related UI */

void ui_set_tape_status(int tape_status)
{
    tape_enabled = tape_status;

    display_tape();
}

void ui_display_tape_motor_status(int motor)
{
    tape_motor = motor;

    display_tape();
}

void ui_display_tape_control_status(int control)
{
    tape_control = control;

    display_tape();
}

void ui_display_tape_counter(int counter)
{
    if (tape_counter != counter)
	{
        display_tape();
    }

    tape_counter = counter;
}

void ui_display_tape_current_image(const char *image)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %s\n", __func__, image);
#endif
}

/* Recording UI */
void ui_display_playback(int playback_status, char *version)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i, \"%s\"\n", __func__, playback_status, version);
#endif
}

void ui_display_recording(int recording_status)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i\n", __func__, recording_status);
#endif
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i, %i\n", __func__, current, total);
#endif
}

/* Joystick UI */
void ui_display_joyport(BYTE *joyport)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %02x %02x %02x %02x %02x\n", __func__, joyport[0], joyport[1], joyport[2], joyport[3], joyport[4]);
#endif
}

/* Volume UI */
void ui_display_volume(int vol)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i\n", __func__, vol);
#endif
}

