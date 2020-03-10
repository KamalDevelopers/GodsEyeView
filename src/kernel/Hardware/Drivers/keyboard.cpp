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

uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp)
{
    uint8_t key = dataport.Read();
    if(key < 0x80)
    {
        switch(key)
        {
            case 0x02: printf("%s", "1"); break;
            case 0x03: printf("%s", "2"); break;
            case 0x04: printf("%s", "3"); break;
            case 0x05: printf("%s", "4"); break;
            case 0x06: printf("%s", "5"); break;
            case 0x07: printf("%s", "6"); break;
            case 0x08: printf("%s", "7"); break;
            case 0x09: printf("%s", "8"); break;
            case 0x0A: printf("%s", "9"); break;
            case 0x0B: printf("%s", "0"); break;

            case 0x10: printf("%s", "q"); break;
            case 0x11: printf("%s", "w"); break;
            case 0x12: printf("%s", "e"); break;
            case 0x13: printf("%s", "r"); break;
            case 0x14: printf("%s", "t"); break;
            case 0x15: printf("%s", "y"); break;
            case 0x16: printf("%s", "u"); break;
            case 0x17: printf("%s", "i"); break;
            case 0x18: printf("%s", "o"); break;
            case 0x19: printf("%s", "p"); break;

            case 0x1E: printf("%s", "a"); break;
            case 0x1F: printf("%s", "s"); break;
            case 0x20: printf("%s", "d"); break;
            case 0x21: printf("%s", "f"); break;
            case 0x22: printf("%s", "g"); break;
            case 0x23: printf("%s", "h"); break;
            case 0x24: printf("%s", "j"); break;
            case 0x25: printf("%s", "k"); break;
            case 0x26: printf("%s", "l"); break;

            case 0x2C: printf("%s", "z"); break;
            case 0x2D: printf("%s", "x"); break;
            case 0x2E: printf("%s", "c"); break;
            case 0x2F: printf("%s", "v"); break;
            case 0x30: printf("%s", "b"); break;
            case 0x31: printf("%s", "n"); break;
            case 0x32: printf("%s", "m"); break;
            case 0x33: printf("%s", ","); break;
            case 0x34: printf("%s", "."); break;
            case 0x35: printf("%s", "-"); break;

            case 0x1C: printf("\n"); break;
            case 0x39: printf("%s", " "); break;
            case 0x0E: printf("\b%s\b", " "); break;

            default:
            {
                char* foo = "KEYBOARD 0x00 ";
                char* hex = "0123456789ABCDEF";
                foo[11] = hex[(key >> 4) & 0xF];
                foo[12] = hex[key & 0xF];
                printf("%s", foo);
                break;
            }
        }
    }
    return esp;
}
