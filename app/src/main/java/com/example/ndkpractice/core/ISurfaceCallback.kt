package com.example.ndkpractice.core

import android.view.Surface

interface ISurfaceCallback {

    fun onSurfaceCreated(surface: Surface)

    fun onSurfaceChanged(surface: Surface, format: Int, width: Int, height: Int)

    fun onSurfaceDestroyed(surface: Surface)
}