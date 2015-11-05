// ExpandX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>

// Chunk-specific variables
int iol, xpos, zpos, xstart, zstart, n;
int i, blockstart, datastart, blocklightstart, skylightstart, biomestart;
int y_plane, h;
int local_biome_list[256];

// Variables for file I/O
FILE *input_file_f, *output_file_f, *log_file_f;
char *afile_path, *aoutput_path, *status;
unsigned long file_size, bytes_read, template_length, section_template_length;

// All our memory handling
unsigned char *nbuffer, *xbuffer, *rbuffer, *ebuffer, *sbuffer, s_num;
unsigned char *biome_list;
int broken = 0, missing = -295;

// Contained in the Biomes.cpp file
void populate_biomes(unsigned char *biome_list);

// Takes an input int and flips it from Big to Little Endian or vice versa.
int EndianFlip(int _input_int)
{

	int retval;
	unsigned char *_input_ptr = (unsigned char *)&_input_int;
	unsigned char *_p_retval = (unsigned char *)&retval;
	
	for (int _i = 0; _i < 4; _i ++)
	{
		_p_retval[3 - _i] = _input_ptr[_i];
	}

	return retval;

}

// Find the offset of the Blocks section of the XBox file, in bytes
int BlockStart(unsigned char *_in_buffer, int _size)
{

	int _i = 0;

	// Loop through looking for the Blocks section
	do
	{
		if (_in_buffer[_i] == 'B')
			if (_in_buffer[_i + 1] == 'l')
				if (_in_buffer[_i + 5] == 's')
					return _i + 10;
		_i ++;
	} while (_i < _size);

	return -1;

}

// Find the offset of the Biomes section of the XBox file, in bytes
int BiomeStart(unsigned char *_in_buffer, int _size)
{

	int _i = 0;

	// Loop through looking for the Blocks section
	do
	{
		if (_in_buffer[_i] == 'B')
			if (_in_buffer[_i + 1] == 'i')
				if (_in_buffer[_i + 5] == 's')
					return _i + 10;
		_i ++;
	} while (_i < _size);

	return -1;

}

// Gets the higher order nybble of a byte
unsigned char HiNybble(unsigned char _in_byte)
{
	return _in_byte >> 4;
}

// Gets the lower order nybble of a byte
unsigned char LoNybble(unsigned char _in_byte)
{
	return _in_byte & 0xF;
}

// Gets the offset of the BlockLight section in the XBox file, in bytes
int BlockLight(unsigned char *_in_buffer, int _size)
{

	int _i = 0;

	// Loop through looking for the BlockLight section
	do
	{
		if (_in_buffer[_i] == 'B')
			if (_in_buffer[_i + 1] == 'l')
				if (_in_buffer[_i + 7] == 'g')
					if (_in_buffer[_i + 8] == 'h')
						return _i + 14;
		_i ++;
	} while (_i < _size);

	return -1;

}

// Gets the offset of the SkyLight section in the XBox file, in bytes
int SkyLight(unsigned char *_in_buffer, int _size)
{

	int _i = 0;

	// Loop through looking for the SkyLight section
	do
	{
		if (_in_buffer[_i] == 'S')
			if (_in_buffer[_i + 1] == 'k')
				if (_in_buffer[_i + 5] == 'g')
					if (_in_buffer[_i + 6] == 'h')
						return _i + 12;
		_i ++;
	} while (_i < _size);

	return -1;

}

// Takes a short and flips it from Big Endian to Little Endian or vice versa.
short EndianFlip(short _input_short)
{

	short retval;
	unsigned char *_input_ptr = (unsigned char *)&_input_short;
	unsigned char *_p_retval = (unsigned char *)&retval;
	
	for (int _i = 0; _i < 2; _i ++)
	{
		_p_retval[1 - _i] = _input_ptr[_i];
	}

	return retval;

}

// Gets the offset of the data section of the XBox file, in bytes
int DataStart(unsigned char *_in_buffer, int _size)
{

	int _i = 0;

	// Loop through looking for the Blocks section
	do
	{
		if (_in_buffer[_i] == 'D')
			if (_in_buffer[_i + 1] == 'a')
				if (_in_buffer[_i + 3] == 'a')
					return _i + 8;
		_i ++;
	} while (_i < _size);

	return -1;

}

// This analyzes an NBT tag to make sure it is not broken.
// _out_buffer: pointer to the start of the tag in memory.
// _size: length of the buffer
// _tag_type: the tag type as defined by Minecraft
// returns: the total length of the tag, in bytes, or -1 if the tag is invalid.
int ProcessTag(unsigned char *_out_buffer, int _size, unsigned char _tag_type)
{
	
	int _n = 0, _i;
	int _tag_num;
	short _name_length;
	int _result;

	// Tag-specific processing now occurs
	switch (_tag_type)
	{
	case 1:

		// Bytes should be OK. Skip over this tag.
		return _n + 1;

	case 2:
			
		// Shorts can be encoded with the strange negative encoding
		// We check for this by seeing if there is an alphanumeric character for the next tag earlier than expected
		return _n + 2;

	case 3:

		// Integers can be encoded with the strange negative encoding
		// We check for this by seeing if there is an alphanumeric character for the next tag earlier than expected
		return _n + 4;

	case 4:

		// Longs can be encoded with the strange negative encoding
		// We check for this by seeing if there is an alphanumeric character for the next tag earlier than expected
		return _n + 8;

	case 5:
			
		// Floats should be OK. Skip this tag.
		return _n + 4;

	case 6:

		// Doubles should be OK; skip this tag.
		return _n + 8;

	case 7:

		// Bytes arrays should be OK. First, get the size, then skip over this tag.
		return _n + 4 + EndianFlip(*((int *)(_out_buffer + _n)));

	case 8:

		// Strings should be OK; first get the length, then skip this tag
		return _n + 2 + EndianFlip(*((short *)(_out_buffer + _n)));

	case 9:

		// Here are the tricky ones. This list type will require sub-processing.
		// First, get the length, then loop through all the entries
		_tag_type = _out_buffer[_n];
		_n += 5;
		_tag_num = EndianFlip(*((int *)(_out_buffer + _n - 4)));
		for (_i = 0; _i < _tag_num; _i ++)
		{
			_result = ProcessTag(_out_buffer + _n, _size, _tag_type);
			if (_result == -1) return -1;
			_n += _result;
		}
		return _n;

	case 10:

		// This is a compound tag. All subtags will be processed in order.
		do
		{

			// Grab the tag type; break if this is the terminating tag (a null byte)
			_tag_type = _out_buffer[_n];
			if (_tag_type == 0) return _n + 1;

			// Grab the name length, then skip over the name
			_name_length = EndianFlip(*((short *)(_out_buffer + _n + 1)));
			_n += _name_length + 3;

			// Process the sub tag.
			_result = ProcessTag(_out_buffer + _n, _size - _n, _tag_type);
			if (_result == -1) return -1;
			_n += _result;

		} while (_n <= _size);

		// If here, we ran out of buffer space before processing finished.
		// This indicates the tag was broken, and we have to bail.
		return -1;

	case 11:

		// This integer array should be OK. Skip this tag.
		return _n + 4 + 4 * EndianFlip(*((int *)(_out_buffer + _n)));

	default:

		// If we are here, there is a serious problem.
		// No tag should have an ID value above 11; return -1 to indicate processing failed.
		return -1;

	}

}

/* This method copies the Entities or TileEntities tag into an empty buffer and checks to make sure it is not broken.
 * _in_buffer: Pointer to the tag in the XBox file
 * _out_buffer: Pointer to the empty buffer into which to copy this tag.
 * _size: size of the tag in bytes.
 * returns: the length of the empty buffer used.
 */
int ProcessEntities(unsigned char *_in_buffer, unsigned char *_out_buffer, int _offset, int _size)
{
	
	int _n = 7;
	int _i = 0;

	// Do the memory copy first
	memcpy(_out_buffer, _in_buffer + _offset, _size);

	// Now pass the TileEntities tag into the global processor.
	return ProcessTag(_out_buffer + _n + 8, _size, 9);

}

/* This looks at the top blocks in every column and makes an attempt at assigning a biome to each column.
 * Entire chunks will share ONE biome. Assignments are made based on which biome the chunk probably is.
 * The method considers only the TOP NON-AIR blocks in the chunk in making its decisions.

 * _in_buffer: pointer to the biomes block to modify.
 * _block_buffer: pointer to the blocks section corresponding to the biomes buffer above.
 * _data_buffer: pointer to the data section corresponding to the biomes buffer above.
 * _flag_no_reset: pointer to a buffer in which to store flags for biome columns that are not to be changed in the second pass.
*/
void ProcessBiomes(unsigned char *_in_buffer, unsigned char *_block_buffer, unsigned char *_data_buffer)
{

	int n, _index;
	int _c_max, _max_ID;
	int max_height = 0;
	int min_height = 128;

	// First clear the input buffer and set all biomes to plains by default
	for (int _i = 0; _i < 256; _i ++)
		_in_buffer[_i] = 0xFF;
	for (int _i = 0; _i < 256; _i ++)
		local_biome_list[_i] = 0;
	for (int _i = 0; _i < 256; _i ++)
		_flag_no_reset[_i] = 0;

	// We want to start at an offset of 128 for this column
	_block_buffer += 128;
	_data_buffer += 64;

	// First pass - get top block and see if it is listed as a known biome
	for (int _i = 0; _i < 256; _i ++)
	{

		// Initial offset for this column
		n = -1;

		// Loop backwards through the block column until we find the first non-air block
		while (_block_buffer[n] == 0 && n > -256)
			n --;
	
		// If no non-air blocks were found, bail now
		if (n == -256) continue;

		// Look at the heights ONLY IF this is a DIRT block.
		if (_block_buffer[n] == 2 || _block_buffer[n] == 3)
		{
			if (128 + n > max_height) max_height = 128 + n;
			if (128 + n < min_height) min_height = 128 + n;
		}

		// Now get the index of this data / block pair
		if (n % 2 == 0)
			_index = LoNybble(_data_buffer[(n - 1) / 2]);
		else
			_index = HiNybble(_data_buffer[(n - 1) / 2]);

		// Add block to index
		_index |= _block_buffer[n] << 4;

		// Update the biome if it is known
		if (biome_list[_index] != 0xFF)
		{
			local_biome_list[biome_list[_index]] ++;
			_in_buffer[_i] = biome_list[_index];
		}
	
		// Reset index
		_index = 0;

		// Move to next column
		_block_buffer += 128;
		_data_buffer += 64;

	}

	// Now get the most common biome in this chunk
	_c_max = -1;
	for (int _i = 0; _i < 256; _i ++)
	{
		if (local_biome_list[_i] > _c_max)
		{
			_c_max = local_biome_list[_i];
			_max_ID = _i;
		}
	}

	// Default to plains
	if (_max_ID == 0) _max_ID = 1;

	// If the maximum height is at least 80, or if the tallest hill is 12 or more blocks higher than than the lowest valley...
	// ... this is extreme hills
	if (max_height >= 80 || max_height - min_height >= 12)
		_max_ID = 3;

	// If more than half of this chunk consists of water, we set it to ocean
	if (local_biome_list[254] > 128)
		_max_ID = 0;

	// If at least one hundred blocks in this chunk consist of sand, we set it to desert
	// Also, if at least one of the blocks in this chunk is sand and there is little water and dirt, this is desert
	if ( (local_biome_list[253] >= 100) || (local_biome_list[253] > 0 && local_biome_list[254] < 64 && local_biome_list[252] == 0 && local_biome_list[1] == 0) )
		_max_ID = 2;

	// If any evidence was found this should be a swamp biome and there was no evidence for a jungle biome...
	// ... call this a swamp. (Ex: vines but no jungle leaves)
	if (local_biome_list[6] > 0)
		_max_ID = 6;
	if (local_biome_list[21] > 0)
		_max_ID = 21;

	// Second pass - assign all to the most common for this chunk.
	for (int _i = 0; _i < 256; _i ++)
	{
		if (_in_buffer[_i] != 30 && _in_buffer[_i] != 21) _in_buffer[_i] = _max_ID;
	}

}

/* Anvil format stores blocks in YZX format instead of XZY format.
 * This method takes an input buffer with blocks stored in XZY order and outputs the data in YZX format into an output buffer.
 * _in_buffer: the block data stored in XZY format.
 * _out_buffer: buffer to write the YZX format data.
 * _y_levels: The number of planes in the y direction to convert.
 */
bool xyz_flip(unsigned char *_in_buffer, unsigned char *_out_buffer, unsigned char _y_level)
{

	unsigned char x, y, z;
	int c_ts = _y_level * 16, c_td;
	bool nair_flag = false;
	int c_inc = 0x70;

	// Loop through all y planes
	for (y = 0; y < 16; y ++)
	{

		// Each new plane begins at an index specified only by the plane's y coordinate.
		c_td = y;

		// There are 16 rows in this plane
		for (z = 0; z < 16; z ++)
		{

			// There are 16 cols in this plane
			for (x = 0; x < 16; x ++)
			{

				// If we find a non-air block, we set a flag to let us know this section is not empty
				if (_in_buffer[c_ts] != 0)
					nair_flag = true;

				// Move memory, then update coordinates
				_out_buffer[c_td] = _in_buffer[c_ts];
				c_td += 1 << 8;
				c_ts ++;

			}

			// Increment the z counter in the destination integer and reset the x counter
			c_td += 1 << 4;
			c_td &= 0xFF;

			// Skip to the next column in the source block
			c_ts += c_inc;

		}

	}

	return nair_flag;

}

/* Anvil format stores blocks in YZX format instead of XZY format.
 * This method takes an input buffer with data nybbles stored in XZY order and outputs the data in YZX format into an output buffer.
 * _in_buffer: the block data stored in XZY format.
 * _out_buffer: buffer to write the YZX format data.
 * _y_levels: The number of planes in the y direction to convert.
 */
void xyz_nybbleflip(unsigned char *_in_buffer, unsigned char *_out_buffer, unsigned char _y_level)
{

	unsigned char x, y, z;
	unsigned char st;
	int c_ts, c_td;
	int c_inc = 0x70;

	// Zero memory first for easier bitwise operation later
	for (c_ts = 0; c_ts < 0x800; c_ts ++)
		_out_buffer[c_ts] = 0;

	// Get the source pointer set
	c_ts = 16 * _y_level;

	// Loop through all y planes
	for (y = 0; y < 16; y ++)
	{

		// Each new plane begins at an index specified only by the plane's y coordinate.
		c_td = y;

		// There are 16 rows in this plane
		for (z = 0; z < 16; z ++)
		{

			// There are 16 cols in this plane
			for (x = 0; x < 16; x ++)
			{

				// Get the nybble we're interested in
				if (c_ts % 2 == 0)
					st = LoNybble(_in_buffer[c_ts / 2]);
				else
					st = HiNybble(_in_buffer[c_ts / 2]);

				// Update the target nybble
				if (c_td % 2 == 0)
					_out_buffer[c_td / 2] |= st;
				else
					_out_buffer[c_td / 2] |= st << 4;

				// Increment the pointer and repeat
				c_td += 1 << 8;
				c_ts ++;

			}

			// Increment the z counter in the destination integer and reset the x counter
			c_td += 1 << 4;
			c_td &= 0xFF;

			// Skip to next column in source block
			c_ts += c_inc;

		}

	}

}

/* XBox files are packed a little more tightly than simple compression.
 * The signal 'FF' followed by a number n and a byte B indicates that B is repeated n + 1 times.
 * If n is less than 3, the signal indicates that the byte 'FF' shall be repeated n + 1 times.
 * This method unpacks a given input buffer in this format to the output buffer given.
 * Return value is the amount of the output buffer used.

 * _input_buffer: the data to expand
 * _output_buffer: buffer in which to output the expanded data
 * _input_size: length of the input buffer to expand
 */
unsigned long ExpandX(unsigned char *_input_buffer, unsigned char *_output_buffer, unsigned long _input_size)
{
	
	unsigned long retval = 0;
	unsigned short _limit;
	unsigned char _repeat;

	// Begin looping through the input buffer
	for (unsigned long _i = 0; _i < _input_size; _i ++)
	{

		// Check to see if the character at this place is FF
		if (_input_buffer[_i] == 0xFF)
		{
			
			// If here, increment outer counter and add the repeated bytes. 
			// Note that the (n + 1)th repeat is taken care of after this if statement.
			_i ++;
			_repeat = _input_buffer[_i + 1];
			_limit = (unsigned short)_input_buffer[_i];
			if (_limit >= 3)
			{
				for (int _n = 0; _n <= _limit; _n ++)
				{
					_output_buffer[retval] = _repeat;
					retval ++;
				}
			}
			else
			{
				_i --;
				for (int _n = 0; _n <= _limit; _n ++)
				{
					_output_buffer[retval] = 0xFF;
					retval ++;
				}
			}

			// Increment the outer counter again to skip the number of repeats and the repeated byte
			_i ++;

		}
		else
		{
			_output_buffer[retval] = _input_buffer[_i];
			retval ++;
		}

	}

	// Return the length of outputted content.
	return retval;

}

/* The entry point for the console application.
 * Call this application with the following:

 * ExpandX _input_directory _output_directory _x_pos _z_pos [Nether|End]
 * _input_directory: path to the directory containing the packed XBox files.
 * _output_directory: path to the directory in which to output the converted PC files.
 * _x_pos: use '-' if this region is negative in the x direction; any other string otherwise.
 * _z_pos: use '-' if this region is negative in the z direction; any other string otherwise.
 * [Nether|End]: Optional; use Nether for a Nether chunk and End for an End chunk. DO NOT SET for an overworld chunk.
 */
int _tmain(int argc, _TCHAR* argv[])
{
	
	// Allocate memory
	xbuffer = new unsigned char[409600];
	nbuffer = new unsigned char[409600];
	rbuffer = new unsigned char[409600];
	ebuffer = new unsigned char[409600];
	sbuffer = new unsigned char[409600];
	afile_path = new char[200];
	aoutput_path = new char[200];
	status = new char[300];
	biome_list = new unsigned char[4096];

	// Load xstart and zstart
	if (argc < 5)
	{
		xstart = 0;
		zstart = 0;
	}
	else
	{
		xstart = (*argv[3] == '-' ? 32 : 0);
		zstart = (*argv[4] == '-' ? 32 : 0);
	}
	
	// If this is the Nether or End, there are less chunks expected
	if (argc > 5) missing = -943;

	// Load the Anvil template file into memory
	input_file_f = fopen("anvil_template.dat", "r");
	fseek(input_file_f, 0L, SEEK_END);
	template_length = ftell(input_file_f);
	fseek(input_file_f, 0L, SEEK_SET);
	fread(rbuffer, sizeof(char), template_length, input_file_f);
	fclose(input_file_f);

	// Load the section template file into memory
	input_file_f = fopen("section_template.dat", "r");
	fseek(input_file_f, 0L, SEEK_END);
	section_template_length = ftell(input_file_f);
	fseek(input_file_f, 0L, SEEK_SET);
	fread(sbuffer, sizeof(char), section_template_length, input_file_f);
	fclose(input_file_f);

	// This assigns biome IDs to specific block types for later reference.
	populate_biomes(biome_list);

	// Open the log file
	log_file_f = fopen("log.txt", "w+b");

	// Loop through input directory
	for (iol = 0; iol < 1024; iol ++)
	{
	
		// Get the input and output paths
		sprintf(afile_path, "%ls\\%d.dat", argv[1], iol);
		sprintf(aoutput_path, "%ls\\%d.dat", argv[2], iol);

		// Next input file
		input_file_f = fopen(afile_path, "rb");
		if (input_file_f != NULL)
		{
			
			// Get the size of the file
			fseek(input_file_f, 0L, SEEK_END);
			file_size = ftell(input_file_f);

			// Read in all data
			fseek(input_file_f, 0L, SEEK_SET);
			bytes_read = fread(nbuffer, sizeof(char), file_size, input_file_f); 
			if (bytes_read != file_size) printf("Warning: could not read %d bytes from file %d.\n", file_size, iol);
			fclose(input_file_f);

			// Perform the actual expansion
			file_size = ExpandX(nbuffer, xbuffer, file_size);

			// Get the start points for the blocks, data, skylight, and blocklight blocks
			blockstart = BlockStart(xbuffer, file_size);
			datastart = DataStart(xbuffer, file_size);
			blocklightstart = BlockLight(xbuffer, file_size);
			skylightstart = SkyLight(xbuffer, file_size);

			// If there is no block or data section, we delete this corrupted file and abort conversion of this chunk
			if (blockstart == -1 || datastart == -1)
			{
				remove(aoutput_path);
				remove(afile_path);
				sprintf(status, "Warning: chunk missing blocks and/or data; not converted at x=%d, z=%d.\r\n\0", (iol % 32) - xstart, (int)(iol / 32) - zstart);
				fwrite(status, 1, strlen(status), log_file_f);
				continue;
			}

			// If here, there was no issue with the chunk file, so we open the output file
			output_file_f = fopen(aoutput_path, "w+b");

			// Attempt to correct the biomes
			ProcessBiomes(rbuffer + 0x28, xbuffer + blockstart, xbuffer + datastart);
			
			// Output xpos and zpos into the template
			xpos = EndianFlip((iol % 32) - xstart);
			zpos = EndianFlip(iol / 32 - zstart);
			memcpy(rbuffer + 0x15B, &xpos, 4);
			memcpy(rbuffer + 0x166, &zpos, 4);

			// If this is the nether, we replace all bedrock with air and set biome to "hell"
			// For the end, set biome to "Sky"
			if (argc > 5)
			{
				if (wcscmp(argv[5], L"Nether") == 0)
				{
					for (i = blockstart; i < blockstart + 0x8000; i ++) if ((i - blockstart) % 128 >= 3) if (xbuffer[i] == 7) xbuffer[i] = 87;
					for (i = 0x28; i < 0x128; i ++) rbuffer[i] = 8;
				}
				else if (wcscmp(argv[5], L"End") == 0)
					for (i = 0x28; i < 0x128; i ++) rbuffer[i] = 9;
			}

			// Find and process the Entities section
			zpos = -1;
			for (int n = file_size - 1; n > 0; n --)
			{
				if (*(xbuffer + n) == 's' && *(xbuffer + n - 1) == 'e' && *(xbuffer + n - 2) == 'i' && *(xbuffer + n - 3) == 't' && *(xbuffer + n - 8) == 8)
				{
					zpos = 11 + ProcessEntities(xbuffer, ebuffer, n - 14, file_size - n + 14);
					break;
				}
			}

			// Begin assembling the final file
			memcpy(nbuffer, rbuffer, 0xB);
			if (zpos != 10)
			{
				memcpy(nbuffer + 0xB, ebuffer + 4, zpos);
				memcpy(nbuffer + 0xB + zpos, rbuffer + 0x1B, 0x14F);
			}
			else
			{
				memcpy(nbuffer + 0xB, nbuffer + 0xB, 0x15F);
				zpos = 0x10;
			}

			// Find the tile entity section and process
			for (int n = file_size - 1; n > 0; n --)
			{
				if (*(xbuffer + n) == 's' && *(xbuffer + n - 1) == 'e' && *(xbuffer + n - 2) == 'i' && *(xbuffer + n - 3) == 't' && *(xbuffer + n - 12) == 12)
				{
					xpos = 15 + ProcessEntities(xbuffer, ebuffer, n - 14, file_size - n + 14);
					break;
				}
			}

			// Add the TileEntities section
			if (xpos != 14)
			{
				memcpy(nbuffer + 0x15A + zpos, ebuffer, xpos);
				memcpy(nbuffer + 0x15A + zpos + xpos, rbuffer + 0x17E, 0x44C);
			}
			else
			{
				memcpy(nbuffer + 0x15A + zpos, rbuffer + 0x16A, 0x460);
				xpos = 0x14;
			}

			// Use zpos as the total length of the upper template file for now
			zpos += 0x5A6 + xpos;

			// Get the number of y planes in the blocks segment
			y_plane = EndianFlip(*(int *)(xbuffer + blockstart - 4)) / 0x1000;

			// Reset section count to zero.
			s_num = 0;
			h = 0;

			// Now, flip the XYZ order of all blocks in each segment
			for (xpos = 0; xpos < y_plane; xpos ++)
			{
			
				// If this is the 8th plane up from the bottom, we advance to the second half of the XBox blocks section.
				if (xpos == 8) h = 0x8000;

				// Perform the flip of blocks. If this if statement returns false, the section was empty.
				// Empty sections are not stored, so if this section is empty we skip it now.
				if (!xyz_flip(xbuffer + blockstart + h, sbuffer + 0x183D, xpos % 8))
					continue;

				// If here, this section is not empty, so now we flip the data, blocklight, and skylight sections.
				xyz_nybbleflip(xbuffer + datastart + (h / 2), sbuffer + 0xB, xpos % 8);
				xyz_nybbleflip(xbuffer + blocklightstart + (h / 2), sbuffer + 0x102B, xpos % 8);
				xyz_nybbleflip(xbuffer + skylightstart + (h / 2), sbuffer + 0x81A, xpos % 8);

				// This assigns the Y coordinate of the section.
				*(sbuffer + 0x182F) = (unsigned char)xpos;

				// Move this section to the end of the template file
				memcpy(nbuffer + zpos + 0x283E * s_num, sbuffer, 0x283E);
				s_num ++;

			}

			// Set the final two bytes to zero and set the number of sections correctly
			for (xpos = 0; xpos < 3; xpos ++) *(nbuffer + zpos + 0x283E * s_num + xpos) = 0;
			*(nbuffer + zpos - 1) = s_num;

			// Here we double check the status of this chunk; if it is broken, we remove it
			if (ProcessTag(nbuffer + 3, zpos + 0x283E * s_num - 1, 10) != zpos + 0x283E * s_num - 1)
			{
				remove(aoutput_path);
				remove(afile_path);
				sprintf(status, "Warning: broken chunk not converted at x=%d, z=%d.\r\n\0", (iol % 32) - xstart, (int)(iol / 32) - zstart);
				fwrite(status, 1, strlen(status), log_file_f);
				broken ++;
			}
			else
			{
				fwrite(nbuffer, 1, zpos + 0x283E * s_num + 2, output_file_f);
			}

			// Reset buffer for next chunk
			memcpy(nbuffer, rbuffer, template_length);

			// Close handle to this output file
			fclose(output_file_f);

		}
		else
			missing ++;

	}

	// Write final info and close log file
	if (missing < 0) missing = 0;
	sprintf(status, "Chunk conversion complete for region x=%d, z=%d. Chunk details:\r\n\0", xstart == 32 ? -1 : 0, zstart == 32 ? -1 : 0);
	fwrite(status, 1, strlen(status), log_file_f);
	sprintf(status, "\tOK: %d\r\n\tMissing: %d\r\n\tBroken: %d\r\n\0", 1024 - missing - broken - (argc > 5 ? 943 : 295), missing, broken);
	fwrite(status, 1, strlen(status), log_file_f);
	fclose(log_file_f);

	// Free buffers
	delete [409600] xbuffer;
	delete [409600] nbuffer;
	delete [409600] rbuffer;
	delete [409600] ebuffer;
	delete [409600] sbuffer;
	delete [200] aoutput_path;
	delete [200] afile_path;
	delete [300] status;
	delete [4096] biome_list;

	return 0;

}

