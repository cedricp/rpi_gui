#ifndef KEYCODE_H
#define KEYCODE_H

enum KB_Scancode
{
    KB_SCANCODE_UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    KB_SCANCODE_A = 4,
    KB_SCANCODE_B = 5,
    KB_SCANCODE_C = 6,
    KB_SCANCODE_D = 7,
    KB_SCANCODE_E = 8,
    KB_SCANCODE_F = 9,
    KB_SCANCODE_G = 10,
    KB_SCANCODE_H = 11,
    KB_SCANCODE_I = 12,
    KB_SCANCODE_J = 13,
    KB_SCANCODE_K = 14,
    KB_SCANCODE_L = 15,
    KB_SCANCODE_M = 16,
    KB_SCANCODE_N = 17,
    KB_SCANCODE_O = 18,
    KB_SCANCODE_P = 19,
    KB_SCANCODE_Q = 20,
    KB_SCANCODE_R = 21,
    KB_SCANCODE_S = 22,
    KB_SCANCODE_T = 23,
    KB_SCANCODE_U = 24,
    KB_SCANCODE_V = 25,
    KB_SCANCODE_W = 26,
    KB_SCANCODE_X = 27,
    KB_SCANCODE_Y = 28,
    KB_SCANCODE_Z = 29,

    KB_SCANCODE_1 = 30,
    KB_SCANCODE_2 = 31,
    KB_SCANCODE_3 = 32,
    KB_SCANCODE_4 = 33,
    KB_SCANCODE_5 = 34,
    KB_SCANCODE_6 = 35,
    KB_SCANCODE_7 = 36,
    KB_SCANCODE_8 = 37,
    KB_SCANCODE_9 = 38,
    KB_SCANCODE_0 = 39,

    KB_SCANCODE_RETURN = 40,
    KB_SCANCODE_ESCAPE = 41,
    KB_SCANCODE_BACKSPACE = 42,
    KB_SCANCODE_TAB = 43,
    KB_SCANCODE_SPACE = 44,

    KB_SCANCODE_MINUS = 45,
    KB_SCANCODE_EQUALS = 46,
    KB_SCANCODE_LEFTBRACKET = 47,
    KB_SCANCODE_RIGHTBRACKET = 48,
    KB_SCANCODE_BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    KB_SCANCODE_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate KB_SCANCODE_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    KB_SCANCODE_SEMICOLON = 51,
    KB_SCANCODE_APOSTROPHE = 52,
    KB_SCANCODE_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    KB_SCANCODE_COMMA = 54,
    KB_SCANCODE_PERIOD = 55,
    KB_SCANCODE_SLASH = 56,

    KB_SCANCODE_CAPSLOCK = 57,

    KB_SCANCODE_F1 = 58,
    KB_SCANCODE_F2 = 59,
    KB_SCANCODE_F3 = 60,
    KB_SCANCODE_F4 = 61,
    KB_SCANCODE_F5 = 62,
    KB_SCANCODE_F6 = 63,
    KB_SCANCODE_F7 = 64,
    KB_SCANCODE_F8 = 65,
    KB_SCANCODE_F9 = 66,
    KB_SCANCODE_F10 = 67,
    KB_SCANCODE_F11 = 68,
    KB_SCANCODE_F12 = 69,

    KB_SCANCODE_PRINTSCREEN = 70,
    KB_SCANCODE_SCROLLLOCK = 71,
    KB_SCANCODE_PAUSE = 72,
    KB_SCANCODE_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    KB_SCANCODE_HOME = 74,
    KB_SCANCODE_PAGEUP = 75,
    KB_SCANCODE_DELETE = 76,
    KB_SCANCODE_END = 77,
    KB_SCANCODE_PAGEDOWN = 78,
    KB_SCANCODE_RIGHT = 79,
    KB_SCANCODE_LEFT = 80,
    KB_SCANCODE_DOWN = 81,
    KB_SCANCODE_UP = 82,

    KB_SCANCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    KB_SCANCODE_KP_DIVIDE = 84,
    KB_SCANCODE_KP_MULTIPLY = 85,
    KB_SCANCODE_KP_MINUS = 86,
    KB_SCANCODE_KP_PLUS = 87,
    KB_SCANCODE_KP_ENTER = 88,
    KB_SCANCODE_KP_1 = 89,
    KB_SCANCODE_KP_2 = 90,
    KB_SCANCODE_KP_3 = 91,
    KB_SCANCODE_KP_4 = 92,
    KB_SCANCODE_KP_5 = 93,
    KB_SCANCODE_KP_6 = 94,
    KB_SCANCODE_KP_7 = 95,
    KB_SCANCODE_KP_8 = 96,
    KB_SCANCODE_KP_9 = 97,
    KB_SCANCODE_KP_0 = 98,
    KB_SCANCODE_KP_PERIOD = 99,

    KB_SCANCODE_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    KB_SCANCODE_APPLICATION = 101, /**< windows contextual menu, compose */
    KB_SCANCODE_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    KB_SCANCODE_KP_EQUALS = 103,
    KB_SCANCODE_F13 = 104,
    KB_SCANCODE_F14 = 105,
    KB_SCANCODE_F15 = 106,
    KB_SCANCODE_F16 = 107,
    KB_SCANCODE_F17 = 108,
    KB_SCANCODE_F18 = 109,
    KB_SCANCODE_F19 = 110,
    KB_SCANCODE_F20 = 111,
    KB_SCANCODE_F21 = 112,
    KB_SCANCODE_F22 = 113,
    KB_SCANCODE_F23 = 114,
    KB_SCANCODE_F24 = 115,
    KB_SCANCODE_EXECUTE = 116,
    KB_SCANCODE_HELP = 117,
    KB_SCANCODE_MENU = 118,
    KB_SCANCODE_SELECT = 119,
    KB_SCANCODE_STOP = 120,
    KB_SCANCODE_AGAIN = 121,   /**< redo */
    KB_SCANCODE_UNDO = 122,
    KB_SCANCODE_CUT = 123,
    KB_SCANCODE_COPY = 124,
    KB_SCANCODE_PASTE = 125,
    KB_SCANCODE_FIND = 126,
    KB_SCANCODE_MUTE = 127,
    KB_SCANCODE_VOLUMEUP = 128,
    KB_SCANCODE_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     KB_SCANCODE_LOCKINGCAPSLOCK = 130,  */
/*     KB_SCANCODE_LOCKINGNUMLOCK = 131, */
/*     KB_SCANCODE_LOCKINGSCROLLLOCK = 132, */
    KB_SCANCODE_KP_COMMA = 133,
    KB_SCANCODE_KP_EQUALSAS400 = 134,

    KB_SCANCODE_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    KB_SCANCODE_INTERNATIONAL2 = 136,
    KB_SCANCODE_INTERNATIONAL3 = 137, /**< Yen */
    KB_SCANCODE_INTERNATIONAL4 = 138,
    KB_SCANCODE_INTERNATIONAL5 = 139,
    KB_SCANCODE_INTERNATIONAL6 = 140,
    KB_SCANCODE_INTERNATIONAL7 = 141,
    KB_SCANCODE_INTERNATIONAL8 = 142,
    KB_SCANCODE_INTERNATIONAL9 = 143,
    KB_SCANCODE_LANG1 = 144, /**< Hangul/English toggle */
    KB_SCANCODE_LANG2 = 145, /**< Hanja conversion */
    KB_SCANCODE_LANG3 = 146, /**< Katakana */
    KB_SCANCODE_LANG4 = 147, /**< Hiragana */
    KB_SCANCODE_LANG5 = 148, /**< Zenkaku/Hankaku */
    KB_SCANCODE_LANG6 = 149, /**< reserved */
    KB_SCANCODE_LANG7 = 150, /**< reserved */
    KB_SCANCODE_LANG8 = 151, /**< reserved */
    KB_SCANCODE_LANG9 = 152, /**< reserved */

    KB_SCANCODE_ALTERASE = 153, /**< Erase-Eaze */
    KB_SCANCODE_SYSREQ = 154,
    KB_SCANCODE_CANCEL = 155,
    KB_SCANCODE_CLEAR = 156,
    KB_SCANCODE_PRIOR = 157,
    KB_SCANCODE_RETURN2 = 158,
    KB_SCANCODE_SEPARATOR = 159,
    KB_SCANCODE_OUT = 160,
    KB_SCANCODE_OPER = 161,
    KB_SCANCODE_CLEARAGAIN = 162,
    KB_SCANCODE_CRSEL = 163,
    KB_SCANCODE_EXSEL = 164,

    KB_SCANCODE_KP_00 = 176,
    KB_SCANCODE_KP_000 = 177,
    KB_SCANCODE_THOUSANDSSEPARATOR = 178,
    KB_SCANCODE_DECIMALSEPARATOR = 179,
    KB_SCANCODE_CURRENCYUNIT = 180,
    KB_SCANCODE_CURRENCYSUBUNIT = 181,
    KB_SCANCODE_KP_LEFTPAREN = 182,
    KB_SCANCODE_KP_RIGHTPAREN = 183,
    KB_SCANCODE_KP_LEFTBRACE = 184,
    KB_SCANCODE_KP_RIGHTBRACE = 185,
    KB_SCANCODE_KP_TAB = 186,
    KB_SCANCODE_KP_BACKSPACE = 187,
    KB_SCANCODE_KP_A = 188,
    KB_SCANCODE_KP_B = 189,
    KB_SCANCODE_KP_C = 190,
    KB_SCANCODE_KP_D = 191,
    KB_SCANCODE_KP_E = 192,
    KB_SCANCODE_KP_F = 193,
    KB_SCANCODE_KP_XOR = 194,
    KB_SCANCODE_KP_POWER = 195,
    KB_SCANCODE_KP_PERCENT = 196,
    KB_SCANCODE_KP_LESS = 197,
    KB_SCANCODE_KP_GREATER = 198,
    KB_SCANCODE_KP_AMPERSAND = 199,
    KB_SCANCODE_KP_DBLAMPERSAND = 200,
    KB_SCANCODE_KP_VERTICALBAR = 201,
    KB_SCANCODE_KP_DBLVERTICALBAR = 202,
    KB_SCANCODE_KP_COLON = 203,
    KB_SCANCODE_KP_HASH = 204,
    KB_SCANCODE_KP_SPACE = 205,
    KB_SCANCODE_KP_AT = 206,
    KB_SCANCODE_KP_EXCLAM = 207,
    KB_SCANCODE_KP_MEMSTORE = 208,
    KB_SCANCODE_KP_MEMRECALL = 209,
    KB_SCANCODE_KP_MEMCLEAR = 210,
    KB_SCANCODE_KP_MEMADD = 211,
    KB_SCANCODE_KP_MEMSUBTRACT = 212,
    KB_SCANCODE_KP_MEMMULTIPLY = 213,
    KB_SCANCODE_KP_MEMDIVIDE = 214,
    KB_SCANCODE_KP_PLUSMINUS = 215,
    KB_SCANCODE_KP_CLEAR = 216,
    KB_SCANCODE_KP_CLEARENTRY = 217,
    KB_SCANCODE_KP_BINARY = 218,
    KB_SCANCODE_KP_OCTAL = 219,
    KB_SCANCODE_KP_DECIMAL = 220,
    KB_SCANCODE_KP_HEXADECIMAL = 221,

    KB_SCANCODE_LCTRL = 224,
    KB_SCANCODE_LSHIFT = 225,
    KB_SCANCODE_LALT = 226, /**< alt, option */
    KB_SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
    KB_SCANCODE_RCTRL = 228,
    KB_SCANCODE_RSHIFT = 229,
    KB_SCANCODE_RALT = 230, /**< alt gr, option */
    KB_SCANCODE_RGUI = 231, /**< windows, command (apple), meta */

    KB_SCANCODE_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    KB_SCANCODE_AUDIONEXT = 258,
    KB_SCANCODE_AUDIOPREV = 259,
    KB_SCANCODE_AUDIOSTOP = 260,
    KB_SCANCODE_AUDIOPLAY = 261,
    KB_SCANCODE_AUDIOMUTE = 262,
    KB_SCANCODE_MEDIASELECT = 263,
    KB_SCANCODE_WWW = 264,
    KB_SCANCODE_MAIL = 265,
    KB_SCANCODE_CALCULATOR = 266,
    KB_SCANCODE_COMPUTER = 267,
    KB_SCANCODE_AC_SEARCH = 268,
    KB_SCANCODE_AC_HOME = 269,
    KB_SCANCODE_AC_BACK = 270,
    KB_SCANCODE_AC_FORWARD = 271,
    KB_SCANCODE_AC_STOP = 272,
    KB_SCANCODE_AC_REFRESH = 273,
    KB_SCANCODE_AC_BOOKMARKS = 274,

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    KB_SCANCODE_BRIGHTNESSDOWN = 275,
    KB_SCANCODE_BRIGHTNESSUP = 276,
    KB_SCANCODE_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    KB_SCANCODE_KBDILLUMTOGGLE = 278,
    KB_SCANCODE_KBDILLUMDOWN = 279,
    KB_SCANCODE_KBDILLUMUP = 280,
    KB_SCANCODE_EJECT = 281,
    KB_SCANCODE_SLEEP = 282,

    KB_SCANCODE_APP1 = 283,
    KB_SCANCODE_APP2 = 284,

    /* @} *//* Walther keys */

    /* Add any other keys here. */

    KB_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
};

#endif
