<br><h1 align="center">GevOS</h1>
GevOS is a 32 bit operating system which implements VESA graphics, a TAR filesystem, an IPV4 networking stack, a LibC & LibC++ standard library, multiple audio and networking drivers, as well as full userland support. It can also run on real hardware without issues. <br><br>

![gevosnew.png](https://i.postimg.cc/1tQhjgVZ/gevos1.png)

### Features
  * System servers which communicate with other processes using IPC, [DisplayServer](Userland/Servers/Display/) & [SoundServer](Userland/Servers/Sound/)
  * Functional es1370 driver for high quality audio playback
  * Kernel and userland libraries such as [LibCompress](Libraries/LibCompress) and [LibFont](Libraries/LibFont)
  * An [IPv4](Kernel/Net/) networking stack, with a selection of multiple different underlying network card drivers
  * Support for the protocols: TCP, UDP, ICMP, DHCP, DNS
  * Read and write [TAR](Kernel/Filesystem/) filesystem as well as a VFS
  * [Terminal](Userland/Apps/Terminal) application, [Shell](Userland/Apps/Shell) program, [Brainfuck](Userland/Apps/Brainfuck) interpreter, and many [more](Userland/Apps/) 
  * Different ports, including Koenk's GameBoy Color emulator [GBC](Userland/Ports/GBC)
  * A tiling and minimalistic [WM](Userland/Servers/Display) inspired by [dwm](https://dwm.suckless.org/)

### Building GevOS

Instructions on how to build GevOS are on the [Wiki](https://github.com/KamalDevelopers/GevOS/wiki/Building-GevOS)

### Contributing to GevOS

Instructions and requirements on contributing to GevOS are on the [Wiki](https://github.com/KamalDevelopers/GevOS/wiki/Contributing-to-GevOS)

