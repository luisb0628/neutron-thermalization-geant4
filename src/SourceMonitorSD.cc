#include "SourceMonitorSD.hh"
#include "G4Step.hh"

SourceMonitorSD::SourceMonitorSD(const G4String& name)
 : G4VSensitiveDetector(name)
{}

SourceMonitorSD::~SourceMonitorSD() = default;

// DESACTIVADO: este detector contaba cada cruce de partícula hacia el
// volumen monitor, así que un neutrón que retrodispersaba en la cápsula y
// volvía a cruzarlo quedaba contado más de una vez (duplicados en
// "SourceSpectrum", con energía/dirección ya modificadas por esa dispersión,
// no las de emisión real). Reemplazado por
// PrimaryGeneratorAction::RecordSourceSpectrum, que registra cada partícula
// primaria exactamente una vez, con la energía/dirección tal como fue
// generada por la fuente.
G4bool SourceMonitorSD::ProcessHits(G4Step*, G4TouchableHistory*)
{
    return false;
}
