#include "regions/WernickesArea.h"
#include "core/IzhikevichNeuron.h"
#include "core/Synapse.h"
#include <random>

namespace biobrain {

std::shared_ptr<BrainRegion> WernickesArea::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "Wernicke", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons: 8000 RS excitatory + 2000 FS inhibitory
    auto& neurons = region->neurons();
    neurons.reserve(NEURON_COUNT);

    for (uint32_t i = 0; i < EXCITATORY; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::RegularSpiking);
        n->id = base_neuron_id + i;
        neurons.push_back(std::move(n));
    }
    for (uint32_t i = 0; i < INHIBITORY; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::FastSpiking);
        n->id = base_neuron_id + EXCITATORY + i;
        neurons.push_back(std::move(n));
    }

    // Internal wiring
    std::mt19937 rng(REGION_ID * 12345);
    auto& synapses = region->internalSynapses();

    // E→E recurrent (semantic attractor networks — Wernicke's has strong recurrence)
    for (uint32_t i = 0; i < EXCITATORY; ++i) {
        uint32_t pre = base_neuron_id + i;
        std::uniform_int_distribution<uint32_t> dist(0, EXCITATORY - 1);
        for (int j = 0; j < 15; ++j) {  // 15 recurrent connections per neuron
            uint32_t post = base_neuron_id + dist(rng);
            if (post == pre) continue;
            SynapseParams p{0.25, 0.5, ReceptorType::AMPA, false};
            synapses.emplace_back(pre, post, p);
        }
    }

    // E→I feedforward
    for (uint32_t i = 0; i < EXCITATORY; i += 10) {
        uint32_t pre = base_neuron_id + i;
        std::uniform_int_distribution<uint32_t> dist(0, INHIBITORY - 1);
        for (int j = 0; j < 3; ++j) {
            uint32_t post = base_neuron_id + EXCITATORY + dist(rng);
            SynapseParams p{0.4, 0.5, ReceptorType::AMPA, false};
            synapses.emplace_back(pre, post, p);
        }
    }

    // I→E feedback inhibition
    for (uint32_t i = 0; i < INHIBITORY; ++i) {
        uint32_t pre = base_neuron_id + EXCITATORY + i;
        std::uniform_int_distribution<uint32_t> dist(0, EXCITATORY - 1);
        for (int j = 0; j < 30; ++j) {  // broad inhibition
            uint32_t post = base_neuron_id + dist(rng);
            SynapseParams p{0.3, 1.0, ReceptorType::GABA_A, false};
            synapses.emplace_back(pre, post, p);
        }
    }

    // Projection to Broca's area (arcuate fasciculus — myelinated, 5ms delay)
    std::vector<Synapse> broca_proj;
    std::uniform_int_distribution<uint32_t> broca_dist(0, 5999);  // Broca's excitatory
    for (uint32_t i = 0; i < EXCITATORY; i += 4) {
        uint32_t pre = base_neuron_id + i;
        for (int j = 0; j < 3; ++j) {
            uint32_t post = broca_dist(rng);  // local index in Broca's
            SynapseParams p{0.35, 5.0, ReceptorType::AMPA, true};  // myelinated
            broca_proj.emplace_back(pre, post, p);
        }
    }
    region->addProjection(BROCA_REGION_ID, std::move(broca_proj));

    return region;
}

} // namespace biobrain
