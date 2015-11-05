// ReconstructP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>

FILE *header_file_f, *output_file_f, *input_file_f;
char *input_pathA, *output_pathA, a = 2;
unsigned char *buffer, *nbuffer, *blbuffer;
unsigned long bytes_read, len, p_len;
int offset;

/* Flips an integer from Big Endian to Little Endian or vice versa. Return value is the processed integer.
 * Minecraft files, both PC and XBox, expect integers in Big Endian.

 * _input_int: The integer to flip.
 */
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

/* _tmain takes a directory full of compressed PC-format chunk files and assembles them into a PC-format .mcr file.
 * _tmain expects the "LastUpdated" header block to be present in a file called "header_output.txt".

 * _tmain expects two command line arguments:
 * argv[1]: Path to the directory containing the PC chunk files.
 * argv[2]: Path to the new .mcr file to create.
 */
int _tmain(int argc, _TCHAR* argv[])
{

	buffer = new unsigned char[8192];
	nbuffer = new unsigned char[409600];
	blbuffer = new unsigned char[5];
	output_pathA = new char[200];
	input_pathA = new char[200];

	for (int i = 0; i < 4096; i ++) buffer[i] = 0;

	// Get the paths set up
	sprintf(output_pathA, "%ls", argv[2]);
	sprintf(input_pathA, "%ls", argv[1]);
	p_len = strlen(input_pathA);

	// Open the header file for reading
	header_file_f = fopen("header_output.txt", "r+b");

	// Create the final file; prepare to write the chunks by advancing the pointer to 8 KB.
	output_file_f = fopen(output_pathA, "wb"); 
	fwrite(buffer, sizeof(char), 8192, output_file_f);
	offset = 2;

	// Loop through reading in the compressed files
	for (int i = 0; i < 1024; i ++)
	{
		
		// Get file path
		sprintf(input_pathA + p_len, "\\%d.dat\0", i);

		// Open the file for reading
		input_file_f = fopen(input_pathA, "rb");
		if (input_file_f != NULL)
		{
			
			// Read all the compressed PC data from the file 
			fseek(input_file_f, 0L, SEEK_END);
			len = ftell(input_file_f);
			rewind(input_file_f);
			bytes_read = fread(nbuffer, sizeof(char), len, input_file_f);
			if (len != bytes_read) printf("Warning: read for compressed file %d may have failed.\n", i);

			// This gets the number of 4 KB chunks that this block will span in the final file
			// If the chunk would span an exact multiple of 4096 bytes, by Minecraft convention the next chunk must be left blank.
			bytes_read = (bytes_read + 5) / 4096 + ((bytes_read / 5) % 4096 == 0 ? 0 : 1);

			// Write the size of this chunk to the header file
			*(int *)blbuffer = EndianFlip(offset);
			blbuffer[4] = bytes_read;
			offset += bytes_read;
			fwrite(blbuffer + 1, sizeof(char), 4, header_file_f);

			// Grab the length of the chunk in bytes and write the entire chunk to the file
			bytes_read = EndianFlip(len + 1);
			fwrite(&bytes_read, sizeof(char), 4, output_file_f);
			fwrite(&a, sizeof(char), 1, output_file_f);
			bytes_read = fwrite(nbuffer, sizeof(char), len, output_file_f);
			if (bytes_read != len) printf("Warning: write for compressed file %d may have failed. Conversion may be incomplete.\n", i);

			// Now we need to pad to 4096 bytes by writing out zeroes
			len = 4096 - ((bytes_read + 5) % 4096);
			fwrite(buffer, sizeof(char), len, output_file_f);

			fclose(input_file_f);

		}
		else
		{
			
			// If opening the file failed, we write blank bytes to the header file to indicate the chunk is not present.
			bytes_read = 0;
			fwrite(&bytes_read, sizeof(char), 4, header_file_f);

		}

	}

	// Finally write the contents of the header_file.txt to the beginning of the .mcr file.
	rewind(header_file_f);
	rewind(output_file_f);
	fread(buffer, sizeof(char), 8192, header_file_f);
	fwrite(buffer, sizeof(char), 8192, output_file_f);

	// Close files
	fclose(header_file_f);
	fclose(output_file_f);

	delete [8192] buffer;
	delete [409600] nbuffer;
	delete [5] blbuffer;
	delete [200] output_pathA;
	delete [200] input_pathA;

	return 0;
}