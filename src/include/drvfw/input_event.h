/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: framebuffer driver framework
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-06-03     JasonHu           Init
 */

#ifndef __DRVFW_INPUT_EVENT_H__
#define __DRVFW_INPUT_EVENT_H__

#include <nxos.h>
#include <base/spin.h>

#define NX_INPUT_EVENT_CMD_GETLED 1

typedef struct NX_InputEvent
{
    NX_U16 type;
    NX_U16 code;
    NX_U32 value;
} NX_InputEvent;

/* input event type */
#define NX_EV_SYNC              0x00
#define NX_EV_KEY               0x01
#define NX_EV_REL               0x02 /* relative */
#define NX_EV_ABS               0x03 /* abslute */
#define NX_EV_MISC              0x04

/* key code */
enum {
    NX_KEY_UNKNOWN      = 0,    /* unknown keycode */
    NX_KEY_FIRST,               /* first key */
    NX_KEY_CLEAR,               /* clear */
    NX_KEY_PAUSE,               /* pause */
    NX_KEY_UP,                  /* up arrow */
    NX_KEY_DOWN,                /* down arrow */
    NX_KEY_RIGHT,               /* right arrow */
    NX_KEY_LEFT,                /* left arrow */
    NX_KEY_BACKSPACE,           /* backspace */
    NX_KEY_TAB,                 /* 9: tab */
    NX_KEY_ENTER,               /* 10: enter */
    NX_KEY_INSERT,              /* insert */
    NX_KEY_HOME,                /* home */
    NX_KEY_END,                 /* end */
    NX_KEY_PAGEUP,              /* page up */
    NX_KEY_PAGEDOWN,            /* page down */
    NX_KEY_F1,                  /* F1 */
    NX_KEY_F2,                  /* F2 */
    NX_KEY_F3,                  /* F3 */
    NX_KEY_F4,                  /* F4 */
    NX_KEY_F5,                  /* F5 */
    NX_KEY_F6,                  /* F6 */
    NX_KEY_F7,                  /* F7 */
    NX_KEY_F8,                  /* F8 */
    NX_KEY_F9,                  /* F9 */
    NX_KEY_F10,                 /* F10 */
    NX_KEY_F11,                 /* F11 */
    NX_KEY_ESCAPE,              /* 27: escape */
    NX_KEY_F12,                 /* F12 */
    NX_KEY_F13,                 /* F13 */
    NX_KEY_F14,                 /* F14 */
    NX_KEY_F15,                 /* F15 */
    NX_KEY_SPACE,               /*  space */
    NX_KEY_EXCLAIM,             /* ! exclamation mark */
    NX_KEY_QUOTEDBL,            /*" double quote */
    NX_KEY_HASH,                /* # hash */
    NX_KEY_DOLLAR,              /* $ dollar */
    NX_KEY_PERSENT,             /* % persent */
    NX_KEY_AMPERSAND,           /* & ampersand */
    NX_KEY_QUOTE,               /* ' single quote */
    NX_KEY_LEFTPAREN,           /* ( left parenthesis */
    NX_KEY_RIGHTPAREN,          /* ) right parenthesis */
    NX_KEY_ASTERISK,            /* * asterisk */
    NX_KEY_PLUS,                /* + plus sign */
    NX_KEY_COMMA,               /* , comma */
    NX_KEY_MINUS,               /* - minus sign */
    NX_KEY_PERIOD,              /* . period/full stop */
    NX_KEY_SLASH,               /* / forward slash */
    NX_KEY_0,                   /* 0 */
    NX_KEY_1,                   /* 1 */
    NX_KEY_2,                   /* 2 */
    NX_KEY_3,                   /* 3 */
    NX_KEY_4,                   /* 4 */
    NX_KEY_5,                   /* 5 */
    NX_KEY_6,                   /* 6 */
    NX_KEY_7,                   /* 7 */
    NX_KEY_8,                   /* 8 */
    NX_KEY_9,                   /* 9 */
    NX_KEY_COLON,               /* : colon */
    NX_KEY_SEMICOLON,           /* ;semicolon */
    NX_KEY_LESS,                /* < less-than sign */
    NX_KEY_EQUALS,              /* = equals sign */
    NX_KEY_GREATER,             /* > greater-then sign */
    NX_KEY_QUESTION,            /* ? question mark */
    NX_KEY_AT,                  /* @ at */
    NX_KEY_A,                   /* A */
    NX_KEY_B,                   /* B */
    NX_KEY_C,                   /* C */
    NX_KEY_D,                   /* D */
    NX_KEY_E,                   /* E */
    NX_KEY_F,                   /* F */
    NX_KEY_G,                   /* G */
    NX_KEY_H,                   /* H */
    NX_KEY_I,                   /* I */
    NX_KEY_J,                   /* J */
    NX_KEY_K,                   /* K */
    NX_KEY_L,                   /* L */
    NX_KEY_M,                   /* M */
    NX_KEY_N,                   /* N */
    NX_KEY_O,                   /* O */
    NX_KEY_P,                   /* P */
    NX_KEY_Q,                   /* Q */
    NX_KEY_R,                   /* R */
    NX_KEY_S,                   /* S */
    NX_KEY_T,                   /* T */
    NX_KEY_U,                   /* U */
    NX_KEY_V,                   /* V */
    NX_KEY_W,                   /* W */
    NX_KEY_X,                   /* X */
    NX_KEY_Y,                   /* Y */
    NX_KEY_Z,                   /* Z */
    NX_KEY_LEFTSQUAREBRACKET,   /* [ left square bracket */
    NX_KEY_BACKSLASH,           /* \ backslash */
    NX_KEY_RIGHTSQUAREBRACKET,  /* ]right square bracket */
    NX_KEY_CARET,               /* ^ caret */
    NX_KEY_UNDERSCRE,           /* _ underscore */
    NX_KEY_BACKQUOTE,           /* ` grave */
    NX_KEY_a,                   /* a */
    NX_KEY_b,                   /* b */
    NX_KEY_c,                   /* c */
    NX_KEY_d,                   /* d */
    NX_KEY_e,                   /* e */
    NX_KEY_f,                   /* f */
    NX_KEY_g,                   /* g */
    NX_KEY_h,                   /* h */
    NX_KEY_i,                   /* i */
    NX_KEY_j,                   /* j */
    NX_KEY_k,                   /* k */
    NX_KEY_l,                   /* l */
    NX_KEY_m,                   /* m */
    NX_KEY_n,                   /* n */
    NX_KEY_o,                   /* o */
    NX_KEY_p,                   /* p */
    NX_KEY_q,                   /* q */
    NX_KEY_r,                   /* r */
    NX_KEY_s,                   /* s */
    NX_KEY_t,                   /* t */
    NX_KEY_u,                   /* u */
    NX_KEY_v,                   /* v */
    NX_KEY_w,                   /* w */
    NX_KEY_x,                   /* x */
    NX_KEY_y,                   /* y */
    NX_KEY_z,                   /* z */
    NX_KEY_LEFTBRACKET,         /* { left bracket */
    NX_KEY_VERTICAL,            /* | vertical virgul */
    NX_KEY_RIGHTBRACKET,        /* } left bracket */
    NX_KEY_TILDE,               /* ~ tilde */
    NX_KEY_DELETE,              /* 127 delete */
    NX_KEY_KP0,                 /* keypad 0 */
    NX_KEY_KP1,                 /* keypad 1 */
    NX_KEY_KP2,                 /* keypad 2 */
    NX_KEY_KP3,                 /* keypad 3 */
    NX_KEY_KP4,                 /* keypad 4 */
    NX_KEY_KP5,                 /* keypad 5 */
    NX_KEY_KP6,                 /* keypad 6 */
    NX_KEY_KP7,                 /* keypad 7 */
    NX_KEY_KP8,                 /* keypad 8 */
    NX_KEY_KP9,                 /* keypad 9 */
    NX_KEY_KP_PERIOD,           /* keypad period    '.' */
    NX_KEY_KP_DIVIDE,           /* keypad divide    '/' */
    NX_KEY_KP_MULTIPLY,         /* keypad multiply  '*' */
    NX_KEY_KP_MINUS,            /* keypad minus     '-' */
    NX_KEY_KP_PLUS,             /* keypad plus      '+' */
    NX_KEY_KP_ENTER,            /* keypad enter     '\r'*/
    NX_KEY_KP_EQUALS,           /* !keypad equals   '=' */
    NX_KEY_NUMLOCK,             /* numlock */
    NX_KEY_CAPSLOCK,            /* capslock */
    NX_KEY_SCROLLOCK,           /* scrollock */
    NX_KEY_RSHIFT,              /* right shift */
    NX_KEY_LSHIFT,              /* left shift */
    NX_KEY_RCTRL,               /* right ctrl */
    NX_KEY_LCTRL,               /* left ctrl */
    NX_KEY_RALT,                /* right alt / alt gr */
    NX_KEY_LALT,                /* left alt */
    NX_KEY_RMETA,               /* right meta */
    NX_KEY_LMETA,               /* left meta */
    NX_KEY_RSUPER,              /* right windows key */
    NX_KEY_LSUPER,              /* left windows key */
    NX_KEY_MODE,                /* mode shift */
    NX_KEY_COMPOSE,             /* compose */
    NX_KEY_HELP,                /* help */
    NX_KEY_PRINT,               /* print-screen */
    NX_KEY_SYSREQ,              /* sys rq */
    NX_KEY_BREAK,               /* break */
    NX_KEY_MENU,                /* menu */
    NX_KEY_POWER,               /* power */
    NX_KEY_EURO,                /* euro */
    NX_KEY_UNDO,                /* undo */
    NX_KEY_MOUSE_LEFT,          /* mouse left */
    NX_KEY_MOUSE_RIGHT,         /* mouse right */
    NX_KEY_MOUSE_MIDDLE,        /* mouse middle */
    NX_KEY_LAST                 /* last one */        
};

/* relative dir code */
enum {
    NX_REL_MISC = 0,
    NX_REL_X,                   /* mouse x relative postion */
    NX_REL_Y,                   /* mouse y relative postion */
    NX_REL_WHEEL                /* mouse wheel */
};

typedef struct NX_InputEventQueue
{
    NX_InputEvent * eventBuf;
    NX_Size maxSize;
    NX_U32 head;
    NX_U32 tail;
    NX_Spin lock;
} NX_InputEventQueue;

NX_Error NX_InputEventQueueInit(NX_InputEventQueue * eventQueue, NX_Size queueSize);
NX_Error NX_InputEventQueueExit(NX_InputEventQueue * eventQueue);
NX_Error NX_InputEventQueuePut(NX_InputEventQueue * eventQueue, NX_InputEvent * event);
NX_Error NX_InputEventQueueGet(NX_InputEventQueue *eventQueue, NX_InputEvent * event);
NX_Bool NX_InputEventQueueEmpty(NX_InputEventQueue *eventQueue);

#endif  /* __DRVFW_INPUT_EVENT_H__ */
