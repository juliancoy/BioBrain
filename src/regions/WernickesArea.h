#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Wernicke's area (posterior superior temporal gyrus):
/// Language comprehension region. Receives visual object representations
/// from IT cortex and transforms them into semantic/phonological codes.
///
/// In the biological brain, Wernicke's area is where visual input
/// ("I see a face") becomes a linguistic concept ("person").
/// Damage here causes receptive aphasia — fluent but meaningless speech.
///
/// Neural organization:
///   - 8,000 RS excitatory (semantic encoding neurons)
///   - 2,000 FS inhibitory (lateral inhibition for category selectivity)
///   - Receives convergent input from IT cortex
///   - Projects to Broca's area via arcuate fasciculus (myelinated, 5ms delay)
class WernickesArea {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT = 10000;
    static constexpr uint32_t EXCITATORY   = 8000;
    static constexpr uint32_t INHIBITORY   = 2000;
    static constexpr uint32_t REGION_ID    = 8;

    static constexpr uint32_t IT_REGION_ID    = 4;  // input from
    static constexpr uint32_t BROCA_REGION_ID = 9;  // projects to
};

} // namespace biobrain
