#include "regions/VTA.h"
#include "core/IzhikevichNeuron.h"
#include <random>

namespace biobrain {

std::shared_ptr<BrainRegion> VTA::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "VTA", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons: 1,500 dopaminergic + 500 GABAergic (FastSpiking)
    auto& neurons = region->neurons();
    neurons.reserve(NEURON_COUNT);

    for (uint32_t i = 0; i < DA_COUNT; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::Dopaminergic);
        n->id = base_neuron_id + i;
        neurons.push_back(std::move(n));
    }
    for (uint32_t i = 0; i < GABA_COUNT; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::FastSpiking);
        n->id = base_neuron_id + DA_COUNT + i;
        neurons.push_back(std::move(n));
    }

    // Internal: GABA interneurons inhibit DA neurons.
    // GABA_A, weight 0.5, delay 1ms, ~20% connectivity.
    std::mt19937 rng(REGION_ID * 12345);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    auto& internal = region->internalSynapses();

    SynapseParams inhibit_params;
    inhibit_params.weight = 0.5;
    inhibit_params.delay = 1.0;
    inhibit_params.receptor = ReceptorType::GABA_A;

    // Each GABA interneuron connects to ~20% of DA neurons = ~300 DA neurons
    constexpr uint32_t GABA_DA_FAN_OUT = 300;

    for (uint32_t i = 0; i < GABA_COUNT; ++i) {
        uint32_t pre = base_neuron_id + DA_COUNT + i;
        for (uint32_t t = 0; t < GABA_DA_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * DA_COUNT);
            if (j >= DA_COUNT) j = DA_COUNT - 1;
            internal.emplace_back(pre, base_neuron_id + j, inhibit_params);
        }
    }

    // Projection to Striatum: diffuse dopaminergic projection.
    // DA neurons connect broadly to Striatum neurons.
    // Placeholder: AMPA synapses, weight 0.1, delay 5ms.
    // Each DA neuron connects to ~50 striatal neurons (diffuse).
    SynapseParams da_params;
    da_params.weight = 0.1;
    da_params.delay = 5.0;
    da_params.receptor = ReceptorType::AMPA;

    std::vector<Synapse> striatum_projection;

    static constexpr uint32_t STRIATUM_NEURON_COUNT = 20000;
    constexpr uint32_t DA_FAN_OUT = 50;

    for (uint32_t i = 0; i < DA_COUNT; ++i) {
        uint32_t pre = base_neuron_id + i;
        for (uint32_t t = 0; t < DA_FAN_OUT; ++t) {
            uint32_t post_local = static_cast<uint32_t>(dist(rng) * STRIATUM_NEURON_COUNT);
            if (post_local >= STRIATUM_NEURON_COUNT) post_local = STRIATUM_NEURON_COUNT - 1;
            striatum_projection.emplace_back(pre, post_local, da_params);
        }
    }

    region->addProjection(STRIATUM_REGION_ID, std::move(striatum_projection));

    return region;
}

} // namespace biobrain
