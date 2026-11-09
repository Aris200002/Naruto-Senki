/****************************************************************************
Copyright (c) 2010-2011 cocos2d-x.org
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
import java.util.Map.Entry;

public class Cocos2dxSound {

    public static final int MAX_SIMULTANEOUS_STREAMS_DEFAULT = 150;
    public static final int MAX_SIMULTANEOUS_STREAMS_I9100 = 20;

    private static final String TAG = "Cocos2dxSound";

    private static final float SOUND_RATE = 1.0f;
    private static final int SOUND_PRIORITY = 1;
    private static final int SOUND_QUALITY = 0;

    private static final int INVALID_SOUND_ID = -1;
    private static final int INVALID_STREAM_ID = -1;

    private final Context mContext;

    private SoundPool mSoundPool;

    private float mLeftVolume;
    private float mRightVolume;

    private final Object mLock = new Object();

    private final HashMap<String, ArrayList<Integer>>
            mPathStreamIDsMap = new HashMap<String, ArrayList<Integer>>();

    private final HashMap<String, Integer>
            mPathSoundIDMap = new HashMap<String, Integer>();

    /*
     * Status pemuatan setiap audio.
     * false = masih dimuat
     * true  = sudah siap diputar
     */
    private final HashMap<String, Boolean>
            mPathLoadedMap = new HashMap<String, Boolean>();

    /*
     * Antrean audio yang dipanggil sebelum proses loading selesai.
     * Boolean menyimpan status loop.
     */
    private final HashMap<String, ArrayList<Boolean>>
            mPendingSounds = new HashMap<String, ArrayList<Boolean>>();

    private int simultaneousStreams;

    private boolean mReleased = false;

    // ===========================================================
    // Constructor
    // ===========================================================

    public Cocos2dxSound(
            final Context pContext,
            final int simultaneousStreams) {

        this.mContext = pContext.getApplicationContext();

        this.simultaneousStreams = simultaneousStreams;

        initData(simultaneousStreams);
    }

    // ===========================================================
    // Initialize SoundPool
    // ===========================================================

    private void initData(final int streams) {

        synchronized (mLock) {

            mSoundPool = new SoundPool(
                    streams,
                    AudioManager.STREAM_MUSIC,
                    SOUND_QUALITY
            );

            final SoundPool currentPool = mSoundPool;

            currentPool.setOnLoadCompleteListener(
                    new OnLoadCompletedListener(currentPool)
            );

            mLeftVolume = 0.5f;
            mRightVolume = 0.5f;

            simultaneousStreams = streams;

            mReleased = false;
        }
    }

    // ===========================================================
    // Preload Effect
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

            if (soundID != null) {
                return soundID.intValue();
            }

            int newSoundID = createSoundIDFromAssetInternal(pPath);

            if (newSoundID == INVALID_SOUND_ID) {
                return INVALID_SOUND_ID;
            }

            mPathSoundIDMap.put(pPath, newSoundID);

            /*
             * SoundPool.load() bekerja secara asynchronous.
             * Status akan berubah menjadi true melalui callback.
             */
            mPathLoadedMap.put(pPath, Boolean.FALSE);

            return newSoundID;
        }
    }

    // ===========================================================
    // Play Effect
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

            /*
             * Audio belum pernah dimuat.
             */
            if (soundID == null) {

                int newSoundID = createSoundIDFromAssetInternal(pPath);

                if (newSoundID == INVALID_SOUND_ID) {
                    return INVALID_STREAM_ID;
                }

                soundID = Integer.valueOf(newSoundID);

                mPathSoundIDMap.put(pPath, soundID);

                mPathLoadedMap.put(pPath, Boolean.FALSE);
            }

            Boolean loaded = mPathLoadedMap.get(pPath);

            /*
             * Audio sudah selesai dimuat.
             */
            if (Boolean.TRUE.equals(loaded)) {

                return doPlayEffect(
                        pPath,
                        soundID.intValue(),
                        pLoop
                );
            }

            /*
             * Audio belum selesai dimuat.
             * Simpan permintaan pemutaran untuk callback.
             */
            ArrayList<Boolean> pending = mPendingSounds.get(pPath);

            if (pending == null) {

                pending = new ArrayList<Boolean>();

                mPendingSounds.put(pPath, pending);
            }

            pending.add(Boolean.valueOf(pLoop));

            /*
             * Pemutaran akan dilakukan otomatis
             * ketika OnLoadCompletedListener dipanggil.
             */
            return INVALID_STREAM_ID;
        }
    }

    // ===========================================================
    // Actual Sound Playback
    // ===========================================================

    private int doPlayEffect(
            final String pPath,
            final int soundId,
            final boolean pLoop) {

        if (mReleased || mSoundPool == null) {
            return INVALID_STREAM_ID;
        }

        int streamID = mSoundPool.play(
                soundId,
                mLeftVolume,
                mRightVolume,
                SOUND_PRIORITY,
                pLoop ? -1 : 0,
                SOUND_RATE
        );

        if (streamID == INVALID_STREAM_ID) {
            return INVALID_STREAM_ID;
        }

        ArrayList<Integer> streamIDs =
                mPathStreamIDsMap.get(pPath);

        if (streamIDs == null) {

            streamIDs = new ArrayList<Integer>();

            mPathStreamIDsMap.put(pPath, streamIDs);
        }

        streamIDs.add(Integer.valueOf(streamID));

        return streamID;
    }

    // ===========================================================
    // Load Completed Listener
    // ===========================================================

    private class OnLoadCompletedListener
            implements SoundPool.OnLoadCompleteListener {

        private final SoundPool mOwnerPool;

        OnLoadCompletedListener(SoundPool pool) {
            mOwnerPool = pool;
        }

        @Override
        public void onLoadComplete(
                SoundPool soundPool,
                int sampleId,
                int status) {

            synchronized (mLock) {

                /*
                 * Abaikan callback dari SoundPool lama.
                 */
                if (mReleased ||
                        mSoundPool == null ||
                        soundPool != mSoundPool ||
                        soundPool != mOwnerPool) {

                    return;
                }

                String loadedPath = null;

                /*
                 * Cari path berdasarkan sample ID.
                 */
                for (Entry<String, Integer> entry :
                        mPathSoundIDMap.entrySet()) {

                    if (entry.getValue().intValue() == sampleId) {

                        loadedPath = entry.getKey();

                        break;
                    }
                }

                if (loadedPath == null) {
                    return;
                }

                /*
                 * Loading gagal.
                 */
                if (status != 0) {

                    Log.e(
                            TAG,
                            "Failed loading sound: " + loadedPath
                    );

                    mPathLoadedMap.remove(loadedPath);

                    mPathSoundIDMap.remove(loadedPath);

                    mPendingSounds.remove(loadedPath);

                    return;
                }

                /*
                 * Tandai audio sudah siap.
                 */
                mPathLoadedMap.put(
                        loadedPath,
                        Boolean.TRUE
                );

                /*
                 * Ambil semua permintaan pemutaran
                 * yang menunggu audio ini.
                 */
                ArrayList<Boolean> pending =
                        mPendingSounds.remove(loadedPath);

                if (pending == null || pending.isEmpty()) {
                    return;
                }

                Integer soundID =
                        mPathSoundIDMap.get(loadedPath);

                if (soundID == null) {
                    return;
                }

                /*
                 * Putar seluruh permintaan secara berurutan.
                 */
                for (Boolean loop : pending) {

                    if (mReleased ||
                            mSoundPool != soundPool) {

                        break;
                    }

                    doPlayEffect(
                            loadedPath,
                            soundID.intValue(),
                            loop != null && loop.booleanValue()
                    );
                }
            }
        }
    }

    // ===========================================================
    // Create Sound ID
    // ===========================================================

    public int createSoundIDFromAsset(final String pPath) {

        if (pPath == null || pPath.length() == 0) {
            return INVALID_SOUND_ID;
        }

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return INVALID_SOUND_ID;
            }

            return createSoundIDFromAssetInternal(pPath);
        }
    }

    private int createSoundIDFromAssetInternal(
            final String pPath) {

        int soundID = INVALID_SOUND_ID;

        try {

            if (pPath.startsWith("/")) {

                soundID = mSoundPool.load(pPath, 0);

            } else {

                soundID = mSoundPool.load(
                        mContext.getAssets().openFd(pPath),
                        0
                );
            }

        } catch (final Exception e) {

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
    // Unload Effect
    // ===========================================================

    public void unloadEffect(final String pPath) {

        if (pPath == null) {
            return;
        }

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return;
            }

            ArrayList<Integer> streamIDs =
                    mPathStreamIDsMap.remove(pPath);

            if (streamIDs != null) {

                for (Integer streamID : streamIDs) {

                    if (streamID != null) {

                        mSoundPool.stop(
                                streamID.intValue()
                        );
                    }
                }
            }

            /*
             * Batalkan antrean hanya untuk audio ini.
             */
            mPendingSounds.remove(pPath);

            mPathLoadedMap.remove(pPath);

            Integer soundID =
                    mPathSoundIDMap.remove(pPath);

            if (soundID != null) {

                mSoundPool.unload(
                        soundID.intValue()
                );
            }
        }
    }

    // ===========================================================
    // Stop Effect
    // ===========================================================

    public void stopEffect(final int pStreamID) {

        if (pStreamID == INVALID_STREAM_ID) {
            return;
        }

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return;
            }

            mSoundPool.stop(pStreamID);

            Iterator<Entry<String, ArrayList<Integer>>> iter =
                    mPathStreamIDsMap.entrySet().iterator();

            while (iter.hasNext()) {

                Entry<String, ArrayList<Integer>> entry =
                        iter.next();

                ArrayList<Integer> streams = entry.getValue();

                streams.remove(Integer.valueOf(pStreamID));

                if (streams.isEmpty()) {
                    iter.remove();
                }
            }
        }
    }

    // ===========================================================
    // Pause / Resume
    // ===========================================================

    public void pauseEffect(final int pStreamID) {

        synchronized (mLock) {

            if (!mReleased && mSoundPool != null &&
                    pStreamID != INVALID_STREAM_ID) {

                mSoundPool.pause(pStreamID);
            }
        }
    }

    public void resumeEffect(final int pStreamID) {

        synchronized (mLock) {

            if (!mReleased && mSoundPool != null &&
                    pStreamID != INVALID_STREAM_ID) {

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

            for (ArrayList<Integer> streams :
                    mPathStreamIDsMap.values()) {

                for (Integer streamID : streams) {

                    if (streamID != null) {

                        mSoundPool.resume(
                                streamID.intValue()
                        );
                    }
                }
            }
        }
    }

    // ===========================================================
    // Stop All Effects
    // ===========================================================

    public void stopAllEffects() {

        synchronized (mLock) {

            if (mReleased || mSoundPool == null) {
                return;
            }

            for (ArrayList<Integer> streams :
                    mPathStreamIDsMap.values()) {

                for (Integer streamID : streams) {

                    if (streamID != null) {

                        mSoundPool.stop(
                                streamID.intValue()
                        );
                    }
                }
            }

            mPathStreamIDsMap.clear();

            /*
             * Batalkan permintaan yang belum sempat diputar.
             * Cache audio tetap dipertahankan.
             */
            mPendingSounds.clear();
        }
    }

    // ===========================================================
    // Volume
    // ===========================================================

    public float getEffectsVolume() {

        synchronized (mLock) {

            return (mLeftVolume + mRightVolume) / 2.0f;
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

            for (ArrayList<Integer> streams :
                    mPathStreamIDsMap.values()) {

                for (Integer streamID : streams) {

                    if (streamID != null) {

                        mSoundPool.setVolume(
                                streamID.intValue(),
                                mLeftVolume,
                                mRightVolume
                        );
                    }
                }
            }
        }
    }

    // ===========================================================
    // End / Reset
    // ===========================================================

    public void end() {

        synchronized (mLock) {

            SoundPool oldPool = mSoundPool;

            if (oldPool != null) {

                oldPool.setOnLoadCompleteListener(null);

                oldPool.release();
            }

            mReleased = true;

            mSoundPool = null;

            mPathStreamIDsMap.clear();

            mPathSoundIDMap.clear();

            mPathLoadedMap.clear();

            mPendingSounds.clear();

            mLeftVolume = 0.5f;
            mRightVolume = 0.5f;

            /*
             * Buat SoundPool baru agar objek dapat digunakan lagi.
             */
            initData(simultaneousStreams);
        }
    }
}