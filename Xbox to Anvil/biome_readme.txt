BIOME DETERMINATION:
----------------------------

As described in the readme, the top, non-air block in each column is considered. There are then 256 considered blocks per chunk.
For a given chunk, the biome will be:

EXTREME HILLS if...
	
	-> The highest block has y coordinate at least 80, OR
	-> The highest hill is at least 12 blocks higher than the lowest valley,

	AND this biome is not later determined to be

	-> jungle OR swampland OR desert

SWAMPLAND if...

	-> at least one of the following blocks is present: VINES, LILY PAD, FERN

	AND
	
	-> the following blocks are not found: JUNGLE LEAVES, JUNGLE WOOD, COCOA BEANS

JUNGLE if...

	-> at least one of the following blocks is present: JUNGLE wOOD, JUNGLE LEAVES, COCOA BEANS

COLD TAIGA if...
	
	-> at least one of the following blocks is present: SNOW, ICE, SPRUCE LEAVES

OCEAN if...
	
	-> at least HALF (128) of the blocks in the chunk are WATER SOURCE blocks

DESERT if...

	-> at least 100 of the blocks in the chunk are SAND, OR
	-> at least one of the following blocks is present: SANDSTONE, CACTUS, or DEAD SHRUB

PLAINS otherwise.