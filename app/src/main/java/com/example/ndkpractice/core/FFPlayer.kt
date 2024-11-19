package com.example.ndkpractice.core

import android.content.res.AssetManager
import android.opengl.GLSurfaceView
import android.util.Log
import com.example.ndkpractice.App
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

private const val TAG = "FFPlayer"

class FFPlayer : GLSurfaceView.Renderer {

    external fun glInit(assetManager: AssetManager)
    external fun glResize(width: Int, height: Int)
    external fun glDraw()

    external fun init()

    external fun release()

    external fun setUrl(path: String)

    external fun start()

    external fun pause()

    external fun stop()

    external fun seek(float: Float)

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        glInit(App.context.assets)
        Log.i(TAG, "onSurfaceCreated: $this")
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        glResize(width, height)
        Log.i(TAG, "onSurfaceChanged: $this $width, $height")
    }

    override fun onDrawFrame(gl: GL10?) {
        //glDraw()
    }

    init {
        System.loadLibrary("x264")
        System.loadLibrary("fdk-aac")
        System.loadLibrary("ffplayer")
    }
}