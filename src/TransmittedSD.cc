#include "TransmittedSD.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"

TransmittedSD::TransmittedSD(const G4String& name)
 : G4VSensitiveDetector(name)
{}

TransmittedSD::~TransmittedSD() = default;

G4bool TransmittedSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    G4StepPoint* pre = aStep->GetPreStepPoint();

    // Solo registrar neutrones que cruzan la frontera geométrica hacia el detector
    if (pre->GetStepStatus() != fGeomBoundary) return false;
    if (aStep->GetTrack()->GetDefinition()->GetParticleName() != "neutron") return false;

    G4double kinE_eV = pre->GetKineticEnergy() / eV;

    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->FillNtupleDColumn(0, 0, kinE_eV);
    analysisManager->AddNtupleRow(0);

    return true;
}
