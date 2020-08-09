#include "part.hpp"

void PartTable::ReadPartitions(Graphics* vga, AdvancedTechnologyAttachment* hd)
{
    MasterBootRecord mbr;
    printf("MBR: ");

    hd->Read28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));

    if(mbr.magicnumber != 0xAA55)
    {
        printf("illegal MBR");
        return;
    }

    for(int i = 0; i < 4; i++)
    {
        if(mbr.primaryPartition[i].partition_id == 0x00)
            continue;

        printf(" Partition ");
        printf("%x", i & 0xFF);

        if(mbr.primaryPartition[i].bootable == 0x80)
            printf(" bootable. Type");
        else
            printf(" not bootable. Type ");

        //printf("%x", mbr.primaryPartition[i].partition_id);
        ReadBiosBlock(vga, hd, mbr.primaryPartition[i].start_lba);
    }
}
