#include "RunAction.hh"
#include "G4Run.hh"
#include "G4RunManager.hh" // Necesario para obtener el EventID
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

RunAction::RunAction() : G4UserRunAction() {}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
  auto analysisManager = G4AnalysisManager::Instance();

  // Crear archivo ROOT
  analysisManager->OpenFile("NeutronData.root"); // Cambié el nombre para ser más descriptivo

  // --- Crear histograma ---
  // Ajusté los bines. 200,000 era excesivo y consumiría mucha memoria.
  // 5000 bines para un rango de 5 MeV (5e6 eV) es más razonable.
  analysisManager->CreateH1("NeutronEnergy", "Espectro de energia de neutrones (eV)",
                            5000, 0., 5.e6); // 0 a 5 MeV

  // --- Definición de la Ntuple (Tabla) ---
  analysisManager->CreateNtuple("NeutronTracks", "Datos de trazas de neutrones");

  // --- Columnas de la Ntuple ---
  // (El orden de creación es el ID de la columna, empezando en 0)
  
  // Información del evento/traza
  analysisManager->CreateNtupleIColumn("EventID");    // Col 0: ID del evento
  analysisManager->CreateNtupleIColumn("TrackID");    // Col 1: ID de la traza
  analysisManager->CreateNtupleIColumn("ParentID");   // Col 2: ID del padre (0 si es primario)

  // Información de energía y tiempo
  analysisManager->CreateNtupleDColumn("KineticEnergy_eV"); // Col 3: Energía cinética final (en eV)
  analysisManager->CreateNtupleDColumn("FinalTime_ns");     // Col 4: Tiempo global final (en ns)

  // Información de posición final
  analysisManager->CreateNtupleDColumn("FinalPosX_mm");     // Col 5: Posición X final (en mm)
  analysisManager->CreateNtupleDColumn("FinalPosY_mm");     // Col 6: Posición Y final (en mm)
  analysisManager->CreateNtupleDColumn("FinalPosZ_mm");     // Col 7: Posición Z final (en mm)

  // Información de la traza (¡muy útil para termalización!)
  analysisManager->CreateNtupleDColumn("TotalTrackLength_mm"); // Col 8: Longitud total de la traza (en mm)
  analysisManager->CreateNtupleIColumn("NumSteps");            // Col 9: Número de pasos/colisiones
  
  // Información del "final" de la traza
  analysisManager->CreateNtupleSColumn("FinalVolume"); // Col 10: Nombre del volumen donde terminó
  analysisManager->CreateNtupleSColumn("FinalProcess"); // Col 11: Proceso que finalizó la traza

  analysisManager->CreateNtupleDColumn("DirX");            // Col 12: Componente X de la dirección
  analysisManager->CreateNtupleDColumn("DirY");            // Col 13: Componente Y de la dirección
  analysisManager->CreateNtupleDColumn("DirZ");            // Col 14: Componente Z de la dirección

  analysisManager->FinishNtuple();
}


void RunAction::EndOfRunAction(const G4Run*)
{
  auto analysisManager = G4AnalysisManager::Instance();
  
  // Es bueno normalizar el histograma si se desea (opcional)
  // G4double norm = ...;
  // analysisManager->ScaleH1(0, norm); // '0' es el ID del histograma

  analysisManager->Write();
  analysisManager->CloseFile();

  G4cout << "Archivo ROOT guardado." << G4endl;
}
