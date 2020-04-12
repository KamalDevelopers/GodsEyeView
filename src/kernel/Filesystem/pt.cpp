#include "pt.hpp"

using namespace Fat32;

void PartitionTable::ReadPartitions(AdvancedTechnologyAttachment *hd)
{
	MasterBootRecord mbr;

	printf("%s", "MBR:");

	hd->Read28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));

	/*
	for (int i = 446; i < 446 + 4*16; i++)
	{
		printf("%x", (uint8_t*)&mbr[i]);
		printf("%s", " ");
	}
	printf("\n");
	*/

	if (mbr.magicnumber != 0xAA55)
	{
		printf("%s\n", "illegal MBR");
		return;
	}

	for (int i = 0; i < 4; i++)
	{
		if (mbr.primary[i].partition_id == 0)
			continue;

		printf("%s", " Partition ");
		printf("%x\n", i & 0xFF);

		if (mbr.primary[i].bootable == 0x80)
			printf("%s", " bootable. Type");
		else
			printf("%s", " not bootable. Type");
		
		printf("%x", mbr.primary[i].partition_id);
		Fat::ReadBiosBlock(hd, mbr.primary[i].start_lba);
	}
}