#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/utsname.h>

#define MB 1000000
#define PAGE_SIZE 4096

int main(int argc, char** argv)
{
    const char img[12][50] {
        "\34\x2\xC             _         ",
        "\34\x2\xC            /\\ \\       ",
        "\34\x2\xC           /  \\ \\      ",
        "\34\x2\xC          / /\\ \\_\\     ",
        "\34\x2\xC         / / /\\/_/     \34\x2\xF",
        "        / / /\34\x2\xC ______   \34\x2\xF",
        "       / / /\34\x2\xC /\\_____\\  \34\x2\xF",
        "      / / /\34\x2\xC  \\/____ /  \34\x2\xF",
        "     / / /_____\34\x2\xC/ / /   \34\x2\xF",
        "    / / /______\34\x2\xC\\/ / \34\x2\xC/\\ \34\x2\xF",
        "    \\/___________\34\x2\xC/ \34\x2\xC/__\\\34\x2\xF",
    };

    const char cinfo[2][10] = { "\34\x2\xF%s\34\x3", "\34\x2\xC%s\34\x3" };
    const char* user = "terry";
    const char uinfo[] = "\34\x2\xC <<<   \34\x2\xF%s\34\x2\x7@\34\x2\xF%s\34\x3";

    struct osinfo info;
    utsname uname_struct;
    uname(&uname_struct);
    sys_osinfo(&info);
    lowercase(uname_struct.sysname);

    uint32_t screen_width = 0;
    uint32_t screen_height = 0;
    int fd = open("/dev/display", O_RDONLY);
    uint32_t buffer[3];
    if (read(fd, buffer, sizeof(uint32_t))) {
        screen_width = buffer[1];
        screen_height = buffer[2];
    }

    uint32_t uptime = info.uptime;
    unsigned uptime_sec = uptime % 60;
    uptime /= 60;
    unsigned uptime_min = uptime % 60;

    int tasks_sleeping = info.procs_sleeping;
    int tasks_polling = info.procs_polling;
    int tasks_zombie = info.procs_zombie;
    int tasks_running = info.procs - tasks_sleeping - tasks_polling - tasks_zombie;

    printf("\n");
    for (uint32_t line = 0; line < 12; line++) {
        printf("%s", img[line]);
        printf("   ");
        if (line == 0)
            printf(uinfo, user, uname_struct.sysname);
        if (line == 2)
            printf("\34\x2\xC       . uptime \34\x2\xF %d min %d sec", uptime_min, uptime_sec);
        if (line == 3)
            printf("\34\x2\xC       . cpu \34\x2\xF %s [%s]", info.cpu_string, info.cpu_is64 ? "64 bit" : "32 bit");
        if (line == 4)
            printf("\34\x2\xC       . kernel \34\x2\xF gevos [dev]");
        if (line == 5)
            printf("\34\x2\xC       . free memory \34\x2\xF %d MB", (info.free_pages * PAGE_SIZE) / MB);
        if (line == 6)
            printf("\34\x2\xC       . used memory \34\x2\xF %d MB", (info.used_pages * PAGE_SIZE) / MB);
        if (line == 7)
            printf("\34\x2\xC       . procs \34\x2\xF %dr %ds %dp %dz", tasks_running, tasks_sleeping, tasks_polling, tasks_zombie);
        if (line == 8)
            printf("\34\x2\xC       . video \34\x2\xF %d x %d", screen_width, screen_height);
        if (line == 10)
            printf("\34\x2\x7 2020 - 2023");
        printf("\n");
    }

    return 0;
}
