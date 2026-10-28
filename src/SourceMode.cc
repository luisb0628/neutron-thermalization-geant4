#include "SourceMode.hh"

namespace {
    SourceMode gMode = SourceMode::Both;
}

void SourceModeConfig::SetMode(SourceMode mode) { gMode = mode; }

SourceMode SourceModeConfig::GetMode() { return gMode; }

G4String SourceModeConfig::GetOutputFileName()
{
    switch (gMode) {
        case SourceMode::AmBeOnly:  return "AmBePhaseSpace.root";
        case SourceMode::Cs137Only: return "Cs137PhaseSpace.root";
        default:                    return "AmBeCs137PhaseSpace.root";
    }
}
