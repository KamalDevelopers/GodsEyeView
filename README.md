<br><h1 align="center">GevOS</h1>
GevOS is a 32-bit operating system that implements VESA graphics, a TAR filesystem, an IPV4 networking stack, a LibC & LibC++ standard library, multiple audio and networking drivers, and userland support. It can also run on real hardware without issues. <br><br>

![gevos.png](https://i.postimg.cc/XJx1H2fb/gevos-screenshot.png)

### Features
  * System servers which communicate with other processes using IPC, [DisplayServer](Userland/Servers/Display/) & [SoundServer](Userland/Servers/Sound/)
  * Functional es1370 driver for high quality audio playback
  * Kernel and userland libraries such as [LibC++](Libraries/LibC++) and [LibFont](Libraries/LibFont)
  * An [IPv4](Kernel/Net/) networking stack, with a selection of multiple different underlying network card drivers
  * Support for the protocols: TCP, UDP, ICMP, DHCP, DNS
  * Read and write [TAR](Kernel/Filesystem/) filesystem as well as a VFS
  * [Terminal](Userland/Apps/Terminal) application, [Shell](Userland/Apps/Shell) program, [Brainfuck](Userland/Apps/Brainfuck) interpreter, and many [more](Userland/Apps/) 
  * Different ports, including Koenk's GameBoy Color emulator [GBC](Userland/Ports/GBC)
  * A tiling and minimalistic [WM](Userland/Servers/Display) inspired by [dwm](https://dwm.suckless.org/) which supports workspaces

### Building GevOS

Instructions on how to build GevOS are on the [Wiki](https://github.com/KamalDevelopers/GevOS/wiki/Building-GevOS)

### Contributing to GevOS

Support with expanding the system would be greatly appreciated! <br>
Instructions on contributing to GevOS are on the [Wiki](https://github.com/KamalDevelopers/GevOS/wiki/Contributing-to-GevOS)

