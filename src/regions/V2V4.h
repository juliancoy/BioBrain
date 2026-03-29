#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// V2/V4 visual areas: 60,000 neurons.
/// 48,000 RS excitatory + 12,000 FS inhibitory.
/// Standard cortical E/I circuit internally.
/// Projects convergently to IT cortex.
class V2V4 {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT     = 60000;
    static constexpr uint32_t EXCITATORY_COUNT = 48000;
    static constexpr uint32_t INHIBITORY_COUNT = 12000;
    static constexpr uint32_t REGION_ID        = 3;

    static constexpr uint32_t IT_REGION_ID     = 4;
};

} // namespace biobrain
