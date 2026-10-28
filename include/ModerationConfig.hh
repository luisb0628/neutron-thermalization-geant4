#ifndef ModerationConfig_h
#define ModerationConfig_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class G4UIdirectory;
class G4UIcmdWithAnInteger;

// Registro paso a paso de la moderacion del neutron primario en la parafina
// (ntuples "ModerationSummary" y "ModerationSteps", ver SteppingAction).
// Estado simple de proceso (no por-hilo), fijado desde el macro antes de
// /run/initialize, igual que SourceModeConfig.
namespace ModerationConfig {
    void   SetEnabled(G4int enabled);
    G4bool IsEnabled();

    // Maximo de neutrones primarios cuyas colisiones se guardan una por una
    // (ntuple ModerationSteps). El resumen por neutron se guarda para todos.
    void  SetMaxStepTracks(G4int n);
    G4int GetMaxStepTracks();
}

// Comandos: /moderation/enable 0|1  y  /moderation/maxStepTracks N
class ModerationMessenger : public G4UImessenger {
public:
    ModerationMessenger();
    ~ModerationMessenger() override;
    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    G4UIdirectory*        fDir;
    G4UIcmdWithAnInteger* fEnableCmd;
    G4UIcmdWithAnInteger* fMaxStepsCmd;
};

#endif
