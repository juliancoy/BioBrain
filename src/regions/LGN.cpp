#include "regions/LGN.h"
#include "core/IzhikevichNeuron.h"
#include <random>

namespace biobrain {

std::shared_ptr<BrainRegion> LGN::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "LGN", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons: 4000 relay (TonicSpiking) + 1000 interneurons (FastSpiking)
    auto& neurons = region->neurons();
    neurons.reserve(NEURON_COUNT);

    for (uint32_t i = 0; i < RELAY_COUNT; ++i) {
        auto neuron = IzhikevichNeuron::create(IzhikevichNeuron::Type::TonicSpiking);
        neuron->id = base_neuron_id + i;
        neurons.push_back(std::move(neuron));
    }
    for (uint32_t i = 0; i < INTERNEURON_COUNT; ++i) {
        auto neuron = IzhikevichNeuron::create(IzhikevichNeuron::Type::FastSpiking);
        neuron->id = base_neuron_id + RELAY_COUNT + i;
        neurons.push_back(std::move(neuron));
    }

    // Internal wiring: interneurons -> relay cells (feedforward inhibition)
    // GABA_A, weight 0.3, delay 1ms
    // Sparse connectivity: ~20% connection probability
    std::mt19937 rng(REGION_ID * 12345);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    constexpr double conn_prob = 0.20;

    SynapseParams inhibit_params;
    inhibit_params.weight = 0.3;
    inhibit_params.delay = 1.0;
    inhibit_params.receptor = ReceptorType::GABA_A;

    auto& internal = region->internalSynapses();

    for (uint32_t inh = 0; inh < INTERNEURON_COUNT; ++inh) {
        uint32_t pre_id = base_neuron_id + RELAY_COUNT + inh;
        // Each interneuron connects to a subset of relay cells
        // To keep O(n) not O(n^2), each interneuron connects to ~20 relay cells
        uint32_t target_count = static_cast<uint32_t>(RELAY_COUNT * conn_prob);
        for (uint32_t t = 0; t < target_count; ++t) {
            uint32_t relay_idx = static_cast<uint32_t>(dist(rng) * RELAY_COUNT);
            if (relay_idx >= RELAY_COUNT) relay_idx = RELAY_COUNT - 1;
            uint32_t post_id = base_neuron_id + relay_idx;
            internal.emplace_back(pre_id, post_id, inhibit_params);
        }
    }

    // Projection to V1: relay cells -> V1 layer 4, topographic with convergence.
    // AMPA, weight 0.4, delay 3ms (myelinated thalamocortical).
    // V1 has 80,000 neurons; layer 4 receives from LGN relay cells.
    // Each relay cell connects to ~20 V1 neurons (topographic neighborhood).
    SynapseParams v1_params;
    v1_params.weight = 0.4;
    v1_params.delay = 3.0;
    v1_params.receptor = ReceptorType::AMPA;
    v1_params.myelinated = true;

    std::vector<Synapse> v1_projection;
    // V1 has 1000 columns x 80 neurons. Relay cells map topographically.
    // 4000 relay -> 1000 columns: ~4 relay cells per column, each targets column RS neurons.
    static constexpr uint32_t V1_COLUMN_COUNT = 1000;
    static constexpr uint32_t V1_NEURONS_PER_COLUMN = 80;

    for (uint32_t r = 0; r < RELAY_COUNT; ++r) {
        uint32_t pre_id = base_neuron_id + r;
        // Map relay cell to target column in V1
        uint32_t target_col = (r * V1_COLUMN_COUNT) / RELAY_COUNT;
        // Connect to RS neurons in that column (first 60 neurons of the column)
        // Sparse: connect to ~5 RS neurons
        for (uint32_t t = 0; t < 5; ++t) {
            uint32_t rs_idx = static_cast<uint32_t>(dist(rng) * 60);
            if (rs_idx >= 60) rs_idx = 59;
            uint32_t post_local = target_col * V1_NEURONS_PER_COLUMN + rs_idx;
            v1_projection.emplace_back(pre_id, post_local, v1_params);
        }
    }

    region->addProjection(V1_REGION_ID, std::move(v1_projection));

    return region;
}

} // namespace biobrain
