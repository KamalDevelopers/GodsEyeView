#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/utsname.hpp>

#define MB 1000000
#define PAGE_SIZE 4096

int main(int argc, char** argv)
{
    const char img[13][50] = {
        "     .:               ",
        "    o+   .......      ",
        "  .oo  :YoooooooT.    ",
        "./oY  ;oo       .o.   ",
        ":oo=  oV   +o.   oo   ",
        ":oot  to    ot   Io   ",
        " ooo   ;;   +o.  Xo   ",
        "  ;ooX;      YoIoX:   ",
        "   .+ooooIiitooY;     ",
        "      :;+ii+;to:      ",
        "   ;.        io       ",
        "    Yo     .V:        ",
        "     .ooooo+          "
    };

    const char cinfo[2][10] = { "\33\x2\x6%s\33\x3", "\33\x2\xF%s\33\x3" };
    const char* user = "terry";
    const char uinfo[] = "------- \33\x2\xF%s\33\x2\x7@\33\x2\xF%s\33\x3 ------- ";
    struct osinfo info;
    utsname uname_struct;
    uname(&uname_struct);
    sys_osinfo(&info);
    lowercase(uname_struct.sysname);

    uint32_t uptime = info.uptime;
    unsigned uptime_sec = uptime % 60;
    uptime /= 60;
    unsigned uptime_min = uptime % 60;

    printf("\n");
    for (uint32_t line = 0; line < 13; line++) {
        printf(cinfo[line % 2], img[line]);
        if (line == 0)
            printf(uinfo, user, uname_struct.sysname);
        if (line == 2)
            printf("\33\x2\xC   . uptime \33\x2\xF %d min %d sec", uptime_min, uptime_sec);
        if (line == 3)
            printf("\33\x2\xC   . used memory \33\x2\xF %d MB", (info.used_pages * PAGE_SIZE) / MB);
        if (line == 4)
            printf("\33\x2\xC   . free memory \33\x2\xF %d MB", (info.free_pages * PAGE_SIZE) / MB);
        if (line == 5)
            printf("\33\x2\xC   . procs \33\x2\xF %d", info.procs);
        if (line == 7)
            printf("\33\x2\xCkamaldevelopers \33\x2\xF  2020-2022");
        printf("\n");
    }

    return 0;
}
