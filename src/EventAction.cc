#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4AutoLock.hh"
#include "G4ios.hh"

#include <iomanip>
#include <sstream>

// ── Estado estático compartido entre todos los hilos worker ──────────────────
G4Mutex                               EventAction::sMutex       = G4MUTEX_INITIALIZER;
G4int                                 EventAction::sLastMilestone = -1;
G4bool                                EventAction::sStarted       = false;
std::chrono::steady_clock::time_point EventAction::sGlobalStart;

EventAction::EventAction(RunAction* runAction)
 : G4UserEventAction(),
   fRunAction(runAction)
{}

EventAction::~EventAction() {}

void EventAction::ResetProgress()
{
    G4AutoLock lock(&sMutex);
    sLastMilestone = -1;
    sStarted       = false;
}

void EventAction::BeginOfEventAction(const G4Event* event)
{
    // ── Inicialización por hilo ───────────────────────────────────────────────
    // La condición es fTotalEvents == 0 (no GetEventID() == 0) porque en MT
    // solo el hilo asignado al evento 0 llegaría a inicializar; el resto
    // quedarían con fTotalEvents=0 → fPrintEvery=1 → flood de "0%".
    if (fTotalEvents == 0) {
        fTotalEvents = G4RunManager::GetRunManager()
                           ->GetCurrentRun()->GetNumberOfEventToBeProcessed();
        fPrintEvery  = std::max(1, fTotalEvents / 100);  // hito cada 1%

        G4AutoLock lock(&sMutex);
        if (!sStarted) {
            sGlobalStart = std::chrono::steady_clock::now();
            sStarted     = true;
            G4cout << "\n[Progreso] Iniciando " << fTotalEvents
                   << " eventos..." << G4endl;
        }
    }

    G4int evID = event->GetEventID();
    if (evID % fPrintEvery != 0) return;

    G4int milestone = evID / fPrintEvery;

    // ── Solo un hilo imprime por hito (evita líneas duplicadas en MT) ─────────
    {
        G4AutoLock lock(&sMutex);
        if (milestone <= sLastMilestone) return;
        sLastMilestone = milestone;
    }

    // ── Calcular progreso y ETA ───────────────────────────────────────────────
    auto now     = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - sGlobalStart).count();
    double pct   = (fTotalEvents > 0) ? 100.0 * evID / fTotalEvents : 0.0;

    std::string eta_str = "--:--:--";
    if (evID > 0 && elapsed > 0.) {
        double rate      = evID / elapsed;
        double remaining = (fTotalEvents - evID) / rate;
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                      (int)(remaining / 3600),
                      (int)(remaining / 60) % 60,
                      (int)(remaining)      % 60);
        eta_str = buf;
    }

    // Barra de 20 caracteres (cada '=' = 5%)
    int filled = std::min(20, (int)(pct / 5));
    std::string bar = "[" + std::string(filled, '=')
                          + std::string(20 - filled, ' ') + "]";

    int h = (int)(elapsed / 3600);
    int m = (int)(elapsed / 60) % 60;
    int s = (int)(elapsed)      % 60;

    G4cout << std::fixed << std::setprecision(1)
           << bar << " " << std::setw(5) << pct << "%"
           << "  Evento " << std::setw(7) << evID << "/" << fTotalEvents
           << "  Elapsed: " << h << "h"
           << std::setw(2) << std::setfill('0') << m << "m"
           << std::setw(2) << s << "s"
           << std::setfill(' ')
           << "  ETA: " << eta_str
           << G4endl;
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