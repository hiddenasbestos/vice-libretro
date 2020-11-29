/*
 * ui.c - PSP user interface.
 *
 * Written by
 *  Akop Karapetyan <dev@psp.akop.org>
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
#include "machine.h"
#include "cmdline.h"
#include "uistatusbar.h"
#include "resources.h"
#include "sid.h"
#include "c64model.h"
#include "userport_joystick.h"
#if  defined(__VIC20__)
#include "vic20model.h"
#include "vic20mem.h"
#elif defined(__PLUS4__)
#include "plus4model.h"
#elif  defined(__X128__)
#include "c128model.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int RETROTDE=0,RETRODRVTYPE=1542,RETROSIDMODL=0,RETROC64MODL=0,RETROVIC20RAM=0,RETRO_BORDERS=1;
int retro_ui_finalized = 0;

static const cmdline_option_t cmdline_options[] =
{
     { NULL }
};

/* Initialization  */
int ui_resources_init(void)
{
	return 0;
}

void ui_resources_shutdown(void)
{

}

int ui_init(void)
{
    return 0;
}

void ui_shutdown(void)
{
}

void ui_check_mouse_cursor(void)
{
   /* needed */
}

void ui_error(const char *format, ...)
{

   char text[512];
   va_list	ap;

   if (format == NULL)return;

   va_start(ap,format );
   vsprintf(text, format, ap);
   va_end(ap);
   fprintf(stderr, "ui_error: %s\n", text);
}

/* Update all the menus according to the current settings.  */

void ui_update_menus(void)
{
}

int ui_extend_image_dialog(void)
{
   return 0;
}

void ui_dispatch_events(void)
{
}

int ui_cmdline_options_init(void)
{
   return cmdline_register_options(cmdline_options);
}

int ui_init_finish(void)
{
   return 0;
}

int ui_init_finalize(void)
{
	if ( RETROTDE == 1 )
	{
		resources_set_int("DriveTrueEmulation", 1);
		resources_set_int("VirtualDevices", 0);
	}
	else if( RETROTDE == 0 )
	{
		resources_set_int("DriveTrueEmulation", 0);
		resources_set_int("VirtualDevices", 1);
	}

   resources_set_int_sprintf("Drive%iType",RETRODRVTYPE , 8);

   sid_set_engine_model((RETROSIDMODL >> 8),  (RETROSIDMODL & 0xff));

#if  defined(__VIC20__)

	vic20model_set(RETROC64MODL);

	resources_set_int("RamBlock0", ( RETROVIC20RAM & VIC_BLK0 ) ? 1:0);
	resources_set_int("RamBlock1", ( RETROVIC20RAM & VIC_BLK1 ) ? 1:0);
	resources_set_int("RamBlock2", ( RETROVIC20RAM & VIC_BLK2 ) ? 1:0);
	resources_set_int("RamBlock3", ( RETROVIC20RAM & VIC_BLK3 ) ? 1:0);
	resources_set_int("RamBlock5", ( RETROVIC20RAM & VIC_BLK5 ) ? 1:0);

#elif defined(__PLUS4__)

	plus4model_set(RETROC64MODL);

	resources_set_int( "TEDBorderMode", RETRO_BORDERS );

#elif defined(__X128__)

   c128model_set(RETROC64MODL);

#else

	c64model_set(RETROC64MODL);

	resources_set_int( "VICIIBorderMode", RETRO_BORDERS );

#endif

   retro_ui_finalized = 1;

   return 0;
}

char* ui_get_file(const char *format,...)
{
   return NULL;
}



