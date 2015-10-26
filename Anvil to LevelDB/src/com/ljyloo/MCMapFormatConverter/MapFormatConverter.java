package com.ljyloo.MCMapFormatConverter;

import org.apache.commons.io.filefilter.SuffixFileFilter;
import org.iq80.leveldb.*;
import org.omg.CORBA.SystemException;

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

public class MapFormatConverter {
	private final static int DATALAYER_BITS = 7;
	private final static int BLOCKS_PER_CHUNK = 16 * 16 * 128;
	private final static int BLOCKDATA_BYTES = 32768;
	private final static int METADATA_BYTES = 16384;
	private final static int SKYLIGHTDATA_BYTES = 16384;
	private final static int BLOCKLIGHTDATA_BYTES = 16384;
	private final static int TERRAINDATA_BYTES = 83200;
	
	private static byte[] PC_Blocks_Filter = new byte[256];
	
	//note: pe 0.11.0
	private static byte[] PE_Blocks_Filter = new byte[256];
	
	public static void main(String[] args) throws IOException {
		initFilter();
		
		if(args.length == 1){
			debugOutput(args[0]);
		}
		//System.out.println((-1 % 32 + 32) % 32);
        if (args.length < 2 || args.length > 3) {
            printUsageAndExit();
        }
        
        int type = 0;
        if(args.length == 3){
        	try{
        		type = Integer.parseInt(args[2]);
        		if(type != 0 && type != 1){
        			System.err.println(type + " is not a valid type value");
        			System.out.println("\n");
        			printUsageAndExit();
        		}
        	}
        	catch(NumberFormatException e){
        		System.err.println(args[2] + " is not a valid type value");
        		System.out.println("\n");
        		printUsageAndExit();
        	}
        }
        
        //check import folder
        try {
            checkFolder(args[0], type);
        } catch (Exception e) {
        	System.err.println("Failed to check folder \"" + args[0] + "\" 's validation. Problem: " + e.getMessage());
        	System.out.println("\n");
            printUsageAndExit();
        }
        
        //check export folder
        File desFolder = null;
        try {
        	desFolder = new File(args[1]);
            if (!desFolder.exists()) {
                throw new RuntimeException(args[1] + " doesn't exist");
            } else if (!desFolder.isDirectory()) {
                throw new RuntimeException(args[1] + " is not a folder");
            }
        } catch (Exception e) {
            System.err.println("export folder problem: " + e.getMessage());
            System.out.println("\n");
            printUsageAndExit();
        }
        
        if(type == 0){
        	levelDB2Anvil(new File(args[0]), desFolder);
        }
        else{
        	anvilToLevelDB(new File(args[0]), desFolder);
        }
	}
	
	public static void levelDB2Anvil(File src, File des) throws IOException{
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
						
						//int regionX = (chunkX >> 5);
						//int regionZ = (chunkZ >> 5);
						
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
	
	public static void anvilToLevelDB(File src, File des) throws IOException{
		long startTime = System.currentTimeMillis();
		
		byte[] nullSkyLight = new byte[16384];
		for(int i = 0; i < 256; i++){
			for(int j = 0; j < 50; j++)
				nullSkyLight[i*64+j] = 0;
			for(int j = 50; j < 64; j++)
				nullSkyLight[i*64+j] = -1;
		}
		
		byte[] nullBlockLight = new byte[16384];
		for(int i = 0; i < 16384; i++){
			nullBlockLight[i] = -1;
		}
		
		byte[] dirtyInfo = new byte[256];
		for(int j = 0; j < 256; j++)
			dirtyInfo[j] = 100;
		
		byte[] grassColorInfo = new byte[1024];
		for(int j = 0; j < 256; j++){
			//3 -126 -82 -128
			grassColorInfo[j*4] = 3;
			grassColorInfo[j*4+1] = -126;
			grassColorInfo[j*4+2] = -82;
			grassColorInfo[j*4+3] = -128;
		}
		
		String[] mcaFiles = src.list(new SuffixFileFilter(".mca"));
		DB db = null;
		WriteBatch batch = null;
		//PrintWriter printWriter = null;
		HashMap<String, RegionFile> regions = new HashMap<String, RegionFile>();
		int totalAnvilFile = mcaFiles.length, totalSection = 0, totalChunk = 0;
		try{
			Options options = new Options();
			options.createIfMissing(true);
			db = factory.open(des, options);
			
			//printWriter = new PrintWriter(new File(des, "rawOutput.txt"));
			System.out.println("Converting...It may takes a long time, be patient");
			int mcaFileNum = 1;
			for(String mcaFileName : mcaFiles){
				String[] tokens = mcaFileName.split("[.]+");
				//System.out.println(mcaFileName);
				//System.out.println(Integer.parseInt(tokens[1]));
				//System.out.println(Integer.parseInt(tokens[2]));
				//for(int i = 0; i < tokens.length; i++)
				//	System.out.println(tokens[i]);
				RegionFile regionFile = new RegionFile(new File(src, mcaFileName));
				regions.put(mcaFileName, regionFile);
				int regionX = Integer.parseInt(tokens[1]);
				int regionZ = Integer.parseInt(tokens[2]);
				
				System.out.println("Converting(" + mcaFileNum + "/" + totalAnvilFile + "): " + mcaFileName);
				mcaFileNum++;
				
				for(int x = 0; x < 32; x++){
					for(int z = 0; z < 32; z++){
						DataInputStream chunkDataInputStream = regionFile.getChunkDataInputStream(x, z);
						if(chunkDataInputStream != null){
							totalChunk++;
							
							int chunkX = regionX * 32 + x;
							int chunkZ = regionZ * 32 + z;
							
							//System.out.println("Converting Chunk: X " + chunkX + " Z " + chunkZ);
							
							LevelDBChunk entryData = new LevelDBChunk(chunkX, chunkZ);
							entryData.blocks = new byte[BLOCKS_PER_CHUNK];
							entryData.data = new OldDataLayer(BLOCKS_PER_CHUNK, DATALAYER_BITS);
							//entryData.skyLight = new OldDataLayer(BLOCKS_PER_CHUNK, DATALAYER_BITS);
							//entryData.blockLight = new OldDataLayer(BLOCKS_PER_CHUNK, DATALAYER_BITS);
							
							CompoundTag tag = NbtIo.read(chunkDataInputStream);
							CompoundTag levelData = (CompoundTag) tag.get("Level");
							@SuppressWarnings("unchecked")
							ListTag<CompoundTag> sectionsTag = (ListTag<CompoundTag>)levelData.getList("Sections");
							int sectionsNum = sectionsTag.size();
							totalSection += sectionsNum;
							
							for(int i = 0; i < sectionsNum; i++){
								CompoundTag section = sectionsTag.get(i);
								int yBase = (int)section.getByte("Y");
								
								//ignore blocks above 128 height
								if(yBase >= 8)
									continue;
								
								byte[] blocks = section.getByteArray("Blocks");
								DataLayer dataValue = new DataLayer(section.getByteArray("Data"), 4);
								//DataLayer skyLight = new DataLayer(section.getByteArray("SkyLight"), 4);
								//DataLayer blockLight = new DataLayer(section.getByteArray("BlockLight"), 4);
								for(int y1 = 0; y1 < 16; y1++){
									for(int z1 = 0; z1 < 16; z1++){
										for(int x1 = 0; x1 < 16; x1++){
											int offset = y1 * 16 * 16 + z1 * 16 + x1;
											int pos = x1 * 16 * 128 + z1 * 128 + (y1 + yBase * 16);
											byte blockId = blocks[offset];
											byte metaId = (byte)dataValue.get(x1, y1, z1);
											
											//Filter the blocks don't exist in PE, replace it with update block(Id:248)
											
											//try{
												int fileterPos = blockId + (((blockId >> 7) & 0x1) * 256);
												if(PE_Blocks_Filter[fileterPos] == -1 || metaId > PE_Blocks_Filter[fileterPos]){
													blockId = (byte)248;
													metaId = 0;
												}
											//}
											//catch(RuntimeException e){
											//	throw e;
											//}
											
											entryData.blocks[pos] = blockId;
											entryData.data.set(x1, y1, z1, metaId);
											//entryData.skyLight.set(x1, 127 - y1, z1, skyLight.get(x1, y1, z1));
											//entryData.blockLight.set(x1, y1, z1, blockLight.get(x1, y1, z1));
										}
									}
								}
							}
							chunkDataInputStream.close();
							
							byte[] value = new byte[TERRAINDATA_BYTES];
							int offset = 0;
							System.arraycopy(entryData.blocks, 0, value, offset, entryData.blocks.length);
							offset += entryData.blocks.length;
							System.arraycopy(entryData.data.data, 0, value, offset, entryData.data.data.length);
							offset += entryData.data.data.length;
							//System.arraycopy(entryData.skyLight.data, 0, value, offset, entryData.skyLight.data.length);
							System.arraycopy(nullSkyLight, 0, value, offset, 16384);
							offset += 16384;
							//System.arraycopy(entryData.blockLight.data, 0, value, offset, entryData.blockLight.data.length);
							System.arraycopy(nullBlockLight, 0, value, offset, 16384);
							offset += 16384;
							System.arraycopy(dirtyInfo, 0, value, offset, 256);
							offset += 256;
							System.arraycopy(grassColorInfo, 0, value, offset, 1024);
							
							
							byte[] key1 = new byte[]{(byte)(chunkX & 0xff), (byte)((chunkX >> 8) & 0xff), (byte)((chunkX >> 16) & 0xff), (byte)((chunkX >> 24) & 0xff), (byte)((chunkZ) & 0xff), (byte)((chunkZ >> 8) & 0xff), (byte)((chunkZ >> 16) & 0xff), (byte)((chunkZ >> 24) & 0xff), (byte)0x30};
							byte[] key2 = new byte[]{(byte)(chunkX & 0xff), (byte)((chunkX >> 8) & 0xff), (byte)((chunkX >> 16) & 0xff), (byte)((chunkX >> 24) & 0xff), (byte)((chunkZ) & 0xff), (byte)((chunkZ >> 8) & 0xff), (byte)((chunkZ >> 16) & 0xff), (byte)((chunkZ >> 24) & 0xff), (byte)0x76};
							try{
								batch = db.createWriteBatch();
								batch.put(key1, value);
								//key[8] = (byte)0x76;
								batch.put(key2, new byte[]{(byte)0x2});
								db.write(batch);
								batch.close();
								//db.put(key, value);
								/*
								printWriter.println("key: " + byte2s(key, false));
								String ascii = "";
								for(byte character : key){
									if(character >= 33 && character <= 126){
										ascii = ascii + Character.toString((char)character) + " ";
									}
									else{
										ascii = ascii + "? ";
									}
								}
								printWriter.println("ASCII: " + ascii);
								printWriter.println("value: " + byte2s(value, true));
								printWriter.println("length: " + value.length);
								printWriter.println("");
								*/
								//0x76 (118 in decimal, 'v' ASCII) for 1-byte data
								//key[8] = (byte)0x76;
								//db.put(key, new byte[]{(byte)0x2});
							}
							catch(DBException e){
								e.printStackTrace();
							}
							finally{
								batch.close();
							}
						}
					}
				}
			}
			//db.write(batch);
			System.out.println("Done!(" + ((System.currentTimeMillis() - startTime) / 1000) + "s) Total: " + totalAnvilFile + " mca files, " + totalChunk + " chunks, " + totalSection + " sections, " + (totalSection * 128) + " blocks");
		}
		catch(SystemException e){
			e.printStackTrace();
			System.exit(1);
		}
		finally{
			//printWriter.close();
			
			batch.close();
			db.close();
			
			Iterator<Entry<String, RegionFile>> iter = regions.entrySet().iterator();
			while (iter.hasNext()){
				Entry<String, RegionFile> entry = iter.next();
				RegionFile region = entry.getValue();
				region.close();
			}
		}
	}
	
	private static void debugOutput(String path) throws IOException{
		DB db = null;
		PrintWriter printWriter = null;
		try{
			System.out.println("Debug output...");
			File levelDB = new File(path);
			File output = new File(levelDB, "debugOutput.txt");
			printWriter = new PrintWriter(output);
			db = factory.open(levelDB, new Options());
			DBIterator iterator = db.iterator();
			for(iterator.seekToFirst(); iterator.hasNext(); iterator.next()){
				byte[] key = iterator.peekNext().getKey();
				String ascii = "";
				for(byte character : key){
					if(character >= 33 && character <= 126){
						ascii = ascii + Character.toString((char)character) + " ";
					}
					else{
						ascii = ascii + "? ";
					}
				}
				byte[] value = iterator.peekNext().getValue();
				
				printWriter.println("key: " + byte2s(key, false, 128));
				printWriter.println("ASCII: " + ascii);
				
				//printWriter.println("value: " + byte2s(value, false));
				
				//System.out.println("key: " + byte2s(key, false) + "\n");
				//System.out.println("value: " + byte2s(value, true) + "\n");
				if(key[8] == 0x30){
					byte[] blocks = new byte[32768];
					byte[] metaData = new byte[16384];
					byte[] skyLight = new byte[16384];
					byte[] blockLight = new byte[16384];
					byte[] dirtyInfo = new byte[256];
					byte[] grassColor = new byte[1024];
					System.arraycopy(value, 0, blocks, 0, 32768);
					System.arraycopy(value, 32768, metaData, 0, 16384);
					System.arraycopy(value, 49152, skyLight, 0, 16384);
					System.arraycopy(value, 65536, blockLight, 0, 16384);
					System.arraycopy(value, 81920, dirtyInfo, 0, 256);
					System.arraycopy(value, 82176, grassColor, 0, 1024);
					printWriter.println("blocks: " + byte2s(blocks, false, 128));
					printWriter.println("metaData: " + byte2s(metaData, false, 64));
					printWriter.println("skyLight: " + byte2s(skyLight, false, 64));
					printWriter.println("blockLight: " + byte2s(blockLight, false, 64));
					printWriter.println("dirtyInfo: " + byte2s(dirtyInfo, false, 16));
					printWriter.println("grassColor: " + byte2s(grassColor, false, 64));
					printWriter.println("length: " + value.length);
					break;
				}
				
				printWriter.println("length: " + value.length);
				printWriter.println("");
			}
			System.out.println("Done!");
		}
		finally{
			db.close();
			printWriter.close();
			System.exit(0);
		}
	}
	
	//type: 
	//0 LevelDB
	//1 Anvil
	private static void checkFolder(String path, int type) throws IOException {
		File folder = null;
		try{
			folder = new File(path);
			if(!folder.exists()){
				throw new RuntimeException(path + " doesn't exist");
			} else if (!folder.isDirectory()){
				throw new RuntimeException(path + " is not a folder");
			} else {
				boolean hasData = false;
				if(type == 0){
					DB db = null;
					try{
						Options options = new Options();
						options.createIfMissing(false);
						db = factory.open(folder, options);
						
						DBIterator iterator = db.iterator();
						for(iterator.seekToFirst(); iterator.hasNext(); iterator.next()){
							byte[] key = iterator.peekNext().getKey();
							if(key.length == 9 && key[8] == 0x30){
								hasData = true;
								break;
							}
						}
						if(!hasData){
							throw new RuntimeException(path + " is not a valid LevelDB folder");
						}
					}
					finally{
						db.close();
					}
				}
				else{
					String[] mcaFiles = folder.list(new SuffixFileFilter(".mca"));
					if(mcaFiles.length < 1){
						throw new RuntimeException(path + " is not a valid Anvil folder");
					}
				}
			}
		} catch (Exception e) {
			throw e;
		}
		
	}
	
	private static void initFilter(){
		//The number represents the maximum value that the meta value can be, besides, -1 means this block isn't exist
		for(int i = 1; i < 256; i++){
			PE_Blocks_Filter[i] = -1;
		}
		
		String PE_EXIST_BLOCK_ID = "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20.21.22.24.27.30.31.32.35.37.38.39.40.41.42.43.44.45.46.47.48.49.50.51.52.53.54.56.57.58.60.61.62.65.66.67.73.78.79.80.81.82.85.86.87.89.91.95.96.97.98.99.100.101.102.103.106.107.108.109.110.111.112.114.120.121.127.128.129.133.134.135.136.139.141.142.152.155.156.157.158.159.161.162.163.164.170.171.172.173.174.175.183.184.185.186.187.198.243.244.245.246.247.248.249.255";
		String[] PE_EXIST_BLOCK_IDS = PE_EXIST_BLOCK_ID.split("[.]+");
		for(String id : PE_EXIST_BLOCK_IDS){
			PE_Blocks_Filter[Integer.parseInt(id)] = 0;
		}
		
		//special id
		PE_Blocks_Filter[5] = 5;
		PE_Blocks_Filter[6] = 5;
		PE_Blocks_Filter[12] = 1;
		PE_Blocks_Filter[17] = 3;
		PE_Blocks_Filter[18] = 3;
		PE_Blocks_Filter[24] = 2;
		PE_Blocks_Filter[31] = 3;
		PE_Blocks_Filter[35] = 15;
		PE_Blocks_Filter[38] = 8;
		PE_Blocks_Filter[43] = 9;
		PE_Blocks_Filter[44] = 7;
		PE_Blocks_Filter[85] = 5;
		PE_Blocks_Filter[97] = 2;
		PE_Blocks_Filter[98] = 3;
		PE_Blocks_Filter[139] = 1;
		PE_Blocks_Filter[155] = 4;
		PE_Blocks_Filter[157] = 5;
		PE_Blocks_Filter[158] = 5;
		PE_Blocks_Filter[159] = 15;
		PE_Blocks_Filter[161] = 1;
		PE_Blocks_Filter[162] = 1;
		PE_Blocks_Filter[171] = 15;
		PE_Blocks_Filter[175] = 5;
		PE_Blocks_Filter[247] = 2;
	}
	
    private static void printUsageAndExit() {
        System.out.println("Map converter for Minecraft:Pocket Edition, from format \"LevelDB\" to \"Anvil\", or from format \"Anvil\" to \"LevelDB\". (c) ljyloo 2015");
        System.out.println("");
        System.out.println("Usage:");
        System.out.println("\tjava -jar Converter.jar <import folder> <export folder> <type>");
        System.out.println("Where:");
        System.out.println("\t<import folder>\tThe full path to the folder containing Minecraft:Pocket Edition world");
        System.out.println("\t<export folder>\tThe full path to the folder which you want to export");
        System.out.println("\t<type>\t0 represents \"LevelDB\" to \"Anvil\", 1 represents \"Anvil\" to \"LevelDB\"");
        System.out.println("Example:");
        System.out.println("\tjava -jar Converter.jar /home/ljyloo/LevelDB /home/ljyloo/Anvil 0");
        System.exit(1);
    }
	
	private static String byte2s(byte[] b, boolean ignoreTooLong, int columnNum){
		String s = "\n";
		int length = b.length;
		boolean tooLong = false;
		if(length > 100){
			if(ignoreTooLong)
				length = 100;
			tooLong = true;
		}
		int calculateColumn = 0;
		for(int i = 0; i < length; i++){
			s = s + b[i] + " ";
			calculateColumn++;
			if (calculateColumn >= columnNum) {
				calculateColumn = 0;
				s += "\n";
			}
		}
		if(tooLong && ignoreTooLong)
			s = s + "...";
		return s;
	}
	
	@SuppressWarnings("unused")
	private static byte[] intToByteArray(int i){
		byte[] result = new byte[4];
		result[0] = (byte)((i >> 24) & 0xFF);
		result[1] = (byte)((i >> 16) & 0xFF);
		result[2] = (byte)((i >> 8) & 0xFF);
		result[3] = (byte)(i & 0xFF);
		return result;
	}
	
	private static int byteArrayToInt(byte[] bytes){
		int value= 0;
		for (int i = 0; i < 4; i++){
			int shift = (4 - 1 - i) * 8;
			value += (bytes[i] & 0x000000FF) << shift;
		}
		return value;
	}
	
	@SuppressWarnings("unused")
	private static class AnvilChunk{
		public DataLayer blockLight;
		public DataLayer skyLight;
		public DataLayer data;
		public byte[] blocks;
		
		public final int x;
		public final int z;
		
		public AnvilChunk(int x, int z){
			this.x = x;
			this.z = z;
		}
	}
	
	private static class LevelDBChunk{
		public OldDataLayer blockLight;
		public OldDataLayer skyLight;
		public OldDataLayer data;
		public byte[] blocks;
		
		@SuppressWarnings("unused")
		public final int x;
		@SuppressWarnings("unused")
		public final int z;
		
		public LevelDBChunk(int x, int z){
			this.x = x;
			this.z = z;
		}
	}
}
