package com.example.ndkpractice

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.provider.MediaStore
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.example.ndkpractice.core.FFPlayer

private const val TAG = "MainPlayerTag"

class MainActivity : AppCompatActivity() {

    private val player by lazy {
        FFPlayer()
    }

    private val glSurfaceView by lazy {
        GLSurfaceView(this)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(glSurfaceView)

        glSurfaceView.setEGLContextClientVersion(3)
        glSurfaceView.setRenderer(player)
        glSurfaceView.renderMode = GLSurfaceView.RENDERMODE_CONTINUOUSLY

        supportActionBar?.apply {
            setDisplayShowCustomEnabled(true)
            setDisplayShowTitleEnabled(false)
            setCustomView(R.layout.layout_action_bar)

            this.customView.findViewById<View>(R.id.btn).setOnClickListener {
                val intent = Intent(Intent.ACTION_PICK)
                intent.type = "video/*"
                startActivityForResult(intent, 1024)
            }
        }

        player

        if(checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE), 1025)
        }
    }

    private fun onFetchUrl(url: String) {
        player.setUrl(url)
        player.init()
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        val uri = data?.data ?: return
        val path = getVideoPathFromUri(uri)
        Log.i(TAG, "onActivityResult: path $path.")
        path ?: return
        onFetchUrl(path)
    }

    fun getVideoPathFromUri(uri: Uri): String? {
        val projection = arrayOf(MediaStore.Video.Media.DATA)
        val cursor = contentResolver.query(uri, projection, null, null, null)
        cursor?.let {
            if (it.moveToFirst()) {
                val columnIndex = it.getColumnIndexOrThrow(MediaStore.Video.Media.DATA)
                return it.getString(columnIndex)
            }
        }
        return null
    }

    override fun onDestroy() {
        super.onDestroy()
        player.release()
    }


}