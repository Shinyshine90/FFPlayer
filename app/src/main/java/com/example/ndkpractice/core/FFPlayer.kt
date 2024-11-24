package com.example.ndkpractice.core

import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView

private const val TAG = "FFPlayer"

class FFPlayer {

    private var nativeHandle: Long = 0L

    private var surfaceView : SurfaceView? = null

    private val surfaceCallback = object : SurfaceHolder.Callback {
        override fun surfaceCreated(holder: SurfaceHolder) {
            setDisplaySurface(holder.surface)
        }

        override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
            resizeDisplaySurface(width, height)
        }

        override fun surfaceDestroyed(holder: SurfaceHolder) {
            removeDisplaySurface()
        }
    }

    fun setSurfaceView(surfaceView: SurfaceView) {
        surfaceView.holder.addCallback(surfaceCallback)
        if (surfaceView.holder.surface.isValid) {
            setDisplaySurface(surfaceView.holder.surface)
        }
    }

    fun releaseSurface() {
        val surfaceView = this.surfaceView ?: return
        this.surfaceView = null
        surfaceView.holder.removeCallback(surfaceCallback)
    }

    private external fun initGlobal()

    private external fun setupNative()

    external fun setUrl(path: String)

    external fun prepare()

    external fun start()

    external fun pause()

    external fun stop()

    external fun seek(float: Float)

    external fun setDisplaySurface(surface: Surface)

    external fun removeDisplaySurface()

    external fun resizeDisplaySurface(w: Int, h: Int)

    external fun release()

    companion object {
        init {
            System.loadLibrary("x264")
            System.loadLibrary("fdk-aac")
            System.loadLibrary("ffplayer")
        }
    }

    init {
        initGlobal()
        setupNative()
        Log.i(TAG, "nativeHandle: $nativeHandle.")
    }

}