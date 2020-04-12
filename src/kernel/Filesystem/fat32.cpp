#include "fat32.hpp"

using namespace Fat32;
void Fat::ReadBiosBlock(AdvancedTechnologyAttachment *hd, uint32_t partitionOffset)
{
	BiosParameterBlock32 bpb;
	hd->Read28(partitionOffset, (uint8_t*)&bpb,  sizeof(BiosParameterBlock32));

	uint32_t fatStart = partitionOffset + bpb.reservedSectors;
	uint32_t fatSize = bpb.tableSize;

	uint32_t dataStart = fatStart + fatSize*bpb.fatCopies;

	uint32_t rootStart = dataStart + bpb.sectorsPerCluster*bpb.rootCluster;

	DirectoryEntryFat32 dirent[16];
	hd->Read28(rootStart, (uint8_t*)&dirent[0], 16*sizeof(DirectoryEntryFat32));

	for (int i = 0; i < 16; i++)
	{
		if (dirent[i].name[0] == 0x00)
			break;

		if ((dirent[i].attributes & 0x0F) == 0x0F)
			continue;

		char* f = "         \n";
		for(int j = 0; j <= 8; j++)
			f[j] = dirent[i].name[j];
		printf("%s", f);

		if ((dirent[i].attributes & 0x10) == 0x10)
			continue;
		uint32_t fileCluster = ((uint32_t)dirent[i].firstClusterHi) << 16
                             | ((uint32_t)dirent[i].firstClusterHi);
        uint32_t fileSector =  dataStart + bpb.sectorsPerCluster * (fileCluster-2);
        uint8_t buffer[512];

        hd->Read28(fileSector, buffer, 512);
        buffer[dirent[i].size] = '\0';
        printf("%s", (char*)buffer);
	}
}