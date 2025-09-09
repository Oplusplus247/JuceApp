// AudioModule.kt
package com.juceaudioapp

import android.util.Log
import com.facebook.react.bridge.*
import com.facebook.react.bridge.ReactMethod

class AudioModule(private val reactCtx: ReactApplicationContext) :
  ReactContextBaseJavaModule(reactCtx) {

  companion object {
    private const val TAG = "AudioModule"
    private var libraryLoaded = false

    init {
      try {
        System.loadLibrary("juce_audio_rn_native")
        libraryLoaded = true
        Log.i(TAG, "Native library loaded")
      } catch (e: UnsatisfiedLinkError) {
        libraryLoaded = false
        Log.e(TAG, "Failed to load native lib", e)
      }
    }
  }

  override fun getName(): String = "AudioModule"

  // ===== Native hooks =====
  private external fun nativeInitializeJuceAudio(): Boolean
  private external fun nativeShutdownJuceAudio()
  private external fun nativeGetJuceVersion(): String
  private external fun nativeGetCurrentSampleRate(): Double
  private external fun nativeSetJuceEnabled(enabled: Boolean)

  @ReactMethod
  fun setJuceEnabled(enabled: Boolean) {
    if (!libraryLoaded) {
      Log.e(TAG, "Native library not loaded - cannot set JUCE enabled")
      return
    }
    try {
      nativeSetJuceEnabled(enabled)
      Log.d(TAG, "JUCE enabled set to: $enabled")
    } catch (e: Exception) {
      Log.e(TAG, "Error setting JUCE enabled", e)
    }
  }

  @ReactMethod
  fun init(promise: Promise) {
    if (!libraryLoaded) {
      Log.e(TAG, "Native library not loaded")
      promise.reject("E_NATIVE", "Native library not loaded")
      return
    }
    Log.d(TAG, "Attempting to initialize audio engine")
    try {
      val ok = nativeInitializeJuceAudio()
      if (ok) {
        Log.i(TAG, "Audio engine started successfully")
        promise.resolve(true)
      } else {
        Log.e(TAG, "Audio engine failed to start")
        promise.reject("E_START", "Failed to start audio engine")
      }
    } catch (e: Exception) {
      Log.e(TAG, "Exception during audio init", e)
      promise.reject("E_EXCEPTION", "Exception during audio initialization: ${e.message}")
    }
  }

  @ReactMethod
  fun shutdown(promise: Promise) {
    if (!libraryLoaded) {
      promise.reject("E_NATIVE", "Native library not loaded")
      return
    }
    try {
      nativeShutdownJuceAudio()
      Log.i(TAG, "Audio engine stopped")
      promise.resolve(true)
    } catch (e: Exception) {
      Log.e(TAG, "Exception during audio shutdown", e)
      promise.reject("E_EXCEPTION", "Exception during audio shutdown: ${e.message}")
    }
  }

  @ReactMethod
  fun version(promise: Promise) {
    if (!libraryLoaded) {
      promise.reject("E_NATIVE", "Native library not loaded")
      return
    }
    try {
      val version = nativeGetJuceVersion()
      promise.resolve(version)
    } catch (e: Exception) {
      Log.e(TAG, "Exception getting version", e)
      promise.reject("E_EXCEPTION", "Exception getting version: ${e.message}")
    }
  }

  @ReactMethod
  fun sampleRate(promise: Promise) {
    if (!libraryLoaded) {
      promise.reject("E_NATIVE", "Native library not loaded")
      return
    }
    try {
      val sampleRate = nativeGetCurrentSampleRate()
      promise.resolve(sampleRate)
    } catch (e: Exception) {
      Log.e(TAG, "Exception getting sample rate", e)
      promise.reject("E_EXCEPTION", "Exception getting sample rate: ${e.message}")
    }
  }

}