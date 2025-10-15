
#include "TransmittedSD.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"

TransmittedSD::TransmittedSD(const G4String& name)
 : G4VSensitiveDetector(name)
{}

TransmittedSD::~TransmittedSD() = default;

G4bool TransmittedSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    G4StepPoint* pre  = aStep->GetPreStepPoint();

    if (pre->GetStepStatus() == fGeomBoundary) {
        auto track = aStep->GetTrack();
        auto particle = track->GetDefinition();

        if (particle->GetParticleName() == "neutron") {
            // Energía cinética en eV
            G4double kinE_eV = pre->GetKineticEnergy() / eV;

            auto analysisManager = G4AnalysisManager::Instance();

            // Guardar en histograma (ID=0)
            analysisManager->FillH1(0, kinE_eV);

            // ntuple 
            analysisManager->FillNtupleDColumn(0, 0, kinE_eV);
            analysisManager->AddNtupleRow(0);

            return true;
        }
    }

    return false;
}
