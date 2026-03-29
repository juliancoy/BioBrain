#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <cstdint>
#include <unordered_set>

// Records spike events to HDF5 files for offline analysis.
// Thread-safe: multiple simulation threads can log spikes concurrently.
class SpikeRecorder {
public:
    explicit SpikeRecorder(const std::string& filepath = "spikes.h5");
    ~SpikeRecorder();

    // Enable/disable recording for a specific region
    void enableRegion(uint32_t region_id);
    void disableRegion(uint32_t region_id);
    bool isRegionEnabled(uint32_t region_id) const;

    // Record a spike event (thread-safe)
    void recordSpike(uint32_t neuron_id, uint32_t region_id, double time_ms);

    // Record a batch of spikes (thread-safe, more efficient)
    void recordSpikes(const std::vector<uint32_t>& neuron_ids,
                      uint32_t region_id,
                      const std::vector<double>& times_ms);

    // Flush buffered data to disk
    void flush();

    // Start/stop recording
    void start();
    void stop();
    bool isRecording() const;

    // Get total number of recorded spikes
    size_t totalSpikes() const;

private:
    struct SpikeEntry {
        uint32_t neuron_id;
        uint32_t region_id;
        double time_ms;
    };

    std::string filepath_;
    mutable std::mutex mutex_;
    std::vector<SpikeEntry> buffer_;
    std::unordered_set<uint32_t> enabled_regions_;
    bool recording_ = false;
    size_t total_spikes_ = 0;
    size_t flush_threshold_ = 10000;  // flush to disk every N spikes

    void flushToFile();
};
