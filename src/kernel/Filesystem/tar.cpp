#include "tar.hpp"

int Tar::OctBin(char *str, int size) {
    int n = 0;
    char *c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

Tar::Tar(AdvancedTechnologyAttachment* ata)
{
	hd = ata;
}

int Tar::ListDir(char* dirname, char** file_ids)
{
	/* Iterate through file names */
	for (int i = 0; i < file_index; i++)
		if (strncmp(files[i].name, dirname, str_len(dirname)) == 0)
			*file_ids++ = files[i].name;

	/* Iterate through directory name */
	for (int i = 0; i < dir_index; i++)
		if (strncmp(dirs[i].name, dirname, str_len(dirname)) == 0)
			if (strcmp(dirs[i].name, dirname) != 0)
				*file_ids++ = dirs[i].name;
	return 0;
}

void Tar::ReadData(uint32_t sector_start, uint8_t* fdata, int count)
{
	uint8_t databuffer[count]; // Reserved space for the entire file
	uint8_t buffer[513]; // Sector buffer
	int SIZE = count;
	int sector_offset = 0;

	/* Iterate through the sectors and store the contents in buffers */
	for(; SIZE > 0; SIZE -= 512)
	{
		hd->Read28(sector_start + sector_offset, buffer, 512);
		buffer[SIZE > 512 ? 512 : SIZE] = '\0';
		strcat((char*)databuffer, (char*)buffer);
		sector_offset++;
	}
	memcpy(fdata, databuffer, count);
}

void Tar::WriteData(uint32_t sector_start, uint8_t* fdata, int count)
{
	uint8_t data[count];
	strncpy((char*)data, (char*)fdata, count);
	data[count] = '\0';
	
	uint8_t buffer[513];
	int SIZE = count;
	int sector_offset = 0;

	for (; SIZE > 0; SIZE -= 512)
	{
		for (int i = 0; i < 512; i++) buffer[i] = '\0';
		hd->Write28(sector_start + sector_offset, buffer, 512); // Fill the sector with null 	
		/* Copy the file data into a sector sized buffer */
		for (int i = sector_offset*512; i < count; i++){ buffer[i] = data[i]; }
		buffer[SIZE > 512 ? 512 : SIZE] = '\0';
		hd->Write28(sector_start + sector_offset, buffer, 512);
		sector_offset++;
	}
	hd->Flush();
}

int Tar::GetMode(int file_id, int utype)
{
	char p = 0;
	if (utype == 0) // All
		p = files[file_id].mode[8];
	if (utype == 1) // Group
		p = files[file_id].mode[7];
	if (utype == 2) // Owner
		p = files[file_id].mode[6];
	switch (p){
		case '7' : return 7; // RWX
		case '6' : return 6; // RW
		case '5' : return 5; // RX
		case '4' : return 4; // R
		case '3' : return 3; // WX
		case '2' : return 2; // W
		case '1' : return 1; // X
		case '0' : return 0; // 0
	}
	return -1;
}

/* Reads file from ram */
int Tar::ReadFileId(int file_id, uint8_t* data)
{
	if (file_id > file_index) return -1;
	int permission = GetMode(file_id, 2);
	if (permission < 4) // No read 
		return -1;
	int data_offset = sector_links_file[file_id]+1; // File data sector index
	int data_size = (OctBin(files[file_id].size, 11)/512)+1; // Get sector index
	ReadData(data_offset, data, data_size*512); // Convert sectors into bytes
	return 0;
}

/* Reads file from ram using file name */
int Tar::ReadFile(char* file_name, uint8_t* data)
{
	int file_id = FindFile(file_name);
	if (file_id > file_index) return -1;
	int permission = GetMode(file_id, 2);
	if (permission < 4) // No read 
		return -1;
	int data_offset = sector_links_file[file_id]+1; // File data sector index
	int data_size = (OctBin(files[file_id].size, 11)/512)+1; // Get sector index
	ReadData(data_offset, data, data_size*512); // Convert sectors into bytes
	return 0;
}

int Tar::BinOct(int decimalNumber)
{
    int rem, i = 1, octalNumber = 0;
    while (decimalNumber != 0)
    {
        rem = decimalNumber % 8;
        decimalNumber /= 8;
        octalNumber += rem * i;
        i *= 10;
    }
    return octalNumber;
}
			
int Tar::CalculateChecksum(posix_header* header_data){
	posix_header* meta_head;
	*meta_head = *header_data;
	unsigned int chck = 0;	
	memset(meta_head->chksum, ' ', 8);

	for (int i = 0; i < 400; i++) chck += ((uint8_t*)meta_head)[i];
	chck = BinOct(chck+32);
	printf("\nD: %s\n", header_data->name);
	return chck;
}

int Tar::WriteFile(char* file_name, uint8_t* data, int data_length)
{
	/* Calculate where the file should be located */
	int file_id = file_index+1;
	int data_size = (OctBin(files[file_id-2].size, 11)/512)+1; // Size of last file
	int data_offset = sector_links_file[file_id-2]; // Position of last file
	int newfile_offset = data_offset+data_size+1; // New file data location
	
	/* Create the header data */
	posix_header meta_head;
	for (int i = 0; i < 99; i++) meta_head.name[i] = '\0';
	strcpy(meta_head.name, file_name);
	printf("\n%d", data_length);
	printf("\n%d", BinOct(data_length));
	
	/* Conver size data to octal */
	char* sizedata;
	itoa(BinOct(data_length), sizedata);
	int octal_offset = 11 - str_len(sizedata);
	char tsize[octal_offset];

	tsize[octal_offset] = '\0';	
	for (int i = 0; i < octal_offset; i++) tsize[i] = '0';
	strcat(tsize, sizedata);
	strcpy(meta_head.size, tsize);

	strcpy(meta_head.magic,   "ustar");
	strcpy(meta_head.version, "\0\0"); 
	strcpy(meta_head.mode,    "0000766\0");      // UNIX Permissions
	strcpy(meta_head.uname,   "terry\0");
	strcpy(meta_head.gname,   "\0");
	strcpy(meta_head.uid,     "0001750\0");      // User id
	strcpy(meta_head.gid,     "0001750\0");      // Group id
	strcpy(meta_head.mtime,   "13715523517\0");  // Temporary, should be calculated	
	strcpy(meta_head.chksum,  "       \0");      // Temporary data that helps with the checksum calculation

	meta_head.typeflag = '0'; // Standard file

	int check = CalculateChecksum(&meta_head); // Calculate the checksum of the header data
	char* checksumdata;
	itoa(check, checksumdata);
	int octal_offset_check = 6 - str_len(checksumdata);
	
	char checksum[octal_offset_check];
	for (int i = 0; i < octal_offset_check; i++) checksum[i] = '0';
	strcat(checksum, checksumdata);
	
	strncpy(meta_head.chksum, checksum, 7);
	meta_head.chksum[6] = '\0';
	meta_head.chksum[7] = ' ';
	
	files[file_id-1] = meta_head;
	/* Clean the sectors */
	for (int i = 0; i < 513; i++) hd->Write28(data_offset+data_size+2+i, (uint8_t*)"\0", 1);
	for (int i = 0; i < 513; i++) hd->Write28(data_offset+data_size+1+i, (uint8_t*)"\0", 1);
	hd->Write28(data_offset+data_size+1, (uint8_t*)&meta_head, sizeof(posix_header));
	WriteData(data_offset+data_size+2, data, data_length);
	
	check = CalculateChecksum(&meta_head); // Calculate the checksum of the header data
	
	return 0;
}

/* Returns the index of fname */
int Tar::FindFile(char* fname)
{
	for (int i = 0; i < file_index; i++)
		if (strcmp(files[i].name, fname) == 0)
			return i;
	return -1;
}

/* Reads all files and directories */
void Tar::Mount()
{
	clear();
	int sector_offset = 0;
	char magic[6] = MAGIC; // Header magic, used for validation checking
	while (1){
	        posix_header meta_head;
		hd->Read28(sector_offset, (uint8_t*)&meta_head, sizeof(posix_header));

		/* Check for valid header else break mount */
		memcpy(magic, meta_head.magic, 4);
		if (strcmp(magic, MAGIC) != 0) break;

		/* Check type and add it to ram */
		if (OctBin((char*)&meta_head.typeflag, 1) == 0){
			//printf("Mount: %s : %d", meta_head.name, CalculateChecksum(&meta_head));
			files[file_index] = meta_head;
			sector_links_file[file_index] = sector_offset;
			file_index++;
		}

		if (OctBin((char*)&meta_head.typeflag, 1) == 5){
			dirs[dir_index] = meta_head;
			sector_links_dir[dir_index] = sector_offset;
			dir_index++;
		}

		/* Skip the file data and reach the next header */
		if (OctBin((char*)&meta_head.typeflag, 1) == 0){
			sector_offset = sector_offset + (OctBin((char*)&meta_head.size, 11)/512); // Skip data
			sector_offset++;
		}
		sector_offset++;
	}
}
