package com.example.ndkpractice.core

import android.view.SurfaceHolder
import android.view.SurfaceView

class SurfaceViewDisplay(private val surfaceView: SurfaceView) : ISurfaceDisplay() {

    private val callback = object : SurfaceHolder.Callback {
        override fun surfaceCreated(holder: SurfaceHolder) {
            surfaceCallback?.onSurfaceCreated(holder.surface)
        }

        override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
            surfaceCallback?.onSurfaceChanged(holder.surface, format, width, height)
        }

        override fun surfaceDestroyed(holder: SurfaceHolder) {
            surfaceCallback?.onSurfaceDestroyed(holder.surface)
        }

    }

    override fun unregisterCallback() {
        super.unregisterCallback()
        surfaceView.holder.removeCallback(callback)
    }

    init {
        surfaceView.holder.addCallback(callback)
    }

}