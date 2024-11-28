package com.example.ndkpractice.core

import android.graphics.SurfaceTexture
import android.view.Surface
import android.view.TextureView

class TextureViewDisplay(private val textureView: TextureView) : ISurfaceDisplay() {

    private var surface: Surface? = null

    private val callback = object : TextureView.SurfaceTextureListener {
        override fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture, width: Int, height: Int) {
            Surface(surfaceTexture).apply {
                surfaceCallback?.onSurfaceCreated(this)
                surface = this
            }
        }

        override fun onSurfaceTextureSizeChanged(surfaceTexture: SurfaceTexture, width: Int, height: Int) {
            val surfaceInternal = surface ?: return
            surfaceCallback?.onSurfaceChanged(surfaceInternal, 0, width, height)
        }

        override fun onSurfaceTextureDestroyed(surfaceTexture: SurfaceTexture): Boolean {
            val surfaceInternal = surface ?: return true
            surfaceCallback?.onSurfaceDestroyed(surfaceInternal)
            return true
        }

        override fun onSurfaceTextureUpdated(surfaceTexture: SurfaceTexture) {

        }
    }

    override fun unregisterCallback() {
        super.unregisterCallback()
        textureView.surfaceTextureListener = null
    }

    init {
        textureView.surfaceTextureListener = callback
    }

}