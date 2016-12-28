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
- The LevelDB to Anvil and the Anvil to LevelDB converter won't convert your level.dat file. The best fix for this is to generate a new flat world in Minecraft PC(LDB to Anvil) or Minecraft PE(Anvil to LDB), and copy that world's level.dat into your converted world's root folder(where the region folder is located). It is known that the converted land and the generated land won't line up together where they meet. There is a level.dat file provided for each of the two converters by us.

## Credits

This program uses [leveldb by tinfoiled](https://github.com/ljyloo/leveldb).

This program contains [code from Mojang](https://mojang.com/2012/02/new-minecraft-map-format-anvil/)

The [LevelDB to Anvil](https://github.com/ljyloo/LevelDb2Avnil) and [Anvil to LevelDB](https://github.com/ljyloo/LevelDb2Avnil/tree/Avnil2LevelDb) converter was written by [ljyloo](https://github.com/ljyloo).

The [Anvil to MCRegion](http://www.minecraftforum.net/forums/mapping-and-modding/minecraft-tools/1264128-anvil-to-mcregion-converter-v1-0) converter was written by [Black_Mantha](http://www.minecraftforum.net/members/Black_Mantha). Alternate download available [here](http://www.mediafire.com/download/13g2zdwq6ram09u/Converter.zip).

The [MCRegion to Anvil](https://mojang.com/2012/02/new-minecraft-map-format-anvil/) converter was written by Jens Bergensten at Mojang.

The [Xbox to Anvil](http://sourceforge.net/projects/xboxtopcminecraftconverter/) converter was writtem by [manfromscene24](http://sourceforge.net/u/manfromscene24/profile/).
