package com.ljyloo.PE2PC;

import org.iq80.leveldb.*;

import com.mojang.nbt.CompoundTag;
import com.mojang.nbt.ListTag;
import com.mojang.nbt.NbtIo;

import net.minecraft.world.level.chunk.DataLayer;
import net.minecraft.world.level.chunk.OldDataLayer;
import net.minecraft.world.level.chunk.storage.*;
import static org.iq80.leveldb.impl.Iq80DBFactory.*;

import java.io.*;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

public class PE2PC {
	private final static int DATALAYER_BITS = 7;
	private final static int BLOCKDATA_BYTES = 32768;
	private final static int METADATA_BYTES = 16384;
	private final static int SKYLIGHTDATA_BYTES = 16384;
	private final static int BLOCKLIGHTDATA_BYTES = 16384;
	
	public static void main(String[] args) throws IOException {
		//System.out.println((-1 % 32 + 32) % 32);
        if (args.length != 2) {
            printUsageAndExit();
        }
        
        File srcFolder;
        try {
            srcFolder = new File(args[0]+"/db");
            if (!srcFolder.exists()) {
                throw new RuntimeException(args[0] + " doesn't exist");
            } else if (!srcFolder.isDirectory()) {
                throw new RuntimeException(args[0] + " is not a folder");
            }
        } catch (Exception e) {
            System.err.println("import folder problem: " + e.getMessage());
            System.out.println("");
            printUsageAndExit();
            return;
        }
        
        File desFolder;
        try {
        	desFolder = new File(args[1]);
            if (!desFolder.exists()) {
                throw new RuntimeException(args[1] + " doesn't exist");
            } else if (!desFolder.isDirectory()) {
                throw new RuntimeException(args[1] + " is not a folder");
            }
        } catch (Exception e) {
            System.err.println("export folder problem: " + e.getMessage());
            System.out.println("");
            printUsageAndExit();
            return;
        }
        
		convert(srcFolder, desFolder);
	}

    private static void printUsageAndExit() {
        System.out.println("Map converter for Minecraft:Pocket Edition, from format \"LevelDB\" to \"Anvil\". (c) ljyloo 2015");
        System.out.println("");
        System.out.println("Usage:");
        System.out.println("\tjava -jar Converter.jar <import folder> <export folder>");
        System.out.println("Where:");
        System.out.println("\t<import folder>\tThe full path to the folder containing Minecraft:Pocket Edition world");
        System.out.println("\t<export folder>\tThe full path to the folder which you want to export");
        System.out.println("Example:");
        System.out.println("\tjava -jar Converter.jar /home/ljyloo/import /home/ljyloo/export");
        System.exit(1);
    }
	
	public static void convert(File src, File des) throws IOException{
		DB db = null;
		try{
			Options options = new Options();
			options.createIfMissing(true);
			db = factory.open(src, options);
			
			DBIterator iterator = db.iterator();
			//ArrayList<byte[]> keys = new ArrayList<byte[]>();
			HashMap<String, RegionFile> regions = new HashMap<String, RegionFile>();
			try{
				for(iterator.seekToFirst(); iterator.hasNext(); iterator.next()){
					byte[] key = iterator.peekNext().getKey();
					if(key.length == 9 && key[8] == 0x30){
						byte[] value = iterator.peekNext().getValue();
						int chunkX = byteArrayToInt(new byte[]{key[3], key[2], key[1], key[0]});
						int chunkZ = byteArrayToInt(new byte[]{key[7], key[6], key[5], key[4]});
						System.out.println("Converting chunk X:"+chunkX+" Z:"+chunkZ);
						CompoundTag tag = new CompoundTag();
						CompoundTag levelData = new CompoundTag();
						tag.put("Level", levelData);
						
						levelData.putByte("LightPopulated", (byte)1);
						levelData.putByte("TerrainPopulated", (byte)1);
						levelData.putByte("V", (byte)1);
						levelData.putInt("xPos", chunkX);
						levelData.putInt("zPos", chunkZ);
						levelData.putLong("InhabitedTime", 0);
						levelData.putLong("LastUpdate", 0);
						byte[] biomes = new byte[16 * 16];
						for(int i = 0; i <256; i++)
							biomes[i] = -1;
						levelData.putByteArray("Biomes", biomes);
						levelData.put("Entities", new ListTag<CompoundTag>("Entities"));
						
						ListTag<CompoundTag> sectionTags = new ListTag<CompoundTag>("Sections");
						
						LevelDBChunk data = new LevelDBChunk(chunkX, chunkZ);
						
						data.blocks = new byte[BLOCKDATA_BYTES];
						System.arraycopy(value, 0, data.blocks, 0, BLOCKDATA_BYTES);
						
						byte[] metadata = new byte[METADATA_BYTES];
						System.arraycopy(value, BLOCKDATA_BYTES, metadata, 0, METADATA_BYTES);
						data.data = new OldDataLayer(metadata, DATALAYER_BITS);
						
						byte[] skyLightData = new byte[SKYLIGHTDATA_BYTES];
						System.arraycopy(value, BLOCKDATA_BYTES + METADATA_BYTES, skyLightData, 0, SKYLIGHTDATA_BYTES);
						data.skyLight = new OldDataLayer(skyLightData, DATALAYER_BITS);
						
						byte[] blockLightData = new byte[BLOCKLIGHTDATA_BYTES];
						System.arraycopy(value, BLOCKDATA_BYTES + METADATA_BYTES + SKYLIGHTDATA_BYTES, blockLightData, 0, BLOCKLIGHTDATA_BYTES);
						data.blockLight = new OldDataLayer(blockLightData, DATALAYER_BITS);
						
						for (int yBase = 0; yBase < (128 / 16); yBase++) {

				            // find non-air
				            boolean allAir = true;
				            for (int x = 0; x < 16 && allAir; x++) {
				                for (int y = 0; y < 16 && allAir; y++) {
				                    for (int z = 0; z < 16; z++) {
				                        int pos = (x << 11) | (z << 7) | (y + (yBase << 4));
				                        int block = data.blocks[pos];
				                        if (block != 0) {
				                            allAir = false;
				                            break;
				                        }
				                    }
				                }
				            }

				            if (allAir) {
				                continue;
				            }

				            // build section
				            byte[] blocks = new byte[16 * 16 * 16];
				            DataLayer dataValues = new DataLayer(blocks.length, 4);
				            DataLayer skyLight = new DataLayer(blocks.length, 4);
				            DataLayer blockLight = new DataLayer(blocks.length, 4);

				            for (int x = 0; x < 16; x++) {
				                for (int y = 0; y < 16; y++) {
				                    for (int z = 0; z < 16; z++) {
				                        int pos = (x << 11) | (z << 7) | (y + (yBase << 4));
				                        int block = data.blocks[pos];

				                        blocks[(y << 8) | (z << 4) | x] = (byte) (block & 0xff);
				                        dataValues.set(x, y, z, data.data.get(x, y + (yBase << 4), z));
				                        skyLight.set(x, y, z, data.skyLight.get(x, y + (yBase << 4), z));
				                        blockLight.set(x, y, z, data.blockLight.get(x, y + (yBase << 4), z));
				                    }
				                }
				            }

				            CompoundTag sectionTag = new CompoundTag();

				            sectionTag.putByte("Y", (byte) (yBase & 0xff));
				            sectionTag.putByteArray("Blocks", blocks);
				            sectionTag.putByteArray("Data", dataValues.data);
				            sectionTag.putByteArray("SkyLight", skyLight.data);
				            sectionTag.putByteArray("BlockLight", blockLight.data);

				            sectionTags.add(sectionTag);
				        }
						levelData.put("Sections", sectionTags);
				        
				        levelData.put("TileEntities", new ListTag<CompoundTag>("TileEntities"));
				        int[] heightMap = new int[256];
				        for(int x = 0; x < 16; x++){
				        	for(int z = 0; z < 16; z++){
				        		for(int y = 127; y >= 0; y--){
				        			int pos = (x << 11) | (z << 7) | y;
				        			int block = data.blocks[pos];
				        			if(block != 0){
				        				heightMap[(x << 4) | z] = y;
				        				break;
				        			}
				        		}
				        	}
				        }
				        levelData.putIntArray("HeightMap", heightMap);
						
						String k = (chunkX >> 5) + "." + (chunkZ >> 5);
						if(!regions.containsKey(k)){
							regions.put(k, new RegionFile(new File(des, "r." + (chunkX >> 5) + "." + (chunkZ >> 5) + ".mca")));
						}
						RegionFile regionDest = regions.get(k);
						int regionX = (chunkX % 32 + 32) % 32;
						int regionZ = (chunkZ % 32 + 32) % 32;
						/*if(chunkX < 0 || chunkZ < 0){
							@SuppressWarnings("unused")
							int i = 1+1;
						}*/
						DataOutputStream chunkDataOutputStream = regionDest.getChunkDataOutputStream(regionX, regionZ);
						if(chunkDataOutputStream == null){
							System.out.println(chunkX % 32);
							System.out.println(chunkZ % 32);
						}
						NbtIo.write(tag, chunkDataOutputStream);
						chunkDataOutputStream.close();
					}
				}
			}
			finally{
				Iterator<Entry<String, RegionFile>> iter = regions.entrySet().iterator();
				while (iter.hasNext()){
					Entry<String, RegionFile> entry = iter.next();
					RegionFile region = entry.getValue();
					region.close();
				}
				iterator.close();
			}
		}
		finally{
			db.close();
		}
		System.out.println("Done!");
	}
	
	public static String byte2s(byte[] b, boolean ignoreTooLong){
		String s = "0x";
		int length = b.length;
		boolean tooLong = false;
		if(length > 100){
			length = 100;
			tooLong = true;
		}
		for(int i = 0; i < length; i++){
			s = s + b[i] + " ";
		}
		if(tooLong && ignoreTooLong)
			s = s + "...";
		return s;
	}
	
	public static byte[] intToByteArray(int i){
		byte[] result = new byte[4];
		result[0] = (byte)((i >> 24) & 0xFF);
		result[1] = (byte)((i >> 16) & 0xFF);
		result[2] = (byte)((i >> 8) & 0xFF);
		result[3] = (byte)(i & 0xFF);
		return result;
	}
	
	public static int byteArrayToInt(byte[] bytes){
		int value= 0;
		for (int i = 0; i < 4; i++){
			int shift = (4 - 1 - i) * 8;
			value += (bytes[i] & 0x000000FF) << shift;
		}
		return value;
	}
	
	public static class LevelDBChunk{
		public OldDataLayer blockLight;
		public OldDataLayer skyLight;
		public OldDataLayer data;
		public byte[] blocks;
		
		public final int x;
		public final int z;
		
		public LevelDBChunk(int x, int z){
			this.x = x;
			this.z = z;
		}
	}
}
