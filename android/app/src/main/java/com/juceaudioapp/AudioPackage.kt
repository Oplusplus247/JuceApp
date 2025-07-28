package com.juceaudioapp

import android.util.Log
import com.facebook.react.ReactPackage
import com.facebook.react.bridge.NativeModule
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.uimanager.ViewManager

class AudioPackage : ReactPackage {
    
    init {
        Log.d("AudioPackage", "🏗️ AudioPackage constructor called")
    }
    
    override fun createNativeModules(reactContext: ReactApplicationContext): List<NativeModule> {
        Log.d("AudioPackage", "🔧 createNativeModules called")
        return try {
            val module = AudioModule(reactContext)
            Log.d("AudioPackage", "✅ AudioModule created successfully")
            listOf(module)
        } catch (e: Exception) {
            Log.e("AudioPackage", "❌ Failed to create AudioModule", e)
            emptyList()
        }
    }

    override fun createViewManagers(reactContext: ReactApplicationContext): List<ViewManager<*, *>> {
        Log.d("AudioPackage", "🔧 createViewManagers called")
        return listOf()
    }
}