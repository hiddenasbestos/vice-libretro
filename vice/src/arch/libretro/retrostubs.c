#include "libretro.h"
#include "joystick.h"
#include "keyboard.h"
#include "machine.h"
#include "fliplist.h"
#include "mouse.h"

#include "kbd.h"
#include "mousedrv.h"
#include "libretro-core.h"

extern retro_input_poll_t input_poll_cb;
extern retro_input_state_t input_state_cb;

extern void save_bkg();
extern void Screen_SetFullUpdate(int scr);

//EMU FLAGS
int SHOWKEY=-1;
int SHIFTON=-1;
int KBMOD=-1;
int RSTOPON=-1;
int CTRLON=-1;
int NPAGE=-1;
int KCOL=1;
int SND=1;
int vkey_pressed;
unsigned char MXjoy[2]; // joy
char Core_Key_Sate[RETROK_LAST];
char Core_old_Key_Sate[RETROK_LAST];
int PAS=4;
int slowdown=0;
int pushi=0; //mouse button
int c64mouse_enable=0;
bool num_locked = false;

extern bool retro_load_ok;

void emu_reset()
{
	machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
}

// Core input Key(not GUI) 
void Core_Processkey(void)
{
   int i;

   for(i=0;i<RETROK_LAST;i++)
      Core_Key_Sate[i]=input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0,i) ? 0x80: 0;

   if(memcmp( Core_Key_Sate,Core_old_Key_Sate , sizeof(Core_Key_Sate) ) )
   {
      for ( i = 0; i < RETROK_LAST; ++i )
	  {
         if(Core_Key_Sate[i] && Core_Key_Sate[i]!=Core_old_Key_Sate[i]  )
         {	
            kbd_handle_keydown(i);
         }	
         else if ( !Core_Key_Sate[i] && Core_Key_Sate[i]!=Core_old_Key_Sate[i]  )
         {
            //printf("release: %d \n",i);
            kbd_handle_keyup(i);
 		 }
	  }
    }	

   memcpy(Core_old_Key_Sate,Core_Key_Sate , sizeof(Core_Key_Sate) );

}

// Core input (not GUI) 
int Core_PollEvent(void)
{
    //   RETRO        B    Y    SLT  STA  UP   DWN  LEFT RGT  A    X    L    R    L2   R2   L3   R3
    //   INDEX        0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15
    //   C64          BOOT VKB  M/J  R/S  UP   DWN  LEFT RGT  B1   GUI  F7   F1   F5   F3   SPC  1 

   int i;
   static int jbt[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   static int vbt[16]={0x1C,0x39,0x01,0x3B,0x01,0x02,0x04,0x08,0x80,0x40,0x15,0x31,0x24,0x1F,0x6E,0x6F};
   static int kbt[4]={0,0,0,0};

   // MXjoy[0]=0;
   if(!retro_load_ok)return 1;
   input_poll_cb();

   int mouse_l;
   int mouse_r;
   int16_t mouse_x,mouse_y;
   mouse_x=mouse_y=0;

   	Core_Processkey();

   if(slowdown>0)return 0;

 

      mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
      mouse_y = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
      mouse_l    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
      mouse_r    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
   

   slowdown=1;

   static int mmbL=0,mmbR=0;

   if(mmbL==0 && mouse_l){

      mmbL=1;		
      pushi=1;
   }
   else if(mmbL==1 && !mouse_l) {

      mmbL=0;
      pushi=0;
   }

   if(mmbR==0 && mouse_r){
      mmbR=1;		
   }
   else if(mmbR==1 && !mouse_r) {
      mmbR=0;
   }

   if(c64mouse_enable){

      mouse_move((int)mouse_x, (int)mouse_y);
      mouse_button(0,mmbL);
      mouse_button(1,mmbR);
  }

  return 1;
}

void retro_poll_event()
{
	Core_PollEvent();

	int retro_port;
	for (retro_port = 0; retro_port < 2; retro_port++)
	{
		int vice_port = retro_port + 1;
		BYTE j = joystick_value[vice_port];
		
		// Directions.

		if (input_state_cb(retro_port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP) ){
			j |= 0x01;
		} else {
			j &= ~0x01;
		}
		if (input_state_cb(retro_port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN) ){
			j |= 0x02;
		} else {
			j &= ~0x02;
		}
		if (input_state_cb(retro_port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT) ){
			j |= 0x04;
		} else {
			j &=~ 0x04;
		}
		if (input_state_cb(retro_port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT) ){
			j |= 0x08;
		} else {
			j &= ~0x08;
		}

		// Fire.
		if (input_state_cb(retro_port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) ||
			input_state_cb(retro_port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) )
		{
			j |= 0x10;
		}
		else
		{
			j &= ~0x10;
		}

		joystick_value[vice_port] = j;
	}
}

