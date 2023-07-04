package cn.touchair.oboeplayer;

import android.os.Handler;
import android.os.Looper;

import androidx.annotation.IntDef;
import androidx.annotation.NonNull;
import androidx.annotation.UiThread;

import java.io.File;
import java.io.FileNotFoundException;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Objects;

public class OboePlayer {

    static {
        System.loadLibrary("oboesystem");
    }

    private static final int DEFAULT_PRIMARY_DEVICE = 0;
    public static final int SAMPLE_RATE_44100Hz = 44100;
    public static final int SAMPLE_RATE_48000Hz = 48000;
    public static final int CHANNEL_COUNT_MONO = 1;
    public static final int CHANNEL_COUNT_STEREO = 2;
    public static final int FORMAT_PCM_FLOAT = 0;
    public static final int FORMAT_PCM_I16 = 2;
    public static final int FORMAT_PCM_I24 = 3;
    public static final int FORMAT_PCM_I32 = 4;

    private OnCompletedListener mCompletedListener;
    private OnErrorListener mErrorListener;

    private Handler mH = new Handler(Looper.myLooper());

    public OboePlayer(int deviceId, @SampleRate int sampleRate, @ChannelCount int channelCount, @Format int fmt) {
        int state = native_newPlayer(deviceId, sampleRate, channelCount, fmt);
        if (state < 0) throw new RuntimeException("Failed to create native player! " + state);
    }

    public OboePlayer(int sampleRate, int channelCount, int fmt) {
        this(DEFAULT_PRIMARY_DEVICE, sampleRate, channelCount, fmt);
    }

    public int setAudioPath(@NonNull String path) throws FileNotFoundException {
        File file = new File(path);
        if (!file.exists() || !file.isFile()) throw new FileNotFoundException();
        native_setAudioPath(path);
        return 0;
    }

    public void prepare() {
        if (native_prepare() < 0) throw new RuntimeException("Failed to prepare native player.");
    }

    public void start() {
        native_start();
    }

    private void onCompleted() {
        if (Objects.nonNull(mCompletedListener)) mH.post(() -> mCompletedListener.onCompleted(this));
    }

    private void onError(int errorCode) {
        if (Objects.nonNull(mErrorListener)) mH.post(() -> mErrorListener.onError(this, errorCode));
    }

    public void stop() {
        native_stop();
    }

    public void pause() {
        native_pause();
    }

    public void release() {
        native_release();
    }

    public void reset() {
        native_reset();
    }

    public void seekTo(float rela) {
        native_seek_to(rela);
    }

    public boolean isPlaying() {
        return native_isPlaying();
    }

    public boolean isLooped() {
        return native_isLooped();
    }

    public void setLoop(boolean isLoop) {
        native_setLoop(isLoop);
    }

    public void setOnCompletedListener(OnCompletedListener listener) {
        mCompletedListener = listener;
    }

    public void setOnErrorListener(OnErrorListener listener) {
        mErrorListener = listener;
    }

    private native int native_newPlayer(int deviceId, int sampleRate, int channelCount, int fmt);
    private native void native_setAudioPath(String path);

    private native int native_prepare();

    private native void native_setLoop(boolean isLoop);

    private native void native_start();

    private native void native_pause();

    private native void native_stop();

    private native void native_release();

    private native boolean native_isPlaying();

    private native boolean native_isLooped();

    private native void native_reset();

    private native void native_seek_to(float rela);


    @IntDef({
            SAMPLE_RATE_44100Hz,
            SAMPLE_RATE_48000Hz
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface SampleRate{}

    @IntDef({
            FORMAT_PCM_FLOAT,
            FORMAT_PCM_I16,
            FORMAT_PCM_I24,
            FORMAT_PCM_I32
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface Format{}

    @IntDef({
            CHANNEL_COUNT_MONO,
            CHANNEL_COUNT_STEREO
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface ChannelCount{}

    public interface OnCompletedListener {
        void onCompleted(OboePlayer player);
    }

    public interface OnErrorListener {
        void onError(OboePlayer player, int errorCode);
    }
}
