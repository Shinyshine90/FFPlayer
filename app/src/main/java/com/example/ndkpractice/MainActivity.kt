package com.example.ndkpractice

import android.os.Bundle
import android.util.Log
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.example.ndkpractice.core.FFPlayer
import com.example.ndkpractice.databinding.ActivityMainBinding

private const val TAG = "MainPlayerTag"

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        findViewById<SurfaceView>(R.id.sv).holder.addCallback(object: SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                FFPlayer.setDisplaySurface(holder.surface)
            }

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) {

            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                FFPlayer.removeDisplaySurface(holder.surface)
            }
        })

        findViewById<View>(R.id.btn_start).setOnClickListener {
            FFPlayer.startPlay("/sdcard/Android/data/com.example.ndkpractice/files/player/curry.mp4")
        }

        findViewById<View>(R.id.btn_pause).setOnClickListener {
            FFPlayer.togglePlay()
        }

        findViewById<SeekBar>(R.id.seek).setOnSeekBarChangeListener(object :
            SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    Log.i(TAG, "onProgressChanged: $progress.")
                    FFPlayer.seek(progress / 100.0)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {

            }

            override fun onStopTrackingTouch(seekBar: SeekBar) {

            }
        })

        FFPlayer.addCallback {
            runOnUiThread {
                findViewById<SeekBar>(R.id.seek).progress = (it * 100).toInt()
            }
        }
    }

}