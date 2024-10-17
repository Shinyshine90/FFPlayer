package com.example.ndkpractice.core

import android.util.Log
import android.view.Surface
import java.util.concurrent.CopyOnWriteArrayList

object FFPlayer {

    private val callbacks = CopyOnWriteArrayList<(Double)-> Unit>()

    external fun getPlayerVersion(): String

    external fun startPlay(url: String): Boolean

    external fun setDisplaySurface(surface: Surface): Boolean

    external fun removeDisplaySurface(surface: Surface): Boolean

    external fun seek(double: Double) : Boolean

    external fun pause()

    external fun resume()

    external fun isPaused(): Boolean

    fun addCallback(callback: (Double)-> Unit) {
        callbacks.add(callback);
    }

    fun onPlayPositionChanged(position: Double) {
        Log.i("FFPlayer", "onPlayPositionChanged: $position.")
        for (c in callbacks) {
            c.invoke(position)
        }
    }

    fun togglePlay() {
        if (isPaused()) {
            resume()
        } else {
            pause()
        }
    }

    init {
        System.loadLibrary("ffplayer")
    }
}