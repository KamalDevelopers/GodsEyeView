#include "keyboard.hpp"

KeyboardDriver::KeyboardDriver(InterruptManager* manager)
: InterruptHandler(manager, 0x21),
dataport(0x60),
commandport(0x64)
{
    while(commandport.Read() & 0x1)
        dataport.Read();
    commandport.Write(0xae); // activate interrupts
    commandport.Write(0x20); // command 0x20 = read controller command byte
    uint8_t status = (dataport.Read() | 1) & ~0x10;
    commandport.Write(0x60); // command 0x60 = set controller command byte
    dataport.Write(status);
    dataport.Write(0xf4);
}

KeyboardDriver::~KeyboardDriver()
{
}

char* KeyboardDriver::GetKeys(char* arr)
{
    int size = str_len(keys);
    for (int i = 0; i < size; i++){
        arr[i] = keys[i];
    }
    arr[size-1] = '\0';

    return keys;
}

char KeyboardDriver::GetLastKey()
{
    return keys[key_press_index-1];
}

void KeyboardDriver::on_key(char keypress, int out_screen)
{
    keys[key_press_index] = keypress;
    key_press_index++;
    if (out_screen == 1){
        if (keypress != '~')
            putc(keypress);
    }
    if (out_screen == 2){
        if (keypress == '~'){
            vga->Print("/~", 0x0);
            return;
        }
        vga->RenderBitMap(font_basic[keypress], color);
    }
}

void KeyboardDriver::ScreenOutput(int i, uint8_t color_index, Graphics *g)
{
    color = color_index;
    vga = g;
    outp = i;
} 

uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp)
{
    uint8_t key = dataport.Read();
    if(key < 0x80)
    {
        switch(key)
        {
            case 0x02: on_key('1', outp); break;
            case 0x03: on_key('2', outp); break;
            case 0x04: on_key('3', outp); break;
            case 0x05: on_key('4', outp); break;
            case 0x06: on_key('5', outp); break;
            case 0x07: on_key('6', outp); break;
            case 0x08: on_key('7', outp); break;
            case 0x09: on_key('8', outp); break;
            case 0x0A: on_key('9', outp); break;
            case 0x0B: on_key('0', outp); break;

            case 0x10: on_key('q', outp); break;
            case 0x11: on_key('w', outp); break;
            case 0x12: on_key('e', outp); break;
            case 0x13: on_key('r', outp); break;
            case 0x14: on_key('t', outp); break;
            case 0x15: on_key('y', outp); break;
            case 0x16: on_key('u', outp); break;
            case 0x17: on_key('i', outp); break;
            case 0x18: on_key('o', outp); break;
            case 0x19: on_key('p', outp); break;

            case 0x1E: on_key('a', outp); break;
            case 0x1F: on_key('s', outp); break;
            case 0x20: on_key('d', outp); break;
            case 0x21: on_key('f', outp); break;
            case 0x22: on_key('g', outp); break;
            case 0x23: on_key('h', outp); break;
            case 0x24: on_key('j', outp); break;
            case 0x25: on_key('k', outp); break;
            case 0x26: on_key('l', outp); break;

            case 0x2C: on_key('z', outp); break;
            case 0x2D: on_key('x', outp); break;
            case 0x2E: on_key('c', outp); break;
            case 0x2F: on_key('v', outp); break;
            case 0x30: on_key('b', outp); break;
            case 0x31: on_key('n', outp); break;
            case 0x32: on_key('m', outp); break;
            case 0x33: on_key(',', outp); break;
            case 0x34: on_key('.', outp); break;
            case 0x35: on_key('-', outp); break;

            case 0x1C: on_key('\n', outp); break;
            case 0x39: on_key(' ', outp); break;
            case 0x0E: printf("\b%s\b", " "); 
                       on_key('~', outp); break;

            default:
            {
                printf("%x", key);
                break;
            }
        }
    }
    return esp;
}
