#pragma once

#include <atomic>
#include <array>
#include <mutex>
#include <cstdint>

// Formant-based vocal synthesizer driven by Broca's area output pools.
// Uses CoreAudio for real-time audio output on macOS.
//
// The synthesizer models simplified vocal tract physics:
// - Glottal source: sawtooth-like pulse at F0 (fundamental frequency)
// - Vocal tract filter: 2-formant resonator (F1, F2)
// - Aspiration noise for unvoiced sounds
//
// Six control parameters (one per Broca's output pool):
//   Pool 0 → F1:    first formant (250-900 Hz, vowel height)
//   Pool 1 → F2:    second formant (700-2500 Hz, vowel frontness)
//   Pool 2 → F0:    fundamental frequency (80-300 Hz, pitch)
//   Pool 3 → Amp:   overall amplitude (0-1)
//   Pool 4 → Tilt:  spectral tilt (breathiness, 0-1)
//   Pool 5 → Noise: aspiration noise mix (0-1)
//
// Biological mapping: firing rate of each pool (0-100 Hz) maps linearly
// to the parameter range. Silent pool = parameter at minimum.
class VocalSynthesizer {
public:
    VocalSynthesizer();
    ~VocalSynthesizer();

    bool start();
    void stop();
    bool isRunning() const { return running_.load(); }

    // Update synthesis parameters from Broca's area pool firing rates.
    // rates[0..5] in Hz (0-100), mapped to synthesis params.
    void updateFromPoolRates(const std::array<double, 6>& rates);

    // Direct parameter access (thread-safe)
    void setF1(double hz);      // 250-900
    void setF2(double hz);      // 700-2500
    void setF0(double hz);      // 80-300
    void setAmplitude(double a); // 0-1
    void setTilt(double t);     // 0-1
    void setNoise(double n);    // 0-1

    // Current parameter values
    double f1() const { return f1_.load(); }
    double f2() const { return f2_.load(); }
    double f0() const { return f0_.load(); }
    double amplitude() const { return amplitude_.load(); }

    // Master volume (user-adjustable)
    void setVolume(double v) { volume_.store(v); }
    double volume() const { return volume_.load(); }

    static constexpr double SAMPLE_RATE = 44100.0;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    std::atomic<bool> running_{false};

    // Synthesis parameters (atomic for lock-free audio thread access)
    std::atomic<double> f1_{350.0};     // first formant
    std::atomic<double> f2_{1200.0};    // second formant
    std::atomic<double> f0_{120.0};     // fundamental
    std::atomic<double> amplitude_{0.0}; // starts silent
    std::atomic<double> tilt_{0.3};
    std::atomic<double> noise_{0.0};
    std::atomic<double> volume_{0.5};   // master volume

    // Internal state for audio callback
    double phase_ = 0.0;           // glottal source phase
    double formant1_state_[2] = {}; // biquad filter state
    double formant2_state_[2] = {}; // biquad filter state

    // Generate one sample of audio
    float generateSample();

public:
    // Audio render callback (called from CoreAudio thread)
    static void renderCallback(void* userData, float* buffer, uint32_t numFrames);

private:
};
