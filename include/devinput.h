/*
 *  Copyright (C) 2016  The BoxedWine Team
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __DEVINPUT_H__
#define __DEVINPUT_H__

FsOpenNode* openDevInputTouch(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);
FsOpenNode* openDevInputMouse(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);
FsOpenNode* openDevInputKeyboard(const std::shared_ptr<FsNode>& node, U32 flags, U32 data);

void onMouseMove(U32 x, U32 y, bool relative);
void onMouseButtonDown(U32 button);
void onMouseButtonUp(U32 button);
void onMouseWheel(S32 value);
void onKeyDown(U32 code);
void onKeyUp(U32 code);

#define K_EV_SYN                 0x00
#define K_EV_KEY                 0x01
#define K_EV_REL                 0x02
#define K_EV_ABS                 0x03
#define K_EV_MSC                 0x04
#define K_EV_SW                  0x05
#define K_EV_LED                 0x11
#define K_EV_SND                 0x12
#define K_EV_REP                 0x14
#define K_EV_FF                  0x15
#define K_EV_PWR                 0x16
#define K_EV_FF_STATUS           0x17
#define K_EV_MAX                 0x1f

#define K_SYN_REPORT             0
#define K_SYN_CONFIG             1
#define K_SYN_MT_REPORT          2
#define K_SYN_DROPPED            3

#define K_ABS_X                  0x00
#define K_ABS_Y                  0x01

#define K_REL_X                  0x00
#define K_REL_Y                  0x01

#define K_BTN_MOUSE               0x110
#define K_BTN_LEFT                0x110
#define K_BTN_RIGHT               0x111
#define K_BTN_MIDDLE              0x112
#define K_BTN_SIDE                0x113
#define K_BTN_EXTRA               0x114
#define K_BTN_FORWARD             0x115
#define K_BTN_BACK                0x116
#define K_BTN_TASK                0x117

#define K_BTN_MOUSEWHEEL_UP       0x118  // Probably wrong?
#define K_BTN_MOUSEWHEEL_DOWN     0x119

#define K_KEY_RESERVED            0
#define K_KEY_ESC                 1
#define K_KEY_1                   2
#define K_KEY_2                   3
#define K_KEY_3                   4
#define K_KEY_4                   5
#define K_KEY_5                   6
#define K_KEY_6                   7
#define K_KEY_7                   8
#define K_KEY_8                   9
#define K_KEY_9                   10
#define K_KEY_0                   11
#define K_KEY_MINUS               12
#define K_KEY_EQUAL               13
#define K_KEY_BACKSPACE           14
#define K_KEY_TAB                 15
#define K_KEY_Q                   16
#define K_KEY_W                   17
#define K_KEY_E                   18
#define K_KEY_R                   19
#define K_KEY_T                   20
#define K_KEY_Y                   21
#define K_KEY_U                   22
#define K_KEY_I                   23
#define K_KEY_O                   24
#define K_KEY_P                   25
#define K_KEY_LEFTBRACE           26
#define K_KEY_RIGHTBRACE          27
#define K_KEY_ENTER               28
#define K_KEY_LEFTCTRL            29
#define K_KEY_A                   30
#define K_KEY_S                   31
#define K_KEY_D                   32
#define K_KEY_F                   33
#define K_KEY_G                   34
#define K_KEY_H                   35
#define K_KEY_J                   36
#define K_KEY_K                   37
#define K_KEY_L                   38
#define K_KEY_SEMICOLON           39
#define K_KEY_APOSTROPHE          40
#define K_KEY_GRAVE               41
#define K_KEY_LEFTSHIFT           42
#define K_KEY_BACKSLASH           43
#define K_KEY_Z                   44
#define K_KEY_X                   45
#define K_KEY_C                   46
#define K_KEY_V                   47
#define K_KEY_B                   48
#define K_KEY_N                   49
#define K_KEY_M                   50
#define K_KEY_COMMA               51
#define K_KEY_DOT                 52
#define K_KEY_SLASH               53
#define K_KEY_RIGHTSHIFT          54
#define K_KEY_KPASTERISK          55
#define K_KEY_LEFTALT             56
#define K_KEY_SPACE               57
#define K_KEY_CAPSLOCK            58
#define K_KEY_F1                  59
#define K_KEY_F2                  60
#define K_KEY_F3                  61
#define K_KEY_F4                  62
#define K_KEY_F5                  63
#define K_KEY_F6                  64
#define K_KEY_F7                  65
#define K_KEY_F8                  66
#define K_KEY_F9                  67
#define K_KEY_F10                 68
#define K_KEY_NUMLOCK             69
#define K_KEY_SCROLLLOCK          70
#define K_KEY_KP7                 71
#define K_KEY_KP8                 72
#define K_KEY_KP9                 73
#define K_KEY_KPMINUS             74
#define K_KEY_KP4                 75
#define K_KEY_KP5                 76
#define K_KEY_KP6                 77
#define K_KEY_KPPLUS              78
#define K_KEY_KP1                 79
#define K_KEY_KP2                 80
#define K_KEY_KP3                 81
#define K_KEY_KP0                 82
#define K_KEY_KPDOT               83

#define K_KEY_ZENKAKUHANKAKU      85
#define K_KEY_102ND               86
#define K_KEY_F11                 87

#define K_KEY_F12                 88
#define K_KEY_RO                  89
#define K_KEY_KATAKANA            90
#define K_KEY_HIRAGANA            91
#define K_KEY_HENKAN              92
#define K_KEY_KATAKANAHIRAGANA    93
#define K_KEY_MUHENKAN            94
#define K_KEY_KPJPCOMMA           95
#define K_KEY_KPENTER             96
#define K_KEY_RIGHTCTRL           97
#define K_KEY_KPSLASH             98
#define K_KEY_SYSRQ               99
#define K_KEY_RIGHTALT            100
#define K_KEY_LINEFEED            101
#define K_KEY_HOME                102
#define K_KEY_UP                  103
#define K_KEY_PAGEUP              104
#define K_KEY_LEFT                105
#define K_KEY_RIGHT               106
#define K_KEY_END                 107
#define K_KEY_DOWN                108
#define K_KEY_PAGEDOWN            109
#define K_KEY_INSERT              110
#define K_KEY_DELETE              111
#define K_KEY_MACRO               112
#define K_KEY_MUTE                113
#define K_KEY_VOLUMEDOWN          114
#define K_KEY_VOLUMEUP            115
#define K_KEY_POWER               116     /* SC System Power Down */
#define K_KEY_KPEQUAL             117
#define K_KEY_KPPLUSMINUS         118
#define K_KEY_PAUSE               119
#define K_KEY_SCALE               120     /* AL Compiz Scale (Expose) */

#define K_KEY_KPCOMMA             121
#define K_KEY_HANGEUL             122
#define K_KEY_HANGUEL             KEY_HANGEUL
#define K_KEY_HANJA               123
#define K_KEY_YEN                 124
#define K_KEY_LEFTMETA            125
#define K_KEY_RIGHTMETA           126
#define K_KEY_COMPOSE             127

#define K_KEY_STOP                128     /* AC Stop */
#define K_KEY_AGAIN               129

#define K_KEY_PROPS               130     /* AC Properties */
#define K_KEY_UNDO                131     /* AC Undo */
#define K_KEY_FRONT               132
#define K_KEY_COPY                133     /* AC Copy */
#define K_KEY_OPEN                134     /* AC Open */
#define K_KEY_PASTE               135     /* AC Paste */
#define K_KEY_FIND                136     /* AC Search */
#define K_KEY_CUT                 137     /* AC Cut */
#define K_KEY_HELP                138     /* AL Integrated Help Center */
#define K_KEY_MENU                139     /* Menu (show menu) */
#define K_KEY_CALC                140     /* AL Calculator */
#define K_KEY_SETUP               141
#define K_KEY_SLEEP               142     /* SC System Sleep */
#define K_KEY_WAKEUP              143     /* System Wake Up */
#define K_KEY_FILE                144     /* AL Local Machine Browser */
#define K_KEY_SENDFILE            145
#define K_KEY_DELETEFILE          146
#define K_KEY_XFER                147
#define K_KEY_PROG1               148
#define K_KEY_PROG2               149
#define K_KEY_WWW                 150     /* AL Internet Browser */
#define K_KEY_MSDOS               151
#define K_KEY_COFFEE              152     /* AL Terminal Lock/Screensaver */
#define K_KEY_SCREENLOCK          KEY_COFFEE
#define K_KEY_DIRECTION           153
#define K_KEY_CYCLEWINDOWS        154
#define K_KEY_MAIL                155
#define K_KEY_BOOKMARKS           156     /* AC Bookmarks */
#define K_KEY_COMPUTER            157
#define K_KEY_BACK                158     /* AC Back */
#define K_KEY_FORWARD             159     /* AC Forward */
#define K_KEY_CLOSECD             160
#define K_KEY_EJECTCD             161
#define K_KEY_EJECTCLOSECD        162
#define K_KEY_NEXTSONG            163
#define K_KEY_PLAYPAUSE           164
#define K_KEY_PREVIOUSSONG        165
#define K_KEY_STOPCD              166
#define K_KEY_RECORD              167
#define K_KEY_REWIND              168
#define K_KEY_PHONE               169     /* Media Select Telephone */
#define K_KEY_ISO                 170
#define K_KEY_CONFIG              171     /* AL Consumer Control Configuration */
#define K_KEY_HOMEPAGE            172     /* AC Home */
#define K_KEY_REFRESH             173     /* AC Refresh */
#define K_KEY_EXIT                174     /* AC Exit */
#define K_KEY_MOVE                175
#define K_KEY_EDIT                176
#define K_KEY_SCROLLUP            177
#define K_KEY_SCROLLDOWN          178
#define K_KEY_KPLEFTPAREN         179
#define K_KEY_KPRIGHTPAREN        180
#define K_KEY_NEW                 181     /* AC New */
#define K_KEY_REDO                182     /* AC Redo/Repeat */

#define K_KEY_F13                 183
#define K_KEY_F14                 184
#define K_KEY_F15                 185
#define K_KEY_F16                 186
#define K_KEY_F17                 187
#define K_KEY_F18                 188
#define K_KEY_F19                 189
#define K_KEY_F20                 190
#define K_KEY_F21                 191
#define K_KEY_F22                 192
#define K_KEY_F23                 193
#define K_KEY_F24                 194

#define K_KEY_PLAYCD              200
#define K_KEY_PAUSECD             201
#define K_KEY_PROG3               202
#define K_KEY_PROG4               203
#define K_KEY_DASHBOARD           204     /* AL Dashboard */
#define K_KEY_SUSPEND             205
#define K_KEY_CLOSE               206     /* AC Close */
#define K_KEY_PLAY                207
#define K_KEY_FASTFORWARD         208
#define K_KEY_BASSBOOST           209
#define K_KEY_PRINT               210     /* AC Print */
#define K_KEY_HP                  211
#define K_KEY_CAMERA              212
#define K_KEY_SOUND               213
#define K_KEY_QUESTION            214
#define K_KEY_EMAIL               215
#define K_KEY_CHAT                216
#define K_KEY_SEARCH              217
#define K_KEY_CONNECT             218
#define K_KEY_FINANCE             219     /* AL Checkbook/Finance */
#define K_KEY_SPORT               220
#define K_KEY_SHOP                221
#define K_KEY_ALTERASE            222
#define K_KEY_CANCEL              223     /* AC Cancel */
#define K_KEY_BRIGHTNESSDOWN      224
#define K_KEY_BRIGHTNESSUP        225
#define K_KEY_MEDIA               226

#define K_KEY_SWITCHVIDEOMODE     227     /* Cycle between available video
                                outputs (Monitor/LCD/TV-out/etc) */
#define K_KEY_KBDILLUMTOGGLE      228
#define K_KEY_KBDILLUMDOWN        229
#define K_KEY_KBDILLUMUP          230

#define K_KEY_SEND                231     /* AC Send */
#define K_KEY_REPLY               232     /* AC Reply */
#define K_KEY_FORWARDMAIL         233     /* AC Forward Msg */
#define K_KEY_SAVE                234     /* AC Save */
#define K_KEY_DOCUMENTS           235

#define K_KEY_BATTERY             236

#define K_KEY_BLUETOOTH           237
#define K_KEY_WLAN                238
#define K_KEY_UWB                 239

#define K_KEY_UNKNOWN             240

#define K_KEY_VIDEO_NEXT          241     /* drive next video source */
#define K_KEY_VIDEO_PREV          242     /* drive previous video source */
#define K_KEY_BRIGHTNESS_CYCLE    243     /* brightness up, after max is min */
#define K_KEY_BRIGHTNESS_ZERO     244     /* brightness off, use ambient */
#define K_KEY_DISPLAY_OFF         245     /* display device to off state */

#define K_KEY_WIMAX               246
#define K_KEY_RFKILL              247     /* Key that controls all radios */

#define K_KEY_MICMUTE             248     /* Mute / unmute the microphone */

#endif
