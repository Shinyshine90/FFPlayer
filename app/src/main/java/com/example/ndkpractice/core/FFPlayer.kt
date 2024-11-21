package com.example.ndkpractice.core

import android.view.Surface

private const val TAG = "FFPlayer"

class FFPlayer {

    companion object {
        init {
            System.loadLibrary("x264")
            System.loadLibrary("fdk-aac")
            System.loadLibrary("ffplayer")
        }
    }

    external fun testThread()

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


}