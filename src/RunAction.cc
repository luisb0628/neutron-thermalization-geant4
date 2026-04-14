#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"

RunAction::RunAction() : G4UserRunAction() {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
  auto analysisManager = G4AnalysisManager::Instance();

  analysisManager->OpenFile("NeutronData.root");

  // Ntuple con una sola columna: energía cinética de cada neutrón detectado
  analysisManager->CreateNtuple("NeutronTracks", "Energia cinetica de neutrones transmitidos");
  analysisManager->CreateNtupleDColumn("KineticEnergy_eV");
  analysisManager->FinishNtuple();
}


void RunAction::EndOfRunAction(const G4Run*)
{
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();

  G4cout << "Archivo ROOT guardado." << G4endl;
}
