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
extern retro_log_printf_t log_cb;


/* ----------------------------------------------------------------- */
/* static functions/variables */

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

static int pitch;
static int draw_offset;

// disk drive
static int drive_enable[ 4 ] = { 0, 0, 0, 0 };
static int drive_track[ 4 ] = { -1, -1, -1, -1 };

static void retro_drive_led_blink(int drive_number)
{
	if ( environ_cb )
	{
		environ_cb( RETRO_ENVIRONMENT_DISK_DRIVE_LED_BLINK, &drive_number );
//		log_cb( RETRO_LOG_INFO, "BLINK:%c; ", "8901"[drive_number] );
	}
}

// tape
static int tape_counter = 0;
static int tape_enabled = 0;
static int tape_motor = 0;
static int tape_control = 0;
static int tape_has_image = 0;

static void retro_set_tape_status(void)
{
	enum retro_tape_state state;

	if ( tape_enabled == 0 || tape_has_image == 0 )
	{
		state = RETROTAPE_STATE_EMPTY;
	}
	else
	{
		if ( tape_motor )
		{
			switch ( tape_control )
			{

			default:
				state = RETROTAPE_STATE_STOPPED;
				break;

			case 1:
				state = RETROTAPE_STATE_PLAYING;
				break;

			case 2:
				state = RETROTAPE_STATE_FFWDING;
				break;

			case 3:
				state = RETROTAPE_STATE_REWINDING;
				break;

			case 4:
				state = RETROTAPE_STATE_RECORDING;
				break;

			};
		}
		else
		{
			if ( tape_control == 1 )
			{
				state = RETROTAPE_STATE_HOLD;
			}
			else
			{
				state = RETROTAPE_STATE_STOPPED;
			}
		}
	}

#if 0

	const char* p_state = "error";
	switch ( state )
	{

	case RETROTAPE_STATE_ABSENT:
		p_state = "RETROTAPE_STATE_ABSENT";
		break;

	case RETROTAPE_STATE_EMPTY:
		p_state = "RETROTAPE_STATE_EMPTY";
		break;

	case RETROTAPE_STATE_STOPPED:
		p_state = "RETROTAPE_STATE_STOPPED";
		break;

	case RETROTAPE_STATE_PLAYING:
		p_state = "RETROTAPE_STATE_PLAYING";
		break;

	case RETROTAPE_STATE_HOLD:
		p_state = "RETROTAPE_STATE_HOLD";
		break;

	case RETROTAPE_STATE_RECORDING:
		p_state = "RETROTAPE_STATE_RECORDING";
		break;

	case RETROTAPE_STATE_REWINDING:
		p_state = "RETROTAPE_STATE_REWINDING";
		break;

	case RETROTAPE_STATE_FFWDING:
		p_state = "RETROTAPE_STATE_FFWDING";
		break;

	}

	log_cb( RETRO_LOG_INFO, "retro_set_tape_status: %s (%d,%d,%d,%d,%d)\n",
		p_state, tape_enabled, tape_has_image, tape_control, tape_motor, tape_enabled ? tape_counter : -1 );

#endif

	// Tell front-end
	if ( environ_cb )
	{
		environ_cb( RETRO_ENVIRONMENT_SET_TAPE_STATE, &state );
		environ_cb( RETRO_ENVIRONMENT_SET_TAPE_COUNTER, &tape_counter );
	}
}


/* ----------------------------------------------------------------- */
/* ui.h */

void ui_display_speed(float percent, float framerate, int warp_flag)
{
    //
}

void ui_display_paused(int flag)
{
    //
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

	for ( drive_number = 0; drive_number < 4; ++drive_number )
	{
		int enable = ( drive_state & 1 ) ? 1 : 0;

		if ( enable != drive_enable[ drive_number ] )
		{
			 drive_enable[ drive_number ] = enable;
			 drive_track[ drive_number ] = -1;
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

	if ( drive_enable[ drive_number ] )
	{
		// Track change?
		if ( drive_track[ drive_number ] != track_number )
		{
			drive_track[ drive_number ] = track_number;
			retro_drive_led_blink( drive_number );
		}
	}
}

/* The pwm value will vary between 0 and 1000.  */
void ui_display_drive_led(int drive_number, unsigned int pwm1, unsigned int led_pwm2)
{
    char c;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s: drive %i, pwm1 = %i, led_pwm2 = %u\n", __func__, drive_number, pwm1, led_pwm2);
#endif

	/*log_cb( RETRO_LOG_INFO, "PWM:%i,%u,%u; ", drive_number, pwm1, led_pwm2 );*/

	// Track change?
	if ( drive_enable[ drive_number ] && ( pwm1 > 666 ) )
	{
		retro_drive_led_blink( drive_number );
	}
}

void ui_display_drive_current_image(unsigned int drive_number, const char *image)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

	/*log_cb( RETRO_LOG_INFO, "ui_display_drive_current_image = %d,%s\n", drive_number, image );*/

}

/* Tape related UI */

void ui_set_tape_status(int tape_status)
{
    tape_enabled = tape_status;

	/*log_cb( RETRO_LOG_INFO, "ui_set_tape_status = %d\n", tape_status );*/

    retro_set_tape_status();
}

void ui_display_tape_motor_status(int motor)
{
    tape_motor = motor;

	/*log_cb( RETRO_LOG_INFO, "ui_display_tape_motor_status = %d\n", motor );*/

    retro_set_tape_status();
}

void ui_display_tape_control_status(int control)
{
    tape_control = control;

	/*log_cb( RETRO_LOG_INFO, "ui_display_tape_control_status = %d\n", control );*/

    retro_set_tape_status();
}

void ui_display_tape_counter(int counter)
{
    if (tape_counter != counter)
	{
        retro_set_tape_status();
    }

    tape_counter = counter;
}

void ui_display_tape_current_image(const char *image)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %s\n", __func__, image);
#endif

	tape_has_image = ( image && image[ 0 ] != 0 );

	log_cb( RETRO_LOG_INFO, "ui_display_tape_current_image = %s\n", image );

	// notify the front-end
	if ( environ_cb )
	{
		environ_cb( RETRO_ENVIRONMENT_SET_TAPE_FILEPATH, &image );
	}

    retro_set_tape_status();
}

/* Recording UI */
void ui_display_playback(int playback_status, char *version)
{
	log_cb( RETRO_LOG_INFO, "ui_display_playback = %d\n", playback_status );

#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i, \"%s\"\n", __func__, playback_status, version);
#endif
}

void ui_display_recording(int recording_status)
{
	log_cb( RETRO_LOG_INFO, "ui_display_recording = %d\n", recording_status );

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

