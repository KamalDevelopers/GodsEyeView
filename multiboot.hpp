#include <LibC/types.h>

/* custom multiboot header */
typedef struct multi {
    uint32_t resv;
    uint32_t vesa_width;
    uint32_t vesa_height;
    uint32_t vesa_bpp;
    uint32_t vesa_pitch;
    uint32_t vesa_framebuffer;
    uint32_t upper_memory;
} __attribute__((packed)) multi_t;

/* grub multiboot header */
struct multiboot_aout_symbol_table {
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
};
typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;

typedef struct multiboot_memory_map {
    unsigned int size;
    unsigned int base_addr_low, base_addr_high;
    unsigned int length_low, length_high;
    unsigned int type;
} multiboot_memory_map_t;
typedef multiboot_memory_map_t mmap_entry_t;

struct multiboot_elf_section_header_table {
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
};
typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;

struct multiboot_info {
    // Multiboot info version number.
    uint32_t flags;

    // Available memory from BIOS.
    uint32_t mem_lower;
    uint32_t mem_upper;

    // "root" partition.
    uint32_t boot_device;

    // Kernel command line.
    uint32_t cmdline;

    // Boot-Module list.
    uint32_t mods_count;
    uint32_t mods_addr;

    union {
        multiboot_aout_symbol_table_t aout_sym;
        multiboot_elf_section_header_table_t elf_sec;
    } u;

    // Memory Mapping buffer.
    uint32_t mmap_length;
    uint32_t mmap_addr;

    // Drive Info buffer.
    uint32_t drives_length;
    uint32_t drives_addr;

    // ROM configuration table.
    uint32_t config_table;

    // Boot Loader Name.
    uint32_t boot_loader_name;

    // APM table.
    uint32_t apm_table;

    // Video.
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2
    uint8_t framebuffer_type;
    union {
        struct
        {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        } fp1;
        struct
        {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        } fp2;
    };
};

typedef struct multiboot_info multiboot_info_t;

static uint32_t detect_memory(multiboot_info_t* multiboot_info_ptr)
{
    mmap_entry_t* entry = (mmap_entry_t*)multiboot_info_ptr->mmap_addr;
    uint32_t total = 0;
    while ((uint32_t)entry < multiboot_info_ptr->mmap_addr + multiboot_info_ptr->mmap_length) {
        entry = (mmap_entry_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
        if (entry->type == 1) {
            uint64_t length = (((uint64_t)entry->length_high) << 32) | ((uint64_t)entry->length_low);
            total += length;
        }
    }

    return total;
}
