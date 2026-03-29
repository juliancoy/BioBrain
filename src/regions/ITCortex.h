#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Inferotemporal Cortex: 30,000 neurons.
/// 24,000 RS excitatory + 6,000 FS inhibitory.
/// Standard cortical E/I circuit internally.
/// Projects to Striatum (D1 and D2 MSNs).
class ITCortex {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT     = 30000;
    static constexpr uint32_t EXCITATORY_COUNT = 24000;
    static constexpr uint32_t INHIBITORY_COUNT = 6000;
    static constexpr uint32_t REGION_ID        = 4;

    static constexpr uint32_t STRIATUM_REGION_ID = 6;
};

} // namespace biobrain
