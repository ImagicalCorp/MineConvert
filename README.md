# MineConvert

MineConvert are 5 map converters compiled into one for Minecraft. It is built upon [ljyloo's](http://github.com/ljyloo) [LevelDb2Anvil](http://github.com/ljyloo/LevelDb2Avnil) and [Anvil2LevelDb](https://github.com/ljyloo/LevelDb2Avnil/tree/Avnil2LevelDb) converter. None of these converters are written or maintained by the Imagical Corporation.

## Converters

- Anvil to LevelDB
- Anvil to MCRegion
- LevelDB to Anvil
- MCRegion to Anvil
- Xbox to Anvil

## Warnings - read before you use the programs

- The LevelDB to Anvil and Anvil to LevelDB converters will experience an error if you have blocks which are not supported by Minecraft PC Edition. It will make your .mca files corrupted, so make sure you replace those blocks with another block that is supported.
- The LevelDB to Anvil converter won't convert your level.dat file. The best fix for this is to generate a new flat world in Minecraft PC, and copy that world's level.dat into your converted world's root folder(where the region folder is located). It is known that the converted land and the generated land won't line up together where they meet. 

## How to use the world converters(except Xbox to Anvil)

- Import your world in and rename that folder to ‘world’. 
- Run the start.bat file(if you’re on Windows) or start.sh(if you’re on Unix) to start the converter. 
- The converter will now convert your world to an output folder(e.g. worldanvil), or the same original world folder depending on the converter you are using.

## How to use the Xbox to Anvil world converter

- Double click Converter.exe.
- Click File, then click Open.
- Navigate to your savegame.dat file and open it.
- Press "Begin conversion", then follow the instructions presented to you by the program.
- Open Minecraft and play your converted world.

## Credits

This program uses [leveldb by tinfoiled](https://github.com/ljyloo/leveldb).

This program contains [code from Mojang](https://mojang.com/2012/02/new-minecraft-map-format-anvil/)

The [LevelDB to Anvil](https://github.com/ljyloo/LevelDb2Avnil) and [Anvil to LevelDB](https://github.com/ljyloo/LevelDb2Avnil/tree/Avnil2LevelDb) converter was written by [ljyloo](https://github.com/ljyloo).

The [Anvil to MCRegion](http://www.minecraftforum.net/forums/mapping-and-modding/minecraft-tools/1264128-anvil-to-mcregion-converter-v1-0) converter was written by [Black_Mantha](http://www.minecraftforum.net/members/Black_Mantha). Alternate download available [here](http://www.mediafire.com/download/13g2zdwq6ram09u/Converter.zip).

The [MCRegion to Anvil](https://mojang.com/2012/02/new-minecraft-map-format-anvil/) converter was written by Jens Bergensten at Mojang.

The [Xbox to Anvil](http://sourceforge.net/projects/xboxtopcminecraftconverter/) converter was writtem by [manfromscene24](http://sourceforge.net/u/manfromscene24/profile/).
