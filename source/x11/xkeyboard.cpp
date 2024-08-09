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
	char ch;
	char chShift;
};

static const KeyInfo usKeyBoard[] =
{
	{ 0x09, 0x01, SDL_SCANCODE_ESCAPE, XK_Escape, XK_Escape, 0, 0 }, 
	{ 0x43, 0x3B, SDL_SCANCODE_F1, XK_F1, XK_F1, 0, 0 },
	{ 0x44, 0x3C, SDL_SCANCODE_F2, XK_F2, XK_F2, 0, 0 },
	{ 0x45, 0x3D, SDL_SCANCODE_F3, XK_F3, XK_F3, 0, 0 },
	{ 0x46, 0x3E, SDL_SCANCODE_F4, XK_F4, XK_F4, 0, 0 },
	{ 0x47, 0x3F, SDL_SCANCODE_F5, XK_F5, XK_F5, 0, 0 },
	{ 0x48, 0x40, SDL_SCANCODE_F6, XK_F6, XK_F6, 0, 0 },
	{ 0x49, 0x41, SDL_SCANCODE_F7, XK_F7, XK_F7, 0, 0 },
	{ 0x4a, 0x42, SDL_SCANCODE_F8, XK_F8, XK_F8, 0, 0 },
	{ 0x4b, 0x42, SDL_SCANCODE_F9, XK_F9, XK_F9, 0, 0 },
	{ 0x4c, 0x43, SDL_SCANCODE_F10, XK_F10, XK_F10, 0, 0 },
	{ 0x5f, 0x57, SDL_SCANCODE_F11, XK_F11, XK_F11, 0, 0 },
	{ 0x60, 0x58, SDL_SCANCODE_F12, XK_F12, XK_F12, 0, 0 },
	{ 0x31, 0x29, SDL_SCANCODE_GRAVE, XK_grave, XK_asciitilde, '`', '~' },
	{ 0x0a, 0x02, SDL_SCANCODE_1, XK_1, XK_exclam, '1', '!' },
	{ 0x0b, 0x03, SDL_SCANCODE_2, XK_2, XK_ampersand, '2', '@' },
	{ 0x0c, 0x04, SDL_SCANCODE_3, XK_3, XK_numbersign, '3', '#' },
	{ 0x0d, 0x05, SDL_SCANCODE_4, XK_4, XK_dollar, '4', '$' },
	{ 0x0e, 0x06, SDL_SCANCODE_5, XK_5, XK_percent, '5', '%' },
	{ 0x0f, 0x07, SDL_SCANCODE_6, XK_6, XK_asciicircum, '6', '^' },
	{ 0x10, 0x08, SDL_SCANCODE_7, XK_7, XK_ampersand, '7', '&' },
	{ 0x11, 0x09, SDL_SCANCODE_8, XK_8, XK_asterisk, '8', '*' },
	{ 0x12, 0x0A, SDL_SCANCODE_9, XK_9, XK_parenleft, '9', '(' },
	{ 0x13, 0x0B, SDL_SCANCODE_0, XK_0, XK_parenright, '0', ')' },
	{ 0x14, 0x0C, SDL_SCANCODE_MINUS, XK_minus, XK_underscore, '-', '_' },
	{ 0x15, 0x0D, SDL_SCANCODE_EQUALS, XK_equal, XK_plus, '=', '+' },
	{ 0x16, 0x0E, SDL_SCANCODE_BACKSPACE, XK_BackSpace, XK_BackSpace, 0, 0 },
	{ 0x17, 0x0F, SDL_SCANCODE_TAB, XK_Tab, XK_ISO_Left_Tab, 0, 0 },
	{ 0x18, 0x10, SDL_SCANCODE_Q, XK_q, XK_Q, 'q', 'Q' },
	{ 0x19, 0x11, SDL_SCANCODE_W, XK_w, XK_W, 'w', 'W' },
	{ 0x1a, 0x12, SDL_SCANCODE_E, XK_e, XK_E, 'e', 'E' },
	{ 0x1b, 0x13, SDL_SCANCODE_R, XK_r, XK_R, 'r', 'R' },
	{ 0x1c, 0x14, SDL_SCANCODE_T, XK_t, XK_T, 't', 'T' },
	{ 0x1d, 0x15, SDL_SCANCODE_Y, XK_y, XK_Y, 'y', 'Y' },
	{ 0x1e, 0x16, SDL_SCANCODE_U, XK_u, XK_U, 'u', 'U' },
	{ 0x1f, 0x17, SDL_SCANCODE_I, XK_i, XK_I, 'i', 'I' },
	{ 0x20, 0x18, SDL_SCANCODE_O, XK_o, XK_O, 'o', 'O' },
	{ 0x21, 0x19, SDL_SCANCODE_P, XK_p, XK_P, 'p', 'P' },
	{ 0x22, 0x1A, SDL_SCANCODE_LEFTBRACKET, XK_bracketleft, XK_braceleft, '[', '{' },
	{ 0x23, 0x1B, SDL_SCANCODE_RIGHTBRACKET, XK_bracketright, XK_braceright, ']', '}' },
	{ 0x33, 0x2B, SDL_SCANCODE_BACKSLASH, XK_backslash, XK_bar, '\\', '|' },
	{ 0x42, 0x3A, SDL_SCANCODE_CAPSLOCK, XK_Caps_Lock, XK_Caps_Lock, 0, 0 },
	{ 0x26, 0x1E, SDL_SCANCODE_A, XK_a, XK_A, 'a', 'A' },
	{ 0x27, 0x1F, SDL_SCANCODE_S, XK_s, XK_S, 's', 'S' },
	{ 0x28, 0x20, SDL_SCANCODE_D, XK_d, XK_D, 'd', 'D' },
	{ 0x29, 0x21, SDL_SCANCODE_F, XK_f, XK_F, 'f', 'F' },
	{ 0x2a, 0x22, SDL_SCANCODE_G, XK_g, XK_G, 'g', 'G' },
	{ 0x2b, 0x23, SDL_SCANCODE_H, XK_h, XK_H, 'h', 'H' },
	{ 0x2c, 0x24, SDL_SCANCODE_J, XK_j, XK_J, 'j', 'J' },
	{ 0x2d, 0x25, SDL_SCANCODE_K, XK_k, XK_K, 'k', 'K' },
	{ 0x2e, 0x26, SDL_SCANCODE_L, XK_l, XK_L, 'l', 'L' },
	{ 0x2f, 0x27, SDL_SCANCODE_SEMICOLON, XK_semicolon, XK_colon, ';', ':' },
	{ 0x30, 0x28, SDL_SCANCODE_APOSTROPHE, XK_apostrophe, XK_quotedbl, '\'', '"' },
	{ 0x24, 0x1C, SDL_SCANCODE_RETURN, XK_Return, XK_Return, 0, 0 },
	{ 0x32, 0x2a, SDL_SCANCODE_LSHIFT, XK_Shift_L, XK_Shift_L, 0, 0 },
	{ 0x34, 0x2C, SDL_SCANCODE_Z, XK_z, XK_Z, 'z', 'Z' },
	{ 0x35, 0x2D, SDL_SCANCODE_X, XK_x, XK_X, 'x', 'X' },
	{ 0x36, 0x2E, SDL_SCANCODE_C, XK_c, XK_C, 'c', 'C' },
	{ 0x37, 0x2F, SDL_SCANCODE_V, XK_v, XK_V, 'v', 'V' },
	{ 0x38, 0x30, SDL_SCANCODE_B, XK_b, XK_B, 'b', 'B' },
	{ 0x39, 0x31, SDL_SCANCODE_N, XK_n, XK_N, 'n', 'N' },
	{ 0x3a, 0x32, SDL_SCANCODE_M, XK_m, XK_M, 'm', 'M' },
	{ 0x3b, 0x33, SDL_SCANCODE_COMMA, XK_comma, XK_less, ',', '<' },
	{ 0x3c, 0x34, SDL_SCANCODE_PERIOD, XK_period, XK_greater, '.', '>' },
	{ 0x3d, 0x35, SDL_SCANCODE_SLASH, XK_slash, XK_question, '/', '?' },
	{ 0x3e, 0x36, SDL_SCANCODE_RSHIFT, XK_Shift_R, XK_Shift_R, 0, 0 },
	{ 0x25, 0x1d, SDL_SCANCODE_LCTRL, XK_Control_L, XK_Control_L, 0, 0 },
	{ 0x7d, 0xE05B, SDL_SCANCODE_LGUI, XK_Meta_L, 0, 0, 0 },
	{ 0x40, 0x38, SDL_SCANCODE_LALT, XK_Alt_L, XK_Alt_L, 0, 0 },
	{ 0x41, 0x39, SDL_SCANCODE_SPACE, XK_space, XK_space, ' ', ' ' },
	{ 0x6c, 0xE038, SDL_SCANCODE_RALT, XK_Alt_R, XK_Alt_R, 0, 0 },
	{ 0x7e, 0xE05C, SDL_SCANCODE_RGUI, XK_Meta_R, 0, 0, 0 },
	{ 0x4d, 0xE01D, SDL_SCANCODE_NUMLOCKCLEAR, XK_Num_Lock, XK_Num_Lock, 0, 0 }
    // {0x4e, ??, SDL_SCANCODE_SCROLLLOCK, XK_Scroll_Lock, XK_Scroll_Lock, 0, 0 }
	// {0x4f, ??, SDL_SCANCODE_HOME, XK_KP_Home, XK_KP_Home, 0, 0 }
	// {0x50, ??, , XK_KP_Up, XK_KP_Up, 0, 0}
	//0xe02ae037e0aae0b7, SDL_SCANCODE_PRINTSCREEN, XK_Print, XK_Print, 0x46, SDL_SCANCODE_SCROLLLOCK, XK_Scroll_Lock, XK_Scroll_Lock, 0xE11D45, SDL_SCANCODE_PAUSE, XK_Pause, XK_Pause,
};

static BHashTable<U16, const KeyInfo*> keyCodeToInfo;
static BHashTable<U16, const KeyInfo*> keySymToInfo;
static U16 minKeyCode;
static U16 maxKeyCode;
#define MAX_KEYS_PER_MODIFIER 2
#define NUMBER_OF_MODIFIERS 8

#define MODIFIER_SHIFT 0x01
#define MODIFIER_CAPS_LOCK 0x02
#define MODIFIER_CONTROL 0x04
#define MODIFIER_ALT 0x08
#define MODIFIER_NUM_LOCK 0x10

#define IS_SHIFT(x) ((x & 2) != 0 && (x & 2) != 3)

// XK_Shift_L / XK_Shift_R, XK_Caps_Lock, XK_Control_L / XK_Control_R, XK_Alt_L / XK_Alt_R, XK_Num_Lock
static U16 modifiers[16] = { 42, 53, 58, 0, 29, 97, 56, 100, 97, 0, 0, 0, 0, 0, 0, 0 };

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
		keySymToInfo.set(usKeyBoard[i].keySymWithShift, &usKeyBoard[i]);
	}
}

void XKeyboard::getMinMaxKeycodes(S32& minKeycode, S32& maxKeycode) {
	minKeycode = minKeyCode;
	maxKeycode = maxKeyCode;
}

const U16* XKeyboard::getModifiers(U32& numberOfModifiers, U32& numberOfKeysPerModifier) {
	numberOfModifiers = NUMBER_OF_MODIFIERS;
	numberOfKeysPerModifier = MAX_KEYS_PER_MODIFIER;
	return modifiers;
}

U32 XKeyboard::keycodeToKeysym(U32 keycode, U32 level) {
	const KeyInfo* info = nullptr;
	U16 key = (U16)keycode;

	if (keyCodeToInfo.get(key, info)) {
		if (level == 0) {
			return info->keySym;
		} else if (level == 1) {
			return info->keySymWithShift;
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

U32 XKeyboard::translate(U32& keysym, U32 modifiers, char* buffer, U32 bufferLen) {
	U16 key = (U16)keysym;
	const KeyInfo* info = nullptr;

	if (!keySymToInfo.get(key, info)) {
		return 0;
	}
	if (IS_SHIFT(modifiers) || keysym == info->keySymWithShift) {
		keysym = info->keySymWithShift;
		if (bufferLen) {
			*buffer = info->chShift;
		}
		return 1;
	}
	if (modifiers & 0xFC) {
		kwarn("XKeyboard::translate modifier = %x not handled", modifiers);
	}
	if (bufferLen) {
		*buffer = info->ch;
	}
	return 1;
}