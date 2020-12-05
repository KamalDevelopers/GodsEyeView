#include "elf.hpp"

int Elf::elf_header_parse(uint8_t* file_data)
{
    uint8_t executable = 0;
    uint8_t valid = 0;

    Elf32_Ehdr elf_header;
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
