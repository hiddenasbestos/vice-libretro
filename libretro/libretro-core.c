#include "libretro.h"
#include "libretro-core.h"

#ifdef __PLUS4__
#include "plus4mem.h"
extern BYTE mem_ram[PLUS4_RAM_SIZE];
#endif // __PLUS4__

#ifdef __VIC20__
#include "vic20mem.h"
#include "vic20-resources.h"
extern BYTE mem_ram[VIC20_RAM_SIZE];
#endif // __VIC20__

//CORE VAR
#ifdef _WIN32
char slash = '\\';
#else
char slash = '/';
#endif

static int is_pal_system = 1;

bool retro_load_ok = false;

retro_log_printf_t log_cb;

char RETRO_DIR[512];

char DISKA_NAME[512]="\0";
char DISKB_NAME[512]="\0";
char TAPE_NAME[512]="\0";

// Our virtual time counter, increased by retro_run()
long microSecCounter=0;
int cpuloop=1;

#ifdef FRONTEND_SUPPORTS_RGB565
	uint16_t *Retro_Screen;
	uint16_t bmp[WINDOW_SIZE];
#else
	unsigned int *Retro_Screen;
	unsigned int bmp[WINDOW_SIZE];
#endif

//SOUND
short signed int SNDBUF[1024*2];
//FIXME: handle 50/60
int snd_sampler = 44100 / 50;

//PATH
char RPATH[512];

int want_quit=0;

int CROP_WIDTH;
int CROP_HEIGHT;
int VIRTUAL_WIDTH;

int retroXS=0;
int retroYS=0;
int retroW=1024;
int retroH=768;
int retrow=1024;
int retroh=768;
int lastW=1024;
int lastH=768;

extern int RETROTDE,RETRODRVTYPE,RETROSIDMODL,RETROC64MODL,RETROVIC20RAM;
extern int retro_ui_finalized;
extern void set_drive_type(int drive,int val);
extern void set_truedrive_emulation(int val);

//VICE DEF BEGIN
#include "resources.h"
#include "sid.h"
#include "c64model.h"
#include "userport_joystick.h"
#if  defined(__VIC20__)
#include "vic20model.h"
#elif defined(__PLUS4__)
#include "plus4model.h"
#elif defined(__X128__)
#include "c128model.h"
#endif
//VICE DEF END

extern void Emu_init(void);
extern void Emu_uninit(void);
extern void vice_main_exit(void);
extern void emu_reset(void);

const char *retro_save_directory;
const char *retro_system_directory;
const char *retro_content_directory;
char retro_system_data_directory[512];;

/*static*/ retro_input_state_t input_state_cb;
/*static*/ retro_input_poll_t input_poll_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
retro_environment_t environ_cb;

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

static char CMDFILE[512];

int loadcmdfile(char *argv)
{
   int res=0;

   FILE *fp = fopen(argv,"r");

   if( fp != NULL )
   {
      if ( fgets (CMDFILE , 512 , fp) != NULL )
         res=1;
      fclose (fp);
   }

   return res;
}

int HandleExtension(char *path,char *ext)
{
   int len = strlen(path);

   if (len >= 4 &&
         path[len-4] == '.' &&
         path[len-3] == ext[0] &&
         path[len-2] == ext[1] &&
         path[len-1] == ext[2])
   {
      return 1;
   }

   return 0;
}

#include <ctype.h>

//Args for experimental_cmdline
static char ARGUV[64][1024];
static unsigned char ARGUC=0;

// Args for Core
static char XARGV[64][1024];
static const char* xargv_cmd[64];
int PARAMCOUNT=0;

extern int  skel_main(int argc, char *argv[]);
void parse_cmdline( const char *argv );

void Add_Option(const char* option)
{
   static int first=0;

   if(first==0)
   {
      PARAMCOUNT=0;
      first++;
   }

   sprintf(XARGV[PARAMCOUNT++],"%s",option);
}

int pre_main(const char *argv)
{
   int i=0;
   bool Only1Arg;

   if (strlen(argv) > strlen("cmd"))
   {
      if( HandleExtension((char*)argv,"cmd") || HandleExtension((char*)argv,"CMD"))
         i=loadcmdfile((char*)argv);
   }

   if(i==1)
   {
      parse_cmdline(CMDFILE);
      LOGI("Starting game from command line :%s\n",CMDFILE);
   }
   else
      parse_cmdline(argv);

   Only1Arg = (strcmp(ARGUV[0],CORE_NAME) == 0) ? 0 : 1;

   for (i = 0; i<64; i++)
      xargv_cmd[i] = NULL;


   if(Only1Arg)
   {  Add_Option(CORE_NAME);
      /*
         if (strlen(RPATH) >= strlen("crt"))
         if(!strcasecmp(&RPATH[strlen(RPATH)-strlen("crt")], "crt"))
         Add_Option("-cartcrt");
         */

#if defined(__VIC20__)
     if (strlen(RPATH) >= strlen(".20"))
       if (!strcasecmp(&RPATH[strlen(RPATH)-strlen(".20")], ".20"))
	 Add_Option("-cart2");

     if (strlen(RPATH) >= strlen(".40"))
       if (!strcasecmp(&RPATH[strlen(RPATH)-strlen(".40")], ".40"))
	 Add_Option("-cart4");

     if (strlen(RPATH) >= strlen(".60"))
       if (!strcasecmp(&RPATH[strlen(RPATH)-strlen(".60")], ".60"))
	 Add_Option("-cart6");

     if (strlen(RPATH) >= strlen(".a0"))
       if (!strcasecmp(&RPATH[strlen(RPATH)-strlen(".a0")], ".a0"))
	 Add_Option("-cartA");

     if (strlen(RPATH) >= strlen(".b0"))
       if (!strcasecmp(&RPATH[strlen(RPATH)-strlen(".b0")], ".b0"))
	 Add_Option("-cartB");
#endif

     Add_Option(RPATH/*ARGUV[0]*/);
   }
   else
   { // Pass all cmdline args
      for(i = 0; i < ARGUC; i++)
         Add_Option(ARGUV[i]);
   }

   for (i = 0; i < PARAMCOUNT; i++)
   {
      xargv_cmd[i] = (char*)(XARGV[i]);
      LOGI("%2d  %s\n",i,XARGV[i]);
   }

   skel_main(PARAMCOUNT,( char **)xargv_cmd);

   xargv_cmd[PARAMCOUNT - 2] = NULL;

   return 0;
}

void parse_cmdline(const char *argv)
{
   char *p,*p2,*start_of_word;
   int c,c2;
   static char buffer[512*4];
   enum states { DULL, IN_WORD, IN_STRING } state = DULL;

   strcpy(buffer,argv);
   strcat(buffer," \0");

   for (p = buffer; *p != '\0'; p++)
   {
      c = (unsigned char) *p; /* convert to unsigned char for is* functions */
      switch (state)
      {
         case DULL: /* not in a word, not in a double quoted string */
            if (isspace(c)) /* still not in a word, so ignore this char */
               continue;
            /* not a space -- if it's a double quote we go to IN_STRING, else to IN_WORD */
            if (c == '"')
            {
               state = IN_STRING;
               start_of_word = p + 1; /* word starts at *next* char, not this one */
               continue;
            }
            state = IN_WORD;
            start_of_word = p; /* word starts here */
            continue;
         case IN_STRING:
            /* we're in a double quoted string, so keep going until we hit a close " */
            if (c == '"')
            {
               /* word goes from start_of_word to p-1 */
               //... do something with the word ...
               for (c2 = 0,p2 = start_of_word; p2 < p; p2++, c2++)
                  ARGUV[ARGUC][c2] = (unsigned char) *p2;
               ARGUC++;

               state = DULL; /* back to "not in word, not in string" state */
            }
            continue; /* either still IN_STRING or we handled the end above */
         case IN_WORD:
            /* we're in a word, so keep going until we get to a space */
            if (isspace(c))
            {
               /* word goes from start_of_word to p-1 */
               //... do something with the word ...
               for (c2 = 0,p2 = start_of_word; p2 <p; p2++,c2++)
                  ARGUV[ARGUC][c2] = (unsigned char) *p2;
               ARGUC++;

               state = DULL; /* back to "not in word, not in string" state */
            }
            continue; /* either still IN_WORD or we handled the end above */
      }
   }
}

long GetTicks(void) {
   // NOTE: Cores should normally not depend on real time, so we return a
   // counter here
   // GetTicks() is used by vsyncarch_gettime() which is used by
   // * Vsync (together with sleep) to sync to 50Hz
   // * Mouse timestamps
   // * Networking
   // Returning a frame based msec counter could potentially break
   // networking but it's not something libretro uses at the moment.
   return microSecCounter;
}

void Screen_SetFullUpdate(int scr)
{
   if(scr==0 ||scr>1)
      memset(&Retro_Screen, 0, sizeof(Retro_Screen));
   if(scr>0)
      memset(&bmp,0,sizeof(bmp));
}

void retro_set_environment(retro_environment_t cb)
{
   static const struct retro_controller_description p1_controllers[] =
   {
      { "Joystick", RETRO_DEVICE_JOYPAD },
   };
   static const struct retro_controller_description p2_controllers[] =
   {
      { "Joystick", RETRO_DEVICE_JOYPAD },
   };

   static const struct retro_controller_info ports[] =
   {
      { p1_controllers, 1  }, // port 1
      { p2_controllers, 1  }, // port 2
      { NULL, 0 }
   };

   struct retro_variable variables[] =
   {

 #ifdef __VIC20__

	  {
         "vice_VIC20Video",
         "Video Standard; PAL|NTSC",
      },

	  {
         "vice_VIC20memory",
         "Memory Expansion; 24KB|NONE|3KB|8KB|16KB|35KB",
      },

#elif __PLUS4__

	  {
         "vice_PLUS4Video",
         "Video Standard; PAL|NTSC",
      },
	  {
         "vice_PLUS4Model",
         "System; PLUS/4|Commodore 16",
      },

#elif __X128__

	  {
         "vice_SidModel",
         "SidModel; 6581F|8580F|6581R|8580R|8580RD",
      },

	  {
         "vice_C128Model",
         "C128Model; C128MODEL_C128_PAL|C128MODEL_C128DCR_PAL|C128MODEL_C128_NTSC|C128MODEL_C128DCR_NTSC|C128MODEL_UNKNOWN",
      },

#elif __X64__

      {
         "vice_C64Model",
         "C64Model; C64MODEL_C64_PAL|C64MODEL_C64C_PAL|C64MODEL_C64_OLD_PAL|C64MODEL_C64_NTSC|C64MODEL_C64C_NTSC|C64MODEL_C64_OLD_NTSC|C64MODEL_C64_PAL_N|C64MODEL_C64SX_PAL|C64MODEL_C64SX_NTSC|C64MODEL_C64_JAP|C64MODEL_C64_GS|C64MODEL_PET64_PAL|C64MODEL_PET64_NTSC|C64MODEL_ULTIMAX|C64MODEL_UNKNOWN",
      },
      {
         "vice_SidModel",
         "SidModel; 6581F|8580F|6581R|8580R|8580RD",
      },
      {
         "vice_Drive8Type",
         "Drive8Type; 1541|1540|1542|1551|1570|1571|1573|1581|2000|4000|2031|2040|3040|4040|1001|8050|8250",
      },
      {
         "vice_DriveTrueEmulation",
         "DriveTrueEmulation; enabled|disabled",
      },

#endif

      { NULL, NULL },
   };

   bool allowNoGameMode;
   int i;

   environ_cb = cb;

   cb( RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports );

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);

   allowNoGameMode = true;
   environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &allowNoGameMode);
}

static void update_variables(void)
{
	struct retro_variable var;

#ifdef __PLUS4__

	var.key = "vice_PLUS4Video";
	var.value = NULL;

#elif __VIC20__

	var.key = "vice_VIC20Video";
	var.value = NULL;

#endif

	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		is_pal_system = 1;
		if (strcmp(var.value, "NTSC") == 0)
			is_pal_system = 0;
	}

#ifdef __PLUS4__

	// PLUS/4 or C-16 ?  PAL or NTSC ?

	var.key = "vice_PLUS4Model";
	var.value = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		int modl = is_pal_system ? PLUS4MODEL_PLUS4_PAL : PLUS4MODEL_PLUS4_NTSC;

		if (strcmp(var.value, "Commodore 16") == 0)
		{
			modl = is_pal_system ? PLUS4MODEL_C16_PAL : PLUS4MODEL_C16_NTSC;
		}

		if ( retro_ui_finalized )
		{
			plus4model_set( modl );
		}
		else
		{
			RETROC64MODL = modl;
		}
	}

#elif __VIC20__

	// VIC 20 model -> just PAL/NTSC options.

	{
		int modl = is_pal_system ? VIC20MODEL_VIC20_PAL : VIC20MODEL_VIC20_NTSC;

		if ( retro_ui_finalized )
		{
			vic20model_set( modl );
		}
		else
		{
			RETROC64MODL = modl;
		}
	}

	// VIC 20 memory expansion

	var.key = "vice_VIC20memory";
	var.value = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		int size = 24; // default

		if (strcmp(var.value, "35KB") == 0)
			size = 35;
		else if (strcmp(var.value, "24KB") == 0)
			size = 24;
		else if (strcmp(var.value, "16KB") == 0)
			size = 16;
		if (strcmp(var.value, "8KB") == 0)
			size = 8;
		if (strcmp(var.value, "3KB") == 0)
			size = 3;
		if (strcmp(var.value, "NONE") == 0)
			size = 0;

		log_cb( RETRO_LOG_INFO, "Memory Expansion: %dKB\n", size );

		switch ( size )
		{

		default:
		case 0:
			RETROVIC20RAM = 0;
			break;

		case 3:
			RETROVIC20RAM = VIC_BLK0;
			break;

		case 8:
			RETROVIC20RAM = VIC_BLK1;
			break;

		case 16:
			RETROVIC20RAM = VIC_BLK1 | VIC_BLK2;
			break;

		case 24:
			RETROVIC20RAM = VIC_BLK1 | VIC_BLK2 | VIC_BLK3;
			break;

		case 35:
			RETROVIC20RAM = VIC_BLK0 | VIC_BLK1 | VIC_BLK2 | VIC_BLK3 | VIC_BLK5;
			break;

		};

		if ( retro_ui_finalized )
		{
			resources_set_int("RamBlock0", ( RETROVIC20RAM & VIC_BLK0 ) ? 1:0);
			resources_set_int("RamBlock1", ( RETROVIC20RAM & VIC_BLK1 ) ? 1:0);
			resources_set_int("RamBlock2", ( RETROVIC20RAM & VIC_BLK2 ) ? 1:0);
			resources_set_int("RamBlock3", ( RETROVIC20RAM & VIC_BLK3 ) ? 1:0);
			resources_set_int("RamBlock5", ( RETROVIC20RAM & VIC_BLK5 ) ? 1:0);
		}
	}

#endif

	// Add a disk drive.

	if ( retro_ui_finalized )
	{
		set_drive_type( 8, 1541 ); // 1551 also supported on PLUS/4.
		set_truedrive_emulation( 1 );
		resources_set_int( "VirtualDevices", 0 ); // <-- key to PRG support?
	}
	else
	{
		RETRODRVTYPE = 1541; // 1551 also supported on PLUS/4.
		RETROTDE = 1;
	}

}

void Emu_init(void)
{
   update_variables();
   pre_main(RPATH);
}

void Emu_uninit(void)
{
   vice_main_exit();
}

void retro_shutdown_core(void)
{
   environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, NULL);
}

void retro_reset(void)
{
   microSecCounter = 0;
   emu_reset();
}

struct DiskImage {
    char* fname;
};

static int diskIndex = 0;
static int diskCount = 0;
static struct DiskImage diskImage[80];
static bool ejected = false;
#include <attach.h>

static bool retro_set_eject_state(bool ejected) {
    log_cb(RETRO_LOG_INFO, "EJECT %d", (int)ejected);
    if(ejected)
        file_system_detach_disk(8);
    else
        file_system_attach_disk(8, diskImage[diskIndex].fname);
}

/* Gets current eject state. The initial state is 'not ejected'. */
static bool retro_get_eject_state(void) {
    return ejected;
}

/* Gets current disk index. First disk is index 0.
 * If return value is >= get_num_images(), no disk is currently inserted.
 */
static unsigned retro_get_image_index(void) {
    return diskIndex;
}

/* Sets image index. Can only be called when disk is ejected.
 * The implementation supports setting "no disk" by using an
 * index >= get_num_images().
 */
static bool retro_set_image_index(unsigned index) {
    diskIndex = index;
}

/* Gets total number of images which are available to use. */
static unsigned retro_get_num_images(void) {
    return diskCount;
}


/* Replaces the disk image associated with index.
 * Arguments to pass in info have same requirements as retro_load_game().
 * Virtual disk tray must be ejected when calling this.
 *
 * Replacing a disk image with info = NULL will remove the disk image
 * from the internal list.
 * As a result, calls to get_image_index() can change.
 *
 * E.g. replace_image_index(1, NULL), and previous get_image_index()
 * returned 4 before.
 * Index 1 will be removed, and the new index is 3.
 */
static bool retro_replace_image_index(unsigned index,
      const struct retro_game_info *info) {
    if(diskImage[index].fname)
        free(diskImage[index].fname);
    if(info == NULL) {
        memcpy(&diskImage[index], &diskImage[index+1], sizeof(struct DiskImage)*(diskCount-index-1));
        diskCount--;
        if(diskIndex > 0)
            diskIndex--;
    } else
    diskImage[index].fname = strdup(info->path);
}

/* Adds a new valid index (get_num_images()) to the internal disk list.
 * This will increment subsequent return values from get_num_images() by 1.
 * This image index cannot be used until a disk image has been set
 * with replace_image_index. */
static bool retro_add_image_index(void) {
    diskImage[diskCount].fname = NULL;
    diskCount++;
    return true;
}

static struct retro_disk_control_callback diskControl = {
    retro_set_eject_state,
    retro_get_eject_state,
    retro_get_image_index,
    retro_set_image_index,
    retro_get_num_images,
    retro_replace_image_index,
    retro_add_image_index,
};

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   /* stub */
}

void retro_init(void)
{
   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = fallback_log;

   const char *system_dir = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir) && system_dir)
   {
      // if defined, use the system directory
      retro_system_directory=system_dir;
   }

   const char *content_dir = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY, &content_dir) && content_dir)
   {
      // if defined, use the system directory
      retro_content_directory=content_dir;
   }

   const char *save_dir = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save_dir) && save_dir)
   {
      // If save directory is defined use it, otherwise use system directory
      retro_save_directory = *save_dir ? save_dir : retro_system_directory;
   }
   else
   {
      // make retro_save_directory the same in case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY is not implemented by the frontend
      retro_save_directory=retro_system_directory;
   }

   if(retro_system_directory==NULL)sprintf(RETRO_DIR, "%s",".");
   else sprintf(RETRO_DIR, "%s", retro_system_directory);

#if defined(__WIN32__)
   sprintf(retro_system_data_directory, "%s\\data",RETRO_DIR);
#else
   sprintf(retro_system_data_directory, "%s/data",RETRO_DIR);
#endif

#ifdef FRONTEND_SUPPORTS_RGB565
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
#else
   enum retro_pixel_format fmt =RETRO_PIXEL_FORMAT_XRGB8888;
#endif

   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      log_cb(RETRO_LOG_ERROR, "PIXEL FORMAT is not supported.\n");
      exit(0);
   }

   Retro_Screen = bmp;

#define RETRO_DESCRIPTOR_BLOCK( _user )                                            \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },               \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },               \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X" },               \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y" },               \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },     \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },       \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },       \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },         \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },             \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },         \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R" },               \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L" },               \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2, "R2" },             \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, "L2" },             \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3, "R3" },             \
   { _user, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3, "L3" }

   struct retro_input_descriptor inputDescriptors[] =
   {
      RETRO_DESCRIPTOR_BLOCK( 0 ),
      RETRO_DESCRIPTOR_BLOCK( 1 ),

      { 0 },
   };

#undef RETRO_DESCRIPTOR_BLOCK

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, &inputDescriptors);

#ifdef __X64__
   environ_cb(RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE, &diskControl);
#endif // __X64__

   microSecCounter = 0;
}

void retro_deinit(void)
{
   Emu_uninit();
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}


void retro_set_controller_port_device( unsigned port, unsigned device )
{
	//
}

void retro_get_system_info(struct retro_system_info *info)
{
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
   memset(info, 0, sizeof(*info));
#ifdef __PLUS4__
   info->library_name     = "VICE PLUS/4";
   info->valid_extensions = "tap|d64";
#elif __VIC20__
   info->library_name     = "VICE VIC20";
   info->valid_extensions = "tap|d64";
#elif __X64__
   info->library_name     = "VICE C64";
   info->valid_extensions = "tap|d64";
#endif
   info->library_version  = "3.0" GIT_VERSION;
   info->need_fullpath    = true;
   info->block_extract    = false;

}

void update_geometry()
{
   struct retro_system_av_info system_av_info;
   system_av_info.geometry.base_width = retroW;
   system_av_info.geometry.base_height = retroH;
   system_av_info.geometry.aspect_ratio = (float)4.0/3.0;
   environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &system_av_info);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   /* FIXME handle PAL/NTSC */
   struct retro_game_geometry geom = { 320, 240, retrow, retroh,4.0 / 3.0 };
   struct retro_system_timing timing = { 50.0, 44100.0 };

   info->geometry = geom;
   info->timing   = timing;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_audio_cb( short l, short r)
{
   audio_cb(l,r);
}

void retro_audiocb(signed short int *sound_buffer,int sndbufsize)
{
   int x;
   for(x=0;x<sndbufsize;x++)
   	audio_cb(sound_buffer[x],sound_buffer[x]);
}

void retro_blit(void)
{
   memcpy(Retro_Screen,bmp,PITCH*WINDOW_SIZE);
}

void retro_run(void)
{
   static int mfirst=1;
   bool updated = false;

   if ( lastW!=retroW || lastH!=retroH )
   {
      update_geometry();
      log_cb(RETRO_LOG_INFO, "Update Geometry Old(%d,%d) New(%d,%d)\n",lastW,lastH,retroW,retroH);
      lastW=retroW;
      lastH=retroH;
   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      update_variables();

   if (mfirst==1)
   {
      mfirst++;
      log_cb(RETRO_LOG_INFO, "First time we return from retro_run()!\n");
      retro_load_ok=true;
      memset(SNDBUF,0,1024*2*2);

      Emu_init();
      return;
   }

	while(cpuloop==1)
		maincpu_mainloop_retro();
	cpuloop=1;

	retro_blit();

	video_cb(Retro_Screen,retroW,retroH,retrow<<PIXEL_BYTES);

	if(want_quit)
		retro_shutdown_core();

	microSecCounter += (1000000/50);
}

/*
   unsigned int lastdown,lastup,lastchar;
   static void keyboard_cb(bool down, unsigned keycode,
   uint32_t character, uint16_t mod)
   {

   log_cb(RETRO_LOG_INFO, "Down: %s, Code: %d, Char: %u, Mod: %u.\n",
   down ? "yes" : "no", keycode, character, mod);


   if(down)lastdown=keycode;
   else lastup=keycode;
   lastchar=character;

   }
   */

bool retro_load_game(const struct retro_game_info *info)
{
   /*
      struct retro_keyboard_callback cb = { keyboard_cb };
      environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &cb);
      */

   if (info)
   {
      const char *full_path = info->path;
      strcpy(RPATH,full_path);
   }
   else
   {
      RPATH[0]=0;
   }

   update_variables();

   return true;
}

void retro_unload_game(void)
{
	//
}

unsigned retro_get_region(void)
{
   return is_pal_system ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   (void)type;
   (void)info;
   (void)num;
   return false;
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *data_, size_t size)
{
   return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
   return false;
}

void *retro_get_memory_data(unsigned id)
{
   switch ( id & RETRO_MEMORY_MASK )
   {

   case RETRO_MEMORY_SYSTEM_RAM:
      return mem_ram;

   }

   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
	switch ( id & RETRO_MEMORY_MASK )
	{

#ifdef __PLUS4__
	case RETRO_MEMORY_SYSTEM_RAM:
		return PLUS4_RAM_SIZE;
#elif __VIC20__
	case RETRO_MEMORY_SYSTEM_RAM:
		return VIC20_RAM_SIZE;
#endif

	}

	return 0;
}

void retro_cheat_reset(void) {}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

