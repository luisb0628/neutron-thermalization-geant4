#include "TransmittedSD.hh"
#include "G4Step.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh" // ¡Necesario para el EventID!
#include "G4VProcess.hh" // ¡Necesario para el nombre del proceso!

TransmittedSD::TransmittedSD(const G4String& name)
 : G4VSensitiveDetector(name)
{}

TransmittedSD::~TransmittedSD() = default;

G4bool TransmittedSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
    // Queremos el estado del neutrón JUSTO ANTES de entrar al volumen
    G4StepPoint* pre = aStep->GetPreStepPoint();

    // Condición: El paso debe haber sido definido por una frontera geométrica
    if (pre->GetStepStatus() == fGeomBoundary) {
        
        auto track = aStep->GetTrack();
        auto particle = track->GetDefinition();

        // Solo nos interesan los neutrones
        if (particle->GetParticleName() == "neutron") {
            
            auto analysisManager = G4AnalysisManager::Instance();
            G4int ntupleID = 0; // El ID de nuestra Ntuple es 0

            // --- Llenar Histograma (ID=0) ---
            G4double kinE_eV = pre->GetKineticEnergy() / eV;
            analysisManager->FillH1(0, kinE_eV);

            // --- Llenar la Ntuple (ID=0) ---
            // (Los IDs de columna empiezan en 0)

            // Col 0: EventID
            analysisManager->FillNtupleIColumn(ntupleID, 0, G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID());
            // Col 1: TrackID
            analysisManager->FillNtupleIColumn(ntupleID, 1, track->GetTrackID());
            // Col 2: ParentID
            analysisManager->FillNtupleIColumn(ntupleID, 2, track->GetParentID());
            
            // Col 3: KineticEnergy_eV (la energía que tiene al entrar)
            analysisManager->FillNtupleDColumn(ntupleID, 3, kinE_eV);
            // Col 4: Tiempo (el tiempo que tardó en llegar)
            analysisManager->FillNtupleDColumn(ntupleID, 4, pre->GetGlobalTime() / ns);

            // Col 5, 6, 7: Posición (dónde tocó el detector)
            G4ThreeVector pos = pre->GetPosition();
            analysisManager->FillNtupleDColumn(ntupleID, 5, pos.x() / mm);
            analysisManager->FillNtupleDColumn(ntupleID, 6, pos.y() / mm);
            analysisManager->FillNtupleDColumn(ntupleID, 7, pos.z() / mm);

            // Col 8: Longitud de la traza (cuánto viajó hasta aquí)
            analysisManager->FillNtupleDColumn(ntupleID, 8, track->GetTrackLength() / mm);

            // Col 9: Número de Pasos (cuántas colisiones tuvo hasta aquí)
            analysisManager->FillNtupleIColumn(ntupleID, 9, track->GetCurrentStepNumber());

            // Col 10: Volumen (El nombre del volumen que estamos tocando)
            // (track->GetVolume() es el volumen actual, o sea, el SD)
            analysisManager->FillNtupleSColumn(ntupleID, 10, track->GetVolume()->GetName());
            
            // Col 11: Proceso (El proceso que llevó a este paso)
            // (Para un cruce de frontera, casi siempre será "Transportation")
            const G4VProcess* process = pre->GetProcessDefinedStep();
            G4String processName = "N/A";
            if (process) processName = process->GetProcessName();
            analysisManager->FillNtupleSColumn(ntupleID, 11, processName);

            // Col 12, 13, 14: Dirección 
            G4ThreeVector direction = pre->GetMomentumDirection();
            analysisManager->FillNtupleDColumn(ntupleID, 12, direction.x());
            analysisManager->FillNtupleDColumn(ntupleID, 13, direction.y());
            analysisManager->FillNtupleDColumn(ntupleID, 14, direction.z());

            // Añadir la fila a la Ntuple
            analysisManager->AddNtupleRow(ntupleID);
            



            
            // track->SetTrackStatus(fStopAndKill);

            return true;
        }
    }

    return false;
}
