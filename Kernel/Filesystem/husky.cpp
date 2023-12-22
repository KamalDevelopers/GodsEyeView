#include "husky.hpp"

Husky::Husky(ATA* ata)
{
    transfer_buffer = (uint8_t*)kmalloc(MAX_TRANSFER_SIZE);
    ptrs_cache = (uint32_t*)kmalloc(sizeof(uint32_t) * 1280);
    memset(transfer_buffer, 0, MAX_TRANSFER_SIZE);
    memset(ptrs_cache, 0, sizeof(uint32_t) * 1280);
    previous_node_ptr = 0;
    previous_block_ptr = 0;
    this->ata = ata;
}

Husky::~Husky()
{
    kfree(ptrs_cache);
    kfree(transfer_buffer);
    kfree(nodes_cache);
}

void Husky::available_disk_size(uint32_t size)
{
    disk_size = size;
}

void Husky::read_data(uint32_t sector_start, uint8_t* fdata, int count, size_t seek)
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

void Husky::write_data(uint32_t sector_start, uint8_t* fdata, int count)
{
    int sector_offset = 0;
    for (uint32_t i = 0; i < count;) {
        uint32_t siz = ((count - i) >= 512) ? 512 : count - i;
        ata->write28(sector_start + sector_offset, fdata + i, siz);
        sector_offset++;
        i += siz;
    }
}

static inline uint32_t murmur3_32_scramble(uint32_t k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

void Husky::hhash_str(hhash_t* hsh, const char* s, const size_t len)
{
    const uint8_t* key = (const uint8_t*)s;
    uint32_t h = len;
    uint32_t k;

    for (size_t i = len >> 2; i; i--) {
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur3_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    k = 0;

    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }

    h ^= murmur3_32_scramble(k);
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    hsh->h = h;
    hsh->c = s[len - 1] + (10000 * s[0]);
}

bool Husky::hhash_cmp(hhash_t* hsh1, hhash_t* hsh2)
{
    if (hsh1->h == hsh2->h && hsh1->c == hsh2->c)
        return true;
    return false;
}

int Husky::mount()
{
    memset(&super_node, 0, sizeof(super_node_t));
    bool has_found = 0;

    disk_size = 10000 * sizeof(node_t);
    while (mount_start_sector * 512 < disk_size) {
        ata->read28(mount_start_sector, (uint8_t*)&super_node, sizeof(super_node_t), 1);
        mount_start_sector++;
        if (super_node.magic[0] != 'm')
            continue;
        if (strncmp(super_node.magic, "magisk.1690243253", 17) == 0) {
            has_found = 1;
            break;
        }
    }

    if (!has_found)
        return -1;

    available_disk_size(super_node.mount_size);
    previous_node_ptr = 0;
    previous_block_ptr = mount_start_sector;
    nodes_cache = (node_t*)kmalloc(super_node.nodes_count * sizeof(node_t));
    read_data(mount_start_sector, (uint8_t*)nodes_cache, super_node.nodes_count * sizeof(node_t));
    create_root_directory();
    return 0;
}

uint8_t Husky::find_empty_node(uint32_t* node_ptr, uint32_t* sector_ptr)
{
    uint32_t runs = 1;
    uint32_t empty_node_ptr = previous_node_ptr;
    if (empty_node_ptr >= super_node.nodes_count) {
        empty_node_ptr = 0;
        runs = 0;
    }

    uint32_t node_siz_div = sizeof(node_t) / 512;
    uint32_t sector_search = empty_node_ptr * node_siz_div;

    for (;;) {
        if (!nodes_cache[empty_node_ptr].type)
            break;

        empty_node_ptr++;
        sector_search += node_siz_div;
        if (empty_node_ptr >= super_node.nodes_count) {
            empty_node_ptr = 0;
            sector_search = 0;
            runs--;
        }

        if (empty_node_ptr >= super_node.nodes_count && !runs)
            return 1;
    }

    *node_ptr = empty_node_ptr;
    *sector_ptr = sector_search;
    previous_node_ptr = empty_node_ptr + 1;
    return 0;
}

uint8_t Husky::find_empty_block(uint32_t* block_ptr, uint32_t* sector_ptr)
{
    block_t block;
    uint32_t runs = 1;
    uint32_t empty_block_ptr = previous_block_ptr;
    if (empty_block_ptr >= super_node.data_blocks_count) {
        empty_block_ptr = 0;
        runs = 0;
    }

    uint32_t block_siz_div = super_node.data_block_size / 512;
    uint32_t sector_search = (super_node.nodes_count * sizeof(node_t)) / 512 + empty_block_ptr * block_siz_div;

    for (;;) {
        ata->read28(mount_start_sector + sector_search, (uint8_t*)&block, sizeof(block_t));
        if (!block.flags)
            break;

        empty_block_ptr++;
        sector_search += block_siz_div;
        if (empty_block_ptr >= super_node.data_blocks_count) {
            empty_block_ptr = 0;
            sector_search = 0;
            runs--;
        }

        if (empty_block_ptr >= super_node.data_blocks_count && !runs)
            return 1;
    }

    *block_ptr = empty_block_ptr;
    *sector_ptr = sector_search;
    previous_block_ptr = empty_block_ptr + 1;
    return 0;
}

void Husky::create_root_directory()
{
    const char* pathname = "/";
    hhash_t hhash;
    hhash_str(&hhash, pathname, 1);

    memset(&root_node, 0, sizeof(node_t));
    root_node.type = 4;
    root_node.creation_time = CMOS::active->timestamp();
    root_node.last_access_time = CMOS::active->timestamp();
    root_node.modification_time = CMOS::active->timestamp();
    root_node.uid = 1000;
    memcpy(&root_node.own_hash, &hhash, sizeof(hhash_t));
    strcpy(root_node.name, pathname);

    memcpy(nodes_cache, &root_node, sizeof(node_t));
    write_data(mount_start_sector, (uint8_t*)&root_node, sizeof(node_t));
}

uint8_t Husky::find_node(hhash_t* hsh, int type, uint32_t* node_ptr)
{
    uint32_t empty_node_ptr = 0;

    for (;;) {
        if (nodes_cache[empty_node_ptr].type == type) {
            if (hhash_cmp(&(nodes_cache[empty_node_ptr].own_hash), hsh))
                break;
        }
        empty_node_ptr++;
        if (empty_node_ptr >= super_node.nodes_count)
            return 1;
    }

    *node_ptr = empty_node_ptr;
    return 0;
}

void Husky::write_node(node_t* node)
{
    uint32_t free_node_ptr = 0;
    uint32_t free_node_sector = 0;
    find_empty_node(&free_node_ptr, &free_node_sector);
    memcpy(&nodes_cache[free_node_ptr], node, sizeof(node_t));
    write_data(mount_start_sector + free_node_sector, (uint8_t*)node, sizeof(node_t));
    ata->flush();
}

void Husky::mkdir(char* pathname)
{
    if (pathname[0] == '\0' || pathname[0] != '/')
        return;

    int subdir_ptr = strlen(pathname) - 1;
    while (subdir_ptr > 0 && pathname[subdir_ptr] != '/')
        subdir_ptr--;

    node_t node;
    memset(&node, 0, sizeof(node_t));
    hhash_str(&node.own_hash, pathname, strlen(pathname));

    if (!subdir_ptr) {
        memcpy(&node.assoc_hash, &root_node.own_hash, sizeof(hhash_t));
    } else {
        hhash_str(&node.assoc_hash, pathname, subdir_ptr);
    }

    node.type = 4;
    node.creation_time = CMOS::active->timestamp();
    node.last_access_time = CMOS::active->timestamp();
    node.modification_time = CMOS::active->timestamp();
    node.uid = 1000;
    strncpy(node.name, pathname + subdir_ptr + 1, strlen(pathname) - subdir_ptr - 1);
    write_node(&node);
}

int Husky::read_dir(char* pathname, fs_entry_t* entries, uint32_t count)
{
    if (pathname[0] == '\0' || pathname[0] != '/')
        return 0;

    uint32_t strsize = strlen(pathname);
    if (strsize > 1 && pathname[strsize - 1] == '/')
        strsize--;

    node_t node;
    uint32_t dir_entries = 0;
    uint32_t empty_node_ptr = 0;
    memset(&node, 0, sizeof(node_t));

    if (strsize == 1 && pathname[0] == '/') {
        memcpy(&node.own_hash, &root_node.own_hash, sizeof(hhash_t));
    } else {
        hhash_str(&node.own_hash, pathname, strsize);
    }

    for (;;) {
        if (nodes_cache[empty_node_ptr].type) {
            if (hhash_cmp(&(nodes_cache[empty_node_ptr].assoc_hash), &node.own_hash)) {
                if (nodes_cache[empty_node_ptr].type == 2)
                    entries[dir_entries].type = 1;
                else if (nodes_cache[empty_node_ptr].type == 4)
                    entries[dir_entries].type = 2;
                else {
                    empty_node_ptr++;
                    continue;
                }

                strcpy(entries[dir_entries].name, nodes_cache[empty_node_ptr].name);
                dir_entries++;
            }
        }

        empty_node_ptr++;
        if (empty_node_ptr >= super_node.nodes_count || dir_entries >= count)
            break;
    }

    return dir_entries;
}

uint8_t Husky::delete_file_data(node_t* node)
{
    if (node->type != 2)
        return 1;
    if (!node->size_in_bytes)
        return 0;

    block_t block;
    block.flags = 0;
    uint32_t size = node->size_in_bytes;
    uint32_t data_blocks_to_remove = 0;

    data_blocks_to_remove = size / (super_node.data_block_size - sizeof(block_t));
    if ((size % (super_node.data_block_size - sizeof(block_t))) != 0)
        data_blocks_to_remove += 1;

    for (uint32_t i = 0; i < sizeof(node->blocks_ptrs) / 4; ++i) {
        if (!data_blocks_to_remove)
            return 0;
        ata->write28(mount_start_sector + node->blocks_ptrs[i], (uint8_t*)&block, sizeof(block_t));
        data_blocks_to_remove--;
    }

    for (uint32_t i = 0; i < sizeof(node->iblocks_ptrs) / 4; ++i) {
        read_data(mount_start_sector + node->iblocks_ptrs[i], (uint8_t*)ptrs_cache, super_node.data_block_size);
        for (uint32_t i = 0; i < 1279; ++i) {
            if (!data_blocks_to_remove)
                return 0;
            ata->write28(mount_start_sector + ptrs_cache[i], (uint8_t*)&block, sizeof(block_t));
            data_blocks_to_remove--;
        }
        ata->write28(mount_start_sector + node->iblocks_ptrs[i], (uint8_t*)&block, sizeof(block_t));
    }

    return 0;
}

int Husky::unlink(char* pathname)
{
    if (pathname[0] == '\0' || pathname[0] != '/')
        return -1;

    node_t node;
    uint32_t node_ptr = 0;
    uint16_t type = 2;
    memset(&node, 0, sizeof(node_t));
    hhash_str(&node.own_hash, pathname, strlen(pathname));
    if (find_node(&node.own_hash, type, &node_ptr)) {
        type = 4;
        if (find_node(&node.own_hash, type, &node_ptr))
            return -1;
    }

    memcpy(&node, &nodes_cache[node_ptr], sizeof(node_t));

    if (type == 2)
        delete_file_data(&node);

    if (type == 4) {
        fs_entry_t entries[1];
        uint32_t count = read_dir(pathname, entries, 1);
        if (count)
            return -1;
    }

    node.type = 0;
    nodes_cache[node_ptr].type = node.type;
    ata->write28(mount_start_sector + (node_ptr * sizeof(node_t)) / 512, (uint8_t*)&node, sizeof(node_t));
    return 0;
}

uint32_t Husky::write_file_data_block_list(uint32_t* ptr_list, const char* written_data_ptr,
    uint32_t* total_written, uint32_t* size_to_write, uint32_t max_blocks)
{
    uint32_t direct_data_blocks_used = 0;
    uint32_t block_ptr = 0;
    uint32_t sector_ptr = 0;
    uint32_t written = 0;

    while (*size_to_write && direct_data_blocks_used < max_blocks) {
        if (find_empty_block(&block_ptr, &sector_ptr))
            return 0;

        block_t block;
        block.flags = 2;
        static char sector[512];
        memset(sector, 0, 512);
        uint32_t sector_size_cpy = 512 - sizeof(block_t);
        if (*size_to_write < sector_size_cpy)
            sector_size_cpy = *size_to_write;

        memcpy(sector, &block, sizeof(block_t));
        memcpy(sector + sizeof(block_t), written_data_ptr, sector_size_cpy);
        ata->write28(mount_start_sector + sector_ptr, (uint8_t*)sector, 512);
        written_data_ptr += sector_size_cpy;
        *size_to_write = *size_to_write - sector_size_cpy;
        written += sector_size_cpy;
        *(ptr_list + direct_data_blocks_used) = sector_ptr;
        sector_ptr++;
        direct_data_blocks_used++;

        uint32_t siz = super_node.data_block_size - 512;
        if (*size_to_write < siz)
            siz = *size_to_write;
        if (!siz)
            break;

        write_data(mount_start_sector + sector_ptr, (uint8_t*)written_data_ptr, siz);
        written += siz;
        written_data_ptr += siz;
        *size_to_write = *size_to_write - siz;
    }

    *total_written = written;
    return direct_data_blocks_used;
}

int8_t Husky::write_file_data(node_t* node, const char* data, size_t size)
{
    char* written_data_ptr = (char*)data;
    uint32_t size_to_write = size;
    uint32_t block_ptr = 0;
    uint32_t sector_ptr = 0;
    uint32_t written = 0;

    uint32_t blocks_used = write_file_data_block_list(ptrs_cache, written_data_ptr, &written, &size_to_write, sizeof(node->blocks_ptrs) / 4);
    written_data_ptr += written;
    for (uint32_t i = 0; i < blocks_used; ++i)
        node->blocks_ptrs[i] = ptrs_cache[i];

    if (!blocks_used && size_to_write)
        return 1;
    if (!size_to_write)
        return 0;

    for (uint32_t indirect_node = 0; indirect_node < sizeof(node->iblocks_ptrs) / 4; ++indirect_node) {
        if (find_empty_block(&block_ptr, &sector_ptr))
            return 0;

        block_t block;
        block.flags = 5;
        write_data(mount_start_sector + sector_ptr, (uint8_t*)&block, sizeof(block_t));

        node->iblocks_ptrs[indirect_node] = sector_ptr;
        uint32_t blocks_used = write_file_data_block_list(ptrs_cache + 1, written_data_ptr, &written, &size_to_write, 1279);
        written_data_ptr += written;
        ptrs_cache[0] = 0;
        memcpy(ptrs_cache, &block.flags, sizeof(block));
        write_data(mount_start_sector + sector_ptr, (uint8_t*)ptrs_cache, super_node.data_block_size);

        if (!blocks_used && size_to_write)
            return 1;
        if (!size_to_write)
            return 0;
    }

    return 0;
}

int Husky::write_file(char* pathname, uint8_t* data, size_t size)
{
    if (pathname[0] == '\0' || pathname[0] != '/')
        return -1;

    int subdir_ptr = strlen(pathname) - 1;
    while (subdir_ptr > 0 && pathname[subdir_ptr] != '/')
        subdir_ptr--;

    node_t node;
    memset(&node, 0, sizeof(node_t));
    hhash_str(&node.own_hash, pathname, strlen(pathname));

    if (!subdir_ptr) {
        memcpy(&node.assoc_hash, &root_node.own_hash, sizeof(hhash_t));
    } else {
        hhash_str(&node.assoc_hash, pathname, subdir_ptr);
    }

    uint32_t parent_node_ptr = 0;
    if (find_node(&node.assoc_hash, 4, &parent_node_ptr))
        return -1;

    uint32_t node_ptr = 0;
    bool overwrite = (find_node(&node.own_hash, 2, &node_ptr) == 0);
    if (overwrite)
        delete_file_data(&nodes_cache[node_ptr]);

    node.type = 2;
    node.size_in_bytes = size;
    node.creation_time = CMOS::active->timestamp();
    node.last_access_time = CMOS::active->timestamp();
    node.modification_time = CMOS::active->timestamp();
    strncpy(node.name, pathname + subdir_ptr + 1, strlen(pathname) - subdir_ptr - 1);
    if (size)
        write_file_data(&node, (const char*)data, size);

    if (overwrite) {
        memcpy(&nodes_cache[node_ptr], &node, sizeof(node_t));
        write_data(mount_start_sector + sizeof(node_t) * node_ptr, (uint8_t*)&node, sizeof(node_t));
        return 0;
    }

    write_node(&node);
    return 0;
}

uint32_t Husky::read_file_data_block_list(uint32_t* ptr_list, uint8_t* read_data_ptr, uint32_t* seek,
    uint32_t* total_read, uint32_t* size_to_read, uint32_t max_blocks)
{
    uint32_t blocks_read = 0;
    uint32_t size_read = 0;

    for (uint32_t i = 0; i < max_blocks; ++i) {
        char sector[512];
        ata->read28(mount_start_sector + ptr_list[i], (uint8_t*)sector, 512);
        block_t block;
        memcpy(&block, sector, sizeof(block_t));

        if (block.flags != 2) {
            kdbg("FS ERROR: points to invalid block\n");
            return 0;
        }

        uint32_t siz = (*seek >= 511) ? 0 : 511 - *seek;
        if (*size_to_read < siz)
            siz = *size_to_read;

        if (*seek < 511) {
            memcpy(read_data_ptr, sector + 1 + *seek, siz);
            read_data_ptr += siz;
            size_read += siz;
            *size_to_read = *size_to_read - siz;
            blocks_read++;
            *seek = 0;
        } else {
            *seek -= 511;
            blocks_read++;
        }

        uint32_t sectors_to_skip = 0;
        uint32_t bytes_to_skip = 0;
        siz = super_node.data_block_size - 512;

        if (*seek >= siz) {
            *seek -= siz;
            continue;
        }
        if (*size_to_read < siz)
            siz = *size_to_read;
        if (!siz)
            break;

        if (*seek) {
            sectors_to_skip = *seek / 512;
            bytes_to_skip = *seek % 512;
            ata->read28(mount_start_sector + ptr_list[i] + 1 + sectors_to_skip, (uint8_t*)sector, 512);
            uint32_t siz = 512 - bytes_to_skip;
            if (*size_to_read < siz)
                siz = *size_to_read;
            memcpy(read_data_ptr, sector + bytes_to_skip, siz);
            read_data_ptr += siz;
            size_read += siz;
            *size_to_read = *size_to_read - siz;
            siz = super_node.data_block_size - 512 - *seek;
            sectors_to_skip++;
            *seek = 0;

            if (!(*size_to_read))
                break;
        }

        if (*size_to_read < siz)
            siz = *size_to_read;
        read_data(mount_start_sector + ptr_list[i] + 1 + sectors_to_skip, read_data_ptr, siz);
        read_data_ptr += siz;
        size_read += siz;
        *size_to_read = *size_to_read - siz;
        if (!(*size_to_read))
            break;
    }

    *total_read = size_read;
    return blocks_read;
}

int Husky::read_file(char* pathname, uint8_t* data, size_t size, size_t seek)
{
    if (pathname[0] == '\0' || pathname[0] != '/')
        return -1;

    node_t node;
    uint32_t node_ptr = 0;
    memset(&node, 0, sizeof(node_t));
    hhash_str(&node.own_hash, pathname, strlen(pathname));
    if (find_node(&node.own_hash, 2, &node_ptr))
        return -1;
    memcpy(&node, &nodes_cache[node_ptr], sizeof(node_t));

    if (!size)
        return 0;
    if (seek >= node.size_in_bytes)
        return 0;
    if (seek + size > node.size_in_bytes)
        size = node.size_in_bytes - seek;
    if (size > node.size_in_bytes)
        size = node.size_in_bytes;

    uint32_t size_to_read = size;
    uint8_t* read_data_ptr = data;
    uint32_t total_read = 0;
    uint32_t seek_blocks = 0;
    uint32_t seek_bytes = 0;

    if (seek) {
        seek_blocks = seek / (super_node.data_block_size - sizeof(block_t));
        seek_bytes = seek - seek_blocks * (super_node.data_block_size - sizeof(block_t));
    }

    for (uint32_t i = 0; i < sizeof(node.blocks_ptrs) / 4; ++i)
        ptrs_cache[i] = node.blocks_ptrs[i];

    if (seek_blocks < (sizeof(node.blocks_ptrs) / 4)) {
        uint32_t blocks_used = read_file_data_block_list(ptrs_cache + seek_blocks, read_data_ptr, &seek_bytes, &total_read, &size_to_read, sizeof(node.blocks_ptrs) / 4 - seek_blocks);
        read_data_ptr += total_read;

        if (!blocks_used && size_to_read)
            return 0;
        if (!size_to_read) {
            return size;
        }
    } else {
        seek_blocks -= sizeof(node.blocks_ptrs) / 4;
    }

    for (uint32_t indirect_node = 0; indirect_node < sizeof(node.iblocks_ptrs) / 4; ++indirect_node) {
        read_data(mount_start_sector + node.iblocks_ptrs[indirect_node], (uint8_t*)ptrs_cache, super_node.data_block_size);
        if (seek_blocks >= 1279) {
            seek_blocks -= 1279;
            continue;
        }
        uint32_t blocks_used = read_file_data_block_list(ptrs_cache + 1 + seek_blocks, read_data_ptr, &seek_bytes, &total_read, &size_to_read, 1279 - seek_blocks);
        read_data_ptr += total_read;

        if (!blocks_used && size_to_read)
            return 0;
        if (!size_to_read) {
            return size;
        }
    }

    return size;
}

int Husky::find_file(char* pathname)
{
    if (read_file(pathname, 0, 0, 0) != -1)
        return 1;
    return -1;
}

int Husky::stat(char* pathname, struct stat* statbuf)
{
    if (pathname[0] == '\0' || pathname[0] != '/')
        return -1;

    node_t node;
    uint32_t node_ptr = 0;
    hhash_str(&node.own_hash, pathname, strlen(pathname));
    if (find_node(&node.own_hash, 2, &node_ptr)) {
        statbuf->st_size = -1;
        return -1;
    }

    memset(statbuf, 0, sizeof(struct stat));
    memcpy(&node, &nodes_cache[node_ptr], sizeof(node_t));

    statbuf->st_size = node.size_in_bytes;
    statbuf->st_gid = node.gid;
    statbuf->st_uid = node.uid;
    statbuf->st_ctime = node.creation_time;
    statbuf->st_atime = node.last_access_time;
    return 0;
}
