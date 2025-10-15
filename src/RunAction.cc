#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

RunAction::RunAction() : G4UserRunAction() {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
  auto analysisManager = G4AnalysisManager::Instance();

  // Crear archivo ROOT
  analysisManager->OpenFile("Energy_neutrons.root");

  // Crear histograma 
  analysisManager->CreateH1("Neutrones", "Espectro de energia de neutrones",
                            200000, 0., .5e7);
   // --- Definición del Ntuple (Tabla) ---
    analysisManager->CreateNtuple("NeutronData", "Datos de neutrones transmitidos");
    // Columna 0: Energía cinética (en eV)
    analysisManager->CreateNtupleDColumn("KineticEnergy_eV"); 

    analysisManager->FinishNtuple();
}


void RunAction::EndOfRunAction(const G4Run*)
{
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();

  G4cout << "Archivo ROOT guardado" << G4endl;
}
