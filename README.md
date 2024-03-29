<br><h1 align="center">God's Eye View Operating System</h1>
<strong>GevOS</strong> is a <strong>32-bit</strong> modern operating system with <strong>VESA</strong> graphics, a <strong>TAR filesystem</strong>, an <strong>IPV4 networking</strong> stack, a <strong>LibC & LibC++</strong> standard library, multiple audio and networking drivers, and different ports including a <strong>GBC emulator</strong>; all with userland support. GevOS runs on <strong>real hardware</strong>. <br><br>

![gevos.png](https://i.postimg.cc/Px4JtnPq/GEVIMG.png)

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
  * A custom [bootloader](Bootloader) with unreal mode, A20, LBA, VBE, and multiboot support
  * Supports SVG and PNG file formats using upng and nanosvg ports [LibImage](Libraries/LibImage)
  * Modern and sleek system with an overall clean design

### WM key sequences
  * `Ctrl+T` spawn a new terminal
  * `Esc` close active window
  * `Ctrl+Space` set active window as master
  * `Ctrl+Enter` fullscreen active window
  * `Ctrl+K` positive change active window
  * `Ctrl+J` negative change active window

### Building GevOS

Instructions on how to build GevOS are on the [Wiki](https://github.com/KamalDevelopers/GevOS/wiki/Building)

### Contributing to GevOS

Support with expanding the system would be greatly appreciated! <br>
Instructions on contributing to GevOS are on the [Wiki](https://github.com/KamalDevelopers/GevOS/wiki/Contributing)


