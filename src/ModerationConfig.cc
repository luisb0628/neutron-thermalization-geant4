#include "ModerationConfig.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"

namespace {
    G4int gEnabled       = 1;
    G4int gMaxStepTracks = 0;
}

void   ModerationConfig::SetEnabled(G4int enabled) { gEnabled = enabled; }
G4bool ModerationConfig::IsEnabled()               { return gEnabled != 0; }
void   ModerationConfig::SetMaxStepTracks(G4int n) { gMaxStepTracks = n; }
G4int  ModerationConfig::GetMaxStepTracks()        { return gMaxStepTracks; }

ModerationMessenger::ModerationMessenger() : G4UImessenger()
{
    fDir = new G4UIdirectory("/moderation/");
    fDir->SetGuidance("Registro paso a paso de la moderacion del neutron en la parafina.");

    fEnableCmd = new G4UIcmdWithAnInteger("/moderation/enable", this);
    fEnableCmd->SetGuidance("1 = guardar ModerationSummary/ModerationSteps (default), 0 = no.");
    fEnableCmd->SetParameterName("enable", false);
    fEnableCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fMaxStepsCmd = new G4UIcmdWithAnInteger("/moderation/maxStepTracks", this);
    fMaxStepsCmd->SetGuidance("Numero maximo de neutrones primarios cuyas colisiones en la parafina se "
                              "guardan una por una (ModerationSteps). 0 = ninguno.");
    fMaxStepsCmd->SetParameterName("n", false);
    fMaxStepsCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

ModerationMessenger::~ModerationMessenger()
{
    delete fMaxStepsCmd;
    delete fEnableCmd;
    delete fDir;
}

void ModerationMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fEnableCmd)        ModerationConfig::SetEnabled(fEnableCmd->GetNewIntValue(newValue));
    else if (command == fMaxStepsCmd) ModerationConfig::SetMaxStepTracks(fMaxStepsCmd->GetNewIntValue(newValue));
}
