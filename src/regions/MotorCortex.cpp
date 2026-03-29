#include "regions/MotorCortex.h"
#include "core/IzhikevichNeuron.h"
#include <random>

namespace biobrain {

std::shared_ptr<BrainRegion> MotorCortex::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "MotorCortex", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons: 4,000 RS excitatory + 1,000 FS inhibitory
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

    // Internal wiring: standard cortical E/I circuit
    std::mt19937 rng(REGION_ID * 12345);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    auto& internal = region->internalSynapses();

    SynapseParams rs_rs_params{0.2, 0.5, ReceptorType::AMPA, false};
    SynapseParams rs_fs_params{0.4, 0.5, ReceptorType::AMPA, false};
    SynapseParams fs_rs_params{0.3, 1.0, ReceptorType::GABA_A, false};

    constexpr uint32_t RS_RS_FAN_OUT = 20;
    constexpr uint32_t RS_FS_FAN_OUT = 5;
    constexpr uint32_t FS_RS_FAN_OUT = 40;

    // RS -> RS
    for (uint32_t i = 0; i < EXCITATORY_COUNT; ++i) {
        uint32_t pre = base_neuron_id + i;

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

    // No outgoing projections — Motor Cortex is the terminal output region.
    // Action selection is performed by reading out population activity externally.

    return region;
}

} // namespace biobrain
