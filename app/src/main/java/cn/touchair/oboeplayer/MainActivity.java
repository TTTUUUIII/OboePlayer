package cn.touchair.oboeplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.io.File;
import java.io.FileNotFoundException;
import java.nio.file.Path;
import java.nio.file.Paths;

import cn.touchair.oboeplayer.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private static final String TAG = MainActivity.class.getSimpleName();

    private static final Path SCAN_SOURCE_PATH = Paths.get(String.format("/sdcard/Android/media/%s/Music", MainActivity.class.getPackage().getName()));

    private static final int REQ_CODE_R_W_EXTERNAL_STORAGE = 1;
    private OboePlayer mPlayer;
    private PlayListViewAdapter mAdapter;
    private ActivityMainBinding binding;

    private boolean mIsPermissionGranted = false;
    private String mSelectedSourceName = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        mPlayer = new OboePlayer(OboePlayer.SAMPLE_RATE_44100Hz, OboePlayer.FORMAT_PCM_I16, OboePlayer.CHANNEL_COUNT_STEREO);
        mPlayer.setOnCompletedListener(player -> {
//            mPlayer.reset();
//            try {
//                mPlayer.setAudioPath("/sdcard/Android/media/cn.touchair.oboeplayer/Music/Running in the Night_short.pcm");
//            } catch (FileNotFoundException e) {
//                throw new RuntimeException(e);
//            }
//            mPlayer.prepare();
//            mPlayer.start();
        });
        mAdapter = new PlayListViewAdapter();
        requestPermission();
        binding.playListRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        binding.playListRecyclerView.setAdapter(mAdapter);
        mAdapter.submit(findPlayList());
        binding.scanPathTextView.setText(SCAN_SOURCE_PATH.toString());
        binding.pauseBtn.setOnClickListener(this);
        binding.startBtn.setOnClickListener(this);
        binding.restartBtn.setOnClickListener(this);
        binding.resetBtn.setOnClickListener(this);
        updateUI();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mPlayer.isPlaying()) {
            mPlayer.stop();
            binding.stateTextView.setText("STOPPED");
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mPlayer.release();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.pause_btn:
                mPlayer.pause();
                mCurrentStatus = "PAUSED";
                updateUI();
                break;
            case R.id.start_btn:
                mPlayer.start();
                mCurrentStatus = "STARTED";
                updateUI();
                break;
            case R.id.restart_btn:
                mPlayer.seekTo(0);
                break;
            case R.id.reset_btn:
                mPlayer.reset();
                mCurrentStatus = "UNINITIALIZED";
                mIsShowControlPanel = false;
                mIsSelectedMode = true;
                mSelectedSourceName = "";
                updateUI();
                break;
        }
    }

    private boolean mIsShowControlPanel = false;

    private boolean mIsLoopMode = false;

    private boolean mIsSelectedMode = true;

    private String mCurrentStatus = "UNINITIALIZED";

    public void updateUI() {
        binding.startBtn.setEnabled(mIsShowControlPanel);
        binding.pauseBtn.setEnabled(mIsShowControlPanel);
        binding.restartBtn.setEnabled(mIsShowControlPanel);
        binding.resetBtn.setEnabled(mIsShowControlPanel);
        binding.stateTextView.setText(mCurrentStatus);
        binding.isLoopModeTextView.setText(mPlayer.isLooped() ? "YES" : "NO");
        binding.isSelectMode.setText(mIsSelectedMode ? "YES" : "NO");
        mAdapter.notifyDataSetChanged();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case REQ_CODE_R_W_EXTERNAL_STORAGE:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
                    mIsPermissionGranted = true;
                break;
        }
    }

    private void requestPermission() {
        if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, REQ_CODE_R_W_EXTERNAL_STORAGE);
        } else mIsPermissionGranted = true;
    }

    private String[] findPlayList() {
        String[] playList = new String[0];
        final File sourceDir = SCAN_SOURCE_PATH.toFile();
        if (sourceDir.exists()) {
            playList = sourceDir.list(((dir, name) -> name.endsWith(".pcm")));
        }
        return playList;
    }

    private class PlayListViewHolder extends RecyclerView.ViewHolder {

        public PlayListViewHolder(@NonNull View itemView) {
            super(itemView);
        }

        public void bind(String itemData) {
            final TextView textView = (TextView) (TextView) itemView;
            textView.setText(itemData);
            textView.setEnabled(mIsSelectedMode);
            textView.setTextColor(mIsSelectedMode ? Color.GREEN : Color.GRAY);
            if (itemData.equals(mSelectedSourceName)) textView.setTextColor(Color.RED);
            itemView.setOnClickListener(v -> {
                if (mIsPermissionGranted) {
                    mIsSelectedMode = false;
                    mIsShowControlPanel = true;
                    mSelectedSourceName = itemData;
                    try {
                        mPlayer.setAudioPath(SCAN_SOURCE_PATH.resolve(itemData).toString());
                        mPlayer.prepare();
                    } catch (FileNotFoundException e) {
                        throw new RuntimeException(e);
                    }
                    updateUI();
                } else {
                    Toast.makeText(getApplicationContext(), "没有足够的权限打开文件！", Toast.LENGTH_SHORT).show();
                }
            });
        }
    }

    private class PlayListViewAdapter extends RecyclerView.Adapter<PlayListViewHolder> {
        private String[] data = new String[0];

        @NonNull
        @Override
        public PlayListViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            final LayoutInflater inflater = LayoutInflater.from(parent.getContext());
            final View itemView = inflater.inflate(R.layout.item_player_source, parent, false);
            return new PlayListViewHolder(itemView);
        }

        @Override
        public void onBindViewHolder(@NonNull PlayListViewHolder holder, int position) {
            holder.bind(data[position]);
        }

        @Override
        public int getItemCount() {
            return data.length;
        }

        public void submit(String[] data) {
            this.data = data;
            notifyDataSetChanged();
        }
    }
}