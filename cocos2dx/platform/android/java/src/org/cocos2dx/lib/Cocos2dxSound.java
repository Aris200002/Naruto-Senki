/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 ****************************************************************************/

package org.cocos2dx.lib;

import android.content.Context;
import android.media.AudioManager;
import android.media.SoundPool;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class Cocos2dxSound {

    public static final int MAX_SIMULTANEOUS_STREAMS_DEFAULT = 150;
    public static final int MAX_SIMULTANEOUS_STREAMS_I9100 = 20;

    private static final String TAG = "Cocos2dxSound";

    private final Context mContext;

    private SoundPool mSoundPool;

    private float mLeftVolume;
    private float mRightVolume;

    private int simultaneousStreams;

    private static final float SOUND_RATE = 1.0f;
    private static final int SOUND_PRIORITY = 1;
    private static final int SOUND_QUALITY = 0;

    private static final int INVALID_SOUND_ID = -1;
    private static final int INVALID_STREAM_ID = -1;

    private final Object mLock = new Object();

    // Loaded sound IDs
    private final HashMap<String, Integer> mPathSoundIDMap =
            new HashMap<String, Integer>();

    // Sound loading status
    private final HashMap<String, Boolean> mPathLoadedMap =
            new HashMap<String, Boolean>();

    // Active stream IDs
    private final HashMap<String, ArrayList<Integer>> mPathStreamIDsMap =
            new HashMap<String, ArrayList<Integer>>();

    // Sounds waiting to play
    private final HashMap<String, ArrayList<Boolean>> mPendingSounds =
            new HashMap<String, ArrayList<Boolean>>();

    private boolean mReleased = false;

    // ===========================================================
    // Constructor
    // ===========================================================

    public Cocos2dxSound(
            final Context pContext,
            final int simultaneousStreams) {

        this.mContext = pContext.getApplicationContext();

        this.initData(simultaneousStreams);
    }

    // ===========================================================
    // Initialize
    // ===========================================================

    private void initData(final int simultaneousStreams) {

        synchronized (mLock) {

            this.simultaneousStreams = simultaneousStreams;

            this.mSoundPool = new SoundPool(
                    simultaneousStreams,
                    AudioManager.STREAM_MUSIC,
                    SOUND_QUALITY
            );

            this.mSoundPool.setOnLoadCompleteListener(
                    new OnLoadCompletedListener()
            );

            this.mLeftVolume = 0.5f;
            this.mRightVolume = 0.5f;

            this.mReleased = false;
        }
    }

    // ===========================================================
    // PRELOAD EFFECT
    // ===========================================================

    public int preloadEffect(final String pPath) {

        if (pPath == null || pPath.length() == 0) {
            return INVALID_SOUND_ID;
        }

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return INVALID_SOUND_ID;
            }

            Integer soundID = mPathSoundIDMap.get(pPath);

            // Already loaded or loading
            if (soundID != null) {
                return soundID;
            }

            soundID = createSoundIDFromAsset(pPath);

            if (soundID != INVALID_SOUND_ID) {

                mPathSoundIDMap.put(pPath, soundID);

                // Loading is asynchronous
                mPathLoadedMap.put(pPath, false);

            }

            return soundID;
        }
    }

    // ===========================================================
    // PLAY EFFECT
    // ===========================================================

    public int playEffect(
            final String pPath,
            final boolean pLoop) {

        if (pPath == null || pPath.length() == 0) {
            return INVALID_STREAM_ID;
        }

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return INVALID_STREAM_ID;
            }

            Integer soundID = mPathSoundIDMap.get(pPath);

            // Sound has never been loaded
            if (soundID == null) {

                soundID = preloadEffect(pPath);

                if (soundID == INVALID_SOUND_ID) {
                    return INVALID_STREAM_ID;
                }
            }

            Boolean loaded = mPathLoadedMap.get(pPath);

            // Sound is ready
            if (loaded != null && loaded) {

                return doPlayEffect(
                        pPath,
                        soundID,
                        pLoop
                );
            }

            // Sound is still loading.
            // Queue playback without blocking the game thread.

            ArrayList<Boolean> pending = mPendingSounds.get(pPath);

            if (pending == null) {

                pending = new ArrayList<Boolean>();

                mPendingSounds.put(pPath, pending);
            }

            pending.add(pLoop);

            // Return immediately.
            // The audio will play automatically after loading.

            return INVALID_STREAM_ID;
        }
    }

    // ===========================================================
    // PLAY INTERNAL
    // ===========================================================

    private int doPlayEffect(
            final String pPath,
            final int soundID,
            final boolean pLoop) {

        if (mReleased || mSoundPool == null) {
            return INVALID_STREAM_ID;
        }

        int streamID = mSoundPool.play(
                soundID,
                mLeftVolume,
                mRightVolume,
                SOUND_PRIORITY,
                pLoop ? -1 : 0,
                SOUND_RATE
        );

        if (streamID != 0) {

            ArrayList<Integer> streamIDs =
                    mPathStreamIDsMap.get(pPath);

            if (streamIDs == null) {

                streamIDs = new ArrayList<Integer>();

                mPathStreamIDsMap.put(pPath, streamIDs);
            }

            streamIDs.add(streamID);

            return streamID;
        }

        return INVALID_STREAM_ID;
    }

    // ===========================================================
    // STOP EFFECT
    // ===========================================================

    public void stopEffect(final int pStreamID) {

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return;
            }

            if (pStreamID == INVALID_STREAM_ID) {
                return;
            }

            mSoundPool.stop(pStreamID);

            Iterator<Map.Entry<String, ArrayList<Integer>>> iter =
                    mPathStreamIDsMap.entrySet().iterator();

            while (iter.hasNext()) {

                Map.Entry<String, ArrayList<Integer>> entry =
                        iter.next();

                entry.getValue().remove((Integer) pStreamID);

                if (entry.getValue().isEmpty()) {
                    iter.remove();
                }
            }
        }
    }

    // ===========================================================
    // PAUSE / RESUME
    // ===========================================================

    public void pauseEffect(final int pStreamID) {

        synchronized (mLock) {

            if (!mReleased && mSoundPool != null) {
                mSoundPool.pause(pStreamID);
            }
        }
    }

    public void resumeEffect(final int pStreamID) {

        synchronized (mLock) {

            if (!mReleased && mSoundPool != null) {
                mSoundPool.resume(pStreamID);
            }
        }
    }

    public void pauseAllEffects() {

        synchronized (mLock) {

            if (!mReleased && mSoundPool != null) {
                mSoundPool.autoPause();
            }
        }
    }

    public void resumeAllEffects() {

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return;
            }

            for (ArrayList<Integer> streamIDs :
                    mPathStreamIDsMap.values()) {

                for (Integer streamID : streamIDs) {

                    mSoundPool.resume(streamID);
                }
            }
        }
    }

    // ===========================================================
    // STOP ALL EFFECTS
    // ===========================================================

    public void stopAllEffects() {

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return;
            }

            for (ArrayList<Integer> streamIDs :
                    mPathStreamIDsMap.values()) {

                for (Integer streamID : streamIDs) {

                    mSoundPool.stop(streamID);
                }
            }

            mPathStreamIDsMap.clear();

            // Cancel pending playback
            mPendingSounds.clear();
        }
    }

    // ===========================================================
    // UNLOAD EFFECT
    // ===========================================================

    public void unloadEffect(final String pPath) {

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return;
            }

            ArrayList<Integer> streamIDs =
                    mPathStreamIDsMap.remove(pPath);

            if (streamIDs != null) {

                for (Integer streamID : streamIDs) {

                    mSoundPool.stop(streamID);
                }
            }

            // Remove pending requests
            mPendingSounds.remove(pPath);

            // Unload sound
            Integer soundID = mPathSoundIDMap.remove(pPath);

            mPathLoadedMap.remove(pPath);

            if (soundID != null) {

                mSoundPool.unload(soundID);
            }
        }
    }

    // ===========================================================
    // VOLUME
    // ===========================================================

    public float getEffectsVolume() {

        synchronized (mLock) {

            return (mLeftVolume + mRightVolume) / 2;
        }
    }

    public void setEffectsVolume(float pVolume) {

        if (pVolume < 0.0f) {
            pVolume = 0.0f;
        }

        if (pVolume > 1.0f) {
            pVolume = 1.0f;
        }

        synchronized (mLock) {

            mLeftVolume = pVolume;
            mRightVolume = pVolume;

            if (mReleased || mSoundPool == null) {
                return;
            }

            for (ArrayList<Integer> streamIDs :
                    mPathStreamIDsMap.values()) {

                for (Integer streamID : streamIDs) {

                    mSoundPool.setVolume(
                            streamID,
                            mLeftVolume,
                            mRightVolume
                    );
                }
            }
        }
    }

    // ===========================================================
    // LOAD SOUND FROM ASSET
    // ===========================================================

    public int createSoundIDFromAsset(final String pPath) {

        int soundID = INVALID_SOUND_ID;

        try {

            if (mReleased || mSoundPool == null) {
                return INVALID_SOUND_ID;
            }

            if (pPath.startsWith("/")) {

                soundID = mSoundPool.load(pPath, 0);

            } else {

                soundID = mSoundPool.load(
                        mContext.getAssets().openFd(pPath),
                        0
                );
            }

        } catch (Exception e) {

            Log.e(
                    TAG,
                    "Failed loading sound: " + pPath,
                    e
            );

            soundID = INVALID_SOUND_ID;
        }

        if (soundID == 0) {
            soundID = INVALID_SOUND_ID;
        }

        return soundID;
    }

    // ===========================================================
    // LOAD COMPLETE LISTENER
    // ===========================================================

    public class OnLoadCompletedListener
            implements SoundPool.OnLoadCompleteListener {

        @Override
        public void onLoadComplete(
                SoundPool soundPool,
                int sampleId,
                int status) {

            synchronized (mLock) {

                if (mReleased || mSoundPool != soundPool) {
                    return;
                }

                String loadedPath = null;

                // Find corresponding path
                for (Map.Entry<String, Integer> entry :
                        mPathSoundIDMap.entrySet()) {

                    if (entry.getValue() == sampleId) {

                        loadedPath = entry.getKey();

                        break;
                    }
                }

                if (loadedPath == null) {
                    return;
                }

                // Loading failed
                if (status != 0) {

                    Log.e(
                            TAG,
                            "Sound loading failed: " + loadedPath
                    );

                    mPathLoadedMap.remove(loadedPath);

                    mPathSoundIDMap.remove(loadedPath);

                    mPendingSounds.remove(loadedPath);

                    return;
                }

                // Sound is ready
                mPathLoadedMap.put(loadedPath, true);

                Log.d(
                        TAG,
                        "Sound ready: " + loadedPath
                );

                // Play all queued requests
                ArrayList<Boolean> pending =
                        mPendingSounds.remove(loadedPath);

                if (pending != null) {

                    Integer soundID =
                            mPathSoundIDMap.get(loadedPath);

                    if (soundID != null) {

                        for (Boolean loop : pending) {

                            doPlayEffect(
                                    loadedPath,
                                    soundID,
                                    loop
                            );
                        }
                    }
                }
            }
        }
    }

    // ===========================================================
    // RELEASE
    // ===========================================================

    public void end() {

        synchronized (mLock) {

            if (mReleased) {
                return;
            }

            mReleased = true;

            if (mSoundPool != null) {

                mSoundPool.setOnLoadCompleteListener(null);

                mSoundPool.release();

                mSoundPool = null;
            }

            mPathStreamIDsMap.clear();

            mPathSoundIDMap.clear();

            mPathLoadedMap.clear();

            mPendingSounds.clear();

            mLeftVolume = 0.5f;

            mRightVolume = 0.5f;
        }
    }
}