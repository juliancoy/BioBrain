#include "regions/V2V4.h"
#include "core/IzhikevichNeuron.h"
#include <random>

namespace biobrain {

std::shared_ptr<BrainRegion> V2V4::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "V2V4", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons: 48,000 RS excitatory + 12,000 FS inhibitory
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

    // Internal wiring: standard cortical E/I circuit (sparse)
    std::mt19937 rng(REGION_ID * 12345);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    auto& internal = region->internalSynapses();

    SynapseParams rs_rs_params{0.2, 0.5, ReceptorType::AMPA, false};
    SynapseParams rs_fs_params{0.4, 0.5, ReceptorType::AMPA, false};
    SynapseParams fs_rs_params{0.3, 1.0, ReceptorType::GABA_A, false};

    // For a region this large, full random connectivity is too expensive.
    // Use a neighborhood-based approach: each neuron connects to a fixed number of targets.
    // RS -> RS: each RS neuron connects to ~20 other RS neurons
    constexpr uint32_t RS_RS_FAN_OUT = 20;
    constexpr uint32_t RS_FS_FAN_OUT = 5;
    constexpr uint32_t FS_RS_FAN_OUT = 40;

    for (uint32_t i = 0; i < EXCITATORY_COUNT; ++i) {
        uint32_t pre = base_neuron_id + i;

        // RS -> RS
        for (uint32_t t = 0; t < RS_RS_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * EXCITATORY_COUNT);
            if (j >= EXCITATORY_COUNT) j = EXCITATORY_COUNT - 1;
            if (j == i) continue;
            internal.emplace_back(pre, base_neuron_id + j, rs_rs_params);
        }

        // RS -> FS
        for (uint32_t t = 0; t < RS_FS_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * INHIBITORY_COUNT);
            if (j >= INHIBITORY_COUNT) j = INHIBITORY_COUNT - 1;
            internal.emplace_back(pre, base_neuron_id + EXCITATORY_COUNT + j, rs_fs_params);
        }
    }

    // FS -> RS
    for (uint32_t i = 0; i < INHIBITORY_COUNT; ++i) {
        uint32_t pre = base_neuron_id + EXCITATORY_COUNT + i;
        for (uint32_t t = 0; t < FS_RS_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * EXCITATORY_COUNT);
            if (j >= EXCITATORY_COUNT) j = EXCITATORY_COUNT - 1;
            internal.emplace_back(pre, base_neuron_id + j, fs_rs_params);
        }
    }

    // Projection to IT: convergent, each IT neuron receives from ~10 V2V4 neurons.
    // IT has 30,000 neurons (24,000 excitatory).
    // Equivalently, each V2V4 RS neuron projects to ~5 IT excitatory neurons
    // (24000 * 10 / 48000 = 5).
    SynapseParams it_params;
    it_params.weight = 0.3;
    it_params.delay = 4.0;
    it_params.receptor = ReceptorType::AMPA;

    std::vector<Synapse> it_projection;

    static constexpr uint32_t IT_EXC_COUNT = 24000;
    constexpr uint32_t FAN_OUT_TO_IT = 5;

    for (uint32_t i = 0; i < EXCITATORY_COUNT; ++i) {
        uint32_t pre = base_neuron_id + i;
        for (uint32_t t = 0; t < FAN_OUT_TO_IT; ++t) {
            uint32_t post_local = static_cast<uint32_t>(dist(rng) * IT_EXC_COUNT);
            if (post_local >= IT_EXC_COUNT) post_local = IT_EXC_COUNT - 1;
            it_projection.emplace_back(pre, post_local, it_params);
        }
    }

    region->addProjection(IT_REGION_ID, std::move(it_projection));

    return region;
}

} // namespace biobrain
