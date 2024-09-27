#include "boxedwine.h"
#include "x11.h"
#include <SDL.h>

// http://www.techhelpmanual.com/57-keyboard_scan_codes.html

struct KeyInfo {
	U16 keyCode;
	U16 scanCode;
	U16 sdlScanCode;
	U16 keySym;
	U16 keySymWithShift;
	U16 keySymWithNum;
	const char* ch;
	const char* chShift;
	const char* chNum;
};

static const KeyInfo usKeyBoard[] =
{
	{ 0x09, 0x01, SDL_SCANCODE_ESCAPE, XK_Escape, XK_Escape, 0, "\x1b", "\x1b", 0 },
	{ 0x43, 0x3B, SDL_SCANCODE_F1, XK_F1, XK_F1, 0, 0, 0, 0 },
	{ 0x44, 0x3C, SDL_SCANCODE_F2, XK_F2, XK_F2, 0, 0, 0, 0 },
	{ 0x45, 0x3D, SDL_SCANCODE_F3, XK_F3, XK_F3, 0, 0, 0, 0 },
	{ 0x46, 0x3E, SDL_SCANCODE_F4, XK_F4, XK_F4, 0, 0, 0, 0 },
	{ 0x47, 0x3F, SDL_SCANCODE_F5, XK_F5, XK_F5, 0, 0, 0, 0 },
	{ 0x48, 0x40, SDL_SCANCODE_F6, XK_F6, XK_F6, 0, 0, 0, 0 },
	{ 0x49, 0x41, SDL_SCANCODE_F7, XK_F7, XK_F7, 0, 0, 0, 0 },
	{ 0x4a, 0x42, SDL_SCANCODE_F8, XK_F8, XK_F8, 0, 0, 0, 0 },
	{ 0x4b, 0x42, SDL_SCANCODE_F9, XK_F9, XK_F9, 0, 0, 0, 0 },
	{ 0x4c, 0x43, SDL_SCANCODE_F10, XK_F10, XK_F10, 0, 0, 0, 0 },
	{ 0x5f, 0x57, SDL_SCANCODE_F11, XK_F11, XK_F11, 0, 0, 0, 0 },
	{ 0x60, 0x58, SDL_SCANCODE_F12, XK_F12, XK_F12, 0, 0, 0, 0 },
	{ 0x31, 0x29, SDL_SCANCODE_GRAVE, XK_grave, XK_asciitilde, 0, "`", "~", 0 },
	{ 0x0a, 0x02, SDL_SCANCODE_1, XK_1, XK_exclam, 0, "1", "!", 0 },
	{ 0x0b, 0x03, SDL_SCANCODE_2, XK_2, XK_ampersand, 0, "2", "@", 0 },
	{ 0x0c, 0x04, SDL_SCANCODE_3, XK_3, XK_numbersign, 0, "3", "#", 0 },
	{ 0x0d, 0x05, SDL_SCANCODE_4, XK_4, XK_dollar, 0, "4", "$", 0 },
	{ 0x0e, 0x06, SDL_SCANCODE_5, XK_5, XK_percent, 0, "5", "%", 0 },
	{ 0x0f, 0x07, SDL_SCANCODE_6, XK_6, XK_asciicircum, 0, "6", "^", 0 },
	{ 0x10, 0x08, SDL_SCANCODE_7, XK_7, XK_ampersand, 0, "7", "&", 0 },
	{ 0x11, 0x09, SDL_SCANCODE_8, XK_8, XK_asterisk, 0, "8", "*", 0 },
	{ 0x12, 0x0A, SDL_SCANCODE_9, XK_9, XK_parenleft, 0, "9", "(", 0 },
	{ 0x13, 0x0B, SDL_SCANCODE_0, XK_0, XK_parenright, 0, "0", ")", 0 },
	{ 0x14, 0x0C, SDL_SCANCODE_MINUS, XK_minus, XK_underscore, 0, "-", "_", 0 },
	{ 0x15, 0x0D, SDL_SCANCODE_EQUALS, XK_equal, XK_plus, 0, "=", "+", 0 },
	{ 0x16, 0x0E, SDL_SCANCODE_BACKSPACE, XK_BackSpace, XK_BackSpace, 0, "\x08", "\x08", 0 },
	{ 0x17, 0x0F, SDL_SCANCODE_TAB, XK_Tab, XK_ISO_Left_Tab, 0, "\x09", 0, 0 },
	{ 0x18, 0x10, SDL_SCANCODE_Q, XK_q, XK_Q, 0, "q", "Q", 0 },
	{ 0x19, 0x11, SDL_SCANCODE_W, XK_w, XK_W, 0, "w", "W", 0 },
	{ 0x1a, 0x12, SDL_SCANCODE_E, XK_e, XK_E, 0, "e", "E", 0 },
	{ 0x1b, 0x13, SDL_SCANCODE_R, XK_r, XK_R, 0, "r", "R", 0 },
	{ 0x1c, 0x14, SDL_SCANCODE_T, XK_t, XK_T, 0, "t", "T", 0 },
	{ 0x1d, 0x15, SDL_SCANCODE_Y, XK_y, XK_Y, 0, "y", "Y", 0 },
	{ 0x1e, 0x16, SDL_SCANCODE_U, XK_u, XK_U, 0, "u", "U", 0 },
	{ 0x1f, 0x17, SDL_SCANCODE_I, XK_i, XK_I, 0, "i", "I", 0 },
	{ 0x20, 0x18, SDL_SCANCODE_O, XK_o, XK_O, 0, "o", "O", 0 },
	{ 0x21, 0x19, SDL_SCANCODE_P, XK_p, XK_P, 0, "p", "P", 0 },
	{ 0x22, 0x1A, SDL_SCANCODE_LEFTBRACKET, XK_bracketleft, XK_braceleft, 0, "[", "{", 0 },
	{ 0x23, 0x1B, SDL_SCANCODE_RIGHTBRACKET, XK_bracketright, XK_braceright, 0, "]", "}", 0 },
	{ 0x33, 0x2B, SDL_SCANCODE_BACKSLASH, XK_backslash, XK_bar, 0, "\\", "|", 0 },
	{ 0x42, 0x3A, SDL_SCANCODE_CAPSLOCK, XK_Caps_Lock, XK_Caps_Lock, 0, 0, 0, 0 },
	{ 0x26, 0x1E, SDL_SCANCODE_A, XK_a, XK_A, 0, "a", "A", 0 },
	{ 0x27, 0x1F, SDL_SCANCODE_S, XK_s, XK_S, 0, "s", "S", 0 },
	{ 0x28, 0x20, SDL_SCANCODE_D, XK_d, XK_D, 0, "d", "D", 0 },
	{ 0x29, 0x21, SDL_SCANCODE_F, XK_f, XK_F, 0, "f", "F", 0 },
	{ 0x2a, 0x22, SDL_SCANCODE_G, XK_g, XK_G, 0, "g", "G", 0 },
	{ 0x2b, 0x23, SDL_SCANCODE_H, XK_h, XK_H, 0, "h", "H", 0 },
	{ 0x2c, 0x24, SDL_SCANCODE_J, XK_j, XK_J, 0, "j", "J", 0 },
	{ 0x2d, 0x25, SDL_SCANCODE_K, XK_k, XK_K, 0, "k", "K", 0 },
	{ 0x2e, 0x26, SDL_SCANCODE_L, XK_l, XK_L, 0, "l", "L", 0 },
	{ 0x2f, 0x27, SDL_SCANCODE_SEMICOLON, XK_semicolon, XK_colon, 0, ";", ":", 0 },
	{ 0x30, 0x28, SDL_SCANCODE_APOSTROPHE, XK_apostrophe, XK_quotedbl, 0, "\'", "\'", 0 },
	{ 0x24, 0x1C, SDL_SCANCODE_RETURN, XK_Return, XK_Return, 0, "\r", "\r", 0 },
	{ 0x32, 0x2a, SDL_SCANCODE_LSHIFT, XK_Shift_L, XK_Shift_L, 0, 0, 0, 0 },
	{ 0x34, 0x2C, SDL_SCANCODE_Z, XK_z, XK_Z, 0, "z", "Z", 0 },
	{ 0x35, 0x2D, SDL_SCANCODE_X, XK_x, XK_X, 0, "x", "X", 0 },
	{ 0x36, 0x2E, SDL_SCANCODE_C, XK_c, XK_C, 0, "c", "C", 0 },
	{ 0x37, 0x2F, SDL_SCANCODE_V, XK_v, XK_V, 0, "v", "V", 0 },
	{ 0x38, 0x30, SDL_SCANCODE_B, XK_b, XK_B, 0, "b", "B", 0 },
	{ 0x39, 0x31, SDL_SCANCODE_N, XK_n, XK_N, 0, "n", "N", 0 },
	{ 0x3a, 0x32, SDL_SCANCODE_M, XK_m, XK_M, 0, "m", "M", 0 },
	{ 0x3b, 0x33, SDL_SCANCODE_COMMA, XK_comma, XK_less, 0, ",", "<", 0 },
	{ 0x3c, 0x34, SDL_SCANCODE_PERIOD, XK_period, XK_greater, 0, ".", ">", 0 },
	{ 0x3d, 0x35, SDL_SCANCODE_SLASH, XK_slash, XK_question, 0, "/", "?", 0 },
	{ 0x3e, 0x36, SDL_SCANCODE_RSHIFT, XK_Shift_R, XK_Shift_R, 0, 0, 0, 0 },
	{ 0x25, 0x1d, SDL_SCANCODE_LCTRL, XK_Control_L, XK_Control_L, 0, 0, 0, 0 },
	{ 0x85, 0x5B, SDL_SCANCODE_LGUI, XK_Meta_L, 0, 0, 0, 0, 0 },
	{ 0x40, 0x38, SDL_SCANCODE_LALT, XK_Alt_L, XK_Alt_L, 0, 0, 0, 0 },
	{ 0x41, 0x39, SDL_SCANCODE_SPACE, XK_space, XK_space, 0, " ", " ", 0 },
	{ 0x6c, 0x38, SDL_SCANCODE_RALT, XK_Alt_R, XK_Alt_R, 0, 0, 0, 0 },
	{ 0x86, 0x5C, SDL_SCANCODE_RGUI, XK_Meta_R, 0, 0, 0, 0, 0 },
	{ 0x69, 0x1d, SDL_SCANCODE_RCTRL, XK_Control_R, XK_Control_R, 0, 0, 0, 0 },
	{ 0x4c, 0x45, SDL_SCANCODE_PRINTSCREEN, XK_Print, XK_Print, 0, 0, 0, 0 }, // 0x4c is a guess
    { 0x4e, 0x46, SDL_SCANCODE_SCROLLLOCK, XK_Scroll_Lock, XK_Scroll_Lock, 0, 0, 0, 0 },
	{ 0x7f, 0x45, SDL_SCANCODE_PAUSE, XK_Pause, XK_Pause, 0, 0, 0, 0 },
	{ 0x76, 0x52, SDL_SCANCODE_INSERT, XK_Insert, XK_Insert, 0, 0, 0, 0 },
	{ 0x6e, 0x47, SDL_SCANCODE_HOME, XK_Home, XK_Home, 0, 0, 0, 0 },
	{ 0x70, 0x49, SDL_SCANCODE_PAGEUP, XK_Page_Up, XK_Page_Up, 0, 0, 0, 0 },
	{ 0x77, 0x53, SDL_SCANCODE_DELETE, XK_Delete, XK_Delete, 0, 0, 0, 0 },
	{ 0x73, 0x4f, SDL_SCANCODE_END, XK_End, XK_End, 0, 0, 0, 0 },
	{ 0x75, 0x4f, SDL_SCANCODE_PAGEDOWN, XK_Page_Down, XK_Page_Down, 0, 0, 0, 0 },
	{ 0x6f, 0x51, SDL_SCANCODE_UP, XK_Up, XK_Up, 0, 0, 0, 0 },
	{ 0x71, 0x4b, SDL_SCANCODE_LEFT, XK_Left, XK_Left, 0, 0, 0, 0 },
	{ 0x72, 0x4d, SDL_SCANCODE_RIGHT, XK_Right, XK_Right, 0, 0, 0, 0 },
	{ 0x74, 0x50, SDL_SCANCODE_DOWN, XK_Down, XK_Down, 0, 0, 0, 0 },
	{ 0x4d, 0x45, SDL_SCANCODE_NUMLOCKCLEAR, XK_Num_Lock, XK_Num_Lock, 0, 0, 0, 0 },
	{ 0x6a, 0x35, SDL_SCANCODE_KP_DIVIDE, XK_KP_Divide, XK_KP_Divide, 0, "/", "/", 0 },
	{ 0x3f, 0x37, SDL_SCANCODE_KP_MULTIPLY, XK_KP_Multiply, XK_KP_Multiply, 0, "*", "*", 0 },
	{ 0x52, 0x4a, SDL_SCANCODE_KP_MINUS, XK_KP_Subtract, XK_KP_Subtract, 0, "-", "-", 0 },
	{ 0x56, 0x4e, SDL_SCANCODE_KP_PLUS, XK_KP_Add, XK_KP_Add, 0, "+", "+", 0 },
	{ 0x68, 0x1c, SDL_SCANCODE_KP_ENTER, XK_KP_Enter, XK_KP_Enter, 0, "\r", "\r", 0 },
	{ 0x4f, 0x47, SDL_SCANCODE_KP_7, XK_KP_Home, XK_KP_Home, XK_KP_7, 0, 0, "7" },
	{ 0x50, 0x48, SDL_SCANCODE_KP_8, XK_KP_Up, XK_KP_Up, XK_KP_8, 0, 0, "8" },
	{ 0x51, 0x49, SDL_SCANCODE_KP_9, XK_KP_Page_Up, XK_KP_Page_Up, XK_KP_9, 0, 0, "9" },
	{ 0x53, 0x4b, SDL_SCANCODE_KP_4, XK_KP_Left, XK_KP_Left, XK_KP_4, 0, 0, "4" },
	{ 0x54, 0x4c, SDL_SCANCODE_KP_5, 0, 0, XK_KP_5, 0, 0, "5" },
	{ 0x55, 0x4d, SDL_SCANCODE_KP_6, XK_KP_Right, XK_KP_Right, XK_KP_6, 0, 0, "6" },
	{ 0x57, 0x4f, SDL_SCANCODE_KP_1, XK_KP_End, XK_KP_End, XK_KP_1, 0, 0, "1" },
	{ 0x58, 0x50, SDL_SCANCODE_KP_2, XK_KP_Down, XK_KP_Down, XK_KP_2, 0, 0, "2" },
	{ 0x59, 0x51, SDL_SCANCODE_KP_3, XK_KP_Page_Down, XK_KP_Page_Down, XK_KP_3, 0, 0, "3" },
	{ 0x5a, 0x52, SDL_SCANCODE_KP_0, XK_KP_Insert, XK_KP_Insert, XK_KP_0, 0, 0, "0" },
	{ 0x5b, 0x53, SDL_SCANCODE_KP_PERIOD, XK_KP_Delete, XK_KP_Delete, XK_KP_Decimal, 0, 0, "." },
};

struct KeyName {
	U16 key;
	const char* name;
};

static const KeyName keyNames[] = {
	{ XK_Escape, "Escape" },
	{ XK_F1, "F1" },
	{ XK_F2, "F2" },
	{ XK_F3, "F3" },
	{ XK_F4, "F4" },
	{ XK_F5, "F5" },
	{ XK_F6, "F6" },
	{ XK_F7, "F7" },
	{ XK_F8, "F8" },
	{ XK_F9, "F9" },
	{ XK_F10, "F10" },
	{ XK_F11, "F11" },
	{ XK_F12, "F12" },
	{ XK_grave, "grave" },
	{ XK_asciitilde, "asciitilde"},
	{ XK_1, "1" },
	{ XK_exclam, "exclam" },
	{ XK_2, "2" },
	{ XK_ampersand, "at" },
	{ XK_3, "3" },
	{ XK_numbersign, "numbersign" },
	{ XK_4, "4" },
	{ XK_dollar, "dollar" },
	{ XK_5, "5" },
	{ XK_percent, "percent" },
	{ XK_6, "6" },
	{ XK_asciicircum, "asciicircum" },
	{ XK_7, "7" },
	{ XK_ampersand, "ampersand" },
	{ XK_8, "8" },
	{ XK_asterisk, "asterisk" },
	{ XK_9, "9" },
	{ XK_parenleft, "parenleft" },
	{ XK_0, "0" },
	{ XK_parenright, "parenright" },
	{ XK_minus, "minus" },
	{ XK_underscore, "underscore" },
	{ XK_equal, "equal" },
	{ XK_plus, "plus" },
	{ XK_BackSpace, "BackSpace" },
	{ XK_Tab, "Tab" },
	{ XK_ISO_Left_Tab, "ISO_Left_Tab" },
	{ XK_q, "q" },
	{ XK_Q, "Q" },
	{ XK_w, "w" },
	{ XK_W, "W" },
	{ XK_e, "e" },
	{ XK_E, "E" },
	{ XK_r, "r" },
	{ XK_R, "R" },
	{ XK_t, "t" },
	{ XK_T, "T" },
	{ XK_y, "y" },
	{ XK_Y, "Y" },
	{ XK_u, "u" },
	{ XK_U, "U" },
	{ XK_i, "i" },
	{ XK_I, "I" },
	{ XK_o, "o" },
	{ XK_O, "O" },
	{ XK_p, "p" },
	{ XK_P, "P" },
	{ XK_bracketleft, "bracketleft" },
	{ XK_braceleft, "braceleft" },
	{ XK_bracketright, "bracketright" },
	{ XK_braceright, "braceright" },
	{ XK_backslash, "backslash" },
	{ XK_bar, "bar" },
	{ XK_Caps_Lock, "Caps_Lock" },
	{ XK_a, "a" },
	{ XK_A, "A" },
	{ XK_s, "s" },
	{ XK_S, "S" },
	{ XK_d, "d" },
	{ XK_D, "D" },
	{ XK_f, "f" },
	{ XK_F, "F" },
	{ XK_g, "g" },
	{ XK_G, "G" },
	{ XK_h, "h" },
	{ XK_H, "H" },
	{ XK_j, "j" },
	{ XK_J, "J" },
	{ XK_k, "k" },
	{ XK_K, "K" },
	{ XK_l, "l" }, 
	{ XK_L, "L" },
	{ XK_semicolon, "semicolon" },
	{ XK_colon, "colon" },
	{ XK_apostrophe, "apostrophe" },
	{ XK_quotedbl, "quotedbl" },
	{ XK_Return, "Return" },
	{ XK_Shift_L, "Shift_L" },
	{ XK_z, "z" },
	{ XK_Z, "Z" },
	{ XK_x, "x" },
	{ XK_X, "X" },
	{ XK_c, "c" },
	{ XK_C, "C" },
	{ XK_v, "v" },
	{ XK_V, "V" },
	{ XK_b, "b" },
	{ XK_B, "B" },
	{ XK_n, "n" },
	{ XK_N, "N" },
	{ XK_m, "m" },
	{ XK_M, "M" },
	{ XK_comma, "," },
	{ XK_less, "<" },
	{ XK_period, "." },
	{ XK_greater, ">" },
	{ XK_slash, "/" },
	{ XK_question, "?" },
	{ XK_Shift_R, "Shift_R" },
	{ XK_Control_L, "Control_R" },
	{ XK_Meta_L, "Super_L" },
	{ XK_Alt_L, "Alt_L" },
	{ XK_space, "space" },
	{ XK_Alt_R, "Alt_R" },
	{ XK_Meta_R, "Super_R" },
	{ XK_Control_R, "Control_R" },
	{ XK_Print, "Print" }, // a guess
	{ XK_Scroll_Lock, "Scroll_Lock" },
	{ XK_Pause, "Pause" },
	{ XK_Insert, "Insert" },
	{ XK_Home, "Home" },
	{ XK_Page_Up, "Prior" },
	{ XK_Delete, "Delete" },
	{ XK_End, "End" },
	{ XK_Page_Down, "Next" },
	{ XK_Up, "Up" },
	{ XK_Left, "Left" },
	{ XK_Right, "Right" },
	{ XK_Down, "Down" },
	{ XK_Num_Lock, "Num_Lock" },
	{ XK_KP_Divide, "KP_Divide" },
	{ XK_KP_Multiply, "KP_Multiply" },
	{ XK_KP_Subtract, "KP_Subtract" },
	{ XK_KP_Add, "KP_Add" },
	{ XK_KP_Enter, "KP_Enter" },
	{ XK_KP_Home, "KP_Home" },
	{ XK_KP_7, "KP_7" },
	{ XK_KP_Up, "KP_Up" },
	{ XK_KP_8, "KP_8" },
	{ XK_KP_Page_Up, "KP_Prior" },
	{ XK_KP_9, "KP_9" },
	{ XK_KP_Left, "KP_Left" },
	{ XK_KP_4, "KP_4" },
	{ XK_KP_5, "KP_5" },
	{ XK_KP_Right, "KP_Right" },
	{ XK_KP_6, "KP_6" },
	{ XK_KP_End, "KP_End" },
	{ XK_KP_1, "KP_1" },
	{ XK_KP_Down, "KP_Down" },
	{ XK_KP_2, "KP_2" },
	{ XK_KP_Page_Down, "KP_Next" },
	{ XK_KP_3, "KP_3" },
	{ XK_KP_Insert, "KP_Insert" },
	{ XK_KP_0, "KP_0" },
	{ XK_KP_Delete, "KP_Delete" },
	{ XK_KP_Decimal, "KP_Decimal" },
};

static BHashTable<U16, const KeyInfo*> keyCodeToInfo;
static BHashTable<U16, const KeyInfo*> keySymToInfo;
static BHashTable<U16, const KeyInfo*> keySdlToInfo;
static BHashTable<U16, const char*> keyToName;

static U16 minKeyCode;
static U16 maxKeyCode;
#define MAX_KEYS_PER_MODIFIER 2
#define NUMBER_OF_MODIFIERS 8

#define MODIFIER_SHIFT 0x01
#define MODIFIER_CAPS_LOCK 0x02
#define MODIFIER_CONTROL 0x04
#define MODIFIER_ALT 0x08
#define MODIFIER_NUM_LOCK 0x10
#define MODIFIER_SCROLL_LOCK 0x20

#define IS_SHIFT(x) ((x & 2) != 0 && (x & 2) != 3)

// shift, caps, control, alt, num, scroll
static U8 modifiers[16] = { 0x32, 0x42, 0x25, 0x40, 0x4d, 0x4e, 0, 0 };

void XKeyboard::init() {
	minKeyCode = 0xFFFF;
	maxKeyCode = 0;
	for (U32 i = 0; i < sizeof(usKeyBoard) / sizeof(KeyInfo); i++) {
		if (usKeyBoard[i].keyCode != 0) {
			if (usKeyBoard[i].keyCode < minKeyCode) {
				minKeyCode = usKeyBoard[i].keyCode;
			}
			if (usKeyBoard[i].keyCode > maxKeyCode) {
				maxKeyCode = usKeyBoard[i].keyCode;
			}
		}
		keyCodeToInfo.set(usKeyBoard[i].keyCode, &usKeyBoard[i]);
		keySymToInfo.set(usKeyBoard[i].keySym, &usKeyBoard[i]);
		keySdlToInfo.set(usKeyBoard[i].sdlScanCode, &usKeyBoard[i]);
		keySymToInfo.set(usKeyBoard[i].keySymWithShift, &usKeyBoard[i]);
		if (usKeyBoard[i].keySymWithNum) {
			keySymToInfo.set(usKeyBoard[i].keySymWithNum, &usKeyBoard[i]);
		}
	}
	for (U32 i = 0; i < sizeof(keyNames) / sizeof(KeyName); i++) {
		keyToName.set(keyNames[i].key, keyNames[i].name);
	}
}

const char* XKeyboard::getKeysymName(U32 keysym) {
	return keyToName[keysym];
}

void XKeyboard::getMinMaxKeycodes(S32& minKeycode, S32& maxKeycode) {
	minKeycode = minKeyCode;
	maxKeycode = maxKeyCode;
}

const U8* XKeyboard::getModifiers() {
	return modifiers;
}

U32 XKeyboard::keycodeToKeysym(U32 keycode, U32 level) {
	const KeyInfo* info = nullptr;
	U16 key = (U16)keycode;

	if (keyCodeToInfo.get(key, info)) {
		if (level == 0) {
			return info->keySym;
		} else if (level == 1 || level == 2) {
			return info->keySymWithShift;
		} else if (level == 5) {
			return info->keySymWithNum;
		}
	}
	return 0;
}

U32 XKeyboard::keysymToKeycode(U32 keysym) {
	const KeyInfo* info = nullptr;
	U16 key = (U16)keysym;

	if (keySymToInfo.get(key, info)) {
		return info->keyCode;
	}
	return 0;
}

U32 XKeyboard::sdl2x11(U32 sdlScanCode) {
	const KeyInfo* info = nullptr;

	if (!keySdlToInfo.get((U16)sdlScanCode, info)) {
		return 0;
	}
	return info->keyCode;
}

U32 XKeyboard::translate(U32& keysym, U32 modifiers, char* buffer, U32 bufferLen) {
	U16 key = (U16)keysym;
	const KeyInfo* info = nullptr;
	U32 len = 0;

	if (!keySymToInfo.get(key, info)) {
		return 0;
	}
	if ((modifiers & NumMask) && info->chNum) {
		if (info->chNum) {
			len = (U32)strlen(info->chNum);
			if (bufferLen) {
				len = std::min(len, bufferLen);
				memcpy(buffer, info->chNum, len);
			}
		}
		keysym = info->keySymWithNum;
		return len;
	}
	if (IS_SHIFT(modifiers) || keysym == info->keySymWithShift) {
		if (info->chShift) {
			len = (U32)strlen(info->chShift);
			if (bufferLen) {
				len = std::min(len, bufferLen);
				memcpy(buffer, info->chShift, len);
			}
		}
		keysym = info->keySymWithShift;
		return len;
	}
	if (info->ch) {
		len = (U32)strlen(info->ch);
		if (bufferLen) {
			len = std::min(len, bufferLen);
			memcpy(buffer, info->ch, len);
		}
	}
	keysym = info->keySym;
	return len;
}