package net.minecraft.world.level.chunk;

public class DataLayer {
    public final byte[] data;
    private final int depthBits;
    private final int depthBitsPlusFour;

    public DataLayer(int length, int depthBits) {
        data = new byte[length >> 1];
        this.depthBits = depthBits;
        depthBitsPlusFour = depthBits + 4;
    }

    public DataLayer(byte[] data, int depthBits) {
        this.data = data;
        this.depthBits = depthBits;
        depthBitsPlusFour = depthBits + 4;
    }

    public int get(int x, int y, int z) {
        int pos = (y << depthBitsPlusFour | z << depthBits | x);
        int slot = pos >> 1;
        int part = pos & 1;

        if (part == 0) {
            return data[slot] & 0xf;
        } else {
            return (data[slot] >> 4) & 0xf;
        }
    }

    public void set(int x, int y, int z, int val) {
        int pos = (y << depthBitsPlusFour | z << depthBits | x);

        int slot = pos >> 1;
        int part = pos & 1;

        if (part == 0) {
            data[slot] = (byte) ((data[slot] & 0xf0) | (val & 0xf));
        } else {
            data[slot] = (byte) ((data[slot] & 0x0f) | ((val & 0xf) << 4));
        }
    }

    public boolean isValid() {
        return data != null;
    }

    public void setAll(int br) {
        byte val = (byte) ((br | (br << 4)) & 0xff);
        for (int i = 0; i < data.length; i++) {
            data[i] = val;
        }
    }
}
