#include <LibC/unistd.h>
#include <LibC/string.h>
#include <LibC/ctype.h>
#include <LibC/stdio.h>
#include <LibC/stat.h>

#define INITL0_PATH "/initl/initl0"
#define LIST_CHRMAX 50

static char list0[20][LIST_CHRMAX];
static char list1[20][LIST_CHRMAX];
static uint16_t list0_size;
static uint16_t list1_size;
static int8_t list0_enabled[20];
static int8_t list1_enabled[20];
static char* buff[1024];

int init1()
{
    for (uint16_t i = 0; i < list1_size; i++)
        if (list1_enabled[i])
            spawn_orphan(list1[i], 0, 0);
    return 0;
}

int init0()
{
    for (uint16_t i = 0; i < list0_size; i++)
        if (list0_enabled[i])
            spawn_orphan(list0[i], 0, 0);
    return 0;
}

int initl()
{
    list0_size = 0;
    list1_size = 0;
    int fd = open(INITL0_PATH, O_RDONLY);
    int rs = read(fd, buff, sizeof(buff) - 1);
    buff[rs] = 0;
    close(fd);

    char* entry = strtok((char*)buff, "\n");

    while (entry) {
        char* entry_cmp = entry;
        uint8_t start_order = 0;
        int8_t is_enabled = 0;
        size_t size = strlen(entry);

        if (size <= 5 || size > LIST_CHRMAX) {
            entry = strtok(NULL, "\n");
            continue;
        }

        if (isdigit(entry[0]) || entry[0] > LIST_CHRMAX) {
            start_order = entry[0] - 48;
            entry += 3;
        }

        if (size >= 12 && strncmp(entry, "enabled", 7) == 0) {
            is_enabled = 1;
            entry += 7;
        }

        while (!isalpha(*entry) && ((entry - entry_cmp) < size))
            entry++;

        char* list = (start_order) ? list1[list1_size] : list0[list0_size];
        strncpy(list, entry, size - (entry - entry_cmp));

        if (is_enabled)
            start_order ? list1_enabled[list1_size]++ : list0_enabled[list0_size]++;
        start_order ? list1_size++ : list0_size++;
        entry = strtok(NULL, "\n");
    }

    return 0;
}

void initl_flush()
{
    memset(buff, 0, sizeof(buff));
    for (size_t i = 0; i < list0_size; i++) {
        if (strlen(list0[i]) == 0)
            continue;
        strcat((char*)buff, "0: ");
        if (list0_enabled[i])
            strcat((char*)buff, "enabled");
        strcat((char*)buff, ": ");
        strcat((char*)buff, list0[i]);
        strcat((char*)buff, "\n");
    }

    for (size_t i = 0; i < list1_size; i++) {
        if (strlen(list1[i]) == 0)
            continue;
        strcat((char*)buff, "1: ");
        if (list1_enabled[i])
            strcat((char*)buff, "enabled");
        strcat((char*)buff, ": ");
        strcat((char*)buff, list1[i]);
        strcat((char*)buff, "\n");
    }

    int fd = open(INITL0_PATH, O_RDWR);
    write(fd, buff, strlen((const char*)buff));
    close(fd);
}

void ctrl(int argc, char** argv)
{
    static const char* help = 
        "Usage: init <command> [entry] [level]\n\n"
        "commands:\n"
        "    list    - list all entries\n"
        "    add     - add an entry\n"
        "    remove  - remove an entry\n"
        "    enable  - enable an entry\n"
        "    disable - disable an entry\n";
    if (!argc || (argc && (strncmp(argv[0], "help", 4) == 0))) {
        printf("%s", help);
        return;
    }

    initl();

    if ((strlen(argv[0]) == 4) && strncmp(argv[0], "list", 4) == 0) {
        printf("[0]\n");
        for (uint16_t i = 0; i < list0_size; i++)
            printf("   %s: %s\n", list0_enabled[i] ?
                    "\34\x2\xF [enabled]\34\x3" : "\34\x2\x1 [disabled]\34\x3", list0[i]);
        printf("[1]\n");
        for (uint16_t i = 0; i < list1_size; i++)
            printf("   %s: %s\n", list1_enabled[i] ?
                    "\34\x2\xF [enabled]\34\x3" : "\34\x2\x1 [disabled]\34\x3", list1[i]);
        return;
    }

    if (argc < 3 || !isdigit(argv[2][0]) || strlen(argv[2]) > 1) {
        printf("%s", help);
        return;
    }

    const char* entry = argv[1];
    uint8_t level = argv[2][0] - 48;
    if (level > 1) {
        printf("Level can only be 0 or 1\n");
        return;
    }

    if (strlen(argv[1]) > 35) {
        printf("Argument size too great\n");
        return;
    }

    if (strcmp(argv[0], "enable") == 0 ||
        strcmp(argv[0], "remove") == 0 ||
        strcmp(argv[0], "disable") == 0) {
        bool toggle = (argv[0][0] == 'e');
        bool remove = (argv[0][0] == 'r');

        if (level) {
            for (size_t i = 0; i < list1_size; i++) {
                if (strcmp(list1[i], argv[1]) == 0) {
                    list1_enabled[i] = toggle;
                    if (remove) list1[i][0] = 0;
                }
            }
        }
        else {
            for (size_t i = 0; i < list0_size; i++) {
                if (strcmp(list0[i], argv[1]) == 0) {
                    list0_enabled[i] = toggle;
                    if (remove) list0[i][0] = 0;
                }
            }
        }

        initl_flush();
        return;
    }

    if (strcmp(argv[0], "add") == 0) {
        struct stat statbuffer;
        if (stat(argv[1], &statbuffer) < 0) {
            printf("Argument \"%s\" is not a valid file\n", argv[1]);
            return;
        }

        char* list = (level) ? list1[list1_size] : list0[list0_size];
        memset(list, 0, LIST_CHRMAX);
        strcpy(list, argv[1]);
        level ? list1_size++ : list0_size++;
        initl_flush();
        return;
    }
}

int main(int argc, char** argv)
{
    int pid = getpid();

    if (pid <= 2) {
        initl();
        init0();
        init1();
        return 0;
    }

    ctrl(argc, argv);
    return 0;
}
