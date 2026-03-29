#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Primary Visual Cortex (V1): 80,000 neurons organized in 1000 cortical columns.
/// Per column: 60 RS + 10 FS + 5 IB (layer 5) + 5 LTS = 80 neurons.
/// 8 orientation preferences distributed across columns.
/// Complex within-column and cross-column wiring patterns.
/// Projects to V2/V4.
class V1 {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT       = 80000;
    static constexpr uint32_t COLUMN_COUNT       = 1000;
    static constexpr uint32_t NEURONS_PER_COLUMN = 80;
    static constexpr uint32_t RS_PER_COLUMN      = 60;
    static constexpr uint32_t FS_PER_COLUMN      = 10;
    static constexpr uint32_t IB_PER_COLUMN      = 5;
    static constexpr uint32_t LTS_PER_COLUMN     = 5;
    static constexpr uint32_t ORIENTATION_COUNT  = 8;
    static constexpr uint32_t REGION_ID          = 2;

    static constexpr uint32_t V2V4_REGION_ID     = 3;
};

} // namespace biobrain
