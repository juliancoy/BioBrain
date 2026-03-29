#include "regions/V1.h"
#include "core/IzhikevichNeuron.h"
#include <random>
#include <cmath>

namespace biobrain {

std::shared_ptr<BrainRegion> V1::create(uint32_t base_neuron_id) {
    auto region = std::make_shared<BrainRegion>(REGION_ID, "V1", NEURON_COUNT);
    region->setBaseNeuronId(base_neuron_id);

    // Create neurons organized in 1000 columns of 80 neurons each.
    // Per column layout (local indices within column):
    //   [0..59]  = RS (RegularSpiking) excitatory
    //   [60..69] = FS (FastSpiking) inhibitory
    //   [70..74] = IB (IntrinsicallyBursting) layer 5
    //   [75..79] = LTS (LowThresholdSpiking)
    auto& neurons = region->neurons();
    neurons.reserve(NEURON_COUNT);

    for (uint32_t col = 0; col < COLUMN_COUNT; ++col) {
        uint32_t col_base = base_neuron_id + col * NEURONS_PER_COLUMN;

        for (uint32_t i = 0; i < RS_PER_COLUMN; ++i) {
            auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::RegularSpiking);
            n->id = col_base + i;
            neurons.push_back(std::move(n));
        }
        for (uint32_t i = 0; i < FS_PER_COLUMN; ++i) {
            auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::FastSpiking);
            n->id = col_base + RS_PER_COLUMN + i;
            neurons.push_back(std::move(n));
        }
        for (uint32_t i = 0; i < IB_PER_COLUMN; ++i) {
            auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::IntrinsicallyBursting);
            n->id = col_base + RS_PER_COLUMN + FS_PER_COLUMN + i;
            neurons.push_back(std::move(n));
        }
        for (uint32_t i = 0; i < LTS_PER_COLUMN; ++i) {
            auto n = IzhikevichNeuron::create(IzhikevichNeuron::Type::LowThresholdSpiking);
            n->id = col_base + RS_PER_COLUMN + FS_PER_COLUMN + IB_PER_COLUMN + i;
            neurons.push_back(std::move(n));
        }
    }

    // Internal wiring
    std::mt19937 rng(REGION_ID * 12345);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    auto& internal = region->internalSynapses();

    // Synapse parameter templates
    SynapseParams rs_rs_params{0.3, 0.5, ReceptorType::AMPA, false};
    SynapseParams rs_fs_params{0.5, 0.5, ReceptorType::AMPA, false};
    SynapseParams fs_rs_params{0.4, 1.0, ReceptorType::GABA_A, false};
    SynapseParams cross_same_params{0.1, 2.0, ReceptorType::AMPA, false};
    SynapseParams cross_diff_params{0.2, 1.0, ReceptorType::GABA_A, false};

    // Each column has an orientation preference: col_idx % 8
    // Within-column connectivity (~20% probability, but we sample fixed counts for efficiency)

    constexpr double within_col_prob = 0.20;
    constexpr double cross_col_prob  = 0.02;

    // Within-column wiring for each column
    for (uint32_t col = 0; col < COLUMN_COUNT; ++col) {
        uint32_t col_base = base_neuron_id + col * NEURONS_PER_COLUMN;

        // RS -> RS within column (sparse ~20%)
        // Sample: each RS neuron connects to ~12 other RS neurons (60 * 0.2)
        for (uint32_t i = 0; i < RS_PER_COLUMN; ++i) {
            uint32_t pre = col_base + i;
            uint32_t n_targets = static_cast<uint32_t>(RS_PER_COLUMN * within_col_prob);
            for (uint32_t t = 0; t < n_targets; ++t) {
                uint32_t j = static_cast<uint32_t>(dist(rng) * RS_PER_COLUMN);
                if (j >= RS_PER_COLUMN) j = RS_PER_COLUMN - 1;
                if (j == i) continue; // no self-connections
                uint32_t post = col_base + j;
                internal.emplace_back(pre, post, rs_rs_params);
            }
        }

        // RS -> FS within column (~20%)
        for (uint32_t i = 0; i < RS_PER_COLUMN; ++i) {
            uint32_t pre = col_base + i;
            uint32_t n_targets = static_cast<uint32_t>(FS_PER_COLUMN * within_col_prob);
            for (uint32_t t = 0; t < n_targets; ++t) {
                uint32_t j = static_cast<uint32_t>(dist(rng) * FS_PER_COLUMN);
                if (j >= FS_PER_COLUMN) j = FS_PER_COLUMN - 1;
                uint32_t post = col_base + RS_PER_COLUMN + j;
                internal.emplace_back(pre, post, rs_fs_params);
            }
        }

        // FS -> RS within column (~20%)
        for (uint32_t i = 0; i < FS_PER_COLUMN; ++i) {
            uint32_t pre = col_base + RS_PER_COLUMN + i;
            uint32_t n_targets = static_cast<uint32_t>(RS_PER_COLUMN * within_col_prob);
            for (uint32_t t = 0; t < n_targets; ++t) {
                uint32_t j = static_cast<uint32_t>(dist(rng) * RS_PER_COLUMN);
                if (j >= RS_PER_COLUMN) j = RS_PER_COLUMN - 1;
                uint32_t post = col_base + j;
                internal.emplace_back(pre, post, fs_rs_params);
            }
        }
    }

    // Cross-column connectivity (~2% probability)
    // To avoid O(N^2) over all 1000 columns, each column samples a few partner columns.
    // Each RS neuron connects to ~2 RS neurons in ~20 other columns (1000 * 0.02).
    uint32_t cross_col_targets = static_cast<uint32_t>(COLUMN_COUNT * cross_col_prob);

    for (uint32_t col = 0; col < COLUMN_COUNT; ++col) {
        uint32_t col_base = base_neuron_id + col * NEURONS_PER_COLUMN;
        uint32_t col_orientation = col % ORIENTATION_COUNT;

        for (uint32_t c = 0; c < cross_col_targets; ++c) {
            uint32_t other_col = static_cast<uint32_t>(dist(rng) * COLUMN_COUNT);
            if (other_col >= COLUMN_COUNT) other_col = COLUMN_COUNT - 1;
            if (other_col == col) continue;

            uint32_t other_base = base_neuron_id + other_col * NEURONS_PER_COLUMN;
            uint32_t other_orientation = other_col % ORIENTATION_COUNT;

            if (col_orientation == other_orientation) {
                // Same orientation: weak RS -> RS excitation
                uint32_t pre_idx = static_cast<uint32_t>(dist(rng) * RS_PER_COLUMN);
                uint32_t post_idx = static_cast<uint32_t>(dist(rng) * RS_PER_COLUMN);
                if (pre_idx >= RS_PER_COLUMN) pre_idx = RS_PER_COLUMN - 1;
                if (post_idx >= RS_PER_COLUMN) post_idx = RS_PER_COLUMN - 1;
                internal.emplace_back(col_base + pre_idx,
                                      other_base + post_idx,
                                      cross_same_params);
            } else {
                // Different orientation: FS -> RS lateral inhibition
                uint32_t pre_fs = static_cast<uint32_t>(dist(rng) * FS_PER_COLUMN);
                uint32_t post_rs = static_cast<uint32_t>(dist(rng) * RS_PER_COLUMN);
                if (pre_fs >= FS_PER_COLUMN) pre_fs = FS_PER_COLUMN - 1;
                if (post_rs >= RS_PER_COLUMN) post_rs = RS_PER_COLUMN - 1;
                internal.emplace_back(col_base + RS_PER_COLUMN + pre_fs,
                                      other_base + post_rs,
                                      cross_diff_params);
            }
        }
    }

    // Projection to V2V4: RS cells -> V2V4, convergent.
    // Each V2V4 neuron receives from ~5 V1 columns.
    // Equivalently, each V1 RS cell projects to a few V2V4 neurons.
    // V2V4 has 60,000 neurons (48,000 excitatory).
    SynapseParams v2v4_params;
    v2v4_params.weight = 0.3;
    v2v4_params.delay = 3.0;
    v2v4_params.receptor = ReceptorType::AMPA;
    v2v4_params.myelinated = true;

    std::vector<Synapse> v2v4_projection;

    // Each V2V4 excitatory neuron (48,000) receives from 5 V1 columns.
    // Reverse: each V1 column projects to ~240 V2V4 neurons (48000*5/1000).
    // Sample: for each column, pick ~240 V2V4 targets and connect one RS neuron each.
    static constexpr uint32_t V2V4_EXC_COUNT = 48000;
    uint32_t targets_per_column = (V2V4_EXC_COUNT * 5) / COLUMN_COUNT;

    for (uint32_t col = 0; col < COLUMN_COUNT; ++col) {
        uint32_t col_base = base_neuron_id + col * NEURONS_PER_COLUMN;
        for (uint32_t t = 0; t < targets_per_column; ++t) {
            uint32_t pre_rs = static_cast<uint32_t>(dist(rng) * RS_PER_COLUMN);
            if (pre_rs >= RS_PER_COLUMN) pre_rs = RS_PER_COLUMN - 1;
            uint32_t post_local = static_cast<uint32_t>(dist(rng) * V2V4_EXC_COUNT);
            if (post_local >= V2V4_EXC_COUNT) post_local = V2V4_EXC_COUNT - 1;
            v2v4_projection.emplace_back(col_base + pre_rs, post_local, v2v4_params);
        }
    }

    region->addProjection(V2V4_REGION_ID, std::move(v2v4_projection));

    return region;
}

} // namespace biobrain
