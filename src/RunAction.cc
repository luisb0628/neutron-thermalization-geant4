#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

RunAction::RunAction() : G4UserRunAction() {}
RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
    auto* am = G4AnalysisManager::Instance();
    am->OpenFile("AmBePhaseSpace.root");

    // Ntuple de phase space: sirve como input para la siguiente simulación
    am->CreateNtuple("PhaseSpace", "Phase space en detector — fuente AmBe");
    am->CreateNtupleSColumn("Particle");   // col 0: nombre de partícula
    am->CreateNtupleDColumn("KinE_eV");    // col 1: energía cinética en eV
    am->CreateNtupleDColumn("PosX_cm");    // col 2
    am->CreateNtupleDColumn("PosY_cm");    // col 3
    am->CreateNtupleDColumn("PosZ_cm");    // col 4
    am->CreateNtupleDColumn("DirX");       // col 5: dirección del momento (vector unitario)
    am->CreateNtupleDColumn("DirY");       // col 6
    am->CreateNtupleDColumn("DirZ");       // col 7
    am->FinishNtuple();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    auto* am = G4AnalysisManager::Instance();
    am->Write();
    am->CloseFile();

    G4int nEvents = run->GetNumberOfEvent();
    G4cout << "\n====================================================\n"
           << "  Simulación AmBe completada\n"
           << "  Eventos simulados : " << nEvents << "\n"
           << "  Archivo           : AmBePhaseSpace.root\n"
           << "\n"
           << "  Para normalizar resultados:\n"
           << "    Tasa real [part/s] = (N_detectadas / N_simulados)\n"
           << "                       x actividad[Ci] x 2.2e6 [n/s/Ci]\n"
           << "====================================================\n" << G4endl;
}
