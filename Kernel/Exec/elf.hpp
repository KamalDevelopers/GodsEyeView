#ifndef ELF_HPP
#define ELF_HPP

#include "../Mem/mm.hpp"
#include "../Mem/paging.hpp"
#include "LibC/stdlib.hpp"
#include "loader.hpp"

#define ELF_NIDENT 16
#define EM_386 (3)     /* x86 Machine Type */
#define EV_CURRENT (1) /* Current Version */

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

typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;

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
} Elf_ident_t;

/* ELF Header */
typedef struct elfhdr {
    Elf_ident_t e_ident;
    Elf32_Half e_type;      /* object file type */
    Elf32_Half e_machine;   /* machine */
    Elf32_Word e_version;   /* object file version */
    Elf32_Addr e_entry;     /* virtual entry point */
    Elf32_Off e_phoff;      /* program header table offset */
    Elf32_Off e_shoff;      /* section header table offset */
    Elf32_Word e_flags;     /* processor-specific flags */
    Elf32_Half e_ehsize;    /* ELF header size */
    Elf32_Half e_phentsize; /* program header entry size */
    Elf32_Half e_phnum;     /* number of program header entries */
    Elf32_Half e_shentsize; /* section header entry size */
    Elf32_Half e_shnum;     /* number of section header entries */
    Elf32_Half e_shstrndx;  /* section header table's "section
					   header string table" entry offset */
} Elf32_Ehdr;

/* Section Header */
typedef struct {
    Elf32_Word sh_name;      /* name - index into section header
					   string table section */
    Elf32_Word sh_type;      /* type */
    Elf32_Word sh_flags;     /* flags */
    Elf32_Addr sh_addr;      /* address */
    Elf32_Off sh_offset;     /* file offset */
    Elf32_Word sh_size;      /* section size */
    Elf32_Word sh_link;      /* section header table index link */
    Elf32_Word sh_info;      /* extra information */
    Elf32_Word sh_addralign; /* address alignment */
    Elf32_Word sh_entsize;   /* section entry size */
} Elf32_Shdr;

/* Program Header */
typedef struct {
    Elf32_Word p_type;   /* segment type */
    Elf32_Off p_offset;  /* segment offset */
    Elf32_Addr p_vaddr;  /* virtual address of segment */
    Elf32_Addr p_paddr;  /* physical address - ignored? */
    Elf32_Word p_filesz; /* number of bytes in file for seg. */
    Elf32_Word p_memsz;  /* number of bytes in mem. for seg. */
    Elf32_Word p_flags;  /* flags */
    Elf32_Word p_align;  /* memory alignment */
} Elf32_Phdr;

class Elf : public Execf {
private:
    char format_name[100];

public:
    Elf(char* n);
    ~Elf();
    int Probe(uint8_t* file_data);
    int Exec(uint8_t* file_data, uint32_t phys_loc);
    char* Name();
};

#endif
