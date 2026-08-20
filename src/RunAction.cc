#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

RunAction::RunAction() : G4UserRunAction()
{
    // En modo MT cada hilo trabajador escribe su propio archivo temporal;
    // esto le pide al AnalysisManager fusionarlos en uno solo al cerrar.
    G4AnalysisManager::Instance()->SetNtupleMerging(true);
}
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

    // Ntuple del espectro justo al salir de las cápsulas de fuente (antes
    // de atravesar plomo/parafina) — mismo esquema de columnas que arriba
    am->CreateNtuple("SourceSpectrum", "Espectro de partículas al salir de las cápsulas de fuente");
    am->CreateNtupleSColumn("Particle");
    am->CreateNtupleDColumn("KinE_eV");
    am->CreateNtupleDColumn("PosX_cm");
    am->CreateNtupleDColumn("PosY_cm");
    am->CreateNtupleDColumn("PosZ_cm");
    am->CreateNtupleDColumn("DirX");
    am->CreateNtupleDColumn("DirY");
    am->CreateNtupleDColumn("DirZ");
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
           << "====================================================\n" << G4endl;
}
