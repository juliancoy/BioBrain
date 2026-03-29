#include "regions/Retina.h"
#include "core/IzhikevichNeuron.h"

namespace biobrain {

std::shared_ptr<BrainRegion> Retina::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "Retina", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons: all TonicSpiking retinal ganglion cells.
    // First 4096 = ON-center, next 4096 = OFF-center.
    auto& neurons = region->neurons();
    neurons.reserve(NEURON_COUNT);
    for (uint32_t i = 0; i < NEURON_COUNT; ++i) {
        auto neuron = IzhikevichNeuron::create(IzhikevichNeuron::Type::TonicSpiking);
        neuron->id = base_neuron_id + i;
        neurons.push_back(std::move(neuron));
    }

    // No internal connections in this retinal model.

    // Projection to LGN: topographic 1:1 mapping.
    // Each RGC connects to one LGN relay cell.
    // LGN has 4000 relay cells; we map 8192 RGCs -> 4000 relays with convergence ~2:1.
    // Use AMPA, weight 0.5, delay 2ms (myelinated optic nerve).
    // LGN base_neuron_id is not known here; projections use absolute neuron IDs.
    // We store projection synapses with post_id as offset within target region.
    // The Simulation layer resolves actual IDs based on region base offsets.
    std::vector<Synapse> lgn_projection;
    lgn_projection.reserve(NEURON_COUNT);

    SynapseParams params;
    params.weight = 0.5;
    params.delay = 2.0;
    params.receptor = ReceptorType::AMPA;
    params.myelinated = true;

    // Map each RGC to a corresponding LGN relay cell (modulo relay count).
    // LGN relay cells are indices 0..3999 within LGN.
    static constexpr uint32_t LGN_RELAY_COUNT = 4000;
    for (uint32_t i = 0; i < NEURON_COUNT; ++i) {
        uint32_t lgn_target_local = i % LGN_RELAY_COUNT;
        // Store pre_id as absolute, post_id as local offset in target region.
        // Simulation resolves post_id = target_base + lgn_target_local.
        lgn_projection.emplace_back(base_neuron_id + i, lgn_target_local, params);
    }

    region->addProjection(LGN_REGION_ID, std::move(lgn_projection));

    return region;
}

} // namespace biobrain
