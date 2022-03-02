#include "elf.hpp"

Elf::Elf(char* name)
    : Execf(name)
{
    strcpy(format_name, name);
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

    char magic_elf[4];
    magic_elf[0] = elf_header.e_ident.ei_magic[1];
    magic_elf[1] = elf_header.e_ident.ei_magic[2];
    magic_elf[2] = elf_header.e_ident.ei_magic[3];
    magic_elf[3] = '\0';

    if ((elf_header.e_ident.ei_magic[0] == 0x7F) && (strcmp(magic_elf, "ELF") == 0))
        valid = 1;

    if (elf_header.e_type == 2)
        executable = 1;

    if ((valid) && (executable))
        return 1;

    if (valid)
        return 0;

    return -1;
}

executable_t Elf::exec(uint8_t* file_data)
{
    executable_t executable;

    if (probe(file_data) != 1) {
        klog("Could not probe!");
        return executable;
    }

    executable.valid = true;
    elf32_ehdr* elf_header = (elf32_ehdr*)file_data;
    elf32_phdr* elf_program_header = (elf32_phdr*)(file_data + elf_header->e_phoff);
    elf32_shdr* elf_section_header = (elf32_shdr*)(file_data + elf_header->e_shoff);

    uint32_t base_addres = UINT32_MAX;
    for (int i = 0; i < elf_header->e_phnum; i++, elf_program_header++) {
        base_addres = MIN(base_addres, elf_program_header->p_vaddr);
    }

    elf_program_header = (elf32_phdr*)(file_data + elf_header->e_phoff);
    uint32_t size = 0;
    for (int i = 0; i < elf_header->e_phnum; i++, elf_program_header++) {
        size = MAX(size, elf_program_header->p_vaddr - base_addres + elf_program_header->p_memsz);
    }

    elf_program_header = (elf32_phdr*)(file_data + elf_header->e_phoff);
    executable.memory.physical_address = (uint32_t)PMM->allocate_pages(size);
    executable.memory.size = size;

    for (int i = 0; i < elf_header->e_phnum; i++, elf_program_header++) {
        uint32_t address = 0;
        switch (elf_program_header->p_type) {
        case PT_LOAD:
            address = (elf_program_header->p_vaddr - base_addres) + executable.memory.physical_address;
            memset((void*)address, 0, elf_program_header->p_memsz);
            memcpy((void*)address, file_data + elf_program_header->p_offset, elf_program_header->p_filesz);
            break;
        }
    }

    executable.eip = elf_header->e_entry + executable.memory.physical_address;
    return executable;
}

char* Elf::name()
{
    return format_name;
}
