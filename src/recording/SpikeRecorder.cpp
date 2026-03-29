#include "recording/SpikeRecorder.h"

#ifdef HAS_HDF5
#include <H5Cpp.h>
#endif

#include <algorithm>

SpikeRecorder::SpikeRecorder(const std::string& filepath)
    : filepath_(filepath) {
    buffer_.reserve(flush_threshold_ * 2);
}

SpikeRecorder::~SpikeRecorder() {
    if (recording_) {
        stop();
    }
}

void SpikeRecorder::enableRegion(uint32_t region_id) {
    std::lock_guard lock(mutex_);
    enabled_regions_.insert(region_id);
}

void SpikeRecorder::disableRegion(uint32_t region_id) {
    std::lock_guard lock(mutex_);
    enabled_regions_.erase(region_id);
}

bool SpikeRecorder::isRegionEnabled(uint32_t region_id) const {
    std::lock_guard lock(mutex_);
    return enabled_regions_.contains(region_id);
}

void SpikeRecorder::recordSpike(uint32_t neuron_id, uint32_t region_id, double time_ms) {
    std::lock_guard lock(mutex_);
    if (!recording_) return;
    if (!enabled_regions_.empty() && !enabled_regions_.contains(region_id)) return;

    buffer_.push_back({neuron_id, region_id, time_ms});
    total_spikes_++;

    if (buffer_.size() >= flush_threshold_) {
        flushToFile();
    }
}

void SpikeRecorder::recordSpikes(const std::vector<uint32_t>& neuron_ids,
                                  uint32_t region_id,
                                  const std::vector<double>& times_ms) {
    std::lock_guard lock(mutex_);
    if (!recording_) return;
    if (!enabled_regions_.empty() && !enabled_regions_.contains(region_id)) return;

    size_t count = std::min(neuron_ids.size(), times_ms.size());
    for (size_t i = 0; i < count; ++i) {
        buffer_.push_back({neuron_ids[i], region_id, times_ms[i]});
    }
    total_spikes_ += count;

    if (buffer_.size() >= flush_threshold_) {
        flushToFile();
    }
}

void SpikeRecorder::flush() {
    std::lock_guard lock(mutex_);
    if (!buffer_.empty()) {
        flushToFile();
    }
}

void SpikeRecorder::start() {
    std::lock_guard lock(mutex_);
    recording_ = true;
}

void SpikeRecorder::stop() {
    std::lock_guard lock(mutex_);
    recording_ = false;
    if (!buffer_.empty()) {
        flushToFile();
    }
}

bool SpikeRecorder::isRecording() const {
    std::lock_guard lock(mutex_);
    return recording_;
}

size_t SpikeRecorder::totalSpikes() const {
    std::lock_guard lock(mutex_);
    return total_spikes_;
}

void SpikeRecorder::flushToFile() {
    // Note: mutex already held by caller

    if (buffer_.empty()) return;

#ifdef HAS_HDF5
    try {
        // Open or create HDF5 file
        H5::H5File file(filepath_, H5F_ACC_CREAT | H5F_ACC_RDWR);

        // Prepare flat arrays
        size_t n = buffer_.size();
        std::vector<uint32_t> nids(n), rids(n);
        std::vector<double> times(n);
        for (size_t i = 0; i < n; ++i) {
            nids[i] = buffer_[i].neuron_id;
            rids[i] = buffer_[i].region_id;
            times[i] = buffer_[i].time_ms;
        }

        auto writeOrExtend = [&](const std::string& name, auto* data, const H5::DataType& type) {
            hsize_t dims[1] = {n};
            if (file.nameExists(name)) {
                // Extend existing dataset
                H5::DataSet ds = file.openDataSet(name);
                hsize_t existing[1];
                ds.getSpace().getSimpleExtentDims(existing);
                hsize_t newSize[1] = {existing[0] + n};
                ds.extend(newSize);

                H5::DataSpace fspace = ds.getSpace();
                hsize_t offset[1] = {existing[0]};
                hsize_t count[1] = {n};
                fspace.selectHyperslab(H5S_SELECT_SET, count, offset);
                H5::DataSpace mspace(1, dims);
                ds.write(data, type, mspace, fspace);
            } else {
                // Create new extensible dataset
                hsize_t maxDims[1] = {H5S_UNLIMITED};
                H5::DataSpace space(1, dims, maxDims);
                H5::DSetCreatPropList props;
                hsize_t chunk[1] = {10000};
                props.setChunk(1, chunk);
                H5::DataSet ds = file.createDataSet(name, type, space, props);
                ds.write(data, type);
            }
        };

        writeOrExtend("neuron_id", nids.data(), H5::PredType::NATIVE_UINT32);
        writeOrExtend("region_id", rids.data(), H5::PredType::NATIVE_UINT32);
        writeOrExtend("time_ms", times.data(), H5::PredType::NATIVE_DOUBLE);

    } catch (const H5::Exception& e) {
        // Silently drop if HDF5 write fails — don't crash the simulation
    }
#else
    // Fallback: write CSV if HDF5 not available
    FILE* f = fopen(filepath_.c_str(), "a");
    if (f) {
        for (auto& entry : buffer_) {
            fprintf(f, "%u,%u,%.4f\n", entry.neuron_id, entry.region_id, entry.time_ms);
        }
        fclose(f);
    }
#endif

    buffer_.clear();
}
