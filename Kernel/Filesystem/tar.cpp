#include "tar.hpp"

/* TODO: support '/' path prefix for compatibility */

Tar::Tar(ATA* ata)
{
    transfer_buffer = (uint8_t*)kmalloc(MAX_TRANSFER_SIZE);
    this->ata = ata;
}

Tar::~Tar()
{
    kfree(transfer_buffer);
}

int Tar::oct_bin(char* str, int size)
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

int Tar::bin_oct(int decimal_num)
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

int Tar::unlink(char* file_name, bool should_update)
{
    /* Removes file entry from RAM */
    int file_id = find_file(file_name);
    int file_size = get_size(file_name) / 512;
    if ((file_id > file_index) || (file_id == -1))
        return -1;

    files[file_id].typeflag = 100;

    /* Removes file from drive */
    if (should_update == 1)
        update_disk(sector_links_file[file_id] - 1, file_size + 1);
    return 0;
}

int Tar::unlink(char* file_name)
{
    return unlink(file_name, true);
}

int Tar::rename_file(char* file_name, char* new_file_name)
{
    int file_name_len = strlen(file_name);
    if (file_name_len >= 99)
        return -1;
    int file_id = find_file(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;

    posix_header meta_head;
    meta_head = files[file_id];

    strcpy(meta_head.name, new_file_name);

    file_calculate_checksum(&meta_head);
    files[file_id] = meta_head;

    ata->write28(sector_links_file[file_id], (uint8_t*)&files[file_id], sizeof(posix_header));
    ata->flush();
    return 0;
}

int Tar::chmod(char* file_name, char* permissions)
{
    int file_id = find_file(file_name);
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

    file_calculate_checksum(&meta_head);
    files[file_id] = meta_head;

    ata->write28(sector_links_file[file_id], (uint8_t*)&files[file_id], sizeof(posix_header));
    ata->flush();
    return 0;
}

int Tar::find_file(char* file_name)
{
    for (int i = 0; i < file_index; i++) {
        if (strncmp(file_name, files[i].name, strlen(file_name)) == 0)
            if (files[i].typeflag != 100)
                return i;
    }
    return -1;
}

int Tar::exists(char* name)
{
    for (int i = 0; i < dir_index; i++)
        if (strcmp(dirs[i].name, name) == 0)
            if (dirs[i].typeflag != 100)
                return 0;

    for (int i = 0; i < file_index; i++)
        if (strcmp(files[i].name, name) == 0)
            if (files[i].typeflag != 100)
                return 0;
    return 1;
}

int Tar::read_dir(char* dirname, fs_entry_t* entries, uint32_t count)
{
    uint32_t index = 0;
    int skip = false;
    posix_header entry;

    for (int entry_type = 0; entry_type < 2; entry_type++) {
        uint32_t size = (entry_type == 0) ? file_index : dir_index;
        for (int i = 0; i < size; i++) {
            skip = false;
            entry = (entry_type == 0) ? files[i] : dirs[i];
            if (entry.typeflag == 100)
                skip = true;
            if (strncmp(entry.name, dirname, strlen(dirname)) == 0) {
                for (int x = strlen(dirname); x < strlen(entry.name) - 1; x++)
                    if (entry.name[x] == '/')
                        skip = true;
                if (skip)
                    continue;

                strcpy(entries[index].name, entry.name + strlen(dirname));
                entries[index].type = entry_type + 1;
                index++;

                if (index >= count)
                    return index;
            }
        }
    }

    return index;
}

void Tar::read_data(uint32_t sector_start, uint8_t* fdata, int count, size_t seek)
{
    int size = count;
    int sector_offset = 0;
    int data_index = 0;
    uint32_t tsize = MED_TRANSFER_SIZE;
    uint32_t ssize = MED_TRANSFER_SECT;

    if (count >= MAX_TRANSFER_SIZE) {
        tsize = MAX_TRANSFER_SIZE;
        ssize = MAX_TRANSFER_SECT;
    }

    if (!ata->is_dma() || count <= MIN_TRANSFER_SIZE) {
        tsize = MIN_TRANSFER_SIZE;
        ssize = MIN_TRANSFER_SECT;
    }

    if (seek > tsize)
        sector_start += (seek - sector_offset) / tsize;

    for (; size > 0; size -= tsize) {
        if (size < tsize) {
            tsize = MIN_TRANSFER_SIZE;
            ssize = MIN_TRANSFER_SECT;
        }

        ata->read28(sector_start + sector_offset, transfer_buffer, tsize, ssize);
        int i = (sector_offset) ? 0 : (seek % tsize);

        for (; i < tsize; i++) {
            if (data_index <= count)
                fdata[data_index] = transfer_buffer[i];
            data_index++;
        }
        sector_offset += ssize;
    }
}

void Tar::write_data(uint32_t sector_start, uint8_t* fdata, int count)
{
    int sector_offset = 0;
    for (uint32_t i = 0; i < count;) {
        ata->write28(sector_start + sector_offset, fdata + i, 512);
        sector_offset++;
        i += 512;
    }
    ata->flush();
}

int Tar::get_mode(int file_id, int utype)
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
int Tar::read_file(int file_id, uint8_t* data, size_t size, size_t seek)
{
    if (file_id > file_index)
        return -1;

    int data_offset = sector_links_file[file_id] + 1;
    int data_size = oct_bin(files[file_id].size, 11);

    if (!size)
        return 0;
    if (seek >= data_size)
        return 0;
    if (size > data_size)
        size = data_size;

    read_data(data_offset, data, size, seek);
    return size;
}

/* Reads file from ram using file name */
int Tar::read_file(char* file_name, uint8_t* data, size_t size, size_t seek)
{
    int file_id = find_file(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;
    return read_file(file_id, data, size, seek);
}

/* Reads file from ram using file name */
int Tar::get_size(char* file_name)
{
    int file_id = find_file(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;
    int data_size = oct_bin(files[file_id].size, 11);
    return data_size;
}

int Tar::get_uid(char* file_name)
{
    int file_id = find_file(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;
    int uid = oct_bin(files[file_id].uid, 7);
    return uid;
}

int Tar::get_gid(char* file_name)
{
    int file_id = find_file(file_name);
    if ((file_id > file_index) || (file_id == -1))
        return -1;
    int gid = oct_bin(files[file_id].gid, 7);
    return gid;
}

int Tar::stat(char* file_name, struct stat* statbuf)
{
    memset(statbuf, 0, sizeof(struct stat));
    int file_id = find_file(file_name);
    if ((file_id > file_index) || (file_id == -1)) {
        statbuf->st_size = -1;
        return -1;
    }

    int gid = oct_bin(files[file_id].gid, 7);
    int data_size = oct_bin(files[file_id].size, 11);
    int uid = oct_bin(files[file_id].uid, 7);
    statbuf->st_size = data_size;
    statbuf->st_gid = gid;
    statbuf->st_uid = uid;
    return 0;
}

/* Checksum write in octal */
posix_header* Tar::file_calculate_checksum(posix_header* header_data)
{
    char checksumdata[25];
    char checksum[25];

    memset(checksumdata, 0, 25);
    memset(checksum, 0, 25);
    int check = calculate_checksum(header_data);

    itoa(check, checksumdata);
    int octal_offset_check = 6 - strlen(checksumdata);

    for (int i = 0; i < octal_offset_check; i++)
        checksum[i] = '0';
    strcat(checksum, checksumdata);

    strncpy(header_data->chksum, checksum, 7);
    header_data->chksum[6] = '\0';
    header_data->chksum[7] = ' ';

    return header_data;
}

int Tar::calculate_checksum(posix_header* header_data)
{
    unsigned int chck = 0;
    memset(header_data->chksum, ' ', 8);

    for (int i = 0; i < sizeof(posix_header); i++)
        chck += ((uint8_t*)header_data)[i];
    chck = bin_oct(chck);
    return chck;
}

int Tar::write_file(char* file_name, uint8_t* data, size_t size)
{
    /* Calculate where the file should be located */
    file_index++;
    int file_id = file_index;
    int data_size = oct_bin(files[file_id - 2].size, 11); // Size of last file
    if (data_size % 512 == 0)
        data_size = data_size / 512;
    else
        data_size = (data_size / 512) + 1;
    int data_offset = sector_links_file[file_id - 2];
    int newfile_offset = data_offset + data_size + 1;
    sector_links_file[file_index - 1] = newfile_offset;

    posix_header* meta_head = &files[file_id - 1];

    /* Create the header data */
    memset(meta_head, 0, sizeof(posix_header));
    strcpy(meta_head->name, file_name);

    /* Convert size data to octal */
    char sizedata[25];
    char tsize[25];
    itoa(bin_oct(size), sizedata);
    int octal_offset = 11 - strlen(sizedata);

    tsize[octal_offset] = '\0';
    for (int i = 0; i < octal_offset; i++)
        tsize[i] = '0';
    strcat(tsize, sizedata);
    strcpy(meta_head->size, tsize);

    strcpy(meta_head->magic, "ustar ");      // Header magic
    strcpy(meta_head->version, " \0");       // USTAR version
    strcpy(meta_head->mode, "0000755");      // UNIX Permissions
    strcpy(meta_head->uname, "terry");       // Owner name
    strcpy(meta_head->gname, "\0");          // Group name
    strcpy(meta_head->uid, "0001750");       // User id
    strcpy(meta_head->gid, "0001750");       // Group id
    strcpy(meta_head->mtime, "13715523517"); // Creation date. Temporary const, should be calculated
    strcpy(meta_head->chksum, "       ");    // Temporary checksum data
    meta_head->typeflag = '0';               // Indicate standard file type
    file_calculate_checksum(meta_head);

    uint8_t buffer[512];
    memset(buffer, 0, 512);
    memcpy(buffer, meta_head, sizeof(posix_header));
    ata->write28(data_offset + data_size + 1, buffer, 512);
    write_data(data_offset + data_size + 2, data, size);
    return 0;
}

void Tar::sector_swap(int sector_src, int sector_dest)
{
    uint8_t buffer[513];
    ata->read28(sector_src, buffer, 512);
    ata->write28(sector_dest, buffer, 512);
    ata->flush();
}

void Tar::update_disk(int uentry, int uentry_size)
{
    int lastloc = uentry + uentry_size;
    int moveto = uentry;
    int sector_offset = 0;

    while (1) {
        if (sector_offset == tar_end + 1)
            break;
        if (sector_offset > lastloc) {
            sector_swap(sector_offset, moveto);
            moveto++;
        }
        sector_offset++;
    }

    uint8_t buffer[513];
    memset(buffer, 0, 513);

    for (uint32_t i = tar_end - uentry_size; i < tar_end; i++)
        ata->write28(i, (uint8_t*)&buffer, 512);

    for (uint32_t i = 0; i < file_index; i++)
        if (sector_links_file[i] > uentry)
            sector_links_file[i] -= uentry_size + 1;

    ata->flush();
}

/* Stores all files and directories in RAM */
int Tar::mount()
{
    uint32_t sector_offset = 0;
    char magic[6] = MAGIC;

    while (1) {
        posix_header meta_head;
        ata->read28(sector_offset, (uint8_t*)&meta_head, sizeof(posix_header));

        /* Check for valid header else break mount */
        memcpy(magic, meta_head.magic, 4);
        if (strcmp(magic, MAGIC) != 0)
            break;

        /* Check type and add it to ram */
        if (oct_bin((char*)&meta_head.typeflag, 1) == 0) {
            files[file_index] = meta_head;
            sector_links_file[file_index] = sector_offset;
            file_index++;
        }

        if (oct_bin((char*)&meta_head.typeflag, 1) == 5) {
            dirs[dir_index] = meta_head;
            dir_index++;
        }

        /* Skip the file data and reach the next header */
        if (oct_bin((char*)&meta_head.typeflag, 1) == 0) {
            int data_size = oct_bin((char*)&meta_head.size, 11);
            sector_offset = sector_offset + (data_size / 512);
            if (data_size % 512 != 0)
                sector_offset++;
        }
        sector_offset++;
    }
    tar_end = sector_offset - 1;

    if (dir_index <= 0 && file_index <= 0)
        return -1;
    return 0;
}
