#include "elf.hpp"

Elf::Elf(char* n)
    : Execf(n)
{
    strcpy(format_name, n);
}

Elf::~Elf()
{
    kfree(format_name);
}

int Elf::probe(uint8_t* file_data)
{
    uint8_t executable = 0;
    uint8_t valid = 0;

    elf32_ehdr elf_header;
    memcpy((uint8_t*)&elf_header, file_data, sizeof(elf_header));

    char* magic_elf;
    magic_elf[0] = elf_header.e_ident.ei_magic[1];
    magic_elf[1] = elf_header.e_ident.ei_magic[2];
    magic_elf[2] = elf_header.e_ident.ei_magic[3];
    magic_elf[3] = '\0';

    if ((elf_header.e_ident.ei_magic[0] == 0x7F) && (strcmp(magic_elf, "ELF\0") == 0))
        valid = 1;
    if (elf_header.e_type == 2)
        executable = 1;

    if ((valid) && (executable))
        return 1;

    if (valid)
        return 0;

    return -1;
}

int Elf::exec(uint8_t* file_data)
{
    elf32_ehdr* elf_header = (elf32_ehdr*)file_data;

    if (probe(file_data) != 1)
        return 0;

    elf32_phdr* elf_program_header = (elf32_phdr*)(file_data + elf_header->e_phoff);
    elf_program_header = (elf32_phdr*)(file_data + elf_header->e_phoff);
    uint32_t phys_location;

    for (int i = 0; i < elf_header->e_phnum; i++, elf_program_header++) {
        switch (elf_program_header->p_type) {
        case 1:
            if (elf_program_header->p_filesz != 0) {
                phys_location = (uint32_t)pmalloc(ROUND_UP(elf_program_header->p_filesz, 4096));
                Paging::map_page(elf_program_header->p_vaddr, phys_location);
            }

            memcpy((void*)elf_program_header->p_vaddr, file_data + elf_program_header->p_offset, elf_program_header->p_filesz);
            memset((void*)(elf_program_header->p_vaddr + elf_program_header->p_filesz), 0, elf_program_header->p_memsz - elf_program_header->p_filesz);
            break;

        default:
            break;
        }
    }
    return elf_header->e_entry + elf_program_header->p_vaddr;
}

char* Elf::name()
{
    return format_name;
}
