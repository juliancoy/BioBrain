#pragma once

#include <cstdint>
#include <vector>
// Simple span-like view for C++17 compatibility (std::span is C++20)
template<typename T>
class Span {
    const T* data_ = nullptr;
    size_t size_ = 0;
public:
    Span() = default;
    Span(const T* data, size_t size) : data_(data), size_(size) {}
    template<typename Container>
    Span(const Container& c) : data_(c.data()), size_(c.size()) {}
    const T* data() const { return data_; }
    size_t size() const { return size_; }
    const T& operator[](size_t i) const { return data_[i]; }
    bool empty() const { return size_ == 0; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }
};

// Forward declarations
namespace biobrain { class BrainRegion; }
using biobrain::BrainRegion;

// Results from a batch neuron update
struct UpdateResult {
    std::vector<uint32_t> spiked_neuron_ids;
    std::vector<double> spike_times;
};

// Abstract interface for neuron computation backends.
// Each BrainRegion holds a ComputeBackend pointer that can be swapped at runtime.
class ComputeBackend {
public:
    virtual ~ComputeBackend() = default;

    // Update all neurons in a region for one timestep.
    // I_syn: synaptic currents indexed by local neuron index.
    // Returns IDs of neurons that spiked.
    virtual UpdateResult updateNeurons(BrainRegion& region, double dt,
                                       Span<const double> I_syn) = 0;

    // Name for UI display
    virtual const char* name() const = 0;
};

enum class ComputeBackendType { CPUEventDriven, MetalGPU, HybridAuto };
