#include "kbd.h"
#include "libretro.h"
#include "joystick.h"
#include "keyboard.h"
#include "datasette.h"

extern retro_log_printf_t log_cb;

int kbd_handle_keydown(int kcode)
{
	// log_cb( RETRO_LOG_INFO, "keydown: %d\n", kcode );

	switch ( kcode )
	{

	/*case RETROK_F6: // PLAY
		datasette_control( DATASETTE_CONTROL_START );
		break;*/

	/*case RETROK_F7: // STOP
		datasette_control( DATASETTE_CONTROL_STOP );
		break;*/

	default:
		keyboard_key_pressed((signed long)kcode);
		break;

	}
	
	return 0;
}

int kbd_handle_keyup(int kcode)
{
	// log_cb( RETRO_LOG_INFO, "keyup: %d\n", kcode );

	keyboard_key_released((signed long)kcode);
	return 0;
}
