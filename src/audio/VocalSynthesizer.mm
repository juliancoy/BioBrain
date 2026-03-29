#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#include "audio/VocalSynthesizer.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// ─── CoreAudio implementation ──────────────────────────────────────────────────

struct VocalSynthesizer::Impl {
    AudioComponentInstance audioUnit = nullptr;
    VocalSynthesizer* owner = nullptr;
};

// CoreAudio render callback — runs on real-time audio thread
static OSStatus audioRenderCallback(
    void* inRefCon,
    AudioUnitRenderActionFlags* /*ioActionFlags*/,
    const AudioTimeStamp* /*inTimeStamp*/,
    UInt32 /*inBusNumber*/,
    UInt32 inNumberFrames,
    AudioBufferList* ioData)
{
    auto* synth = static_cast<VocalSynthesizer*>(inRefCon);
    float* buffer = static_cast<float*>(ioData->mBuffers[0].mData);

    VocalSynthesizer::renderCallback(synth, buffer, inNumberFrames);

    // Copy mono to stereo if needed
    if (ioData->mNumberBuffers > 1) {
        float* right = static_cast<float*>(ioData->mBuffers[1].mData);
        std::memcpy(right, buffer, inNumberFrames * sizeof(float));
    }

    return noErr;
}

// ─── Synthesizer implementation ────────────────────────────────────────────────

VocalSynthesizer::VocalSynthesizer()
    : impl_(std::make_unique<Impl>())
{
    impl_->owner = this;
}

VocalSynthesizer::~VocalSynthesizer() {
    stop();
}

bool VocalSynthesizer::start() {
    if (running_.load()) return true;

    @autoreleasepool {
        // Set up audio session
        AudioComponentDescription desc{};
        desc.componentType = kAudioUnitType_Output;
        desc.componentSubType = kAudioUnitSubType_DefaultOutput;
        desc.componentManufacturer = kAudioUnitManufacturer_Apple;

        AudioComponent component = AudioComponentFindNext(nullptr, &desc);
        if (!component) {
            NSLog(@"BioBrain Audio: No output audio component found");
            return false;
        }

        OSStatus status = AudioComponentInstanceNew(component, &impl_->audioUnit);
        if (status != noErr) {
            NSLog(@"BioBrain Audio: Failed to create audio unit: %d", (int)status);
            return false;
        }

        // Set format: 44.1kHz, mono, float32
        AudioStreamBasicDescription format{};
        format.mSampleRate = SAMPLE_RATE;
        format.mFormatID = kAudioFormatLinearPCM;
        format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        format.mBitsPerChannel = 32;
        format.mChannelsPerFrame = 1;
        format.mFramesPerPacket = 1;
        format.mBytesPerFrame = sizeof(float);
        format.mBytesPerPacket = sizeof(float);

        status = AudioUnitSetProperty(impl_->audioUnit,
                                       kAudioUnitProperty_StreamFormat,
                                       kAudioUnitScope_Input, 0,
                                       &format, sizeof(format));
        if (status != noErr) {
            NSLog(@"BioBrain Audio: Failed to set stream format: %d", (int)status);
            AudioComponentInstanceDispose(impl_->audioUnit);
            impl_->audioUnit = nullptr;
            return false;
        }

        // Set render callback
        AURenderCallbackStruct callbackStruct;
        callbackStruct.inputProc = audioRenderCallback;
        callbackStruct.inputProcRefCon = this;

        status = AudioUnitSetProperty(impl_->audioUnit,
                                       kAudioUnitProperty_SetRenderCallback,
                                       kAudioUnitScope_Input, 0,
                                       &callbackStruct, sizeof(callbackStruct));
        if (status != noErr) {
            NSLog(@"BioBrain Audio: Failed to set render callback: %d", (int)status);
            AudioComponentInstanceDispose(impl_->audioUnit);
            impl_->audioUnit = nullptr;
            return false;
        }

        // Initialize and start
        status = AudioUnitInitialize(impl_->audioUnit);
        if (status != noErr) {
            NSLog(@"BioBrain Audio: Failed to initialize audio unit: %d", (int)status);
            AudioComponentInstanceDispose(impl_->audioUnit);
            impl_->audioUnit = nullptr;
            return false;
        }

        status = AudioOutputUnitStart(impl_->audioUnit);
        if (status != noErr) {
            NSLog(@"BioBrain Audio: Failed to start audio unit: %d", (int)status);
            AudioUnitUninitialize(impl_->audioUnit);
            AudioComponentInstanceDispose(impl_->audioUnit);
            impl_->audioUnit = nullptr;
            return false;
        }

        running_.store(true);
        NSLog(@"BioBrain Audio: Vocal synthesizer started (%.0f Hz, mono)", SAMPLE_RATE);
        return true;
    }
}

void VocalSynthesizer::stop() {
    if (!running_.load()) return;
    running_.store(false);

    if (impl_->audioUnit) {
        AudioOutputUnitStop(impl_->audioUnit);
        AudioUnitUninitialize(impl_->audioUnit);
        AudioComponentInstanceDispose(impl_->audioUnit);
        impl_->audioUnit = nullptr;
    }

    NSLog(@"BioBrain Audio: Vocal synthesizer stopped");
}

void VocalSynthesizer::updateFromPoolRates(const std::array<double, 6>& rates) {
    // Map pool firing rates (0-100 Hz) to synthesis parameters
    auto map = [](double rate, double min, double max) {
        double t = std::clamp(rate / 100.0, 0.0, 1.0);
        return min + t * (max - min);
    };

    setF1(map(rates[0], 250.0, 900.0));
    setF2(map(rates[1], 700.0, 2500.0));
    setF0(map(rates[2], 80.0, 300.0));
    setAmplitude(std::clamp(rates[3] / 100.0, 0.0, 1.0));
    setTilt(std::clamp(rates[4] / 100.0, 0.0, 1.0));
    setNoise(std::clamp(rates[5] / 100.0, 0.0, 1.0));
}

void VocalSynthesizer::setF1(double hz)      { f1_.store(std::clamp(hz, 250.0, 900.0)); }
void VocalSynthesizer::setF2(double hz)      { f2_.store(std::clamp(hz, 700.0, 2500.0)); }
void VocalSynthesizer::setF0(double hz)      { f0_.store(std::clamp(hz, 80.0, 300.0)); }
void VocalSynthesizer::setAmplitude(double a) { amplitude_.store(std::clamp(a, 0.0, 1.0)); }
void VocalSynthesizer::setTilt(double t)     { tilt_.store(std::clamp(t, 0.0, 1.0)); }
void VocalSynthesizer::setNoise(double n)    { noise_.store(std::clamp(n, 0.0, 1.0)); }

// ─── Audio synthesis ───────────────────────────────────────────────────────────

float VocalSynthesizer::generateSample() {
    double f0  = f0_.load();
    double f1  = f1_.load();
    double f2  = f2_.load();
    double amp = amplitude_.load();
    double tlt = tilt_.load();
    double nse = noise_.load();
    double vol = volume_.load();

    if (amp < 0.001) return 0.0f;  // silence

    // ── Glottal source: band-limited sawtooth approximation ──
    // Phase advances at F0
    phase_ += f0 / SAMPLE_RATE;
    if (phase_ >= 1.0) phase_ -= 1.0;

    // Rosenberg glottal pulse (more natural than raw sawtooth)
    // Open phase: 0 to 0.6, closed phase: 0.6 to 1.0
    double glottal;
    if (phase_ < 0.6) {
        double t = phase_ / 0.6;
        glottal = 3.0 * t * t - 2.0 * t * t * t;  // smooth opening
    } else {
        double t = (phase_ - 0.6) / 0.4;
        glottal = (1.0 - t);  // abrupt closing
    }
    glottal = glottal * 2.0 - 1.0;  // center around zero

    // Apply spectral tilt (low-pass filter, simulates breathiness)
    glottal *= (1.0 - tlt * 0.5);

    // ── Aspiration noise ──
    double noise_sample = (static_cast<double>(rand()) / RAND_MAX * 2.0 - 1.0);

    // Mix glottal + noise
    double source = glottal * (1.0 - nse) + noise_sample * nse;

    // ── Vocal tract formant filter (2 resonators in series) ──
    // Simple 2-pole resonator: H(z) = 1 / (1 - 2r*cos(w)*z^-1 + r^2*z^-2)
    auto resonator = [](double input, double freq, double bw,
                        double state[2], double sr) -> double {
        double w = 2.0 * M_PI * freq / sr;
        double r = std::exp(-M_PI * bw / sr);
        double a1 = -2.0 * r * std::cos(w);
        double a2 = r * r;
        double output = input - a1 * state[0] - a2 * state[1];
        state[1] = state[0];
        state[0] = output;
        return output * (1.0 - r);  // normalize gain
    };

    // F1 bandwidth ~80 Hz, F2 bandwidth ~120 Hz (typical values)
    double filtered = resonator(source, f1, 80.0, formant1_state_, SAMPLE_RATE);
    filtered = resonator(filtered, f2, 120.0, formant2_state_, SAMPLE_RATE);

    // Apply amplitude and master volume
    double output = filtered * amp * vol;

    // Soft clip to prevent harsh distortion
    output = std::tanh(output * 2.0) * 0.5;

    return static_cast<float>(output);
}

void VocalSynthesizer::renderCallback(void* userData, float* buffer, uint32_t numFrames) {
    auto* synth = static_cast<VocalSynthesizer*>(userData);
    for (uint32_t i = 0; i < numFrames; ++i) {
        buffer[i] = synth->generateSample();
    }
}
