#ifndef TransmittedSD_h
#define TransmittedSD_h 1

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4THitsCollection.hh"

class TransmittedSD : public G4VSensitiveDetector {
  public:
    TransmittedSD(const G4String& name);
    ~TransmittedSD() override;
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory*) override;
    void EndOfEvent(G4HCofThisEvent*) override {}
};

#endif
