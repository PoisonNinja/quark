#pragma once

#include <types.h>

namespace input
{
// Borrowed from Linux
constexpr int KEY_INVALID    = 0;
constexpr int KEY_ESC        = 1;
constexpr int KEY_1          = 2;
constexpr int KEY_2          = 3;
constexpr int KEY_3          = 4;
constexpr int KEY_4          = 5;
constexpr int KEY_5          = 6;
constexpr int KEY_6          = 7;
constexpr int KEY_7          = 8;
constexpr int KEY_8          = 9;
constexpr int KEY_9          = 10;
constexpr int KEY_0          = 11;
constexpr int KEY_MINUS      = 12;
constexpr int KEY_EQUAL      = 13;
constexpr int KEY_BACKSPACE  = 14;
constexpr int KEY_TAB        = 15;
constexpr int KEY_Q          = 16;
constexpr int KEY_W          = 17;
constexpr int KEY_E          = 18;
constexpr int KEY_R          = 19;
constexpr int KEY_T          = 20;
constexpr int KEY_Y          = 21;
constexpr int KEY_U          = 22;
constexpr int KEY_I          = 23;
constexpr int KEY_O          = 24;
constexpr int KEY_P          = 25;
constexpr int KEY_LEFTBRACE  = 26;
constexpr int KEY_RIGHTBRACE = 27;
constexpr int KEY_ENTER      = 28;
constexpr int KEY_LEFTCTRL   = 29;
constexpr int KEY_A          = 30;
constexpr int KEY_S          = 31;
constexpr int KEY_D          = 32;
constexpr int KEY_F          = 33;
constexpr int KEY_G          = 34;
constexpr int KEY_H          = 35;
constexpr int KEY_J          = 36;
constexpr int KEY_K          = 37;
constexpr int KEY_L          = 38;
constexpr int KEY_SEMICOLON  = 39;
constexpr int KEY_APOSTROPHE = 40;
constexpr int KEY_GRAVE      = 41;
constexpr int KEY_LEFTSHIFT  = 42;
constexpr int KEY_BACKSLASH  = 43;
constexpr int KEY_Z          = 44;
constexpr int KEY_X          = 45;
constexpr int KEY_C          = 46;
constexpr int KEY_V          = 47;
constexpr int KEY_B          = 48;
constexpr int KEY_N          = 49;
constexpr int KEY_M          = 50;
constexpr int KEY_COMMA      = 51;
constexpr int KEY_DOT        = 52;
constexpr int KEY_SLASH      = 53;
constexpr int KEY_RIGHTSHIFT = 54;
constexpr int KEY_KPASTERISK = 55;
constexpr int KEY_LEFTALT    = 56;
constexpr int KEY_SPACE      = 57;
constexpr int KEY_CAPSLOCK   = 58;
constexpr int KEY_F1         = 59;
constexpr int KEY_F2         = 60;
constexpr int KEY_F3         = 61;
constexpr int KEY_F4         = 62;
constexpr int KEY_F5         = 63;
constexpr int KEY_F6         = 64;
constexpr int KEY_F7         = 65;
constexpr int KEY_F8         = 66;
constexpr int KEY_F9         = 67;
constexpr int KEY_F10        = 68;
constexpr int KEY_NUMLOCK    = 69;
constexpr int KEY_SCROLLLOCK = 70;
constexpr int KEY_KP7        = 71;
constexpr int KEY_KP8        = 72;
constexpr int KEY_KP9        = 73;
constexpr int KEY_KPMINUS    = 74;
constexpr int KEY_KP4        = 75;
constexpr int KEY_KP5        = 76;
constexpr int KEY_KP6        = 77;
constexpr int KEY_KPPLUS     = 78;
constexpr int KEY_KP1        = 79;
constexpr int KEY_KP2        = 80;
constexpr int KEY_KP3        = 81;
constexpr int KEY_KP0        = 82;
constexpr int KEY_KPDOT      = 83;

constexpr int KEY_ZENKAKUHANKAKU   = 85;
constexpr int KEY_102ND            = 86;
constexpr int KEY_F11              = 87;
constexpr int KEY_F12              = 88;
constexpr int KEY_RO               = 89;
constexpr int KEY_KATAKANA         = 90;
constexpr int KEY_HIRAGANA         = 91;
constexpr int KEY_HENKAN           = 92;
constexpr int KEY_KATAKANAHIRAGANA = 93;
constexpr int KEY_MUHENKAN         = 94;
constexpr int KEY_KPJPCOMMA        = 95;
constexpr int KEY_KPENTER          = 96;
constexpr int KEY_RIGHTCTRL        = 97;
constexpr int KEY_KPSLASH          = 98;
constexpr int KEY_SYSRQ            = 99;
constexpr int KEY_RIGHTALT         = 100;
constexpr int KEY_LINEFEED         = 101;
constexpr int KEY_HOME             = 102;
constexpr int KEY_UP               = 103;
constexpr int KEY_PAGEUP           = 104;
constexpr int KEY_LEFT             = 105;
constexpr int KEY_RIGHT            = 106;
constexpr int KEY_END              = 107;
constexpr int KEY_DOWN             = 108;
constexpr int KEY_PAGEDOWN         = 109;
constexpr int KEY_INSERT           = 110;
constexpr int KEY_DELETE           = 111;
constexpr int KEY_MACRO            = 112;
constexpr int KEY_MUTE             = 113;
constexpr int KEY_VOLUMEDOWN       = 114;
constexpr int KEY_VOLUMEUP         = 115;
constexpr int KEY_POWER            = 116; /* SC System Power Down */
constexpr int KEY_KPEQUAL          = 117;
constexpr int KEY_KPPLUSMINUS      = 118;
constexpr int KEY_PAUSE            = 119;
constexpr int KEY_SCALE            = 120; /* AL Compiz Scale (Expose) */

constexpr int KEY_KPCOMMA   = 121;
constexpr int KEY_HANGEUL   = 122;
constexpr int KEY_HANGUEL   = KEY_HANGEUL;
constexpr int KEY_HANJA     = 123;
constexpr int KEY_YEN       = 124;
constexpr int KEY_LEFTMETA  = 125;
constexpr int KEY_RIGHTMETA = 126;
constexpr int KEY_COMPOSE   = 127;

constexpr int KEY_STOP       = 128; /* AC Stop */
constexpr int KEY_AGAIN      = 129;
constexpr int KEY_PROPS      = 130; /* AC Properties */
constexpr int KEY_UNDO       = 131; /* AC Undo */
constexpr int KEY_FRONT      = 132;
constexpr int KEY_COPY       = 133; /* AC Copy */
constexpr int KEY_OPEN       = 134; /* AC Open */
constexpr int KEY_PASTE      = 135; /* AC Paste */
constexpr int KEY_FIND       = 136; /* AC Search */
constexpr int KEY_CUT        = 137; /* AC Cut */
constexpr int KEY_HELP       = 138; /* AL Integrated Help Center */
constexpr int KEY_MENU       = 139; /* Menu (show menu) */
constexpr int KEY_CALC       = 140; /* AL Calculator */
constexpr int KEY_SETUP      = 141;
constexpr int KEY_SLEEP      = 142; /* SC System Sleep */
constexpr int KEY_WAKEUP     = 143; /* System Wake Up */
constexpr int KEY_FILE       = 144; /* AL Local Machine Browser */
constexpr int KEY_SENDFILE   = 145;
constexpr int KEY_DELETEFILE = 146;
constexpr int KEY_XFER       = 147;
constexpr int KEY_PROG1      = 148;
constexpr int KEY_PROG2      = 149;
constexpr int KEY_WWW        = 150; /* AL Internet Browser */
constexpr int KEY_MSDOS      = 151;
constexpr int KEY_COFFEE     = 152; /* AL Terminal Lock/Screensaver */
constexpr int KEY_SCREENLOCK = KEY_COFFEE;
constexpr int KEY_ROTATE_DISPLAY =
    153; /* Display orientation for e.g. tablets */
constexpr int KEY_DIRECTION    = KEY_ROTATE_DISPLAY;
constexpr int KEY_CYCLEWINDOWS = 154;
constexpr int KEY_MAIL         = 155;
constexpr int KEY_BOOKMARKS    = 156; /* AC Bookmarks */
constexpr int KEY_COMPUTER     = 157;
constexpr int KEY_BACK         = 158; /* AC Back */
constexpr int KEY_FORWARD      = 159; /* AC Forward */
constexpr int KEY_CLOSECD      = 160;
constexpr int KEY_EJECTCD      = 161;
constexpr int KEY_EJECTCLOSECD = 162;
constexpr int KEY_NEXTSONG     = 163;
constexpr int KEY_PLAYPAUSE    = 164;
constexpr int KEY_PREVIOUSSONG = 165;
constexpr int KEY_STOPCD       = 166;
constexpr int KEY_RECORD       = 167;
constexpr int KEY_REWIND       = 168;
constexpr int KEY_PHONE        = 169; /* Media Select Telephone */
constexpr int KEY_ISO          = 170;
constexpr int KEY_CONFIG       = 171; /* AL Consumer Control Configuration */
constexpr int KEY_HOMEPAGE     = 172; /* AC Home */
constexpr int KEY_REFRESH      = 173; /* AC Refresh */
constexpr int KEY_EXIT         = 174; /* AC Exit */
constexpr int KEY_MOVE         = 175;
constexpr int KEY_EDIT         = 176;
constexpr int KEY_SCROLLUP     = 177;
constexpr int KEY_SCROLLDOWN   = 178;
constexpr int KEY_KPLEFTPAREN  = 179;
constexpr int KEY_KPRIGHTPAREN = 180;
constexpr int KEY_NEW          = 181; /* AC New */
constexpr int KEY_REDO         = 182; /* AC Redo/Repeat */

constexpr int KEY_F13 = 183;
constexpr int KEY_F14 = 184;
constexpr int KEY_F15 = 185;
constexpr int KEY_F16 = 186;
constexpr int KEY_F17 = 187;
constexpr int KEY_F18 = 188;
constexpr int KEY_F19 = 189;
constexpr int KEY_F20 = 190;
constexpr int KEY_F21 = 191;
constexpr int KEY_F22 = 192;
constexpr int KEY_F23 = 193;
constexpr int KEY_F24 = 194;

constexpr int KEY_PLAYCD         = 200;
constexpr int KEY_PAUSECD        = 201;
constexpr int KEY_PROG3          = 202;
constexpr int KEY_PROG4          = 203;
constexpr int KEY_DASHBOARD      = 204; /* AL Dashboard */
constexpr int KEY_SUSPEND        = 205;
constexpr int KEY_CLOSE          = 206; /* AC Close */
constexpr int KEY_PLAY           = 207;
constexpr int KEY_FASTFORWARD    = 208;
constexpr int KEY_BASSBOOST      = 209;
constexpr int KEY_PRINT          = 210; /* AC Print */
constexpr int KEY_HP             = 211;
constexpr int KEY_CAMERA         = 212;
constexpr int KEY_SOUND          = 213;
constexpr int KEY_QUESTION       = 214;
constexpr int KEY_EMAIL          = 215;
constexpr int KEY_CHAT           = 216;
constexpr int KEY_SEARCH         = 217;
constexpr int KEY_CONNECT        = 218;
constexpr int KEY_FINANCE        = 219; /* AL Checkbook/Finance */
constexpr int KEY_SPORT          = 220;
constexpr int KEY_SHOP           = 221;
constexpr int KEY_ALTERASE       = 222;
constexpr int KEY_CANCEL         = 223; /* AC Cancel */
constexpr int KEY_BRIGHTNESSDOWN = 224;
constexpr int KEY_BRIGHTNESSUP   = 225;
constexpr int KEY_MEDIA          = 226;

constexpr int KEY_SWITCHVIDEOMODE = 227; /* Cycle between available video \
                                 outputs (Monitor/LCD/TV-out/etc) */
constexpr int KEY_KBDILLUMTOGGLE = 228;
constexpr int KEY_KBDILLUMDOWN   = 229;
constexpr int KEY_KBDILLUMUP     = 230;

constexpr int KEY_SEND        = 231; /* AC Send */
constexpr int KEY_REPLY       = 232; /* AC Reply */
constexpr int KEY_FORWARDMAIL = 233; /* AC Forward Msg */
constexpr int KEY_SAVE        = 234; /* AC Save */
constexpr int KEY_DOCUMENTS   = 235;

constexpr int KEY_BATTERY = 236;

constexpr int KEY_BLUETOOTH = 237;
constexpr int KEY_WLAN      = 238;
constexpr int KEY_UWB       = 239;

constexpr int KEY_UNKNOWN = 240;

constexpr int KEY_VIDEO_NEXT       = 241; /* drive next video source */
constexpr int KEY_VIDEO_PREV       = 242; /* drive previous video source */
constexpr int KEY_BRIGHTNESS_CYCLE = 243; /* brightness up, after max is min */
constexpr int KEY_BRIGHTNESS_AUTO =
    244; /* Set Auto Brightness: manual \
 brightness control is off,  \  rely on ambient */
constexpr int KEY_BRIGHTNESS_ZERO = KEY_BRIGHTNESS_AUTO;
constexpr int KEY_DISPLAY_OFF     = 245; /* display device to off state */

constexpr int KEY_WWAN   = 246; /* Wireless WAN (LTE, UMTS, GSM, etc.) */
constexpr int KEY_WIMAX  = KEY_WWAN;
constexpr int KEY_RFKILL = 247; /* Key that controls all radios */

constexpr int KEY_MICMUTE = 248; /* Mute / unmute the microphone */

extern uint16_t base_map[];
extern uint16_t shift_map[];

constexpr int NUM_CODES = 128;
} // namespace input
