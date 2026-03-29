#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Motor Cortex: 5,000 neurons.
/// 4,000 RS excitatory + 1,000 FS inhibitory.
/// Standard cortical E/I circuit internally.
/// No outgoing projections (terminal output region).
/// Action decoding via population-coded output.
class MotorCortex {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT     = 5000;
    static constexpr uint32_t EXCITATORY_COUNT = 4000;
    static constexpr uint32_t INHIBITORY_COUNT = 1000;
    static constexpr uint32_t REGION_ID        = 7;
};

} // namespace biobrain
