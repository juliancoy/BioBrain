#include "regions/Striatum.h"
#include "core/IzhikevichNeuron.h"
#include <random>

namespace biobrain {

std::shared_ptr<BrainRegion> Striatum::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "Striatum", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons:
    //   [0..7999]       = D1 MSNs (Go pathway)
    //   [8000..15999]   = D2 MSNs (NoGo pathway)
    //   [16000..19999]  = FS interneurons
    auto& neurons = region->neurons();
    neurons.reserve(NEURON_COUNT);

    for (uint32_t i = 0; i < D1_COUNT; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::MediumSpinyD1);
        n->id = base_neuron_id + i;
        neurons.push_back(std::move(n));
    }
    for (uint32_t i = 0; i < D2_COUNT; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::MediumSpinyD2);
        n->id = base_neuron_id + D1_COUNT + i;
        neurons.push_back(std::move(n));
    }
    for (uint32_t i = 0; i < INTERNEURON_COUNT; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::FastSpiking);
        n->id = base_neuron_id + D1_COUNT + D2_COUNT + i;
        neurons.push_back(std::move(n));
    }

    // Internal wiring
    std::mt19937 rng(REGION_ID * 12345);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    auto& internal = region->internalSynapses();

    SynapseParams lateral_params{0.2, 1.0, ReceptorType::GABA_A, false};
    SynapseParams ff_inhibit_params{0.4, 1.0, ReceptorType::GABA_A, false};

    // D1 -> D1 lateral inhibition: each D1 MSN connects to ~20 other D1 MSNs
    constexpr uint32_t LATERAL_FAN_OUT = 20;
    constexpr uint32_t FS_FAN_OUT = 40;

    for (uint32_t i = 0; i < D1_COUNT; ++i) {
        uint32_t pre = base_neuron_id + i;
        for (uint32_t t = 0; t < LATERAL_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * D1_COUNT);
            if (j >= D1_COUNT) j = D1_COUNT - 1;
            if (j == i) continue;
            internal.emplace_back(pre, base_neuron_id + j, lateral_params);
        }
    }

    // D2 -> D2 lateral inhibition: each D2 MSN connects to ~20 other D2 MSNs
    for (uint32_t i = 0; i < D2_COUNT; ++i) {
        uint32_t pre = base_neuron_id + D1_COUNT + i;
        for (uint32_t t = 0; t < LATERAL_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * D2_COUNT);
            if (j >= D2_COUNT) j = D2_COUNT - 1;
            if (j == i) continue;
            internal.emplace_back(pre, base_neuron_id + D1_COUNT + j, lateral_params);
        }
    }

    // FS -> D1 feedforward inhibition
    for (uint32_t i = 0; i < INTERNEURON_COUNT; ++i) {
        uint32_t pre = base_neuron_id + D1_COUNT + D2_COUNT + i;
        for (uint32_t t = 0; t < FS_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * D1_COUNT);
            if (j >= D1_COUNT) j = D1_COUNT - 1;
            internal.emplace_back(pre, base_neuron_id + j, ff_inhibit_params);
        }
    }

    // FS -> D2 feedforward inhibition
    for (uint32_t i = 0; i < INTERNEURON_COUNT; ++i) {
        uint32_t pre = base_neuron_id + D1_COUNT + D2_COUNT + i;
        for (uint32_t t = 0; t < FS_FAN_OUT; ++t) {
            uint32_t j = static_cast<uint32_t>(dist(rng) * D2_COUNT);
            if (j >= D2_COUNT) j = D2_COUNT - 1;
            internal.emplace_back(pre, base_neuron_id + D1_COUNT + j, ff_inhibit_params);
        }
    }

    // Projection to Motor:
    // D1 MSNs -> Motor (AMPA, weight 0.3, delay 3ms) - excitatory via disinhibition
    // D2 MSNs -> Motor (GABA_A, weight 0.3, delay 3ms) - inhibitory
    SynapseParams d1_motor_params{0.3, 3.0, ReceptorType::AMPA, false};
    SynapseParams d2_motor_params{0.3, 3.0, ReceptorType::GABA_A, false};

    std::vector<Synapse> motor_projection;

    static constexpr uint32_t MOTOR_EXC_COUNT = 4000;
    constexpr uint32_t D1_MOTOR_FAN_OUT = 3;
    constexpr uint32_t D2_MOTOR_FAN_OUT = 3;

    // D1 -> Motor (excitatory path)
    for (uint32_t i = 0; i < D1_COUNT; ++i) {
        uint32_t pre = base_neuron_id + i;
        for (uint32_t t = 0; t < D1_MOTOR_FAN_OUT; ++t) {
            uint32_t post_local = static_cast<uint32_t>(dist(rng) * MOTOR_EXC_COUNT);
            if (post_local >= MOTOR_EXC_COUNT) post_local = MOTOR_EXC_COUNT - 1;
            motor_projection.emplace_back(pre, post_local, d1_motor_params);
        }
    }

    // D2 -> Motor (inhibitory path)
    for (uint32_t i = 0; i < D2_COUNT; ++i) {
        uint32_t pre = base_neuron_id + D1_COUNT + i;
        for (uint32_t t = 0; t < D2_MOTOR_FAN_OUT; ++t) {
            uint32_t post_local = static_cast<uint32_t>(dist(rng) * MOTOR_EXC_COUNT);
            if (post_local >= MOTOR_EXC_COUNT) post_local = MOTOR_EXC_COUNT - 1;
            motor_projection.emplace_back(pre, post_local, d2_motor_params);
        }
    }

    region->addProjection(MOTOR_REGION_ID, std::move(motor_projection));

    return region;
}

} // namespace biobrain
