/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description Input subsystem interfaces
 *
 * @author bad3p
 */

#ifndef INPUT_INTERFACES_INCLUDED
#define INPUT_INTERFACES_INCLUDED

namespace input
{

/**
 * keycodes
 */

#define K_ESCAPE    0x01
#define K_1         0x02 
#define K_2         0x03    
#define K_3         0x04
#define K_4         0x05
#define K_5         0x06
#define K_6         0x07
#define K_7         0x08
#define K_8         0x09
#define K_9         0x0A
#define K_0         0x0B
#define K_MINUS     0x0C /* "-" */
#define K_EQUALS    0x0D /* "=" */
#define K_BACKSPACE 0x0E
#define K_TAB       0x0F
#define K_Q         0x10
#define K_W         0x11
#define K_E         0x12
#define K_R         0x13
#define K_T         0x14
#define K_Y         0x15
#define K_U         0x16
#define K_I         0x17
#define K_O         0x18
#define K_P         0x19
#define K_LBRAKET   0x1A /* "[" */
#define K_RBRAKET   0x1B /* "]" */
#define K_RETURN    0x1C
#define K_LCTRL     0x1D
#define K_A         0x1E
#define K_S         0x1F
#define K_D         0x20
#define K_F         0x21
#define K_G         0x22
#define K_H         0x23
#define K_J         0x24
#define K_K         0x25
#define K_L         0x26
#define K_SEMICOLON 0x27 /* ";" */
#define K_APOSTR    0x28 /* "\"" */
#define K_COLUMN    0x29 /* "|" */
#define K_LSHIFT    0x2A
#define K_BACKSLASH 0x2B /* "\\" */
#define K_Z         0x2C
#define K_X         0x2D
#define K_C         0x2E
#define K_V         0x2F
#define K_B         0x30
#define K_N         0x31
#define K_M         0x32
#define K_LESS      0x33 /* "<" */
#define K_GREATER   0x34 /* ">" */
#define K_SLASH     0x35 /* "/" */
#define K_RSHIFT    0x36
#define K_NUMSTAR   0x37 /* "Num *" */
#define K_LALT      0x38
#define K_SPACE     0x39
#define K_F1        0x3B
#define K_F2        0x3C
#define K_F3        0x3D
#define K_F4        0x3E
#define K_F5        0x3F
#define K_F6        0x40
#define K_F7        0x41
#define K_F8        0x42
#define K_F9        0x43
#define K_F10       0x44
#define K_NUMLOCK   0x45
#define K_SCRLOCK   0x46
#define K_NUM7      0x47
#define K_NUM8      0x48
#define K_NUM9      0x49
#define K_NUMMINUS  0x4A
#define K_NUM4      0x4B
#define K_NUM5      0x4C
#define K_NUM6      0x4D
#define K_NUMPLUS   0x4E
#define K_NUM1      0x4F
#define K_NUM2      0x50
#define K_NUM3      0x51
#define K_NUM0      0x52
#define K_NUMDOT    0x53 /* "Num ." */
#define K_F11       0x57
#define K_F12       0x58
#define K_RALT      0xB8
#define K_PAUSE     0xC5
#define K_HOME      0xC7
#define K_UP        0xC8
#define K_PAGEUP    0xC9
#define K_LEFT      0xCB
#define K_RIGHT     0xCD
#define K_END       0xCF
#define K_DOWN      0xD0
#define K_PAGEDOWN  0xD1
#define K_INSERT    0xD2
#define K_DELETE    0xD3

/**
 * device state structs
 */

struct KeyboardState
{
    unsigned char keyState[256];
};

struct MouseState
{
    int           dX, dY, dZ;
    unsigned char buttonState[8];
};

/**
 * input device
 */

class IInputDevice : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x60003);
public:
    /**
     * method fills the keyboard state structure
     * @return false if device is unacquired and cannot be re-acquire this time
     */
    virtual bool __stdcall getKeyboardState(KeyboardState* state) = 0;
    /**
     * method fills the mouse state structure
     * @return false if device is unacquired and cannot be re-acquire this time
     */
    virtual bool __stdcall getMouseState(MouseState* state) = 0;
    /**
     * method releases the input device
     */
    virtual void __stdcall release(void) = 0;
};

/**
 * input interface
 */

class IInput : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x60004);
public:
    /**
     * @return new input device
     */
    virtual IInputDevice* __stdcall createInputDevice(void) = 0;
    /**
     * @convert input key code to character code
     */
    virtual char __stdcall getChar(int keyCode, bool shift) = 0;
};

}

#endif
