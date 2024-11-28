package com.example.ndkpractice.core

abstract class ISurfaceDisplay {

    protected var surfaceCallback: ISurfaceCallback? = null

    fun registerCallback(callback: ISurfaceCallback) {
        this.surfaceCallback = callback
    }

    open fun unregisterCallback() {
        this.surfaceCallback = null
    }
}