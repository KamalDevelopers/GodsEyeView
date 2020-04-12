#ifndef PT_HPP
#define PT_HPP

#include "types.hpp"
#include "../Hardware/Drivers/ata.hpp"
#include "fat32.hpp"

namespace Fat32
{
	struct PartitionTableEntry
	{
		uint8_t bootable;
		uint8_t start_head;
		uint8_t start_sector : 6;
		uint16_t start_cylinder : 10;

		uint8_t partition_id;

		uint8_t end_head;
		uint8_t end_sector : 6;
		uint16_t end_cylinder : 10;

		uint32_t start_lba;
		uint32_t length;
	} __attribute__((packed));

	struct MasterBootRecord
	{
		uint8_t bootloader[440];
		uint32_t signature;
		uint16_t unused;

		PartitionTableEntry primary[4];
		uint16_t magicnumber;

	} __attribute__((packed));

	class PartitionTable
	{
	public:
		static void ReadPartitions(AdvancedTechnologyAttachment *hd);
	};
}
#endif