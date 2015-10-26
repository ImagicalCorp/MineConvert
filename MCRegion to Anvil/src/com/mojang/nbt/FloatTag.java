package com.mojang.nbt;

/**
 * Copyright Mojang AB.
 * 
 * Don't do evil.
 */

import java.io.*;

public class FloatTag extends Tag {
    public float data;

    public FloatTag(String name) {
        super(name);
    }

    public FloatTag(String name, float data) {
        super(name);
        this.data = data;
    }

    void write(DataOutput dos) throws IOException {
        dos.writeFloat(data);
    }

    void load(DataInput dis) throws IOException {
        data = dis.readFloat();
    }

    public byte getId() {
        return TAG_Float;
    }

    public String toString() {
        return "" + data;
    }

    @Override
    public Tag copy() {
        return new FloatTag(getName(), data);
    }

    @Override
    public boolean equals(Object obj) {
        if (super.equals(obj)) {
            FloatTag o = (FloatTag) obj;
            return data == o.data;
        }
        return false;
    }

}
