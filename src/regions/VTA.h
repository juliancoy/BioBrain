#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Ventral Tegmental Area: 2,000 neurons.
/// 1,500 dopaminergic (Izhikevich Dopaminergic) + 500 GABAergic (FastSpiking).
/// Internal: GABA interneurons inhibit DA neurons.
/// Projection to Striatum: diffuse dopaminergic (placeholder AMPA synapses).
class VTA {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT     = 2000;
    static constexpr uint32_t DA_COUNT         = 1500;
    static constexpr uint32_t GABA_COUNT       = 500;
    static constexpr uint32_t REGION_ID        = 5;

    static constexpr uint32_t STRIATUM_REGION_ID = 6;
};

} // namespace biobrain
