#include "gdt.hpp"

extern "C" void tss_flush();
extern "C" void gdt_flush(uint32_t gdt_ptr);

static gdt_entry_t gdt_entries[GDT_NUM_DESCRIPTORS];
static gdt_ptr_t gdt_ptr;
static tss_entry_t kernel_tss;

void gdt()
{
    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base = (uint32_t)gdt_entries;

    // NULL segment, required
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Kernel code, access(9A = 1 00 1 1 0 1 0)
        1   present
        00  ring 0
        1   always 1
        1   code segment
        0   can be executed by ring lower or equal to DPL,
        1   code segment is readable
        0   access bit, always 0, cpu set this to 1 when accessing this sector
    */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    /* Call to assembly */
    /* NOTE: lgdt from C does not work
     * when kernel GCC optimizations are on */
    gdt_flush((uint32_t)(&gdt_ptr));
}

void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt_entry_t* entry = &gdt_entries[index];

    entry->base_low = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->base_high = (base >> 24 & 0xFF);

    entry->limit_low = limit & 0xFFFF;
    entry->granularity = (limit >> 16) & 0x0F;

    entry->access = access;
    entry->granularity = entry->granularity | (gran & 0xF0);
}

void tss(uint32_t idx, uint32_t kss, uint32_t kesp)
{
    uint32_t base = (uint32_t)&kernel_tss;
    gdt_set_entry(idx, base, base + sizeof(tss_entry_t), 0xE9, 0);
    /* Kernel tss, access(E9 = 1 11 0 1 0 0 1)
        1   present
        11  ring 3
        0   should always be 1, why 0? may be this value doesn't matter at all
        1   code?
        0   can not be executed by ring lower or equal to DPL,
        0   not readable
        1   access bit, always 0, cpu set this to 1 when accessing this sector(why 0 now?)
    */
    memset(&kernel_tss, 0, sizeof(tss_entry_t));
    kernel_tss.ss0 = kss;
    kernel_tss.esp0 = kesp;
    kernel_tss.cs = 0x0b;
    kernel_tss.ds = 0x13;
    kernel_tss.es = 0x13;
    kernel_tss.fs = 0x13;
    kernel_tss.gs = 0x13;
    kernel_tss.ss = 0x13;
    tss_flush();
}

void tss_set_stack(uint32_t kss, uint32_t kesp)
{
    kernel_tss.ss0 = kss;
    kernel_tss.esp0 = kesp;
}
