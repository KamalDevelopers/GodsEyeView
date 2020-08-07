#include "mouse.hpp"

MouseDriver::MouseDriver(InterruptManager* manager, int screenw, int screenh)//, Graphics* v)
    : InterruptHandler(manager, 0x2C)
    , dataport(0x60)
    , commandport(0x64)
{
    //vga = v;
    //gui = d;
    offset = 0;
    buttons = 0;
    //w = v->GetScreenW();
    //h = v->GetScreenH();
    w = screenw;
    h = screenh;
    
    commandport.Write(0xA8);
    commandport.Write(0x20);
    uint8_t status = dataport.Read() | 2;
    commandport.Write(0x60);
    dataport.Write(status);

    commandport.Write(0xD4);
    dataport.Write(0xF4);
    dataport.Read();
}

MouseDriver::~MouseDriver()
{
}

void MouseDriver::OnMouseMove(int x, int y)
{
    //x /= 2;
    //y /= 2;

    int32_t newMouseX = MouseX + x;
    if (newMouseX < 0)
        newMouseX = 0;
    if (newMouseX >= w - 2)
        newMouseX = w - 2;

    int32_t newMouseY = MouseY + y;
    if (newMouseY < 0)
        newMouseY = 0;
    if (newMouseY >= h - 20)
       newMouseY = h - 20;

    //for (int t_y = 0; t_y < 10; t_y++)
    //    vga->PutPixel(MouseX, MouseY+t_y, 0x1);

    //for (int t_x = 0; t_x < 5; t_x++)
    //    vga->PutPixel(MouseX+t_x, MouseY+5, 0x1);

    //for (int t_x = 0; t_x < 5; t_x++)
    //    vga->PutPixel(MouseX-t_x, MouseY+5, 0x1);

    MouseX = newMouseX;
    MouseY = newMouseY;
}

void MouseDriver::OnMouseUp(int b)
{
    //gui->MouseRelease(MouseX, MouseY, b);
    MousePress = 0;
}

void MouseDriver::OnMouseDown(int b)
{
    //gui->MousePress(MouseX, MouseY, b);
    MousePress = 1;
}

uint32_t MouseDriver::HandleInterrupt(uint32_t esp)
{
    uint8_t status = commandport.Read();
    if (!(status & 0x20))
        return esp;

    buffer[offset] = dataport.Read();
    offset = (offset + 1) % 3;

    if (offset == 0) {
        if (buffer[1] != 0 || buffer[2] != 0) {
            OnMouseMove((int8_t)buffer[1], -((int8_t)buffer[2]));
        }

        for (uint8_t i = 0; i < 3; i++) {
            if ((buffer[0] & (0x1 << i)) != (buttons & (0x1 << i))) {
                if (buttons & (0x1 << i)) //Todo
                    OnMouseUp(i + 1);
                else
                    OnMouseDown(i + 1);
            }
        }

        buttons = buffer[0];
    }
    return esp;
}
