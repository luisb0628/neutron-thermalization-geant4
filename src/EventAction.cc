#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"

EventAction::EventAction(RunAction* runAction)
 : G4UserEventAction(),
   fRunAction(runAction)
{}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*) {}

void EventAction::EndOfEventAction(const G4Event*) {}

void EventAction::RecordNeutronEnergy(G4double energy)
{
    auto analysisManager = G4AnalysisManager::Instance();

    // Convertir energía a eV
    G4double energy_eV = energy / eV;

    // Llenar histograma
    analysisManager->FillH1(0, energy_eV);
}
