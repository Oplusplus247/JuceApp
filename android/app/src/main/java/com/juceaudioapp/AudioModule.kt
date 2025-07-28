package com.juceaudioapp

import androidx.annotation.NonNull
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReactContextBaseJavaModule
import com.facebook.react.bridge.ReactMethod
import com.facebook.react.bridge.Arguments
import com.facebook.react.bridge.Promise
import android.util.Log

class AudioModule(reactContext: ReactApplicationContext) : ReactContextBaseJavaModule(reactContext) {
    
    companion object {
        private const val TAG = "AudioModule"
        private var libraryLoaded = false
        
        init {
            try {
                System.loadLibrary("juce_audio_rn_native")
                libraryLoaded = true
                Log.i(TAG, "Native library loaded successfully")
            } catch (e: UnsatisfiedLinkError) {
                Log.e(TAG, "Failed to load juce_audio_rn_native library", e)
                libraryLoaded = false
            }
        }
    }
    
    override fun getName(): String = "AudioModule"
    
    private external fun stringFromJNI(): String
    private external fun initializeJuceAudio(): Boolean
    private external fun shutdownJuceAudio()
    private external fun getJuceVersion(): String
    private external fun getCurrentSampleRate(): Double
    private external fun getCurrentAudioDeviceName(): String
    private external fun getOutputChannels(): Int
    private external fun getBufferSize(): Int
    
    private external fun startTone()
    private external fun stopTone()
    private external fun setToneFrequency(frequency: Float)
    private external fun setToneVolume(volume: Float)
    private external fun isTonePlaying(): Boolean
    private external fun getToneFrequency(): Float
    private external fun getToneVolume(): Float
    
    @ReactMethod
    fun getVersion(promise: Promise) {
        promise.resolve("Audio Module Android v2.0 with JUCE + Tone Generator")
    }
    
    @ReactMethod
    fun testBridge(message: String, promise: Promise) {
        promise.resolve("Android Echo: $message")
    }
    
    @ReactMethod
    fun getStringFromJNI(promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            val message = stringFromJNI()
            Log.i(TAG, "✅ Got message from JNI: $message")
            promise.resolve(message)
        } catch (e: Exception) {
            Log.e(TAG, "Error in getStringFromJNI", e)
            promise.reject("JNI_ERROR", "Failed to get string from JNI: ${e.message}", e)
        }
    }
    
    @ReactMethod
    fun initializeJUCE(promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            Log.i(TAG, "🎵 Initializing JUCE audio engine...")
            val success = initializeJuceAudio()
            Log.i(TAG, "🎵 JUCE initialization result: $success")
            promise.resolve(success)
        } catch (e: Exception) {
            Log.e(TAG, " Error initializing JUCE", e)
            promise.reject("JUCE_INIT_ERROR", "Failed to initialize JUCE: ${e.message}", e)
        }
    }
    
    @ReactMethod
    fun shutdownJUCE(promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            shutdownJuceAudio()
            Log.i(TAG, "🎵 JUCE audio engine shut down")
            promise.resolve(true)
        } catch (e: Exception) {
            Log.e(TAG, "Error shutting down JUCE", e)
            promise.reject("JUCE_SHUTDOWN_ERROR", "Failed to shutdown JUCE: ${e.message}", e)
        }
    }
    
    @ReactMethod
    fun getJUCEVersion(promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            val version = getJuceVersion()
            promise.resolve(version)
        } catch (e: Exception) {
            Log.e(TAG, "Error getting JUCE version", e)
            promise.reject("JUCE_VERSION_ERROR", "Failed to get JUCE version: ${e.message}", e)
        }
    }
    
    @ReactMethod
    fun getAudioInfo(promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            val sampleRate = getCurrentSampleRate()
            val deviceName = getCurrentAudioDeviceName()
            val outputChannels = getOutputChannels()
            val bufferSize = getBufferSize()
            
            val audioInfo = Arguments.createMap().apply {
                putDouble("sampleRate", sampleRate)
                putString("deviceName", deviceName)
                putInt("outputChannels", outputChannels)
                putInt("bufferSize", bufferSize)
            }
            
            promise.resolve(audioInfo)
        } catch (e: Exception) {
            Log.e(TAG, "Error getting audio info", e)
            promise.reject("AUDIO_INFO_ERROR", "Failed to get audio info: ${e.message}", e)
        }
    }
    
    @ReactMethod
    fun isNativeLibraryLoaded(promise: Promise) {
        promise.resolve(libraryLoaded)
    }
    
    @ReactMethod
    fun startToneGenerator(promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            startTone()
            Log.i(TAG, "🎵 Tone generator started")
            promise.resolve(true)
        } catch (e: Exception) {
            Log.e(TAG, "Error starting tone generator", e)
            promise.reject("TONE_START_ERROR", "Failed to start tone: ${e.message}", e)
        }
    }
    
    @ReactMethod
    fun stopToneGenerator(promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            stopTone()
            Log.i(TAG, "🎵 Tone generator stopped")
            promise.resolve(true)
        } catch (e: Exception) {
            Log.e(TAG, "Error stopping tone generator", e)
            promise.reject("TONE_STOP_ERROR", "Failed to stop tone: ${e.message}", e)
        }
    }
    
    @ReactMethod
    fun setToneGeneratorFrequency(frequency: Double, promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            setToneFrequency(frequency.toFloat())
            Log.i(TAG, "🎵 Tone frequency set to: $frequency Hz")
            promise.resolve(true)
        } catch (e: Exception) {
            Log.e(TAG, "Error setting tone frequency", e)
            promise.reject("TONE_FREQ_ERROR", "Failed to set frequency: ${e.message}", e)
        }
    }
    
    @ReactMethod
    fun setToneGeneratorVolume(volume: Double, promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            setToneVolume(volume.toFloat())
            Log.i(TAG, "🎵 Tone volume set to: $volume")
            promise.resolve(true)
        } catch (e: Exception) {
            Log.e(TAG, "Error setting tone volume", e)
            promise.reject("TONE_VOL_ERROR", "Failed to set volume: ${e.message}", e)
        }
    }

    @ReactMethod  
    fun crashTest() {  
         throw RuntimeException("This should crash if native code is linked")  
    }  
    
    @ReactMethod
    fun getToneGeneratorStatus(promise: Promise) {
        if (!libraryLoaded) {
            promise.reject("LIBRARY_ERROR", "Native library not loaded")
            return
        }
        
        try {
            val isPlaying = isTonePlaying()
            val frequency = getToneFrequency()
            val volume = getToneVolume()
            
            val status = Arguments.createMap().apply {
                putBoolean("isPlaying", isPlaying)
                putDouble("frequency", frequency.toDouble())
                putDouble("volume", volume.toDouble())
            }
            
            promise.resolve(status)
        } catch (e: Exception) {
            Log.e(TAG, "Error getting tone status", e)
            promise.reject("TONE_STATUS_ERROR", "Failed to get tone status: ${e.message}", e)
        }
    }
}