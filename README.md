<h2 align="center">GevOS</h2>
GevOS is a 32 bit operating system which implements VESA graphics, a TAR filesystem, an IPV4 networking stack, a LibC standard library, multiple audio and networking drivers, as well as full userland support. It can also run on real hardware without issues.<br><br>

![gevosnew.png](https://i.postimg.cc/Z5mL69dx/gevos.png)

### Features
  * System servers which communicate with other processes using IPC, [DisplayServer](Userland/Servers/Display/) & [SoundServer](Userland/Servers/Sound/)
  * Functional es1370 driver for high quality audio playback
  * A WIP networking stack, with a selection of multiple different underlying network card drivers
  * Read and write [TAR](Kernel/Filesystem/) filesystem as well as a VFS
  * [Terminal](Userland/Apps/Terminal) application, [Shell](Userland/Apps/Shell) program, [Brainfuck](Userland/Apps/Brainfuck) interpreter, and many [more](Userland/Apps/) 
  * A tiling and minimalistic [WM](Userland/Servers/Display) inspired by [dwm](https://dwm.suckless.org/)

### Building GevOS

Instructions on how to build GevOS are on the [Wiki](https://github.com/KamalDevelopers/GevOS/wiki/Building-GevOS)

### Contributing to GevOS

Instructions and requirements on contributing to GevOS are on the [Wiki](https://github.com/KamalDevelopers/GevOS/wiki/Contributing-to-GevOS)

