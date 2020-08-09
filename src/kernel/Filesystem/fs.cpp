#include "fs.hpp"

FileSystem::FileSystem(AdvancedTechnologyAttachment* ata, char** err)
{
	hd = ata;
	errormessage = err;
}

int FileSystem::Make()
{
	SuperBlock super_block;
	/* set super block values */
	super_block.dir_index = 1;
	super_block.dir_len = 0;
	super_block.data_index = 2;

	/* write super block */
	hd->Write28(superblockoffset, (uint8_t*)&super_block, sizeof(SuperBlock));

	/* set first block values */
	FileInfo firstdir;

	char *dirname = "root";
	if (str_len(dirname) < MAX_FILENAME_LEN)
		strcat(firstdir.name, dirname);
	else { *errormessage = "Dirname exceeded max directory name length"; return -1; }

	firstdir.size = 1;
	firstdir.head_index = super_block.dir_index;
	firstdir.type = 2; //Dir

	hd->Write28(super_block.dir_index, (uint8_t*)&firstdir, sizeof(FileInfo));
	hd->Flush();
	return 0;
}

int FileSystem::Mount()
{
	/* read super block*/
	hd->Read28(superblockoffset, (uint8_t*)&super_block, sizeof(SuperBlock));

	/* read first block */
	FileInfo firstdir;
	hd->Read28(super_block.dir_index, (uint8_t*)&firstdir, sizeof(FileInfo));

	//printf("%s%d%s%s", "Root Directory Index: ", super_block.dir_index, " Root Directory Name: ", firstdir.name);
	return 0;
}

uint8_t* FileSystem::ReadFile(int index, uint8_t* edata)
{
	int npos = index/MAX_FILE_SIZE;
	if (index == 0) npos = 0;

	//if (files[npos].type != 1)
	FileInfo file;
	hd->Read28(index, (uint8_t*)&file, sizeof(FileInfo));
	files[npos] = file;

	klog((char*)files[npos].name);
	char *size;
	itoa(files[npos].size, size);
	klog(size);

	uint8_t* data = 0;
	if (files[npos].size <= 512) { hd->Read28(index + 1, data, files[npos].size); }
	else {
		uint8_t databuffer[files[npos].size];
		uint8_t buffer[513];
		int SIZE = files[npos].size;
		int sector_offset = 0;

		for(; SIZE > 0; SIZE -= 512)
		{
			hd->Read28(files[npos].head_index + sector_offset, buffer, 512);
			//for (int i = 0; i < 512; i++){ *data++ = buffer[i]; }
			//printf("%s", (char*)buffer);
			strcat((char*)data, (char*)buffer);
			buffer[SIZE > 512 ? 512 : SIZE] = '\0';
			sector_offset++;
		}
	}

	edata = data;
	return data;
}

int FileSystem::WriteFile(int index, char* fname, uint8_t* data, uint32_t size)
{
	if (size > MAX_FILE_SIZE) { return -1; }
	int npos = index/MAX_FILE_SIZE;
	if (index == 0) npos = 0;

	FileInfo file;
	file.size = size;
	file.head_index = index + 1;
	file.tail_index = index + MAX_FILE_SIZE;
	file.tail_index = 1;
	strcpy(file.name, fname);
	file.type = 1;

	hd->Write28(index, (uint8_t*)&file, sizeof(FileInfo));
	if (size <= 512) { hd->Write28(index + 1, data, size); }
	else {
		int bufindex = 0;
		int sector_index = 1;
		uint8_t buffer[513];
		while (bufindex != size)
		{
			int i = 0;
			while  (i < 512)
			{
				buffer[i] = *data++;
				bufindex++;
				i++;
				if (bufindex >= size) break;
			}
			uint8_t* fdata = buffer;
			//printf("%s%d", "FBUFFER: ", fdata);
			hd->Write28(index + sector_index, fdata, i);
			sector_index++;
		}
	}

	hd->Flush();
	return 0;
}
