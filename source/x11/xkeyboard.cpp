#include "boxedwine.h"
#include "xkeyboard.h"
#include <SDL.h>

// http://www.techhelpmanual.com/57-keyboard_scan_codes.html

struct KeyInfo {
	U16 keyCode;
	U16 keyCodeWithShift;
	U16 scanCode;
	U16 sdlScanCode;
	U16 keySym;
	U16 keySymWithShift;
};

static const KeyInfo usKeyBoard[] =
{
	{ 0x1b, 0x1b, 0x01, SDL_SCANCODE_ESCAPE, XK_Escape, XK_Escape }, 
	{ 0x00, 0x00, 0x3B, SDL_SCANCODE_F1, XK_F1, XK_F1 },
	{ 0x00, 0x00, 0x3C, SDL_SCANCODE_F2, XK_F2, XK_F2 },
	{ 0x00, 0x00, 0x3D, SDL_SCANCODE_F3, XK_F3, XK_F3 },
	{ 0x00, 0x00, 0x3E, SDL_SCANCODE_F4, XK_F4, XK_F4 },
	{ 0x00, 0x00, 0x3F, SDL_SCANCODE_F5, XK_F5, XK_F5 },
	{ 0x00, 0x00, 0x40, SDL_SCANCODE_F6, XK_F6, XK_F6 },
	{ 0x00, 0x00, 0x41, SDL_SCANCODE_F7, XK_F7, XK_F7 },
	{ 0x00, 0x00, 0x42, SDL_SCANCODE_F8, XK_F8, XK_F8 },
	{ 0x00, 0x00, 0x42, SDL_SCANCODE_F9, XK_F9, XK_F9 },
	{ 0x00, 0x00, 0x43, SDL_SCANCODE_F10, XK_F10, XK_F10 },
	{ 0x00, 0x00, 0x57, SDL_SCANCODE_F11, XK_F11, XK_F11 },
	{ 0x00, 0x00, 0x58, SDL_SCANCODE_F12, XK_F12, XK_F12 },
	{ 0x60, 0x7e, 0x29, SDL_SCANCODE_GRAVE, XK_grave, XK_asciitilde },
	{ 0x31, 0x21, 0x02, SDL_SCANCODE_1, XK_1, XK_exclam },
	{ 0x32, 0x40, 0x03, SDL_SCANCODE_2, XK_2, XK_ampersand },
	{ 0x33, 0x23, 0x04, SDL_SCANCODE_3, XK_3, XK_numbersign },
	{ 0x34, 0x24, 0x05, SDL_SCANCODE_4, XK_4, XK_dollar },
	{ 0x35, 0x25, 0x06, SDL_SCANCODE_5, XK_5, XK_percent },
	{ 0x36, 0x5e, 0x07, SDL_SCANCODE_6, XK_6, XK_asciicircum },
	{ 0x37, 0x26, 0x08, SDL_SCANCODE_7, XK_7, XK_ampersand },
	{ 0x38, 0x2a, 0x09, SDL_SCANCODE_8, XK_8, XK_asterisk },
	{ 0x39, 0x28, 0x0A, SDL_SCANCODE_9, XK_9, XK_parenleft },
	{ 0x30, 0x29, 0x0B, SDL_SCANCODE_0, XK_0, XK_parenright },
	{ 0x2d, 0x5f, 0x0C, SDL_SCANCODE_MINUS, XK_minus, XK_underscore },
	{ 0x3d, 0x2b, 0x0D, SDL_SCANCODE_EQUALS, XK_equal, XK_plus },
	{ 0x7f, 0x7f, 0x0E, SDL_SCANCODE_BACKSPACE, XK_BackSpace, XK_BackSpace },
	{ 0x09, 0x09, 0x0F, SDL_SCANCODE_TAB, XK_Tab, XK_ISO_Left_Tab },
	{ 0x71, 0x51, 0x10, SDL_SCANCODE_Q, XK_q, XK_Q },
	{ 0x77, 0x57, 0x11, SDL_SCANCODE_W, XK_w, XK_W },
	{ 0x65, 0x45, 0x12, SDL_SCANCODE_E, XK_e, XK_E },
	{ 0x72, 0x52, 0x13, SDL_SCANCODE_R, XK_r, XK_R },
	{ 0x74, 0x54, 0x14, SDL_SCANCODE_T, XK_t, XK_T },
	{ 0x79, 0x59, 0x15, SDL_SCANCODE_Y, XK_y, XK_Y },
	{ 0x75, 0x55, 0x16, SDL_SCANCODE_U, XK_u, XK_U },
	{ 0x69, 0x49, 0x17, SDL_SCANCODE_I, XK_i, XK_I },
	{ 0x6f, 0x4f, 0x18, SDL_SCANCODE_O, XK_o, XK_O },
	{ 0x70, 0x50, 0x19, SDL_SCANCODE_P, XK_p, XK_P },
	{ 0x5b, 0x7b, 0x1A, SDL_SCANCODE_LEFTBRACKET, XK_bracketleft, XK_braceleft },
	{ 0x5d, 0x7d, 0x1B, SDL_SCANCODE_RIGHTBRACKET, XK_bracketright, XK_braceright },
	{ 0x5c, 0x7c, 0x2B, SDL_SCANCODE_BACKSLASH, XK_backslash, XK_bar },
	{ 0x00, 0x00, 0x3A, SDL_SCANCODE_CAPSLOCK, XK_Caps_Lock, XK_Caps_Lock },
	{ 0x61, 0x41, 0x1E, SDL_SCANCODE_A, XK_a, XK_A },
	{ 0x73, 0x53, 0x1F, SDL_SCANCODE_S, XK_s, XK_S },
	{ 0x64, 0x44, 0x20, SDL_SCANCODE_D, XK_d, XK_D },
	{ 0x66, 0x46, 0x21, SDL_SCANCODE_F, XK_f, XK_F },
	{ 0x67, 0x47, 0x22, SDL_SCANCODE_G, XK_g, XK_G },
	{ 0x68, 0x48, 0x23, SDL_SCANCODE_H, XK_h, XK_H },
	{ 0x6a, 0x4a, 0x24, SDL_SCANCODE_J, XK_j, XK_J },
	{ 0x6b, 0x4b, 0x25, SDL_SCANCODE_K, XK_k, XK_K },
	{ 0x6c, 0x4c, 0x26, SDL_SCANCODE_L, XK_l, XK_L },
	{ 0x3b, 0x3a, 0x27, SDL_SCANCODE_SEMICOLON, XK_semicolon, XK_colon },
	{ 0x27, 0x22, 0x28, SDL_SCANCODE_APOSTROPHE, XK_apostrophe, XK_quotedbl },
	{ 0x0d, 0x0d, 0x1C, SDL_SCANCODE_RETURN, XK_Return, XK_Return },
	{ 0x00, 0x00, 0x2a, SDL_SCANCODE_LSHIFT, XK_Shift_L, XK_Shift_L },
	{ 0x7a, 0x5a, 0x2C, SDL_SCANCODE_Z, XK_z, XK_Z },
	{ 0x78, 0x58, 0x2D, SDL_SCANCODE_X, XK_x, XK_X },
	{ 0x63, 0x43, 0x2E, SDL_SCANCODE_C, XK_c, XK_C },
	{ 0x76, 0x56, 0x2F, SDL_SCANCODE_V, XK_v, XK_V },
	{ 0x62, 0x42, 0x30, SDL_SCANCODE_B, XK_b, XK_B },
	{ 0x6e, 0x4e, 0x31, SDL_SCANCODE_N, XK_n, XK_N },
	{ 0x6d, 0x4d, 0x32, SDL_SCANCODE_M, XK_m, XK_M },
	{ 0x2c, 0x3c, 0x33, SDL_SCANCODE_COMMA, XK_comma, XK_less },
	{ 0x2e, 0x3e, 0x34, SDL_SCANCODE_PERIOD, XK_period, XK_greater },
	{ 0x2f, 0x3f, 0x35, SDL_SCANCODE_SLASH, XK_slash, XK_question },
	{ 0x00, 0x00, 0x36, SDL_SCANCODE_RSHIFT, XK_Shift_R, XK_Shift_R },
	{ 0x00, 0x00, 0x1d, SDL_SCANCODE_LCTRL, XK_Control_L, XK_Control_L },
	{ 0x00, 0x00, 0xE05B, SDL_SCANCODE_LGUI, XK_Meta_L, XK_Meta_L },
	{ 0x00, 0x00, 0x38, SDL_SCANCODE_LALT, XK_Alt_L, XK_Alt_L },
	{ 0x20, 0x00, 0x39, SDL_SCANCODE_SPACE, XK_space, XK_space },
	{ 0x00, 0x00, 0xE038, SDL_SCANCODE_RALT, XK_Alt_R, XK_Alt_R },
	{ 0x00, 0x00, 0xE05C, SDL_SCANCODE_RGUI, XK_Meta_R, XK_Meta_R },
	{ 0x00, 0x00, 0xE01D, SDL_SCANCODE_RCTRL, XK_Control_R, XK_Control_R }
	//0xe02ae037e0aae0b7, SDL_SCANCODE_PRINTSCREEN, XK_Print, XK_Print, 0x46, SDL_SCANCODE_SCROLLLOCK, XK_Scroll_Lock, XK_Scroll_Lock, 0xE11D45, SDL_SCANCODE_PAUSE, XK_Pause, XK_Pause,
};

static BHashTable<U16, const KeyInfo*> keyCodeToInfo;
static U16 minKeyCode;
static U16 maxKeyCode;

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
		if (usKeyBoard[i].keyCodeWithShift != 0) {
			if (usKeyBoard[i].keyCodeWithShift < minKeyCode) {
				minKeyCode = usKeyBoard[i].keyCodeWithShift;
			}
			if (usKeyBoard[i].keyCodeWithShift > maxKeyCode) {
				maxKeyCode = usKeyBoard[i].keyCodeWithShift;
			}
		}
		keyCodeToInfo.set(usKeyBoard[i].keyCode, &usKeyBoard[i]);
	}
}

void XKeyboard::getMinMaxKeycodes(S32& minKeycode, S32& maxKeycode) {
	minKeycode = minKeyCode;
	maxKeycode = maxKeyCode;
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