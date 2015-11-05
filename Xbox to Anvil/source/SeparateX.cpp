// SeparateX.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

FILE *input_file_f, *output_file_f;
unsigned char *buffer, *relay_buffer;
unsigned long bytes_read;
int i, size, offset;
char *output_pathA, *input_pathA;

/* _tmain takes an XBox .mcr file and reads all the compressed chunk blocks into individual files.
 * The method also copies the 4 KB "last updated" header of the .mcr file into a new file called header_output.txt.
 * This header_output.txt will be used by later programs when assembling the final PC .mcr file.

 * _tmain expects two command line arguments:
 * argv[0]: Path to the .mcr file from which the data will be read.
 * argv[1]: Path to the directory into which the files should be output.
 */
int _tmain(int argc, _TCHAR* argv[])
{

	// Allocate memory for buffer and offsets and sizes
	buffer = new unsigned char[4096];
	relay_buffer = new unsigned char[4096];
	input_pathA = new char[200];
	output_pathA = new char[200];

	// Open the file and read in the first 4096.
	sprintf(input_pathA, "%ls", argv[1]);
	input_file_f = fopen(input_pathA, "rb");
	bytes_read = fread(buffer, sizeof(char), 4096, input_file_f);

	// Create the header_output.txt file and write out two blocks: the offset block and last updated block.
	// The offset block will be overwritten by a later program.
	output_file_f = fopen("header_output.txt", "wb");
	bytes_read = fread(relay_buffer, sizeof(char), 4096, input_file_f);
	fwrite(buffer, sizeof(char), 4096, output_file_f);
	fwrite(relay_buffer, sizeof(char), 4096, output_file_f);
	fclose(output_file_f);

	// Now loop through the 1024 chunk entries
	for (i = 0; i < 4096; i += 4)
	{
		
		// Grab offset and size for each
		offset = buffer[i] * 65536 + buffer[i + 1] * 256 + buffer[i + 2];
		size = buffer[i  + 3];

		// If the offset is zero, we do nothing more; this chunk has no associated data.
		if (offset != 0)
		{

			// Create an output file for this chunk
			sprintf(output_pathA, "%ls\\%d.dat", argv[2], i / 4);
			output_file_f = fopen(output_pathA, "wb");

			// Move the input file pointer to the location of the chunk
			// The first four bytes are the length of the data; store that first.
			fseek(input_file_f, 4096 * offset, SEEK_SET);
			fread(relay_buffer, sizeof(char), 4, input_file_f);
			offset = ((relay_buffer[1] << 16) + (relay_buffer[2] << 8) + relay_buffer[3]) % 4096;

			// Relay the input immediately to the output file in 4KB chunks
			for (int x = 1; x < size; x ++)
			{
				
				// Try to read the next segment.
				bytes_read = fread(relay_buffer, sizeof(char), 4096, input_file_f);
				if (bytes_read != 4096) printf("Warning: could not read sector %d for chunk %d.\n", x, i / 4);
				bytes_read = fwrite(relay_buffer, sizeof(char), 4096, output_file_f);
				if (bytes_read != 4096) printf("Warning: could not write sector %d for chunk %d.\n", x, i / 4);

			}

			// Read the final segment without its trailing zeros.
			bytes_read = fread(relay_buffer, sizeof(char), offset, input_file_f);
			if (bytes_read != offset) printf("Warning: could not read final sector for chunk %d.\n", i / 4);

			// Write the final segment to the file and close the handle.
			fwrite(relay_buffer, sizeof(char), bytes_read, output_file_f);
			if (bytes_read != offset) printf("Warning: could not write final sector for chunk %d.\n", i / 4);
			fclose(output_file_f);

		}

	}

	fclose(output_file_f);

	// Clear memory
	delete [4096] buffer;
	delete [200] input_pathA;
	delete [200] output_pathA;
	delete [4096] relay_buffer;

	return 0;

}

