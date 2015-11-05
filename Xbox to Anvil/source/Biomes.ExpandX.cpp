#include "stdafx.h"

/* This gets the index in the biomes array of a block given a block ID and data pair.
 * _data: data value of this block.
 * _block: block ID of this block.
 */
short _index(unsigned char _data, unsigned char _block)
{
	return _block << 4 | (_data & 0xF);
}

/* Set the biome type for all possible data values of the given block ID.
 * _block: The block ID
 * _biome_ID: The biome ID for this block
 * biome_list: Pointer to the buffer containing the biome IDs
 */
void _pc_all(unsigned char _block, unsigned char _biome_id, unsigned char *biome_list)
{
	for (int i = 0; i < 15; i ++)
		biome_list[_index(i, _block)] = _biome_id;
}

/* Set the biome type for all biome-specific blocks; store these in an array for reference.
 * biome_list: pointer to a list in which to store the biome IDs.
 */
void populate_biomes(unsigned char *biome_list)
{

	// Reset biomes
	for (int i = 0; i < 4096; i ++)
		biome_list[i] = 0xFF;

	// Desert
	_pc_all(81, 2, biome_list); // cactus
	biome_list[_index(0, 31)] = 2; // dead shrub
	biome_list[_index(0, 32)] = 2; // dead shrub
	_pc_all(24, 2, biome_list); // sandstone

	// Sand, dirt, and water are handled separately
	_pc_all(9, 254, biome_list);
	_pc_all(12, 253, biome_list);
	_pc_all(2, 252, biome_list);
	_pc_all(3, 252, biome_list);

	// Cold taiga
	biome_list[_index(5, 18)] = 30; // spruce leaves
	biome_list[_index(1, 18)] = 30; // spruce leaves
	biome_list[_index(13, 18)] = 30; // spruce leaves
	biome_list[_index(9, 18)] = 30; // spruce leaves
	_pc_all(78, 30, biome_list); // snow
	_pc_all(79, 30, biome_list); // ice
	_pc_all(80, 30, biome_list); // block of snow

	// Plains
	_pc_all(37, 1, biome_list); // flowers
	_pc_all(38, 1, biome_list); // flowers
	biome_list[_index(1, 31)] = 1; // clump of grass

	// Mushroom island
	_pc_all(110, 14, biome_list); // mycelium
	_pc_all(99, 14, biome_list); // giant mushroom
	_pc_all(100, 14, biome_list); // giant mushroom

	// Jungle
	biome_list[_index(3, 18)] = 21; // jungle leaves
	biome_list[_index(7, 18)] = 21; // jungle leaves
	biome_list[_index(11, 18)] = 21; // jungle leaves
	biome_list[_index(15, 18)] = 21; // jungle leaves
	biome_list[_index(3, 17)] = 21; // jungle wood
	biome_list[_index(7, 17)] = 21; // jungle wood
	biome_list[_index(11, 17)] = 21; // jungle wood
	biome_list[_index(15, 17)] = 21; // jungle wood
	_pc_all(127, 21, biome_list); // cocoa beans

	// Swampland
	_pc_all(106, 6, biome_list); // vines
	_pc_all(111, 6, biome_list); // lily pad
	biome_list[_index(2, 31)] = 6; // fern
	biome_list[_index(3, 31)] = 6; // fern

	// Forest
	biome_list[_index(2, 18)] = 4; // birch leaves
	biome_list[_index(6, 18)] = 4; // birch leaves
	biome_list[_index(10, 18)] = 4; // birch leaves
	biome_list[_index(14, 18)] = 4; // birch leaves
	biome_list[_index(2, 17)] = 4; // birch wood
	biome_list[_index(6, 17)] = 4; // birch wood
	biome_list[_index(10, 17)] = 4; // birch wood
	biome_list[_index(14, 17)] = 4; // birch wood
	biome_list[_index(0, 18)] = 4; // oak leaves
	biome_list[_index(4, 18)] = 4; // oak leaves
	biome_list[_index(8, 18)] = 4; // oak leaves
	biome_list[_index(12, 18)] = 4; // oak leaves

}