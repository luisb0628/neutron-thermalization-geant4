#include "RunAction.hh"
#include "SourceMode.hh"
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
    // Nombre de archivo segun /source/mode (ver SourceMode.hh): corridas
    // AmBe-only y Cs137-only quedan en .root separados, para combinarlos
    // despues en el analisis en vez de mezclar ambas fuentes por evento.
    am->OpenFile(SourceModeConfig::GetOutputFileName());

    // Ntuple de phase space: sirve como input para la siguiente simulación
    am->CreateNtuple("PhaseSpace", "Phase space en detector — fuente AmBe y/o Cs-137 (ver /source/mode)");
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

    // Moderacion del neutron primario en la parafina (ver SteppingAction).
    // Ntuple 2: una fila por neutron primario.
    am->CreateNtuple("ModerationSummary", "Resumen de la moderacion de cada neutron primario en la parafina");
    am->CreateNtupleIColumn("EventID");        // 0
    am->CreateNtupleDColumn("E0_eV");          // 1  energia inicial
    am->CreateNtupleDColumn("EEntry_eV");      // 2  energia al entrar a la parafina (-1 si no entra)
    am->CreateNtupleIColumn("EnteredBlock");   // 3
    am->CreateNtupleIColumn("nElasticH");      // 4  colisiones elasticas sobre H en la parafina
    am->CreateNtupleIColumn("nElasticC");      // 5  ... sobre C
    am->CreateNtupleIColumn("nElasticOther");  // 6
    am->CreateNtupleIColumn("nInelastic");     // 7
    am->CreateNtupleIColumn("nCapture");       // 8
    am->CreateNtupleIColumn("nCollOther");     // 9  interacciones hadronicas fuera de la parafina
    am->CreateNtupleDColumn("PathBlock_cm");   // 10 recorrido dentro de la parafina
    am->CreateNtupleIColumn("nColl_05eV");     // 11 colisiones en la parafina hasta E < 0.5 eV (-1 si nunca)
    am->CreateNtupleDColumn("Time_05eV_ns");   // 12
    am->CreateNtupleDColumn("Path_05eV_cm");   // 13 recorrido total hasta E < 0.5 eV
    am->CreateNtupleDColumn("R_05eV_cm");      // 14 distancia desde la entrada a la parafina
    am->CreateNtupleDColumn("Dz_05eV_cm");     // 15 profundidad en z desde la entrada
    am->CreateNtupleIColumn("nColl_0025eV");   // 16
    am->CreateNtupleDColumn("Time_0025eV_ns"); // 17
    am->CreateNtupleDColumn("EFinal_eV");      // 18
    am->CreateNtupleSColumn("FinalVolume");    // 19
    am->CreateNtupleSColumn("FinalProcess");   // 20
    am->CreateNtupleDColumn("TimeEnd_ns");     // 21
    am->CreateNtupleDColumn("TimeEntry_ns");   // 22 tiempo al entrar a la parafina
    am->FinishNtuple();

    // Ntuple 3: una fila por colision en la parafina (solo los primeros
    // /moderation/maxStepTracks neutrones).
    am->CreateNtuple("ModerationSteps", "Colisiones del neutron primario dentro de la parafina");
    am->CreateNtupleIColumn("EventID");        // 0
    am->CreateNtupleIColumn("CollIndex");      // 1  numero de colision en la parafina
    am->CreateNtupleSColumn("Process");        // 2
    am->CreateNtupleIColumn("TargetZ");        // 3
    am->CreateNtupleIColumn("TargetA");        // 4
    am->CreateNtupleDColumn("EPre_eV");        // 5
    am->CreateNtupleDColumn("EPost_eV");       // 6
    am->CreateNtupleDColumn("Time_ns");        // 7
    am->CreateNtupleDColumn("StepLength_cm");  // 8  recorrido desde la colision anterior
    am->CreateNtupleDColumn("PathTotal_cm");   // 9
    am->CreateNtupleDColumn("PosX_cm");        // 10
    am->CreateNtupleDColumn("PosY_cm");        // 11
    am->CreateNtupleDColumn("PosZ_cm");        // 12
    am->CreateNtupleDColumn("CosThetaLab");    // 13 coseno del angulo de dispersion (lab)
    am->FinishNtuple();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    auto* am = G4AnalysisManager::Instance();
    am->Write();
    am->CloseFile();

    G4int nEvents = run->GetNumberOfEvent();
    G4cout << "\n====================================================\n"
           << "  Simulación completada\n"
           << "  Eventos simulados : " << nEvents << "\n"
           << "  Archivo           : " << SourceModeConfig::GetOutputFileName() << "\n"
           << "====================================================\n" << G4endl;
}
