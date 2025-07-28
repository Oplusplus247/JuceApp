#include <jni.h>
#include <android/log.h>
#include <string>
#include <memory>
#include <atomic>
#include <cmath>

#define TAG "JuceAudioJNI"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#if HAS_JUCE
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

class SimpleToneGenerator {
public:
    SimpleToneGenerator() 
        : frequency(440.0f)
        , volume(0.1f)
        , isPlaying(false)
    {
        ALOGI("🎵 SimpleToneGenerator created");
    }
    
    void startTone() {
        isPlaying.store(true);
        ALOGI("🎵 Tone started: %.1f Hz", frequency.load());
    }
    
    void stopTone() {
        isPlaying.store(false);
        ALOGI("🎵 Tone stopped");
    }
    
    void setFrequency(float freq) {
        frequency.store(freq);
        ALOGI("🎵 Frequency set to: %.1f Hz", freq);
    }
    
    void setVolume(float vol) {
        vol = std::max(0.0f, std::min(1.0f, vol));
        volume.store(vol);
        ALOGI("🎵 Volume set to: %.2f", vol);
    }
    
    bool getIsPlaying() const {
        return isPlaying.load();
    }
    
    float getFrequency() const {
        return frequency.load();
    }
    
    float getVolume() const {
        return volume.load();
    }

private:
    std::atomic<float> frequency;
    std::atomic<float> volume;
    std::atomic<bool> isPlaying;
};

class RealJuceAudioEngine {
    public:
        RealJuceAudioEngine() : initialized(false) {
            ALOGI("🎵 Real JUCE Audio Engine created (minimal version)");
            toneGenerator = std::make_unique<SimpleToneGenerator>();
        }
        
        ~RealJuceAudioEngine() {
            shutdown();
        }
        
        bool initialize() {
            try {
                ALOGI("🎵 Initializing Real JUCE (minimal mode - no AudioDeviceManager)...");
                
                initialized = true;
                
                ALOGI("✅ JUCE minimal initialization successful!");
                ALOGI("✅ Version: %s", juce::SystemStats::getJUCEVersion().toRawUTF8());
                ALOGI("✅ Tone generator ready (no real audio output yet)");
                
                return true;
                
            } catch (const std::exception& e) {
                ALOGE("Exception during JUCE initialization: %s", e.what());
                return false;
            }
        }
        
        void shutdown() {
            if (initialized) {
                ALOGI("🎵 Shutting down Real JUCE Audio Engine...");
                
                if (toneGenerator) {
                    toneGenerator->stopTone();
                }
                
                initialized = false;
                ALOGI("Real JUCE Audio Engine shut down");
            }
        }
        
        bool isInitialized() const { return initialized; }
        
        std::string getJuceVersion() const {
            return "Real JUCE " + juce::SystemStats::getJUCEVersion().toStdString() + " (Minimal Mode)";
        }
        
        double getCurrentSampleRate() const { return 44100.0; }
        std::string getCurrentAudioDeviceName() const { return "Minimal JUCE Device (No Real Audio)"; }
        int getOutputChannels() const { return 2; }
        int getBufferSize() const { return 256; }
        
        void startTone() {
            if (toneGenerator && initialized) {
                toneGenerator->startTone();
            }
        }
        
        void stopTone() {
            if (toneGenerator && initialized) {
                toneGenerator->stopTone();
            }
        }
        
        void setToneFrequency(float frequency) {
            if (toneGenerator) {
                toneGenerator->setFrequency(frequency);
            }
        }
        
        void setToneVolume(float volume) {
            if (toneGenerator) {
                toneGenerator->setVolume(volume);
            }
        }
        
        bool isTonePlaying() const {
            if (toneGenerator) {
                return toneGenerator->getIsPlaying();
            }
            return false;
        }
        
        float getToneFrequency() const {
            if (toneGenerator) {
                return toneGenerator->getFrequency();
            }
            return 0.0f;
        }
        
        float getToneVolume() const {
            if (toneGenerator) {
                return toneGenerator->getVolume();
            }
            return 0.0f;
        }

    private:
        bool initialized;
        std::unique_ptr<SimpleToneGenerator> toneGenerator;
    };

    static std::unique_ptr<RealJuceAudioEngine> realJuceEngine;

    #else
    class BasicAudioEngine {
    public:
        bool initialize() { 
            ALOGI("Basic Audio Engine initialized (no JUCE)");
            return true; 
        }
        void shutdown() { 
            ALOGI("Basic Audio Engine shut down");
        }
        bool isInitialized() const { return true; }
        std::string getJuceVersion() const { return "No JUCE (Basic Engine)"; }
        double getCurrentSampleRate() const { return 48000.0; }
        std::string getCurrentAudioDeviceName() const { return "Basic Audio Device"; }
        int getOutputChannels() const { return 2; }
        int getBufferSize() const { return 256; }
        
        void startTone() { ALOGI("Basic: Tone start (no audio)"); }
        void stopTone() { ALOGI("Basic: Tone stop (no audio)"); }
        void setToneFrequency(float freq) { ALOGI("Basic: Set frequency %.1f Hz (no audio)", freq); }
        void setToneVolume(float vol) { ALOGI("Basic: Set volume %.2f (no audio)", vol); }
        bool isTonePlaying() const { return false; }
        float getToneFrequency() const { return 440.0f; }
        float getToneVolume() const { return 0.1f; }
    };

    static std::unique_ptr<BasicAudioEngine> basicEngine;
    #endif

    extern "C" {

    JNIEXPORT jstring JNICALL
    Java_com_juceaudioapp_AudioModule_stringFromJNI(JNIEnv* env, jobject) {
    #if HAS_JUCE
        std::string message = "Hello from Real JUCE JNI! (Minimal Mode)";
    #else
        std::string message = "Hello from Basic Audio JNI (No JUCE)";
    #endif
        ALOGI("%s", message.c_str());
        return env->NewStringUTF(message.c_str());
    }

    JNIEXPORT jboolean JNICALL
    Java_com_juceaudioapp_AudioModule_initializeJuceAudio(JNIEnv* env, jobject) {
        try {
            ALOGI("🎵 JNI: Initialize audio called");
            
    #if HAS_JUCE
            if (!realJuceEngine) {
                realJuceEngine = std::make_unique<RealJuceAudioEngine>();
            }
            bool success = realJuceEngine->initialize();
    #else
            if (!basicEngine) {
                basicEngine = std::make_unique<BasicAudioEngine>();
            }
            bool success = basicEngine->initialize();
    #endif
            
            ALOGI("🎵 Audio initialization result: %s", success ? "SUCCESS" : "FAILED");
            return success ? JNI_TRUE : JNI_FALSE;
            
        } catch (const std::exception& e) {
            ALOGE("Exception in audio initialization: %s", e.what());
            return JNI_FALSE;
        }
    }

    JNIEXPORT void JNICALL
    Java_com_juceaudioapp_AudioModule_shutdownJuceAudio(JNIEnv* env, jobject) {
        ALOGI("🎵 JNI: Shutdown audio called");
        
    #if HAS_JUCE
        if (realJuceEngine) {
            realJuceEngine->shutdown();
            realJuceEngine.reset();
        }
    #else
        if (basicEngine) {
            basicEngine->shutdown();
            basicEngine.reset();
        }
    #endif
    }

    JNIEXPORT jstring JNICALL
    Java_com_juceaudioapp_AudioModule_getJuceVersion(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine) {
            std::string version = realJuceEngine->getJuceVersion();
            return env->NewStringUTF(version.c_str());
        }
    #else
        if (basicEngine) {
            std::string version = basicEngine->getJuceVersion();
            return env->NewStringUTF(version.c_str());
        }
    #endif
        return env->NewStringUTF("Audio Engine Not Initialized");
    }

    JNIEXPORT jdouble JNICALL
    Java_com_juceaudioapp_AudioModule_getCurrentSampleRate(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            return realJuceEngine->getCurrentSampleRate();
        }
    #else
        if (basicEngine) {
            return basicEngine->getCurrentSampleRate();
        }
    #endif
        return 0.0;
    }

    JNIEXPORT jstring JNICALL
    Java_com_juceaudioapp_AudioModule_getCurrentAudioDeviceName(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            std::string deviceName = realJuceEngine->getCurrentAudioDeviceName();
            return env->NewStringUTF(deviceName.c_str());
        }
    #else
        if (basicEngine) {
            std::string deviceName = basicEngine->getCurrentAudioDeviceName();
            return env->NewStringUTF(deviceName.c_str());
        }
    #endif
        return env->NewStringUTF("No Device");
    }

    JNIEXPORT jint JNICALL
    Java_com_juceaudioapp_AudioModule_getOutputChannels(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            return realJuceEngine->getOutputChannels();
        }
    #else
        if (basicEngine) {
            return basicEngine->getOutputChannels();
        }
    #endif
        return 0;
    }

    JNIEXPORT jint JNICALL
    Java_com_juceaudioapp_AudioModule_getBufferSize(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            return realJuceEngine->getBufferSize();
        }
    #else
        if (basicEngine) {
            return basicEngine->getBufferSize();
        }
    #endif
        return 0;
    }

    JNIEXPORT void JNICALL
    Java_com_juceaudioapp_AudioModule_startTone(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            realJuceEngine->startTone();
        }
    #else
        if (basicEngine) {
            basicEngine->startTone();
        }
    #endif
    }

    JNIEXPORT void JNICALL
    Java_com_juceaudioapp_AudioModule_stopTone(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            realJuceEngine->stopTone();
        }
    #else
        if (basicEngine) {
            basicEngine->stopTone();
        }
    #endif
    }

    JNIEXPORT void JNICALL
    Java_com_juceaudioapp_AudioModule_setToneFrequency(JNIEnv* env, jobject, jfloat frequency) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            realJuceEngine->setToneFrequency(frequency);
        }
    #else
        if (basicEngine) {
            basicEngine->setToneFrequency(frequency);
        }
    #endif
    }

    JNIEXPORT void JNICALL
    Java_com_juceaudioapp_AudioModule_setToneVolume(JNIEnv* env, jobject, jfloat volume) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            realJuceEngine->setToneVolume(volume);
        }
    #else
        if (basicEngine) {
            basicEngine->setToneVolume(volume);
        }
    #endif
    }

    JNIEXPORT jboolean JNICALL
    Java_com_juceaudioapp_AudioModule_isTonePlaying(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            return realJuceEngine->isTonePlaying() ? JNI_TRUE : JNI_FALSE;
        }
    #else
        if (basicEngine) {
            return basicEngine->isTonePlaying() ? JNI_TRUE : JNI_FALSE;
        }
    #endif
        return JNI_FALSE;
    }

    JNIEXPORT jfloat JNICALL
    Java_com_juceaudioapp_AudioModule_getToneFrequency(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            return realJuceEngine->getToneFrequency();
        }
    #else
        if (basicEngine) {
            return basicEngine->getToneFrequency();
        }
    #endif
        return 440.0f;
    }

    JNIEXPORT jfloat JNICALL
    Java_com_juceaudioapp_AudioModule_getToneVolume(JNIEnv* env, jobject) {
    #if HAS_JUCE
        if (realJuceEngine && realJuceEngine->isInitialized()) {
            return realJuceEngine->getToneVolume();
        }
    #else
        if (basicEngine) {
            return basicEngine->getToneVolume();
        }
    #endif
        return 0.1f;
    }

}