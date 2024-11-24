package com.example.ndkpractice

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.media.MediaPlayer
import android.net.Uri
import android.os.Bundle
import android.provider.MediaStore
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.example.ndkpractice.core.FFPlayer
import com.example.ndkpractice.databinding.ActivityMainBinding

private const val TAG = "MainPlayerTag"

class MainActivity : AppCompatActivity() {

    private var player: FFPlayer? = null


    private val binding by lazy {
        ActivityMainBinding.inflate(layoutInflater)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(binding.root)

        Log.i(TAG, "onCreate: ")

        supportActionBar?.apply {
            setDisplayShowCustomEnabled(true)
            setDisplayShowTitleEnabled(false)
            setCustomView(R.layout.layout_action_bar)

            this.customView.findViewById<View>(R.id.btn1).setOnClickListener {
                val intent = Intent(Intent.ACTION_PICK)
                intent.type = "video/*"
                startActivityForResult(intent, 1024)
            }

            this.customView.findViewById<View>(R.id.btn2).setOnClickListener {
                player?.apply {
                    releaseSurface()
                    stop()
                    release()
                }
            }
        }
        //onFetchUrl("/storage/emulated/0/Pictures/WeiXin/wx_camera_1731658601272.mp4")

        if(checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE), 1025)
        }
        //FFPlayer()
    }

    private fun onFetchUrl(url: String) {
        player?.apply {
            releaseSurface()
            stop()
            release()
        }
        FFPlayer().apply {
            //
            //setUrl("http://demo-videos.qnsdk.com/only-video-1080p-60fps.m4s")
            //setUrl("rtmp://liteavapp.qcloud.com/live/liteavdemoplayerstreamid")
            Log.i(TAG, "onFetchUrl: $url")
            setUrl(url)
            prepare()
            start()
            setSurfaceView(binding.sv)
            player = this
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        val uri = data?.data ?: return
        val path = getVideoPathFromUri(uri)
        Log.i(TAG, "onActivityResult: path $path.")
        path ?: return
        onFetchUrl(path)
    }

    fun getVideoPathFromUri(uri: Uri): String? {
        val projection = arrayOf(MediaStore.Video.Media.DATA)
        val cursor = contentResolver.query(uri, projection, null, null, null)
        cursor?.let {
            if (it.moveToFirst()) {
                val columnIndex = it.getColumnIndexOrThrow(MediaStore.Video.Media.DATA)
                return it.getString(columnIndex)
            }
        }
        return null
    }

    override fun onResume() {
        super.onResume()
        Log.i(TAG, "onResume: ${binding.sv.holder.surface.isValid}")
        //player.setDisplaySurface(binding.sv.holder.surface)
    }

    override fun onPause() {
        super.onPause()
        Log.i(TAG, "onPause: ")
    }

    override fun onDestroy() {
        super.onDestroy()
        player?.releaseSurface()
        player?.release()
    }

    init {
        val mediaPlayer = MediaPlayer()
        mediaPlayer.start()
    }

}