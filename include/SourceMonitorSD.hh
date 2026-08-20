#ifndef SourceMonitorSD_h
#define SourceMonitorSD_h 1

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"

// Registra el espectro de partículas justo al salir de las cápsulas de
// fuente (antes de atravesar plomo/parafina), en el ntuple "SourceSpectrum".
class SourceMonitorSD : public G4VSensitiveDetector {
public:
    SourceMonitorSD(const G4String& name);
    ~SourceMonitorSD() override;
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*) override;
    void EndOfEvent(G4HCofThisEvent*) override {}
};

#endif
