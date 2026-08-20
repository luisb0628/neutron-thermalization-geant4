#include "SourceMessenger.hh"
#include "DetectorConstruction.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"

SourceMessenger::SourceMessenger(DetectorConstruction* detector)
 : fDetector(detector)
{
    fAmBeDir = new G4UIdirectory("/ambe/");
    fAmBeDir->SetGuidance("Comandos para la fuente AmBe.");

    fDistanceCmd = new G4UIcmdWithADoubleAndUnit("/ambe/distance", this);
    fDistanceCmd->SetGuidance("Distancia de la fuente a la cara frontal del plomo.");
    fDistanceCmd->SetParameterName("dist", false);
    fDistanceCmd->SetDefaultUnit("cm");
    fDistanceCmd->SetUnitCategory("Length");
    fDistanceCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fActivityCmd = new G4UIcmdWithADouble("/ambe/activity", this);
    fActivityCmd->SetGuidance("Actividad de la fuente en Ci (para normalización de resultados).");
    fActivityCmd->SetParameterName("act", false);
    fActivityCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

SourceMessenger::~SourceMessenger()
{
    delete fDistanceCmd;
    delete fActivityCmd;
    delete fAmBeDir;
}

void SourceMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fDistanceCmd)
        fDetector->SetSourceDistance(fDistanceCmd->GetNewDoubleValue(newValue));
    else if (command == fActivityCmd)
        fDetector->SetActivity(fActivityCmd->GetNewDoubleValue(newValue));
}
