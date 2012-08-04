#ifndef USE_MENUCOMMANDS
#define USE_MENUCOMMANDS

///////////////////////////////////////
// The command IDs for the menu options

// Main menu
#define MENU_CMD_SINGLE_PLAYER		0
#define MENU_CMD_BLOODBATH			1
#define MENU_CMD_OPTIONS			2
#define MENU_CMD_HELP				3
#define MENU_CMD_CREDITS			4
#define MENU_CMD_QUIT				5	// Brings up the confirmation dialog (in non-demo mode only)
#define MENU_CMD_EXIT				6	// Actually quits the game

// Single player
#define MENU_CMD_START_STORY		100
#define MENU_CMD_START_ACTION		101
#define MENU_CMD_LOAD_GAME_MENU		102
#define MENU_CMD_SAVE_GAME_MENU		103
#define MENU_CMD_CUSTOM_LEVEL		104
#define MENU_CMD_START_NIGHTMARES	105

// BloodBath
#define MENU_CMD_HOST_GAME							200
#define MENU_CMD_JOIN_GAME							201
#define MENU_CMD_CHARACTER_SETUP					202	
#define MENU_CMD_CHARACTER_SETUP_RESVERIFY			203
#define MENU_CMD_CHARACTER_SETUP_SWITCH_RUN			204
#define MENU_CMD_CHARACTER_SETUP_SWITCH_RES_BACK	205

// Options
#define MENU_CMD_DISPLAY			300
#define MENU_CMD_AUDIO				301
#define MENU_CMD_CUSTOMIZE_CONTROLS	302
#define MENU_CMD_MOUSE				303
#define MENU_CMD_JOYSTICK			304
#define MENU_CMD_KEYBOARD			305

// Difficulty
#define	MENU_CMD_DIFFICULTY_EASY	400
#define	MENU_CMD_DIFFICULTY_MEDIUM	401
#define	MENU_CMD_DIFFICULTY_HARD	402

// Load a custom level
#define MENU_CMD_CUSTOM_LEVEL_LOAD	500

// Load a saved game
#define MENU_CMD_LOAD_SAVE_GAME		600

// Save a game
#define MENU_CMD_SAVE_GAME			700

// Change control
#define MENU_CMD_CHANGE_CONTROL		800

// Character screen
#define	MENU_CMD_LEFT_ARROW_CHAR	900
#define	MENU_CMD_RIGHT_ARROW_CHAR	901
#define MENU_CMD_SAVE_CHARACTER		902
#define MENU_CMD_LOAD_CHARACTER		903
#define MENU_CMD_DELETE_CHARACTER	904
#define MENU_CMD_B2C_FILE			950

// Character selection menu
#define MENU_CMD_START_CALEB		2000
#define MENU_CMD_START_GABRIELLA	2001
#define MENU_CMD_START_ISHMAEL		2002
#define MENU_CMD_START_OPHELIA		2003

// Message box defines
#define MENU_CMD_KILL_MESSAGEBOX	3000

//*************************************************************************

#define MENU_ACTION_SAVE			0
#define MENU_ACTION_LOAD			1
#define MENU_ACTION_DELETE			2

#endif
