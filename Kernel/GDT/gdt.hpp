#ifndef GDT_HPP
#define GDT_HPP

#include <LibC/stdlib.h>

#define GDT_NUM_DESCRIPTORS 8
#define GDT_CODE_DATA_SEGMENT 0x08
#define GDT_KERNEL_DATA_SEGMENT 0x10

/* save kernel stack;
 * due to potential syscalls from userspace */
#define tss_save_stack()                        \
    uint32_t __tss_save_stack_esp;              \
    asm volatile("mov %%esp, %0"                \
                 : "=r"(__tss_save_stack_esp)); \
    tss_set_stack(GDT_KERNEL_DATA_SEGMENT, __tss_save_stack_esp);

typedef struct tss_entry {
    uint32_t prevtss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap;
} tss_entry_t;

typedef struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

void gdt();
void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

void tss(uint32_t idx, uint32_t kss, uint32_t kesp);
void tss_set_stack(uint32_t kss, uint32_t kesp);

#endif
