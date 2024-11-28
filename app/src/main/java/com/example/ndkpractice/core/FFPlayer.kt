package com.example.ndkpractice.core

import android.view.Surface
import android.view.SurfaceView

private const val TAG = "FFPlayer"

class FFPlayer {

    private var nativeHandle: Long = 0L

    private var display: ISurfaceDisplay? = null

    private val surfaceCallback = object :  ISurfaceCallback {
        override fun onSurfaceCreated(surface: Surface) {
            _setDisplaySurface(surface)
        }

        override fun onSurfaceChanged(surface: Surface, format: Int, width: Int, height: Int) {
            _resizeDisplaySurface(width, height)
        }

        override fun onSurfaceDestroyed(surface: Surface) {
            _removeDisplaySurface()
        }
    }

    fun setDataSource(uri: String) {
        _setDataSource(uri)
    }

    fun setDisplay(surfaceView: SurfaceView) {
        display?.unregisterCallback()
        SurfaceViewDisplay(surfaceView).apply {
            this.registerCallback(surfaceCallback)
            this@FFPlayer.display = this
        }
    }

    fun removeDisplay() {
        display?.unregisterCallback()
        display = null
    }

    fun prepare() {
        _prepare()
    }

    fun start() {
        _start()
    }

    fun pause() {
        _pause()
    }

    fun stop() {
        _stop()
    }

    fun seek(percent: Float) {
        _seek(percent)
    }

    fun release() {
        _release()
    }

    private external fun _initGlobal()

    private external fun _setupNative()

    external fun _setDisplaySurface(surface: Surface)

    external fun _removeDisplaySurface()

    external fun _setDataSource(path: String)

    external fun _prepare()

    external fun _start()

    external fun _pause()

    external fun _stop()

    external fun _seek(float: Float)

    external fun _resizeDisplaySurface(w: Int, h: Int)

    external fun _release()

    companion object {
        init {
            System.loadLibrary("x264")
            System.loadLibrary("fdk-aac")
            System.loadLibrary("ffplayer")
        }
    }

    init {
        _initGlobal()
        _setupNative()
    }

}