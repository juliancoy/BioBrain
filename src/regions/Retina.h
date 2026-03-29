#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Retina: 8,192 retinal ganglion cells (4096 ON-center + 4096 OFF-center).
/// All neurons: Izhikevich TonicSpiking type.
/// No internal connections. Projects topographically 1:1 to LGN.
class Retina {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT    = 8192;
    static constexpr uint32_t ON_CENTER_COUNT = 4096;
    static constexpr uint32_t OFF_CENTER_COUNT = 4096;
    static constexpr uint32_t REGION_ID       = 0;

    // Projection target region IDs
    static constexpr uint32_t LGN_REGION_ID   = 1;
};

} // namespace biobrain
