/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2019  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : USBH_MIDI_GUI_Keyboard.c
Purpose : This sample is designed to present emUSBH's capability to
          enumerate MIDIStreaming devices and read MIDI events from
          a MIDI controller.

Additional information:
  Preparation:
    This is configured for the LCD on an STM32F746G-Discovery board.
    For other LCD sizes, you can configure dimensions accordingly.

  Expected behavior:
    When a MIDI controller is connected and events are sent, the
    events are decoded and displayed on the virtual keyboard.
    Running a finger over the virtual keyboard sends the played
    notes to all connected MIDI devices on the defined MIDI channel.

    Note that the host device operates in Omni mode and accepts
    and interprets events from any MIDI channel.
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "RTOS.h"
#include "BSP.h"
#include "BSP_GUI.h"
#include "GUI.h"
#include "USBH_MIDI.h"
#include "SEGGER.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Event decomposition
*/
#define MIDI_GET_CABLE(E)    ((E) >> 28)
#define MIDI_GET_CID(E)      (((E) >> 24) & 0xF)
#define MIDI_GET_STATUS(E)   (((E) >> 16) & 0xFF)
#define MIDI_GET_CHANNEL(E)  (((E) >> 16) & 0xF)
#define MIDI_GET_DATA2(E)    (unsigned)(((E) >> 8) & 0x7Fu)
#define MIDI_GET_DATA3(E)    (unsigned)((E) & 0x7Fu)
#define MIDI_GET_DATAX(E)    ((MIDI_GET_DATA3(E) << 7) + MIDI_GET_DATA2(E))

/*********************************************************************
*
*       Event composition
*/
#define MIDI_PUT_CABLE(E)    (((E) & 0xF) << 28)
#define MIDI_PUT_CID(E)      (((E) & 0xF) << 24)
#define MIDI_PUT_STATUS(E)   (((E) & 0xFF) << 16)
#define MIDI_PUT_CHANNEL(E)  (((E) & 0xF) << 16)
#define MIDI_PUT_DATA2(E)    (((E) & 0x7Fu) << 8)
#define MIDI_PUT_DATA3(E)    ((E) & 0x7Fu)
#define MIDI_PUT_DATAX(E)    (MIDI_PUT_DATA3((E) >> 7) + MIDI_PUT_DATA2(E))

/*********************************************************************
*
*       MIDI note numbers
*/
#define MIDI_NOTE_B7          107
#define MIDI_NOTE_A7_SHARP    106
#define MIDI_NOTE_A7          105
#define MIDI_NOTE_G7_SHARP    104
#define MIDI_NOTE_G7          103
#define MIDI_NOTE_F7_SHARP    102
#define MIDI_NOTE_F7          101
#define MIDI_NOTE_E7          100
#define MIDI_NOTE_D7_SHARP     99
#define MIDI_NOTE_D7           98
#define MIDI_NOTE_C7_SHARP     97
#define MIDI_NOTE_C7           96
#define MIDI_NOTE_B6           95
#define MIDI_NOTE_A6_SHARP     94
#define MIDI_NOTE_A6           93
#define MIDI_NOTE_G6_SHARP     92
#define MIDI_NOTE_G6           91
#define MIDI_NOTE_F6_SHARP     90
#define MIDI_NOTE_F6           89
#define MIDI_NOTE_E6           88
#define MIDI_NOTE_D6_SHARP     87
#define MIDI_NOTE_D6           86
#define MIDI_NOTE_C6_SHARP     85
#define MIDI_NOTE_C6           84
#define MIDI_NOTE_B5           83
#define MIDI_NOTE_A5_SHARP     82
#define MIDI_NOTE_A5           81
#define MIDI_NOTE_G5_SHARP     80
#define MIDI_NOTE_G5           79
#define MIDI_NOTE_F5_SHARP     78
#define MIDI_NOTE_F5           77
#define MIDI_NOTE_E5           76
#define MIDI_NOTE_D5_SHARP     75
#define MIDI_NOTE_D5           74
#define MIDI_NOTE_C5_SHARP     73
#define MIDI_NOTE_C5           72
#define MIDI_NOTE_B4           71
#define MIDI_NOTE_A4_SHARP     70
#define MIDI_NOTE_A4           69
#define MIDI_NOTE_G4_SHARP     68
#define MIDI_NOTE_G4           67
#define MIDI_NOTE_F4_SHARP     66
#define MIDI_NOTE_F4           65
#define MIDI_NOTE_E4           64
#define MIDI_NOTE_D4_SHARP     63
#define MIDI_NOTE_D4           62
#define MIDI_NOTE_C4_SHARP     61
#define MIDI_NOTE_C4           60
#define MIDI_NOTE_B3           59
#define MIDI_NOTE_A3_SHARP     58
#define MIDI_NOTE_A3           57
#define MIDI_NOTE_G3_SHARP     56
#define MIDI_NOTE_G3           55
#define MIDI_NOTE_F3_SHARP     54
#define MIDI_NOTE_F3           53
#define MIDI_NOTE_E3           52
#define MIDI_NOTE_D3_SHARP     51
#define MIDI_NOTE_D3           50
#define MIDI_NOTE_C3_SHARP     49
#define MIDI_NOTE_C3           48
#define MIDI_NOTE_B2           47
#define MIDI_NOTE_A2_SHARP     46
#define MIDI_NOTE_A2           45
#define MIDI_NOTE_G2_SHARP     44
#define MIDI_NOTE_G2           43
#define MIDI_NOTE_F2_SHARP     42
#define MIDI_NOTE_F2           41
#define MIDI_NOTE_E2           40
#define MIDI_NOTE_D2_SHARP     39
#define MIDI_NOTE_D2           38
#define MIDI_NOTE_C2_SHARP     37
#define MIDI_NOTE_C2           36
#define MIDI_NOTE_B1           35
#define MIDI_NOTE_A1_SHARP     34
#define MIDI_NOTE_A1           33
#define MIDI_NOTE_G1_SHARP     32
#define MIDI_NOTE_G1           31
#define MIDI_NOTE_F1_SHARP     30
#define MIDI_NOTE_F1           29
#define MIDI_NOTE_E1           28
#define MIDI_NOTE_D1_SHARP     27
#define MIDI_NOTE_D1           26
#define MIDI_NOTE_C1_SHARP     25
#define MIDI_NOTE_C1           24
#define MIDI_NOTE_B0           23
#define MIDI_NOTE_A0_SHARP     22
#define MIDI_NOTE_A0           21

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define MAX_MIDI_DEVICES        4     // Maximum number of attched MIDI devices
#define MAX_DATA_ITEMS         10     // Maximum number of outstanding MIDI events

/*********************************************************************
*
*       Virtual keyboard
*/
#define VKB_MIDI_CHANNEL        0     // MIDI output channel, 0...15
#define VKB_NOTE_ON_VELOCITY   64     // Velocity for Note On, 1...127
#define VKB_NOTE_OFF_VELOCITY  64     // Velocity for Note Off, 0...127

/*********************************************************************
*
*       Palette
*/
#define COLOR_BACKGROUND     GUI_MAKE_COLOR(0x801010)
#define COLOR_KEY_BORDER     GUI_MAKE_COLOR(0x473E34)
#define COLOR_KEY_DOWN       GUI_MAKE_COLOR(0x30C030)
#define COLOR_KEY_ERROR      GUI_MAKE_COLOR(0x3030D0)

/*********************************************************************
*
*       Key dimensions
*/
#define WHITE_KEY_WIDTH      19
#define WHITE_KEY_LENGTH     100
#define BLACK_KEY_LENGTH     ((int)(WHITE_KEY_LENGTH * 0.6))
#define BLACK_KEY_WIDTH      ((int)(WHITE_KEY_WIDTH * (13.7/23.5)))

/*********************************************************************
*
*       Keybed dimensions
*/
#define KEYBED_LOW_NOTE      MIDI_NOTE_C2
#define KEYBED_HIGH_NOTE     MIDI_NOTE_F5

/*********************************************************************
*
*       Keybed position
*/
#define MARGIN_LEFT          12
#define MARGIN_TOP           80

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

enum {
  TASK_PRIO_KB_DISP = 150,
  TASK_PRIO_KB_RECV,
  TASK_PRIO_APP,
  TASK_PRIO_KB_PLAY,
  TASK_PRIO_USBH_MAIN,
  TASK_PRIO_USBH_ISR
};

typedef struct {
  U32 KeyColor;
  U32 KeyBorder;
  U32 LegendColor;
} KEY_PALETTE;

typedef struct {
  USBH_MIDI_HANDLE hMIDI;
  OS_TASK          RxTask;
  char             aProductName      [32];
  char             aManufacturerName [32];
  char             aTaskName         [64];
  OS_STACKPTR int  aStack            [1024/sizeof(int)];
} MIDI_DEVICE;

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

static const KEY_PALETTE WhiteKeyOff    = { GUI_WHITE,       COLOR_KEY_BORDER, GUI_GRAY_60 };
static const KEY_PALETTE WhiteKeyOn     = { COLOR_KEY_DOWN,  COLOR_KEY_BORDER, GUI_WHITE   };
static const KEY_PALETTE BlackKeyOff    = { GUI_BLACK,       COLOR_KEY_BORDER, GUI_BLACK   };
static const KEY_PALETTE BlackKeyOn     = { COLOR_KEY_DOWN,  COLOR_KEY_BORDER, GUI_WHITE   };

typedef struct {
  enum { WHITE, BLACK } Color;
  unsigned              Index;
} KEY_DESC;

static const KEY_DESC _aKeyMapping[] = {
  { WHITE, 0 },  // A
  { BLACK, 0 },  // A#
  { WHITE, 1 },  // B
  { WHITE, 2 },  // C
  { BLACK, 2 },  // C#
  { WHITE, 3 },  // D
  { BLACK, 3 },  // D#
  { WHITE, 4 },  // E
  { WHITE, 5 },  // F
  { BLACK, 5 },  // F#
  { WHITE, 6 },  // G
  { BLACK, 6 },  // G#
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static OS_STACKPTR int        _StackMain[1536/sizeof(int)];
static OS_TASK                _TCBMain;
static OS_STACKPTR int        _StackIsr[1276/sizeof(int)];
static OS_TASK                _TCBIsr;
static OS_STACKPTR int        _StackKbDisp[1276/sizeof(int)];
static OS_TASK                _TCBKbDisp;
static OS_STACKPTR int        _StackKbPlay[1276/sizeof(int)];
static OS_TASK                _TCBKbPlay;
static MIDI_DEVICE            _aMidiDevice[MAX_MIDI_DEVICES];
static U8                     _aNoteOn[128];
static GUI_MEMDEV_Handle      _hMemDev;
static volatile int           _NoteChange;
static USBH_NOTIFICATION_HOOK _Hook;
static OS_MAILBOX             _MidiMailbox;
static U32                    _aMidiEvents[MAX_DATA_ITEMS];

/*********************************************************************
*
*       Static Code
*
**********************************************************************
*/

/*********************************************************************
*
*       _GetWhiteRect()
*
*  Function description
*    Get rectangle corresponding to a white key.
*
*  Parameters
*    pRect - Pointer to the object that receives the rectangle.
*    Pos   - White key position.
*/
static void _GetWhiteRect(GUI_RECT *pRect, unsigned Pos) {
  pRect->x0 = MARGIN_LEFT + Pos * WHITE_KEY_WIDTH;
  pRect->y0 = MARGIN_TOP;
  pRect->x1 = pRect->x0 + WHITE_KEY_WIDTH;  // Overlap white key rectangles by one pixel
  pRect->y1 = pRect->y0 + WHITE_KEY_LENGTH - 1;
}

/*********************************************************************
*
*       _GetBlackRect()
*
*  Function description
*    Get rectangle corresponding to a black key.
*
*  Parameters
*    pRect - Pointer to the object that receives the rectangle.
*    Pos   - Black key position.
*/
static void _GetBlackRect(GUI_RECT *pRect, unsigned Pos) {
  pRect->x0 = MARGIN_LEFT + Pos * WHITE_KEY_WIDTH + WHITE_KEY_WIDTH - BLACK_KEY_WIDTH/2;
  pRect->y0 = MARGIN_TOP;
  pRect->x1 = pRect->x0 + BLACK_KEY_WIDTH  - 1;
  pRect->y1 = pRect->y0 + BLACK_KEY_LENGTH - 1;
}

/*********************************************************************
*
*       _DrawWhiteKey()
*
*  Function description
*    Draw white key.
*
*  Parameters
*    Pos      - White key position.
*    pPalette - Pointer to key palette colors.
*    sLabel   - Optional label for key.
*/
static void _DrawWhiteKey(unsigned Pos, const KEY_PALETTE *pPalette, const char *sLabel) {
  GUI_RECT Rect;
  //
  _GetWhiteRect(&Rect, Pos);
  GUI_SetColor(pPalette->KeyColor);
  GUI_FillRectEx(&Rect);
  GUI_SetColor(pPalette->KeyBorder);
  GUI_DrawRectEx(&Rect);
  //
  if (sLabel != NULL) {
    GUI_SetFont(&GUI_Font13B_ASCII);
    GUI_SetBkColor(pPalette->KeyColor);
    GUI_SetColor(pPalette->LegendColor);
    GUI_DispStringHCenterAt(sLabel, Rect.x0 + WHITE_KEY_WIDTH/2 + 2, Rect.y1-15);
  }
}

/*********************************************************************
*
*       _DrawBlackKey()
*
*  Function description
*    Draw black key.
*
*  Parameters
*    Pos      - Black key position.
*    pPalette - Pointer to key palette colors.
*/
static void _DrawBlackKey(unsigned Pos, const KEY_PALETTE *pPalette) {
  GUI_RECT Rect;
  //
  _GetBlackRect(&Rect, Pos);
  //
  GUI_SetColor(pPalette->KeyColor);
  GUI_FillRectEx(&Rect);
  GUI_SetColor(pPalette->KeyBorder);
  GUI_DrawRectEx(&Rect);
}

/*********************************************************************
*
*       _DecodeKey()
*
*  Function description
*    Decode MIDI note.
*
*  Parameters
*    Key - MIDI note, 0...127.
*
*  Return value
*    Zero-terminated string with note plus octave (e.g. C4, A5#).
*/
static const char * _DecodeKey(unsigned Key) {
  static const U8 aDecode1[] = { "AABCCDDEFFGG" };
  static const U8 aDecode2[] = { " #  # #  # #" };
  static char aKey[5];
  //
  int Octave;
  int Semitone;
  //
  Octave   = (Key - 9) / 12;
  Semitone = (Key + 3) % 12;
  //
  if (Octave == -2) {
    aKey[0] = aDecode1[Semitone];
    aKey[1] = '-';
    aKey[2] = '2';
    aKey[3] = aDecode2[Semitone];
    aKey[4] = 0;
  } else if (Octave == -1) {
    aKey[0] = aDecode1[Semitone];
    aKey[1] = '-';
    aKey[2] = '1';
    aKey[3] = aDecode2[Semitone];
    aKey[4] = 0;
  } else if (Key <= 127) {
    aKey[0] = aDecode1[Semitone];
    aKey[1] = '0' + Octave;
    aKey[2] = aDecode2[Semitone];
    aKey[3] = 0;
  } else {
    SEGGER_snprintf(aKey, sizeof(aKey), "%u", Key);
  }
  return aKey;
}

/*********************************************************************
*
*       _CalcKeyBase()
*
*  Function description
*    Convert MIDI note number to virtual keyboard position.
*
*  Parameters
*    Note - MIDI note number, 0...127.
*
*  Return value
*    Left x-coordinate of key.
*/
static int _CalcKeyBase(unsigned Note) {
  int Octave;
  int Semitone;
  //
  Octave   = (Note - 9) / 12;
  Semitone = (Note + 3) % 12;
  //
  return Octave*7 + _aKeyMapping[Semitone].Index;
}

/*********************************************************************
*
*       _DrawKey()
*
*  Function description
*    Draw key corresponding to a MIDI note.
*
*  Parameters
*    Note  - MIDI note number, 0...127.
*    Color - Whote or black indication.
*/
static void _DrawKey(int Note, unsigned Color) {
  int          Semitone;
  int          XPos;
  const char * sNoteName;
  //
  sNoteName = _DecodeKey(Note);
  Semitone  = (Note + 3) % 12;
  XPos      = _CalcKeyBase(Note) - _CalcKeyBase(KEYBED_LOW_NOTE);
  //
  if (_aKeyMapping[Semitone].Color == Color) {
    if (Color == WHITE) {
      if (Semitone == 3) {
        _DrawWhiteKey(XPos, _aNoteOn[Note] ? &WhiteKeyOn : &WhiteKeyOff, sNoteName);  // Legend always shown for C
      } else {
        _DrawWhiteKey(XPos, _aNoteOn[Note] ? &WhiteKeyOn : &WhiteKeyOff, _aNoteOn[Note] ? sNoteName : NULL);  // Show legend if key pressed
      }
    } else {
      _DrawBlackKey(XPos, _aNoteOn[Note] ? &BlackKeyOn : &BlackKeyOff);
    }
  }
}

/*********************************************************************
*
*       _DrawKeyboard()
*
*  Function description
*    Draw entire keyboard with note down indication.
*/
static void _DrawKeyboard(void) {
  unsigned Key;
  unsigned Index;
  int      Y;
  //
  GUI_MEMDEV_Select(_hMemDev);
  //
  GUI_SetColor(COLOR_BACKGROUND);
  GUI_FillRect(0, 0, LCD_GetXSize()-1, LCD_GetYSize()-1);
  //
  GUI_SetFont(&GUI_Font20B_ASCII);
  GUI_SetBkColor(COLOR_BACKGROUND);
  GUI_SetColor(GUI_WHITE);
  Y = MARGIN_TOP - 10;
  for (Index = 0; Index < SEGGER_COUNTOF(_aMidiDevice); ++Index) {
    if (_aMidiDevice[Index].hMIDI != USBH_MIDI_INVALID_HANDLE) {
      Y -= GUI_GetYSizeOfFont(&GUI_Font20B_ASCII);
      if (Y > 0) {
        GUI_DispStringHCenterAt(_aMidiDevice[Index].aTaskName, LCD_GetXSize()/2, Y);
      }
    }
  }
  //
  for (Key = KEYBED_LOW_NOTE; Key < KEYBED_HIGH_NOTE; ++Key) {
    _DrawKey(Key, WHITE);
  }
  for (Key = KEYBED_LOW_NOTE; Key < KEYBED_HIGH_NOTE; ++Key) {
    _DrawKey(Key, BLACK);
  }
  //
  GUI_MEMDEV_Select(0);
  GUI_MEMDEV_CopyToLCD(_hMemDev);
}

/*********************************************************************
*
*       _PointInRect()
*
*  Function description
*    Query if point lies in rectangle.
*
*  Parameters
*    pRect - Pointer to containing rectangle.
*    x     - X coordinate of point to test.
*    y     - Y coordinate of point to test.
*
*  Return value
*    == 0 - Point not contained in rectangle.
*    != 0 - Poiint is contained in rectangle.
*/
static int _PointInRect(const GUI_RECT *pRect, int x, int y) {
  return (pRect->x0 <= x && x <= pRect->x1) &&
         (pRect->y0 <= y && y <= pRect->y1);
}

/*********************************************************************
*
*       _HitKey()
*
*  Function description
*    Query if point lies in key rectangle.
*
*  Parameters
*    Note  - MIDI note number, 0...127.
*    Color - Note color, black or white.
*    x     - X coordinate of point to test.
*    y     - Y coordinate of point to test.
*
*  Return value
*    == 0 - Point not contained in key rectangle.
*    != 0 - Poiint is contained in key rectangle.
*/
static int _HitKey(int Note, unsigned Color, int x, int y) {
  GUI_RECT Rect;
  int      Semitone;
  int      XPos;
  //
  Semitone = (Note + 3) % 12;
  XPos     = _CalcKeyBase(Note) - _CalcKeyBase(KEYBED_LOW_NOTE);
  //
  if (_aKeyMapping[Semitone].Color == Color) {
    if (Color == WHITE) {
      _GetWhiteRect(&Rect, XPos);
    } else {
      _GetBlackRect(&Rect, XPos);
    }
    return _PointInRect(&Rect, x, y);
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       _FindKey()
*
*  Function description
*    Find MIDI note corresponding to point.
*
*  Parameters
*    x - X coordinate of point to test.
*    y - Y coordinate of point to test.
*
*  Return value
*    <  0 - Coordinate does not correspond to any on-display key.
*    >= 0 - MIDI note number corresponding to point.
*/
static int _FindKey(int x, int y) {
  unsigned Key;
  //
  for (Key = KEYBED_LOW_NOTE; Key < KEYBED_HIGH_NOTE; ++Key) {
    if (_HitKey(Key, BLACK, x, y)) {
      return Key;
    }
  }
  for (Key = KEYBED_LOW_NOTE; Key < KEYBED_HIGH_NOTE; ++Key) {
    if (_HitKey(Key, WHITE, x, y)) {
      return Key;
    }
  }
  return -1;
}

/*********************************************************************
*
*       _BroadcastEvent()
*
*  Function description
*    Send MIDI message to all connected cables.
*
*  Parameters
*    Event - USB MIDI event to broadcast.
*/
static void _BroadcastEvent(unsigned Event) {
  USBH_MIDI_DEVICE_INFO DevInfo;
  unsigned              Index;
  unsigned              Cable;
  //
  // Copy MIDI event to CID.
  //
  Event |= MIDI_PUT_CID(MIDI_GET_STATUS(Event) >> 4);
  //
  for (Index = 0; Index < SEGGER_COUNTOF(_aMidiDevice); ++Index) {
    if (_aMidiDevice[Index].hMIDI != USBH_MIDI_INVALID_HANDLE) {
      USBH_MIDI_GetDeviceInfo(_aMidiDevice[Index].hMIDI, &DevInfo);
      for (Cable = 0; Cable < DevInfo.NumInCables; ++Cable) {
        USBH_MIDI_WrEvent(_aMidiDevice[Index].hMIDI, Event | MIDI_PUT_CABLE(Cable));
      }
    }
  }
}

/*********************************************************************
*
*       _KbDispTask()
*
*  Function description
*    Virtual keyboard display task.
*
*  Additional information
*    Waits for note changed events and redisplays the virtual
*    keyboard through the GUI.
*/
static void _KbDispTask(void) {
  U32 Event;
  U32 LastDraw;
  int KeyboardChanged;
  //
  _DrawKeyboard();
  KeyboardChanged = 0;
  LastDraw = OS_GetTime32();
  //
  for (;;) {
    if (OS_GetMailTimed(&_MidiMailbox, &Event, 10) == 0) {
      if (Event == 0) {
        // Forced redraw on device event
        KeyboardChanged = 1;
      } else if ((MIDI_GET_STATUS(Event) & 0xF0) == 0x90) {                   // Note On
        _aNoteOn[MIDI_GET_DATA2(Event)] = MIDI_GET_DATA3(Event) != 0;  // Velocity 0 is note off
        KeyboardChanged = 1;
      } else if ((MIDI_GET_STATUS(Event) & 0xF0) == 0x80) {            // Note Off
        _aNoteOn[MIDI_GET_DATA2(Event)] = 0;
        KeyboardChanged = 1;
      }
    } else if (KeyboardChanged) {
      if (OS_GetTime32() - LastDraw > 30) {
        _DrawKeyboard();
        KeyboardChanged = 0;
        LastDraw        = OS_GetTime32();
      }
    }
  }
}

/*********************************************************************
*
*       _KbPlayTask()
*
*  Function description
*    Virtual keyboard play task.
*
*  Additional information
*    Waits for pointer inputs on the keyboard, constructs MIDI
*    events that correspond to the key states, sends them to the
*    display task to update the virtual keyboard, and and
*    broadcasts the MIDI events to all connected devices.
*/
static void _KbPlayTask(void) {
  GUI_PID_STATE State;
  int           Key;
  int           LastKey;
  U32           Event;
  //
  LastKey = -1;
  for (;;) {
    GUI_PID_GetCurrentState(&State);
    Key = State.Pressed ? _FindKey(State.x, State.y) : -1;
    if (Key != LastKey) {
      if (LastKey > 0) {
        Event = MIDI_PUT_STATUS(0x80) | MIDI_PUT_CHANNEL(VKB_MIDI_CHANNEL) | MIDI_PUT_DATA2(LastKey) | MIDI_PUT_DATA3(VKB_NOTE_OFF_VELOCITY);
        OS_PutMail(&_MidiMailbox, &Event);
        _BroadcastEvent(Event);
      }
      if (Key >= 0) {
        Event = MIDI_PUT_STATUS(0x90) | MIDI_PUT_CHANNEL(VKB_MIDI_CHANNEL) | MIDI_PUT_DATA2(Key) | MIDI_PUT_DATA3(VKB_NOTE_ON_VELOCITY);
        OS_PutMail(&_MidiMailbox, &Event);
        _BroadcastEvent(Event);
      }
      LastKey = Key;
    }
    OS_Delay(10);
  }
}

/*********************************************************************
*
*       _KbRecvTask()
*
*  Function description
*    Virtual keyboard receive task.
*
*  Parameters
*    pContext - Pointer to user-provided context.
*
*  Additional information
*    Waits for incoming MIDI events and posts them to the display
*    task which decodes them and updates the virtual keyboard to
*    reflect the physical key state.
*/
static void _KbRecvTask(void *pContext) {
  MIDI_DEVICE * pInst;
  U32           Event;
  //
  pInst = (MIDI_DEVICE *)pContext;
  //
  for (;;) {
    switch (USBH_MIDI_RdEvent(pInst->hMIDI, &Event)) {
      //
     case USBH_STATUS_SUCCESS:
       OS_PutMail(&_MidiMailbox, &Event);
       break;
       //
     case USBH_STATUS_DEVICE_REMOVED:
     case USBH_STATUS_NOT_OPENED:
       OS_Terminate(NULL);
       break;
       //
     default:
       break;
    }
  }
}

/*********************************************************************
*
*       _MIDI_DeviceEvent()
*
*  Function description
*    Called when a device is added or removed.
*
*  Parameters
*    pContext - Pointer to user-provided context.
*    DevIndex - Zero-based device index.
*    Event    - Reason for callback.
*
*  Additional information
*    Called in the context of the USBH task.
*/
static void _MIDI_DeviceEvent(void * pContext, U8 DevIndex, USBH_DEVICE_EVENT Event) {
  USBH_MIDI_DEVICE_INFO DevInfo;
  USBH_STATUS           Status;
  //
  (void)pContext;
  //
  switch (Event) {
  case USBH_DEVICE_EVENT_ADD:
    if (_aMidiDevice[DevIndex].hMIDI != USBH_MIDI_INVALID_HANDLE) {
      USBH_Warnf_Application("MIDI device %d already open!", DevIndex);
    } else {
      _aMidiDevice[DevIndex].hMIDI = USBH_MIDI_Open(DevIndex);
      if (_aMidiDevice[DevIndex].hMIDI != USBH_MIDI_INVALID_HANDLE) {
        Status = USBH_MIDI_GetDeviceInfo(_aMidiDevice[DevIndex].hMIDI, &DevInfo);
        if (Status == USBH_STATUS_SUCCESS) {
          USBH_Logf_Application("MIDI device %d added: %u In cables, %u Out cables", DevIndex, DevInfo.NumInCables, DevInfo.NumOutCables);
        } else {
          USBH_Warnf_Application("Error getting MIDI device %d information!", DevIndex);
        }
      } else {
        USBH_Warnf_Application("Error opening MIDI device %d!", DevIndex);
      }
    }
    break;
    //
  case USBH_DEVICE_EVENT_REMOVE:
    if (_aMidiDevice[DevIndex].hMIDI == USBH_MIDI_INVALID_HANDLE) {
      USBH_Warnf_Application("MIDI device %d already removed!", DevIndex);
    } else {
      //
      // Wait for receiver task to terminate itself.
      //
      while (OS_TASK_IsTask(&_aMidiDevice[DevIndex].RxTask)) {
        OS_Delay(10);
      }
      //
      (void)USBH_MIDI_Close(_aMidiDevice[DevIndex].hMIDI);
      //
      _aMidiDevice[DevIndex].hMIDI = USBH_MIDI_INVALID_HANDLE;
      USBH_Logf_Application("MIDI device %d removed", DevIndex);
    }
    break;
    //
  default:
    break;
  }
  //
  // Now get the device and manufacturer name of the "primary" device.
  //
  if (_aMidiDevice[DevIndex].hMIDI != USBH_MIDI_INVALID_HANDLE) {
    USBH_INTERFACE_HANDLE hInterface;
    U8                    aDesc[18];
    unsigned              DescLen;
    U8                    iManufacturer;
    U8                    iProduct;
    //
    _aMidiDevice[DevIndex].aProductName[0]      = 0;
    _aMidiDevice[DevIndex].aManufacturerName[0] = 0;
    //
    USBH_MIDI_GetDeviceInfo(_aMidiDevice[DevIndex].hMIDI, &DevInfo);
    Status = USBH_OpenInterface(DevInfo.InterfaceId, 0, &hInterface);
    if (Status == USBH_STATUS_SUCCESS) {
      Status = USBH_GetDeviceDescriptor(hInterface, aDesc, &DescLen);
      if (Status == USBH_STATUS_SUCCESS) {
        iManufacturer = aDesc[14];
        iProduct      = aDesc[15];
        Status = USBH_GetStringDescriptorASCII(hInterface,
                                               iManufacturer,
                                               &_aMidiDevice[DevIndex].aManufacturerName[0],
                                               sizeof(_aMidiDevice[DevIndex].aManufacturerName));
        if (Status != USBH_STATUS_SUCCESS) {
          _aMidiDevice[DevIndex].aManufacturerName[0] = 0;
        }
        Status = USBH_GetStringDescriptorASCII(hInterface,
                                               iProduct,
                                               &_aMidiDevice[DevIndex].aProductName[0],
                                               sizeof(_aMidiDevice[DevIndex].aProductName));
        if (Status != USBH_STATUS_SUCCESS) {
          _aMidiDevice[DevIndex].aProductName[0] = 0;
        }
        //
      }
      USBH_CloseInterface(hInterface);
    }
    //
    if (_aMidiDevice[DevIndex].aManufacturerName[0] == 0) {
      strcpy(_aMidiDevice[DevIndex].aManufacturerName, "Unknown");
    }
    if (_aMidiDevice[DevIndex].aProductName[0] == 0) {
      strcpy(_aMidiDevice[DevIndex].aProductName, "Product");
    }
    //
    strcpy(_aMidiDevice[DevIndex].aTaskName, _aMidiDevice[DevIndex].aManufacturerName);
    strcat(_aMidiDevice[DevIndex].aTaskName, " ");
    strcat(_aMidiDevice[DevIndex].aTaskName, _aMidiDevice[DevIndex].aProductName);
    //
    OS_CreateTaskEx(&_aMidiDevice[DevIndex].RxTask,
                    _aMidiDevice[DevIndex].aTaskName,
                    TASK_PRIO_KB_RECV,
                    _KbRecvTask,
                    &_aMidiDevice[DevIndex].aStack[0],
                    sizeof(_aMidiDevice[DevIndex].aStack)
                    CTPARA_TIMESLICE,
                    &_aMidiDevice[DevIndex]);
  }
  //
  // Post a null event to update the display.
  //
  OS_PutMail(&_MidiMailbox, 0u);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       MainTask()
*
*  Function description
*    Start all virtual keyboard tasks.
*/
void MainTask(void) {
  //
  // If the GUI is was not initialized before, enable the following lines:
  //
#if 1
  GUI_Init();
#endif
  //
  USBH_Init();
  USBH_MIDI_Init();
  USBH_MIDI_AddNotification(&_Hook, _MIDI_DeviceEvent, NULL);
  USBH_SetWarnFilter(0);
  //                                                                                          // Tasks using emUSB-Host API should always have a lower priority than emUSB-Host main and ISR tasks.
  _hMemDev = GUI_MEMDEV_Create(0, 0, LCD_GetXSize(), LCD_GetYSize());
  if (_hMemDev == 0) {
    GUI_X_Warn("Cannot create memdev!");
    return;
  }
  //
  // Create mailbox to store the MIDI events.
  //
  OS_CREATEMB(&_MidiMailbox, sizeof(U32), MAX_DATA_ITEMS, &_aMidiEvents);
  //
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_APP);                                               // This task has the lowest prio for real-time application.
  OS_CREATETASK(&_TCBMain,   "USBH_Task", USBH_Task,    TASK_PRIO_USBH_MAIN, _StackMain);      // Start USBH main task
  OS_CREATETASK(&_TCBIsr ,   "USBH_ISR",  USBH_ISRTask, TASK_PRIO_USBH_ISR,  _StackIsr);       // Start USBH ISR task
  OS_CREATETASK(&_TCBKbDisp, "KbDisp",    _KbDispTask,  TASK_PRIO_KB_DISP,   _StackKbDisp);    // Start task managing keyboard display
  OS_CREATETASK(&_TCBKbPlay, "KbPlay",    _KbPlayTask,  TASK_PRIO_KB_PLAY,   _StackKbPlay);    // Start task managing keyboard play
  //
  for (;;) {
    OS_Delay(250);
    BSP_ToggleLED(2);
  }
}

/*************************** End of file ****************************/
