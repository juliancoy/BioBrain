#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Striatum: 20,000 neurons.
/// 8,000 D1 MSNs (Go pathway) + 8,000 D2 MSNs (NoGo pathway) + 4,000 FS interneurons.
/// Internal: lateral inhibition within D1/D2 populations, FS feedforward inhibition.
/// Projection: D1 -> Motor (AMPA, disinhibition), D2 -> Motor (GABA_A, inhibition).
class Striatum {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT     = 20000;
    static constexpr uint32_t D1_COUNT         = 8000;
    static constexpr uint32_t D2_COUNT         = 8000;
    static constexpr uint32_t INTERNEURON_COUNT = 4000;
    static constexpr uint32_t REGION_ID        = 6;

    static constexpr uint32_t MOTOR_REGION_ID  = 7;
};

} // namespace biobrain
