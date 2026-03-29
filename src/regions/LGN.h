#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Lateral Geniculate Nucleus: 5,000 neurons.
/// 4,000 relay cells (TonicSpiking) + 1,000 interneurons (FastSpiking).
/// Internal: interneurons provide feedforward inhibition to relay cells.
/// Projects topographically to V1 layer 4.
class LGN {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT     = 5000;
    static constexpr uint32_t RELAY_COUNT      = 4000;
    static constexpr uint32_t INTERNEURON_COUNT = 1000;
    static constexpr uint32_t REGION_ID        = 1;

    static constexpr uint32_t V1_REGION_ID     = 2;
};

} // namespace biobrain
