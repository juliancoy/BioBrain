#include "regions/ITCortex.h"
#include "core/IzhikevichNeuron.h"
#include <random>

namespace biobrain {

std::shared_ptr<BrainRegion> ITCortex::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "ITCortex", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons: 24,000 RS excitatory + 6,000 FS inhibitory
    auto& neurons = region->neurons();
    neurons.reserve(NEURON_COUNT);

    for (uint32_t i = 0; i < EXCITATORY_COUNT; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::RegularSpiking);
        n->id = base_neuron_id + i;
        neurons.push_back(std::move(n));
    }
    for (uint32_t i = 0; i < INHIBITORY_COUNT; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::FastSpiking);
        n->id = base_neuron_id + EXCITATORY_COUNT + i;
        neurons.push_back(std::move(n));
    }

    // Internal wiring: same pattern as V2V4
    std::mt19937 rng(REGION_ID * 12345);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    auto& internal = region->internalSynapses();

    SynapseParams rs_rs_params{0.2, 0.5, ReceptorType::AMPA, false};
    SynapseParams rs_fs_params{0.4, 0.5, ReceptorType::AMPA, false};
    SynapseParams fs_rs_params{0.3, 1.0, ReceptorType::GABA_A, false};

    constexpr uint32_t RS_RS_FAN_OUT = 20;
    constexpr uint32_t RS_FS_FAN_OUT = 5;
    constexpr uint32_t FS_RS_FAN_OUT = 40;

    for (uint32_t i = 0; i < EXCITATORY_COUNT; ++i) {
        uint32_t pre = base_neuron_id + i;

        for (uint32_t t = 0; t < RS_RS_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * EXCITATORY_COUNT);
            if (j >= EXCITATORY_COUNT) j = EXCITATORY_COUNT - 1;
            if (j == i) continue;
            internal.emplace_back(pre, base_neuron_id + j, rs_rs_params);
        }

        for (uint32_t t = 0; t < RS_FS_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * INHIBITORY_COUNT);
            if (j >= INHIBITORY_COUNT) j = INHIBITORY_COUNT - 1;
            internal.emplace_back(pre, base_neuron_id + EXCITATORY_COUNT + j, rs_fs_params);
        }
    }

    for (uint32_t i = 0; i < INHIBITORY_COUNT; ++i) {
        uint32_t pre = base_neuron_id + EXCITATORY_COUNT + i;
        for (uint32_t t = 0; t < FS_RS_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * EXCITATORY_COUNT);
            if (j >= EXCITATORY_COUNT) j = EXCITATORY_COUNT - 1;
            internal.emplace_back(pre, base_neuron_id + j, fs_rs_params);
        }
    }

    // Projection to Striatum: IT -> D1 and D2 MSNs, relatively dense.
    // Striatum has 8000 D1 + 8000 D2 + 4000 FS = 20,000.
    // D1 MSNs: local indices 0..7999, D2 MSNs: 8000..15999.
    // Each IT excitatory neuron connects to ~3 D1 and ~3 D2 MSNs.
    SynapseParams striatum_params;
    striatum_params.weight = 0.3;
    striatum_params.delay = 5.0;
    striatum_params.receptor = ReceptorType::AMPA;

    std::vector<Synapse> striatum_projection;

    static constexpr uint32_t D1_COUNT = 8000;
    static constexpr uint32_t D2_COUNT = 8000;
    constexpr uint32_t FAN_OUT_D1 = 3;
    constexpr uint32_t FAN_OUT_D2 = 3;

    for (uint32_t i = 0; i < EXCITATORY_COUNT; ++i) {
        uint32_t pre = base_neuron_id + i;

        // IT -> D1 MSNs
        for (uint32_t t = 0; t < FAN_OUT_D1; ++t) {
            uint32_t post_local = static_cast<uint32_t>(dist(rng) * D1_COUNT);
            if (post_local >= D1_COUNT) post_local = D1_COUNT - 1;
            striatum_projection.emplace_back(pre, post_local, striatum_params);
        }

        // IT -> D2 MSNs
        for (uint32_t t = 0; t < FAN_OUT_D2; ++t) {
            uint32_t post_local = D1_COUNT + static_cast<uint32_t>(dist(rng) * D2_COUNT);
            if (post_local >= D1_COUNT + D2_COUNT) post_local = D1_COUNT + D2_COUNT - 1;
            striatum_projection.emplace_back(pre, post_local, striatum_params);
        }
    }

    region->addProjection(STRIATUM_REGION_ID, std::move(striatum_projection));

    return region;
}

} // namespace biobrain
