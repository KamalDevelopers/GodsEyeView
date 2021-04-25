#include "tar.hpp"

Tar::Tar(AdvancedTechnologyAttachment* ata)
{
    hd = ata;
}

int Tar::OctBin(char* str, int size)
{
    int n = 0;
    char* c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

int Tar::BinOct(int decimal_num)
{
    int rem, i = 1, octaln = 0;
    while (decimal_num != 0) {
        rem = decimal_num % 8;
        decimal_num /= 8;
        octaln += rem * i;
        i *= 10;
    }
    return octaln;
}

int Tar::Unlink(char* file_name, bool update)
{
    /* Removes file entry from RAM */
    int file_id = FindFile(file_name);
    int file_size = GetSize(file_name) / 512;
    if ((file_id > file_index) || (file_id == -1))
        return -1;

    for (int i = 0; i < file_index; i++) {
        if (i <= file_id)
            continue;
        files[i - 1] = files[i];
    }
    /* Removes file from hd */
    if (update == 1)
        Update(sector_links_file[file_id], file_size);
    return 0;
}

int Tar::RenameFile(char* file_name, char* new_file_name)
{
    int file_name_len = strlen(file_name);
    if (file_name_len >= 99)
        return -1;
    int file_id = FindFile(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;

    posix_header meta_head;
    meta_head = files[file_id];

    strcpy(meta_head.name, new_file_name);

    memcpy((void*)&meta_head, (void*)FileCalculateChecksum(&meta_head), sizeof(posix_header));
    files[file_id] = meta_head;

    hd->Write28(sector_links_file[file_id], (uint8_t*)&files[file_id], sizeof(posix_header));
    hd->Flush();
    return 0;
}

int Tar::Chmod(char* file_name, char* permissions)
{
    int file_id = FindFile(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;

    char mode[8];
    int prefix_len = 7 - strlen(permissions);
    for (int i = 0; i < prefix_len; i++)
        mode[i] = '0';
    mode[prefix_len] = '\0';

    strcat(mode, permissions);

    posix_header meta_head;
    meta_head = files[file_id];

    strcpy(meta_head.mode, mode);

    memcpy((void*)&meta_head, (void*)FileCalculateChecksum(&meta_head), sizeof(posix_header));
    files[file_id] = meta_head;

    hd->Write28(sector_links_file[file_id], (uint8_t*)&files[file_id], sizeof(posix_header));
    hd->Flush();
    return 0;
}

/* Returns the index of fname */
int Tar::FindFile(char* file_name)
{
    for (int i = 0; i < file_index; i++) {
        if (strncmp(file_name, files[i].name, strlen(file_name)) == 0)
            return i;
    }
    klog("File not found!");
    return -1;
}

int Tar::ReadDir(char* dirname, char** file_ids)
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
    uint8_t buffer[513];

    int SIZE = count;
    int sector_offset = 0;
    int data_index = 0;

    /* Iterate through the sectors and store the contents in buffers */
    for (; SIZE > 0; SIZE -= 512) {
        hd->Read28(sector_start + sector_offset, buffer, 512);
        for (int i = 0; i < 512; i++) {
            fdata[data_index] = buffer[i];
            data_index++;
        }
        buffer[SIZE > 512 ? 512 : SIZE] = '\0';
        sector_offset++;
    }
}

void Tar::WriteData(uint32_t sector_start, uint8_t* fdata, int count)
{
    uint8_t* databuffer = new uint8_t[count];

    if (databuffer == 0)
        klog("Not enough heap memory!");

    memcpy((char*)databuffer, (char*)fdata, count);
    databuffer[count] = '\0';

    uint8_t buffer[513];
    int SIZE = count;
    int sector_offset = 0;

    for (; SIZE > 0; SIZE -= 512) {
        for (int i = 0; i <= 512; i++)
            buffer[i] = '\0';
        hd->Write28(sector_start + sector_offset, buffer, 512);

        for (int i = sector_offset * 512; i < count; i++)
            buffer[i] = databuffer[i];

        buffer[SIZE > 512 ? 512 : SIZE] = '\0';
        hd->Write28(sector_start + sector_offset, buffer, 512);
        sector_offset++;
    }
    hd->Flush();
    kfree(databuffer);
}

int Tar::GetMode(int file_id, int utype)
{
    char p = 0;
    if (utype == 0) // All
        p = files[file_id].mode[6];
    if (utype == 1) // Group
        p = files[file_id].mode[5];
    if (utype == 2) // Owner
        p = files[file_id].mode[4];
    switch (p) {
    case '7':
        return 7; // RWX
    case '6':
        return 6; // RW
    case '5':
        return 5; // RX
    case '4':
        return 4; // R
    case '3':
        return 3; // WX
    case '2':
        return 2; // W
    case '1':
        return 1; // X
    case '0':
        return 0; // 0
    }
    return -1;
}

/* Reads file from ram */
int Tar::ReadFile(int file_id, uint8_t* data)
{
    if (file_id > file_index)
        return -1;
    int data_offset = sector_links_file[file_id] + 1;            // File data sector index
    int data_size = (OctBin(files[file_id].size, 11) / 512) + 1; // Get sector index
    ReadData(data_offset, data, data_size * 512);                // Convert sectors into bytes
    return 0;
}

/* Reads file from ram using file name */
int Tar::ReadFile(char* file_name, uint8_t* data)
{
    int file_id = FindFile(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;
    return ReadFile(file_id, data);
}

/* Reads file from ram using file name */
int Tar::GetSize(char* file_name)
{
    int file_id = FindFile(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;
    int data_offset = sector_links_file[file_id] + 1;            // File data sector index
    int data_size = (OctBin(files[file_id].size, 11) / 512) + 1; // Get sector index
    return data_size * 512;                                      // Convert sectors into bytes
}

/* Checksum write in octal */
posix_header* Tar::FileCalculateChecksum(posix_header* header_data)
{
    char* checksumdata;
    int check = CalculateChecksum(header_data);

    itoa(check, checksumdata);
    int octal_offset_check = 6 - str_len(checksumdata);

    char* checksum = new char[octal_offset_check];

    for (int i = 0; i < octal_offset_check; i++)
        checksum[i] = '0';
    strcat(checksum, checksumdata);

    strncpy(header_data->chksum, checksum, 7);
    header_data->chksum[6] = '\0';
    header_data->chksum[7] = ' ';

    kfree(checksum);
    return header_data;
}

int Tar::CalculateChecksum(posix_header* header_data)
{
    posix_header* meta_head;
    *meta_head = *header_data;
    unsigned int chck = 0;
    memset(meta_head->chksum, ' ', 8);

    for (int i = 0; i < 400; i++)
        chck += ((uint8_t*)meta_head)[i];
    chck = BinOct(chck + 32);
    return chck;
}

int Tar::WriteFile(char* file_name, uint8_t* data, int data_length)
{
    /* Calculate where the file should be located */
    int file_id = file_index + 1;
    int data_size = OctBin(files[file_id - 2].size, 11); // Size of last file
    if (data_size % 512 == 0)
        data_size = data_size / 512;
    else
        data_size = (data_size / 512) + 1;
    int data_offset = sector_links_file[file_id - 2];
    int newfile_offset = data_offset + data_size + 1;

    /* Create the header data */
    posix_header meta_head;
    for (int i = 0; i < 99; i++)
        meta_head.name[i] = '\0';
    strcpy(meta_head.name, file_name);

    /* Convert size data to octal */
    char* sizedata;
    char tsize[12];
    itoa(BinOct(data_length), sizedata);
    int octal_offset = 11 - str_len(sizedata);

    tsize[octal_offset] = '\0';
    for (int i = 0; i < octal_offset; i++)
        tsize[i] = '0';
    strcat(tsize, sizedata);
    strcpy(meta_head.size, tsize);

    strcpy(meta_head.magic, "ustar");         // Header magic
    strcpy(meta_head.version, "\0\0");        // USTAR version
    strcpy(meta_head.mode, "0000766\0");      // UNIX Permissions
    strcpy(meta_head.uname, "terry\0");       // Owner name
    strcpy(meta_head.gname, "\0");            // Group name
    strcpy(meta_head.uid, "0001750\0");       // User id
    strcpy(meta_head.gid, "0001750\0");       // Group id
    strcpy(meta_head.mtime, "13715523517\0"); // Creation date. Temporary const, should be calculated
    strcpy(meta_head.chksum, "       \0");    // Temporary checksum data
    meta_head.typeflag = '0';                 // Indicate standard file type

    memcpy((void*)&meta_head, (void*)FileCalculateChecksum(&meta_head), sizeof(posix_header));
    files[file_id - 1] = meta_head;

    /* Clean the sectors */
    for (int i = 0; i < 513; i++)
        hd->Write28(data_offset + data_size + 2 + i, (uint8_t*)"\0", 1);
    for (int i = 0; i < 513; i++)
        hd->Write28(data_offset + data_size + 1 + i, (uint8_t*)"\0", 1);
    hd->Write28(data_offset + data_size + 1, (uint8_t*)&meta_head, sizeof(posix_header));
    WriteData(data_offset + data_size + 2, data, data_length);
    return 0;
}

void Tar::SectorSwap(int sector_src, int sector_dest)
{
    uint8_t buffer[513];
    hd->Read28(sector_src, buffer, 512);
    hd->Write28(sector_dest, buffer, 512);
    hd->Flush();
}

/* Remove entry from archive 
 * FIXME: Should update all entry changes */
void Tar::Update(int uentry, int uentry_size)
{
    int lastloc = uentry + uentry_size;
    int moveto = uentry;
    int sector_offset = 0;

    while (1) {
        if (sector_offset == tar_end)
            break;
        if (sector_offset > lastloc) {
            SectorSwap(sector_offset, moveto);
            moveto++;
        }
        sector_offset++;
    }

    uint8_t buffer[513];
    for (int i = 0; i <= 512; i++)
        buffer[i] = '\0';

    int sectors_overflow = uentry_size - (moveto - uentry);
    while (moveto < lastloc) {
        hd->Write28(moveto, (uint8_t*)&buffer, 512);
        hd->Flush();
        moveto++;
    }
}

/* Stores all files and directories in RAM */
void Tar::Mount()
{
    int sector_offset = 0;
    char magic[6] = MAGIC;

    while (1) {
        posix_header meta_head;
        hd->Read28(sector_offset, (uint8_t*)&meta_head, sizeof(posix_header));

        /* Check for valid header else break mount */
        memcpy(magic, meta_head.magic, 4);
        if (strcmp(magic, MAGIC) != 0)
            break;

        /* Check type and add it to ram */
        if (OctBin((char*)&meta_head.typeflag, 1) == 0) {
            files[file_index] = meta_head;
            sector_links_file[file_index] = sector_offset;
            file_index++;
        }

        if (OctBin((char*)&meta_head.typeflag, 1) == 5) {
            dirs[dir_index] = meta_head;
            sector_links_dir[dir_index] = sector_offset;
            dir_index++;
        }

        /* Skip the file data and reach the next header */
        if (OctBin((char*)&meta_head.typeflag, 1) == 0) {
            int data_size = OctBin((char*)&meta_head.size, 11);
            sector_offset = sector_offset + (data_size / 512);
            if (data_size % 512 != 0)
                sector_offset++;
        }
        sector_offset++;
    }
    tar_end = sector_offset - 1;
}
