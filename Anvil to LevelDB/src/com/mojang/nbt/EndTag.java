package com.mojang.nbt;

/**
 * Copyright Mojang AB.
 * 
 * Don't do evil.
 */

import java.io.*;

public class EndTag extends Tag {

    public EndTag() {
        super(null);
    }

    void load(DataInput dis) throws IOException {
    }

    void write(DataOutput dos) throws IOException {
    }

    public byte getId() {
        return TAG_End;
    }

    public String toString() {
        return "END";
    }

    @Override
    public Tag copy() {
        return new EndTag();
    }

    @Override
    public boolean equals(Object obj) {
        return super.equals(obj);
    }

}
