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

void EventAction::BeginOfEventAction(const G4Event*) 
{
    // Este método se llama al inicio de cada evento
    // Puedes usarlo para inicializar variables por evento si lo necesitas
}

void EventAction::EndOfEventAction(const G4Event*) 
{
    // Este método se llama al final de cada evento
    // Puedes usarlo para procesar "Hits Collections" si las usaras,
    // pero para el llenado directo, lo dejamos vacío.
}

// La función RecordNeutronEnergy() se ha eliminado
// porque TransmittedSD.cc ahora maneja el llenado
// del histograma Y la ntuple.