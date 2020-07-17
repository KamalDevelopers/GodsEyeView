#ifndef FS_HPP
#define FS_HPP

#include "../Hardware/Drivers/ata.hpp"
#include "stdio.hpp"

#define MAX_FILENAME_LEN    15
#define MAX_FILE_DESCRIPTOR 32
#define MAX_FILE_SIZE       100000
#define MAX_FILES           10

class FileSystem {
private:
	AdvancedTechnologyAttachment* hd;
	int superblockoffset = 0;
	char** errormessage;

	struct SuperBlock {
		// Directory info
		int dir_index;
		int dir_len;

		// Data info
		int data_index;
	} __attribute__((packed));
	SuperBlock super_block;

	struct FileInfo {
		uint32_t size;
		int head_index;
		int tail_index;
		int type;
		char name[MAX_FILENAME_LEN];
	} __attribute__((packed));
	FileInfo files[MAX_FILES];

public:
	FileSystem(AdvancedTechnologyAttachment* ata, char** err);
	int Make();
	int Mount();
	int Unmount();
	//int ReadDir(int index);
	//int ReadNode(int index);
	int WriteFile(int index, char* fname, uint8_t* data, uint32_t size);
	uint8_t* ReadFile(int index, uint8_t* edata);

};

#endif