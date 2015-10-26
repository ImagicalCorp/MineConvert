package net.minecraft.world.level.storage;

/**
 * Copyright Mojang AB.
 * 
 * Don't do evil.
 */

public interface ProgressListener {
    public void progressStartNoAbort(String string);

    public void progressStart(String string);

    public void progressStage(String string);

    public void progressStagePercentage(int i);

}
