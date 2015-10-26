package com.mojang.nbt;

/**
 * Copyright Mojang AB.
 * 
 * Don't do evil.
 */

import java.io.*;

public class IntTag extends Tag {
    public int data;

    public IntTag(String name) {
        super(name);
    }

    public IntTag(String name, int data) {
        super(name);
        this.data = data;
    }

    void write(DataOutput dos) throws IOException {
        dos.writeInt(data);
    }

    void load(DataInput dis) throws IOException {
        data = dis.readInt();
    }

    public byte getId() {
        return TAG_Int;
    }

    public String toString() {
        return "" + data;
    }

    @Override
    public Tag copy() {
        return new IntTag(getName(), data);
    }

    @Override
    public boolean equals(Object obj) {
        if (super.equals(obj)) {
            IntTag o = (IntTag) obj;
            return data == o.data;
        }
        return false;
    }

}
