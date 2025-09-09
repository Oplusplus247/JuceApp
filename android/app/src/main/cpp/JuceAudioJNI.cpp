// Keep JUCE headless (no JUCEApplicationBase)
#define JUCE_APPLICATION_BASE 0
#define JUCE_MODULE_AVAILABLE_juce_core 1
#define JUCE_MODULE_AVAILABLE_juce_audio_basics 1
#define JUCE_MODULE_AVAILABLE_juce_audio_formats 1
#define JUCE_MODULE_AVAILABLE_juce_dsp 1

#include <jni.h>
#include <android/log.h>
#include <string>
#include <memory>
#include <atomic>
#include <cmath>
#include <cstring>
#include <mutex>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TAG "JuceAudioJNI"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

#ifndef HAS_JUCE
#define HAS_JUCE 1
#endif

#if HAS_JUCE
  #include <juce_core/juce_core.h>
  #include <juce_audio_basics/juce_audio_basics.h>
  #include <juce_audio_formats/juce_audio_formats.h>
  #include <juce_dsp/juce_dsp.h>
  using namespace juce;

  // Provide expected globals for some JUCE builds
  namespace juce {
    const char* juce_compilationDate = __DATE__;
    const char* juce_compilationTime = __TIME__;
  }
#endif

// Oboe for Android audio I/O
#include <oboe/Oboe.h>

// =================== Ring Buffer for Audio ===================
template<typename T>
class RingBuffer {
public:
    RingBuffer(size_t capacity) : buffer(capacity), capacity_(capacity), readPos(0), writePos(0) {}
    
    bool write(const T* data, size_t count) {
        if (getAvailableWrite() < count) {
            return false; // Buffer full
        }
        for (size_t i = 0; i < count; ++i) {
            buffer[writePos] = data[i];
            writePos = (writePos + 1) % capacity_;
        }
        return true;
    }
    
    bool read(T* data, size_t count) {
        if (getAvailableRead() < count) {
            return false; // Not enough data
        }
        for (size_t i = 0; i < count; ++i) {
            data[i] = buffer[readPos];
            readPos = (readPos + 1) % capacity_;
        }
        return true;
    }
    
    size_t getAvailableRead() const {
        return (writePos >= readPos) ? (writePos - readPos) : (capacity_ - readPos + writePos);
    }
    
    size_t getAvailableWrite() const {
        return capacity_ - getAvailableRead() - 1;
    }
    
    void clear() {
        readPos = 0;
        writePos = 0;
    }

private:
    std::vector<T> buffer;
    size_t capacity_;
    std::atomic<size_t> readPos;
    std::atomic<size_t> writePos;
};

// =================== Improved Oboe Audio Engine ===================
class OboeEngine : public oboe::AudioStreamCallback {
public:
    OboeEngine()
        : playing(false),
          sampleRateHint(48000.0),
          juceEnabled(true),
          outputCallbackCount(0),
          inputCallbackCount(0),
          ringBuffer(48000) // 1 second buffer at 48kHz
    {
        ALOGI("OboeEngine created - CLEAR EFFECTS VERSION");
        
#if HAS_JUCE
        // Initialize JUCE DSP components
        spec.sampleRate = sampleRateHint;
        spec.maximumBlockSize = 512;
        spec.numChannels = 1;
        
        gain.prepare(spec);
        gain.setGainLinear(1.0f); // Unity gain

        // More pronounced effects
        chorus.prepare(spec);
        chorus.setRate(2.5f);           // Faster modulation
        chorus.setDepth(0.8f);          // Deeper effect
        chorus.setCentreDelay(10.0f);   // More delay
        chorus.setFeedback(0.3f);       // More feedback
        chorus.setMix(0.5f);            // 50% wet/dry mix

        // Add a phaser for more dramatic effect
        phaser.prepare(spec);
        phaser.setRate(1.2f);
        phaser.setDepth(0.9f);
        phaser.setCentreFrequency(800.0f);
        phaser.setFeedback(0.7f);
        phaser.setMix(0.6f);

        ALOGI("JUCE DSP initialized with CLEAR effects (Chorus + Phaser)");
#endif
    }

    bool start() {
        if (playing.load()) {
            ALOGI("Already playing, returning true");
            return true;
        }

        ALOGI("Starting audio engine with CLEAR effects...");
        ringBuffer.clear();

        // === INPUT STREAM (Microphone) ===
        oboe::AudioStreamBuilder inputBuilder;
        inputBuilder.setDirection(oboe::Direction::Input);
        inputBuilder.setFormat(oboe::AudioFormat::Float);
        inputBuilder.setChannelCount(1);
        inputBuilder.setSharingMode(oboe::SharingMode::Shared);
        inputBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        inputBuilder.setSampleRate(static_cast<int32_t>(sampleRateHint));
        inputBuilder.setCallback(this);
        inputBuilder.setFramesPerCallback(256);
        
        oboe::Result result = inputBuilder.openStream(inputStream);
        if (result != oboe::Result::OK) {
            ALOGE("Failed to open INPUT stream. Error: %s", oboe::convertToText(result));
            return false;
        }
        
        ALOGI("Input stream opened - SR: %d, Buffer: %d", 
              inputStream->getSampleRate(), inputStream->getBufferSizeInFrames());

        // === OUTPUT STREAM (Speaker) ===
        oboe::AudioStreamBuilder outputBuilder;
        outputBuilder.setDirection(oboe::Direction::Output);
        outputBuilder.setFormat(oboe::AudioFormat::Float);
        outputBuilder.setChannelCount(1);
        outputBuilder.setSharingMode(oboe::SharingMode::Shared);
        outputBuilder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
        outputBuilder.setSampleRate(inputStream->getSampleRate());
        outputBuilder.setCallback(this);
        outputBuilder.setFramesPerCallback(256);
        
        result = outputBuilder.openStream(outputStream);
        if (result != oboe::Result::OK) {
            ALOGE("Failed to open OUTPUT stream. Error: %s", oboe::convertToText(result));
            inputStream->close();
            inputStream.reset();
            return false;
        }
        
        ALOGI("Output stream opened - SR: %d, Buffer: %d", 
              outputStream->getSampleRate(), outputStream->getBufferSizeInFrames());

        // Update JUCE DSP with actual sample rate
        double actualSampleRate = inputStream->getSampleRate();
        
#if HAS_JUCE
        spec.sampleRate = actualSampleRate;
        gain.prepare(spec);
        gain.setGainLinear(1.0f);

        chorus.prepare(spec);
        phaser.prepare(spec);
        
        ALOGI("JUCE DSP updated for SR: %.0f", actualSampleRate);
#endif

        // Start streams
        result = inputStream->requestStart();
        if (result != oboe::Result::OK) {
            ALOGE("Failed to start INPUT stream");
            return false;
        }

        result = outputStream->requestStart();
        if (result != oboe::Result::OK) {
            ALOGE("Failed to start OUTPUT stream");
            inputStream->stop();
            inputStream->close();
            inputStream.reset();
            return false;
        }

        playing.store(true);
        ALOGI("Audio engine started successfully - CLEAR EFFECT MODE");
        return true;
    }

    void stop() {
        ALOGI("Stopping audio engine...");
        playing.store(false);
        
        if (inputStream) {
            inputStream->stop();
            inputStream->close();
            inputStream.reset();
        }
        
        if (outputStream) {
            outputStream->stop();
            outputStream->close();
            outputStream.reset();
        }
    }

    double getSampleRate() const {
        if (outputStream) {
            return outputStream->getSampleRate();
        }
        return sampleRateHint;
    }

    void setJuceEnabled(bool enabled) {
        juceEnabled.store(enabled);
        ALOGI("JUCE effects %s - YOU SHOULD HEAR A CLEAR DIFFERENCE NOW!", enabled ? "ENABLED" : "DISABLED");
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* stream, void* audioData, int32_t numFrames) override {
        if (stream->getDirection() == oboe::Direction::Input) {
            inputCallbackCount++;
            float* inputData = static_cast<float*>(audioData);
            if (!ringBuffer.write(inputData, static_cast<size_t>(numFrames))) {
                if (inputCallbackCount % 500 == 0) ALOGD("Ring buffer full, dropping input frames");
            }
            return oboe::DataCallbackResult::Continue;
        }
        
        if (stream->getDirection() == oboe::Direction::Output) {
            outputCallbackCount++;
            float* outputData = static_cast<float*>(audioData);
            
            bool hasData = ringBuffer.read(outputData, static_cast<size_t>(numFrames));
            if (hasData) {
#if HAS_JUCE
                // Always build block from channel array
                float* channels[] = { outputData };
                juce::dsp::AudioBlock<float> block (channels, 1, (size_t) numFrames);

                // Create the context here so it's available in both branches
                juce::dsp::ProcessContextReplacing<float> ctx(block);

                if (juceEnabled.load()) {
                    // APPLY CLEAR, AUDIBLE EFFECTS (no loudness normalization)
                    // Process: Chorus -> Phaser (both clearly audible)
                    chorus.process(ctx);
                    phaser.process(ctx);
                    
                    // Slight volume boost to make effect more noticeable
                    gain.setGainLinear(3.0f); // 20% volume boost when effects are on
                    gain.process(ctx);

                    if (outputCallbackCount % 500 == 0) {
                        ALOGI("EFFECTS ON: Chorus + Phaser active (volume +20%%)");
                    }
                } else {
                    // Effects bypassed - clean pass-through at unity gain
                    gain.setGainLinear(3.0f);
                    gain.process(ctx);
                    
                    if (outputCallbackCount % 500 == 0) {
                        ALOGI("EFFECTS OFF: Clean pass-through");
                    }
                }
#endif

                if (outputCallbackCount % 1000 == 0) {
                    float maxLevel = 0.0f;
                    for (int i = 0; i < numFrames; ++i) maxLevel = std::max(maxLevel, std::abs(outputData[i]));
                    ALOGI("OUTPUT #%d: maxLevel=%.4f, JUCE=%s", 
                          outputCallbackCount, maxLevel, juceEnabled.load() ? "ON" : "OFF");
                }
            } else {
                std::memset(outputData, 0, static_cast<size_t>(numFrames) * sizeof(float));
            }
            return oboe::DataCallbackResult::Continue;
        }
        return oboe::DataCallbackResult::Continue;
    }

    void onErrorBeforeClose(oboe::AudioStream* stream, oboe::Result error) override {
        ALOGE("Audio stream error: %s", oboe::convertToText(error));
    }

    void onErrorAfterClose(oboe::AudioStream* stream, oboe::Result error) override {
        ALOGE("Audio stream closed with error: %s", oboe::convertToText(error));
        playing.store(false);
    }

private:
    std::shared_ptr<oboe::AudioStream> inputStream;
    std::shared_ptr<oboe::AudioStream> outputStream;
    RingBuffer<float> ringBuffer;
    std::atomic<bool> playing;
    std::atomic<bool> juceEnabled;
    double sampleRateHint;
    int outputCallbackCount;
    int inputCallbackCount;

#if HAS_JUCE
    juce::dsp::ProcessSpec spec;
    juce::dsp::Gain<float> gain;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Phaser<float> phaser;
#endif
};

// =============== Global Engine + JNI bridge ===============
static std::unique_ptr<OboeEngine> gEngine;

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_juceaudioapp_AudioModule_nativeInitializeJuceAudio(JNIEnv*, jobject) {
    ALOGI("=== INITIALIZING CLEAR EFFECTS AUDIO ===");
    if (!gEngine) {
        gEngine = std::make_unique<OboeEngine>();
    }
    bool result = gEngine->start();
    ALOGI("=== INIT RESULT: %s ===", result ? "SUCCESS" : "FAILED");
    return result ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_juceaudioapp_AudioModule_nativeShutdownJuceAudio(JNIEnv*, jobject) {
    ALOGI("=== SHUTTING DOWN AUDIO ===");
    if (gEngine) {
        gEngine->stop();
        gEngine.reset();
    }
    ALOGI("=== SHUTDOWN COMPLETE ===");
}

JNIEXPORT jstring JNICALL
Java_com_juceaudioapp_AudioModule_nativeGetJuceVersion(JNIEnv* env, jobject) {
#if HAS_JUCE
    return env->NewStringUTF("JUCE with CLEAR Chorus+Phaser effects");
#else
    return env->NewStringUTF("JUCE disabled");
#endif
}

JNIEXPORT jdouble JNICALL
Java_com_juceaudioapp_AudioModule_nativeGetCurrentSampleRate(JNIEnv*, jobject) {
    return gEngine ? gEngine->getSampleRate() : 0.0;
}

JNIEXPORT void JNICALL
Java_com_juceaudioapp_AudioModule_nativeSetJuceEnabled(JNIEnv*, jobject, jboolean enabled) {
    ALOGI("=== SETTING JUCE ENABLED: %s ===", enabled == JNI_TRUE ? "TRUE" : "FALSE");
    if (gEngine) gEngine->setJuceEnabled(enabled == JNI_TRUE);
}

} // extern "C"