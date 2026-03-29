#pragma once

#include "core/BrainRegion.h"
#include <memory>

namespace biobrain {

/// Broca's area (inferior frontal gyrus, pars opercularis + triangularis):
/// Speech production and motor planning for language output.
///
/// Receives semantic/phonological input from Wernicke's area via the
/// arcuate fasciculus. Transforms linguistic representations into
/// motor commands for vocalization.
///
/// Damage causes expressive aphasia — understanding intact but
/// cannot produce fluent speech.
///
/// For BioBrain, Broca's output neurons drive the vocal synthesizer:
///   - 6 population pools (one per formant/pitch parameter)
///   - Population firing rates decode to audio synthesis parameters
///   - Pool 0-1: F1/F2 formant frequencies (vowel identity)
///   - Pool 2: F0 fundamental frequency (pitch)
///   - Pool 3: amplitude (volume)
///   - Pool 4-5: spectral tilt and noise (consonant-like features)
///
/// Neural organization:
///   - 6,000 RS excitatory (including 6 output pools of 500 each = 3000)
///   - 2,000 FS inhibitory
class BrocasArea {
public:
    static std::shared_ptr<BrainRegion> create(uint32_t base_neuron_id);

    static constexpr uint32_t NEURON_COUNT   = 8000;
    static constexpr uint32_t EXCITATORY     = 6000;
    static constexpr uint32_t INHIBITORY     = 2000;
    static constexpr uint32_t REGION_ID      = 9;

    // Output pools for vocal synthesis (first 3000 excitatory neurons)
    static constexpr uint32_t OUTPUT_POOLS   = 6;
    static constexpr uint32_t POOL_SIZE      = 500;

    // Pool indices
    static constexpr uint32_t POOL_F1        = 0;  // first formant
    static constexpr uint32_t POOL_F2        = 1;  // second formant
    static constexpr uint32_t POOL_F0        = 2;  // fundamental frequency
    static constexpr uint32_t POOL_AMP       = 3;  // amplitude
    static constexpr uint32_t POOL_TILT      = 4;  // spectral tilt
    static constexpr uint32_t POOL_NOISE     = 5;  // noise/aspiration
};

} // namespace biobrain
