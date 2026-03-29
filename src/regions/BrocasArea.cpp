#include "regions/BrocasArea.h"
#include "core/IzhikevichNeuron.h"
#include "core/Synapse.h"
#include <random>

namespace biobrain {

std::shared_ptr<BrainRegion> BrocasArea::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "Broca", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    auto& neurons = region->neurons();
    neurons.reserve(NEURON_COUNT);

    // 6000 RS excitatory (first 3000 are output pool neurons)
    for (uint32_t i = 0; i < EXCITATORY; ++i) {
        // Output pool neurons use Intrinsically Bursting type for rhythmic output
        auto type = (i < OUTPUT_POOLS * POOL_SIZE)
            ? IzhikevichNeuron::Type::IntrinsicallyBursting
            : IzhikevichNeuron::Type::RegularSpiking;
        auto n = IzhikevichNeuron::create(type);
        n->id = base_neuron_id + i;
        neurons.push_back(std::move(n));
    }

    // 2000 FS inhibitory
    for (uint32_t i = 0; i < INHIBITORY; ++i) {
        auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::FastSpiking);
        n->id = base_neuron_id + EXCITATORY + i;
        neurons.push_back(std::move(n));
    }

    // Internal wiring
    std::mt19937 rng(REGION_ID * 12345);
    auto& synapses = region->internalSynapses();

    // Within each output pool: strong recurrent excitation (for sustained activity)
    for (uint32_t pool = 0; pool < OUTPUT_POOLS; ++pool) {
        uint32_t pool_start = base_neuron_id + pool * POOL_SIZE;
        for (uint32_t i = 0; i < POOL_SIZE; ++i) {
            uint32_t pre = pool_start + i;
            std::uniform_int_distribution<uint32_t> dist(0, POOL_SIZE - 1);
            for (int j = 0; j < 10; ++j) {
                uint32_t post = pool_start + dist(rng);
                if (post == pre) continue;
                SynapseParams p{0.3, 0.5, ReceptorType::AMPA, false};
                synapses.emplace_back(pre, post, p);
            }
        }
    }

    // Between pools: lateral inhibition (mutual exclusion for distinct phonemes)
    for (uint32_t pool_a = 0; pool_a < OUTPUT_POOLS; ++pool_a) {
        for (uint32_t pool_b = 0; pool_b < OUTPUT_POOLS; ++pool_b) {
            if (pool_a == pool_b) continue;
            // Sparse cross-pool inhibition via interneurons
            for (int k = 0; k < 50; ++k) {
                uint32_t pre = base_neuron_id + pool_a * POOL_SIZE + (rng() % POOL_SIZE);
                uint32_t inh = base_neuron_id + EXCITATORY + (rng() % INHIBITORY);
                uint32_t post = base_neuron_id + pool_b * POOL_SIZE + (rng() % POOL_SIZE);
                SynapseParams p1{0.3, 0.5, ReceptorType::AMPA, false};
                SynapseParams p2{0.25, 1.0, ReceptorType::GABA_A, false};
                synapses.emplace_back(pre, inh, p1);
                synapses.emplace_back(inh, post, p2);
            }
        }
    }

    // Non-pool E→I and I→E (general background inhibition)
    for (uint32_t i = OUTPUT_POOLS * POOL_SIZE; i < EXCITATORY; ++i) {
        uint32_t pre = base_neuron_id + i;
        std::uniform_int_distribution<uint32_t> dist(0, INHIBITORY - 1);
        for (int j = 0; j < 5; ++j) {
            uint32_t post = base_neuron_id + EXCITATORY + dist(rng);
            SynapseParams p{0.4, 0.5, ReceptorType::AMPA, false};
            synapses.emplace_back(pre, post, p);
        }
    }
    for (uint32_t i = 0; i < INHIBITORY; ++i) {
        uint32_t pre = base_neuron_id + EXCITATORY + i;
        std::uniform_int_distribution<uint32_t> dist(0, EXCITATORY - 1);
        for (int j = 0; j < 20; ++j) {
            uint32_t post = base_neuron_id + dist(rng);
            SynapseParams p{0.3, 1.0, ReceptorType::GABA_A, false};
            synapses.emplace_back(pre, post, p);
        }
    }

    // No outgoing projections — Broca's output is read by the vocal synthesizer
    return region;
}

} // namespace biobrain
