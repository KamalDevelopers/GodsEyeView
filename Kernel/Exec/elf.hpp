#ifndef ELF_HPP
#define ELF_HPP

#include "../Mem/mm.hpp"
#include "../Mem/paging.hpp"
#include "LibC/stdlib.hpp"
#include "loader.hpp"

#define ELF_NIDENT 16
#define EM_386 (3)     /* x86 Machine Type */
#define EV_CURRENT (1) /* Current Version */
#define ROUND_UP(N, S) ((((N) + (S)-1) / (S)) * (S))
#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

/* e_ident[] magic number */
#define ELFMAG0 0x7f     /* e_ident[EI_MAG0] */
#define ELFMAG1 'E'      /* e_ident[EI_MAG1] */
#define ELFMAG2 'L'      /* e_ident[EI_MAG2] */
#define ELFMAG3 'F'      /* e_ident[EI_MAG3] */
#define ELFMAG "\177ELF" /* magic */
#define SELFMAG 4        /* size of magic */

/* e_ident[] file class */
#define ELFCLASSNONE 0 /* invalid */
#define ELFCLASS32 1   /* 32-bit objs */
#define ELFCLASS64 2   /* 64-bit objs */
#define ELFCLASSNUM 3  /* number of classes */

/* e_ident[] data encoding */
#define ELFDATANONE 0 /* invalid */
#define ELFDATA2LSB 1 /* Little-Endian */
#define ELFDATA2MSB 2 /* Big-Endian */
#define ELFDATANUM 3  /* number of data encode defines */

/* e_ident[] Operating System/ABI */
#define ELFOSABI_SYSV 0         /* UNIX System V ABI */
#define ELFOSABI_HPUX 1         /* HP-UX operating system */
#define ELFOSABI_NETBSD 2       /* NetBSD */
#define ELFOSABI_LINUX 3        /* GNU/Linux */
#define ELFOSABI_HURD 4         /* GNU/Hurd */
#define ELFOSABI_86OPEN 5       /* 86Open common IA32 ABI */
#define ELFOSABI_SOLARIS 6      /* Solaris */
#define ELFOSABI_MONTEREY 7     /* Monterey */
#define ELFOSABI_IRIX 8         /* IRIX */
#define ELFOSABI_FREEBSD 9      /* FreeBSD */
#define ELFOSABI_TRU64 10       /* TRU64 UNIX */
#define ELFOSABI_MODESTO 11     /* Novell Modesto */
#define ELFOSABI_OPENBSD 12     /* OpenBSD */
#define ELFOSABI_ARM 97         /* ARM */
#define ELFOSABI_STANDALONE 255 /* Standalone (embedded) application */

typedef uint16_t elf32_half;
typedef uint32_t elf32_off;
typedef uint32_t elf32_addr;
typedef uint32_t elf32_word;
typedef int32_t elf32_sword;

typedef struct {
    uint32_t sig;
} elf_priv_data;

typedef struct {
    unsigned char ei_magic[4]; /* 0x7F"ELF" */
    uint8_t ei_subarch;
    uint8_t ei_endian;
    uint8_t ei_elfver;
    uint8_t ei_abi;
    unsigned char ei_unused[8];
} elf_ident_t;

/* ELF Header */
typedef struct elfhdr {
    elf_ident_t e_ident;
    elf32_half e_type;      /* object file type */
    elf32_half e_machine;   /* machine */
    elf32_word e_version;   /* object file version */
    elf32_addr e_entry;     /* virtual entry point */
    elf32_off e_phoff;      /* program header table offset */
    elf32_off e_shoff;      /* section header table offset */
    elf32_word e_flags;     /* processor-specific flags */
    elf32_half e_ehsize;    /* ELF header size */
    elf32_half e_phentsize; /* program header entry size */
    elf32_half e_phnum;     /* number of program header entries */
    elf32_half e_shentsize; /* section header entry size */
    elf32_half e_shnum;     /* number of section header entries */
    elf32_half e_shstrndx;  /* section header table's "section
                                           header string table" entry offset */
} elf32_ehdr;

/* Section Header */
typedef struct {
    elf32_word sh_name;      /* name - index into section header
                                           string table section */
    elf32_word sh_type;      /* type */
    elf32_word sh_flags;     /* flags */
    elf32_addr sh_addr;      /* address */
    elf32_off sh_offset;     /* file offset */
    elf32_word sh_size;      /* section size */
    elf32_word sh_link;      /* section header table index link */
    elf32_word sh_info;      /* extra information */
    elf32_word sh_addralign; /* address alignment */
    elf32_word sh_entsize;   /* section entry size */
} elf32_shdr;

/* Program Header */
typedef struct {
    elf32_word p_type;   /* segment type */
    elf32_off p_offset;  /* segment offset */
    elf32_addr p_vaddr;  /* virtual address of segment */
    elf32_addr p_paddr;  /* physical address - ignored? */
    elf32_word p_filesz; /* number of bytes in file for seg. */
    elf32_word p_memsz;  /* number of bytes in mem. for seg. */
    elf32_word p_flags;  /* flags */
    elf32_word p_align;  /* memory alignment */
} elf32_phdr;

class Elf : public Execf {
private:
    char format_name[100];

public:
    Elf(char* name);
    ~Elf();
    int probe(uint8_t* file_data);
    executable_t exec(uint8_t* file_data);
    char* name();
};

#endif
