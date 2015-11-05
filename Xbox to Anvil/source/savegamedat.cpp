// savegamedat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>

FILE *sgame_dat, *xbox_sgame;
char *sgame_path, *NBT_path;
const char *s_s = ".\0";
unsigned char *lbuffer, *xbuffer, *obuffer;
int template_length, xbox_length;
short string_length;
int inventory_length, inventory_offset;

// This structure represents an entry in the level.dat file.
struct entry
{
public:
	const char *path;
	int length;
	int offset;
} player_entries[28], savegame_entries[8];	

// Flips an integer from little to big endian or vice versa.
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

// Flips a short from little to big endian or vice versa.
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

/* This is a slight variation of the method in ExpandX.cpp. 
 * Here we not only check the tag to make sure it is not broken, but check its name to see if we want to copy it to the PC file.
 * If the name matches a known name, we copy it over.
 
 * _out_buffer: buffer pointing to the tag to process.
 * _size: length of the buffer to process.
 * _tag_type: integer specifying the tag type.
 * _path: current path to the tag.
 * _search: array of entries to search for known values.
 * _entry_length: number of entries in the array.
 */
int ProcessTag(unsigned char *_out_buffer, int _size, unsigned char _tag_type, char *_path, entry *_search, int _entry_length)
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
			_result = ProcessTag(_out_buffer + _n, _size, _tag_type, _path, _search, _entry_length);
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

			// Grab the name length, now 
			_name_length = EndianFlip(*((short *)(_out_buffer + _n + 1)));
			memcpy(_path, _out_buffer + _n + 3, _name_length);
			memcpy(_path + _name_length, s_s, 2);
			_n += _name_length + 3;

			// Look through the structs to see if there are any matches
			for (_i = 0; _i < _entry_length; _i ++)
			{
				if (strcmp(_search[_i].path, NBT_path) == 0)
				{
					memcpy(lbuffer + _search[_i].offset, _out_buffer + _n, _search[_i].length);
				}
			}

			// If this is the inventory tag, store the length for later.
			if (strcmp("Inventory.", NBT_path) == 0)
			{
				_result = ProcessTag(_out_buffer + _n, _size - _n, _tag_type, _path + _name_length + 1, _search, _entry_length);
				inventory_length = _result;
				inventory_offset = _n + 3;
			}
			else
				_result = ProcessTag(_out_buffer + _n, _size - _n, _tag_type, _path + _name_length + 1, _search, _entry_length);

			// Process the sub tag.
			_result = ProcessTag(_out_buffer + _n, _size - _n, _tag_type, _path + _name_length + 1, _search, _entry_length);

			// If processing failed, break now.
			// Otherwise, add the result length to the pointer and go again.
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

int _tmain(int argc, _TCHAR* argv[])
{
	
	// Allocate memory
	sgame_path = new char[300];
	lbuffer = new unsigned char[496000];
	xbuffer = new unsigned char[496000];
	obuffer = new unsigned char[496000];
	NBT_path = new char[1000];

	// Emptry entry values to begin
	for (int i = 0; i < 28; i ++)
		player_entries[i] = entry();

	for (int i = 0; i < 8; i ++)
		savegame_entries[i] = entry();

	// Populate the entries
	// Player position
	player_entries[0].path = "Pos.";
	player_entries[0].length = 0x1D;
	player_entries[0].offset = 0x2C7;

	// Player HurtTime
	player_entries[1].path = "HurtTime.";
	player_entries[1].length = 2;
	player_entries[1].offset = 0x294;

	// Player OnGround
	player_entries[2].path = "OnGround.";
	player_entries[2].length = 1;
	player_entries[2].offset = 0x288;

	// Player motion
	player_entries[3].path = "Motion.";
	player_entries[3].length = 0x1D;
	player_entries[3].offset = 0x147;

	// Player food exhaustion level
	player_entries[4].path = "foodExhaustionLevel.";
	player_entries[4].length = 4;
	player_entries[4].offset = 0x17A;

	// Player current dimension location
	player_entries[5].path = "Dimension.";
	player_entries[5].length = 4;
	player_entries[5].offset = 0x2B5;

	// Player current dimension location
	player_entries[6].path = "Health.";
	player_entries[6].length = 2;
	player_entries[6].offset = 0x1A9;

	// Player Air
	player_entries[7].path = "Air.";
	player_entries[7].length = 2;
	player_entries[7].offset = 0x2BF;

	// Player camera rotation
	player_entries[8].path = "Rotation.";
	player_entries[8].length = 0xD;
	player_entries[8].offset = 0x3A9;

	// Player current fall distance
	player_entries[9].path = "FallDistance.";
	player_entries[9].length = 4;
	player_entries[9].offset = 0x38C;

	// Player current time on fire
	player_entries[10].path = "Fire.";
	player_entries[10].length = 2;
	player_entries[10].offset = 0x1EA;

	// Player current time since death
	player_entries[11].path = "DeathTime.";
	player_entries[11].length = 2;
	player_entries[11].offset = 0x239;

	// Player spawn location Z coordinate
	player_entries[12].path = "SpawnZ.";
	player_entries[12].length = 4;
	player_entries[12].offset = 0x48A;

	// Player current time since last attack
	player_entries[13].path = "AttackTime.";
	player_entries[13].length = 2;
	player_entries[13].offset = 0x1D5;

	// Player sleeping
	player_entries[14].path = "Sleeping.";
	player_entries[14].length = 1;
	player_entries[14].offset = 0x1E2;

	// Player SleepTimer
	player_entries[15].path = "SleepTimer.";
	player_entries[15].length = 2;
	player_entries[15].offset = 0x27B;

	// Player XpP
	player_entries[16].path = "XpP.";
	player_entries[16].length = 4;
	player_entries[16].offset = 0x253;

	// Player Experience Level
	player_entries[17].path = "XpLevel.";
	player_entries[17].length = 4;
	player_entries[17].offset = 0x19C;

	// Player Experience Level
	player_entries[18].path = "XpTotal.";
	player_entries[18].length = 4;
	player_entries[18].offset = 0x39A;

	// Player Spawn location X coordinate
	player_entries[19].path = "SpawnX.";
	player_entries[19].length = 4;
	player_entries[19].offset = 0x44D;

	// Player Spawn location Y coordinate
	player_entries[20].path = "SpawnY.";
	player_entries[20].length = 4;
	player_entries[20].offset = 0x466;

	// Player foodLevel
	player_entries[21].path = "foodLevel.";
	player_entries[21].length = 4;
	player_entries[21].offset = 0x20D;

	// Player foodTickTimer
	player_entries[22].path = "foodTickTimer.";
	player_entries[22].length = 4;
	player_entries[22].offset = 0x18E;
	
	// Player foodSaturationLevel
	player_entries[23].path = "foodSaturationLevel.";
	player_entries[23].length = 4;
	player_entries[23].offset = 0x2FA;

	// Player currently flying
	player_entries[24].path = "abilities.flying.";
	player_entries[24].length = 1;
	player_entries[24].offset = 0x328;

	// Player able to build instantly
	player_entries[25].path = "abilities.instabuild.";
	player_entries[25].length = 1;
	player_entries[25].offset = 0x336;

	// Player invulnerable
	player_entries[26].path = "abilities.invulnerable.";
	player_entries[26].length = 1;
	player_entries[26].offset = 0x350;

	// Player able to fly
	player_entries[27].path = "abilities.mayfly.";
	player_entries[27].length = 1;
	player_entries[27].offset = 0x340;
	
	// Savegame is raining
	savegame_entries[0].path = "Data.raining.";
	savegame_entries[0].length = 1;
	savegame_entries[0].offset = 0x431;

	// Savegame is thundering
	savegame_entries[1].path = "Data.thundering.";
	savegame_entries[1].length = 1;
	savegame_entries[1].offset = 0x17;

	// Savegame duration of rainstorm
	savegame_entries[2].path = "Data.rainTime.";
	savegame_entries[2].length = 4;
	savegame_entries[2].offset = 0x5B2;

	// Savegame duration of rainstorm
	savegame_entries[3].path = "Data.thunderTime.";
	savegame_entries[3].length = 4;
	savegame_entries[3].offset = 0x440;

	// Savegame version
	// Note: the typo is intentional. We do not currently want to copy this value.
	savegame_entries[4].path = "Data.versoon.";
	savegame_entries[4].length = 4;
	savegame_entries[4].offset = 0x403;

	// Savegame timestamp LastPlayed
	savegame_entries[5].path = "Data.LastPlayed.";
	savegame_entries[5].length = 8;
	savegame_entries[5].offset = 0x25;

	// Savegame seed
	savegame_entries[6].path = "Data.RandomSeed.";
	savegame_entries[6].length = 8;
	savegame_entries[6].offset = 0x3D3;

	// Savegame time
	savegame_entries[7].path = "Data.Time.";
	savegame_entries[7].length = 8;
	savegame_entries[7].offset = 0x41F;

	// Open the output file
	sprintf(sgame_path, "%ls", argv[1]);
	sgame_dat = fopen(sgame_path, "r+b");

	// Open the XBox savegame.dat
	sprintf(sgame_path, "%ls", argv[2]);
	xbox_sgame = fopen(sgame_path, "r+b");

	// Load the template into memory
	fseek(sgame_dat, 0, SEEK_END);
	template_length = ftell(sgame_dat);
	rewind(sgame_dat);
	fread(lbuffer, 1, template_length, sgame_dat);

	// Load the XBox file into memory
	fseek(xbox_sgame, 0, SEEK_END);
	xbox_length = ftell(xbox_sgame);
	rewind(xbox_sgame);
	fread(xbuffer, 1, xbox_length, xbox_sgame);

	// This method browses through the NBT tags in the XBox file
	// Any that match the template file are written out there
	ProcessTag(xbuffer + 3, xbox_length, 10, NBT_path, savegame_entries, 8);

	// Now do the same for the player file
	// Open the XBox savegame.dat
	fclose(xbox_sgame);
	sprintf(sgame_path, "%ls", argv[3]);
	xbox_sgame = fopen(sgame_path, "r+b");

	// Load the XBox file into memory
	fseek(xbox_sgame, 0, SEEK_END);
	xbox_length = ftell(xbox_sgame);
	rewind(xbox_sgame);
	fread(xbuffer, 1, xbox_length, xbox_sgame);

	// This method browses through the NBT tags in the XBox file
	// Any that match the template file are written out there
	ProcessTag(xbuffer + 3, xbox_length, 10, NBT_path, player_entries, 28);

	// Generate world name
	sprintf(NBT_path + 2, "%ls\0", argv[4]);
	*((short *)(NBT_path)) = EndianFlip((short)strlen(NBT_path + 2));

	// Now begin assembling the final file
	// The inventory is professed differently here.
	memcpy(obuffer, lbuffer, 0x1C3);
	memcpy(obuffer + 0x1C3, xbuffer + inventory_offset, inventory_length);
	memcpy(obuffer + 0x1C3 + inventory_length, lbuffer + 0x1C8, 0x2AE);
	memcpy(obuffer + 0x1C3 + inventory_length + 0x2AE, NBT_path, strlen(NBT_path + 2) + 2);
	memcpy(obuffer + 0x1C3 + inventory_length + 0x2AE + strlen(NBT_path + 2) + 2, lbuffer + 0x481, 0x14E);

	// Now discard the level.dat file and repoen
	fclose(xbox_sgame);
	sprintf(sgame_path, "%ls", argv[2]);
	xbox_sgame = fopen(sgame_path, "w+b");

	// Go to the beginning of the file and write
	rewind(xbox_sgame);
	fwrite(obuffer, 1, 0x1C3 + inventory_length + 0x2AE + strlen(NBT_path + 2) + 2 + 0x14E, xbox_sgame);

	// Close files
	fclose(sgame_dat);
	fclose(xbox_sgame);

	// Free memory
	delete [300] sgame_path;
	delete [496000] lbuffer;
	delete [496000] xbuffer;
	delete [496000] obuffer;
	delete [1000] NBT_path;
	
	return 0;

}

