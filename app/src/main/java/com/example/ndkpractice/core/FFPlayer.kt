package com.example.ndkpractice.core

import android.view.Surface

private const val TAG = "FFPlayer"

class FFPlayer {
    external fun testThread()

    external fun init()

    external fun release()

    external fun setUrl(path: String)

    external fun start()

    external fun pause()

    external fun stop()

    external fun seek(float: Float)

    external fun setDisplaySurface(surface: Surface)

    external fun removeDisplaySurface()

    external fun resizeDisplaySurface(w: Int, h: Int)

    init {
        System.loadLibrary("x264")
        System.loadLibrary("fdk-aac")
        System.loadLibrary("ffplayer")
    }
}