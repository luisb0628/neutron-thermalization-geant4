#include "TransmittedSD.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"

TransmittedSD::TransmittedSD(const G4String& name)
 : G4VSensitiveDetector(name)
{}

TransmittedSD::~TransmittedSD() = default;

// Columnas de la ntuple PhaseSpace (definidas en RunAction):
//  0  Particle   (S)  nombre de la partícula
//  1  KinE_eV    (D)  energía cinética en eV
//  2  PosX_cm    (D)  posición X en cm
//  3  PosY_cm    (D)  posición Y en cm
//  4  PosZ_cm    (D)  posición Z en cm
//  5  DirX       (D)  componente X de la dirección del momento
//  6  DirY       (D)  componente Y
//  7  DirZ       (D)  componente Z

G4bool TransmittedSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    G4StepPoint* pre = aStep->GetPreStepPoint();

    // Solo registrar partículas que cruzan la frontera hacia el detector
    if (pre->GetStepStatus() != fGeomBoundary) return false;

    auto* track = aStep->GetTrack();
    G4String      name = track->GetDefinition()->GetParticleName();
    G4double      kinE = pre->GetKineticEnergy();
    G4ThreeVector pos  = pre->GetPosition();
    G4ThreeVector dir  = pre->GetMomentumDirection();

    auto* am = G4AnalysisManager::Instance();
    am->FillNtupleSColumn(0, 0, name);
    am->FillNtupleDColumn(0, 1, kinE / eV);
    am->FillNtupleDColumn(0, 2, pos.x() / cm);
    am->FillNtupleDColumn(0, 3, pos.y() / cm);
    am->FillNtupleDColumn(0, 4, pos.z() / cm);
    am->FillNtupleDColumn(0, 5, dir.x());
    am->FillNtupleDColumn(0, 6, dir.y());
    am->FillNtupleDColumn(0, 7, dir.z());
    am->AddNtupleRow(0);

    return true;
}
