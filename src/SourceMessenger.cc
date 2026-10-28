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

    fGammaYieldCmd = new G4UIcmdWithADouble("/ambe/gammaYield", this);
    fGammaYieldCmd->SetGuidance(
        "Factor de escala (0-1) del gamma de 4.438 MeV coincidente con el canal n1st. "
        "1 = literatura (46.5% de los eventos AmBe emiten el gamma), 0 = desactivado. "
        "No cambia las fracciones nGND/n1st/n2nd del espectro de neutrones.");
    fGammaYieldCmd->SetParameterName("factor", false);
    // Sin SetRange compuesto ("a >= 0 && a <= 1"): en este build de Geant4
    // (11.0.0) ese chequeo compuesto falla siempre y aborta el macro incluso
    // con valores válidos (ver nota igual en neutron-capture-graphene-boron-geant4/
    // DetectorConstruction.cc, SetBoronFraction). Rango válido documentado en la guidance.
    fGammaYieldCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

SourceMessenger::~SourceMessenger()
{
    delete fDistanceCmd;
    delete fActivityCmd;
    delete fGammaYieldCmd;
    delete fAmBeDir;
}

void SourceMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fDistanceCmd)
        fDetector->SetSourceDistance(fDistanceCmd->GetNewDoubleValue(newValue));
    else if (command == fActivityCmd)
        fDetector->SetActivity(fActivityCmd->GetNewDoubleValue(newValue));
    else if (command == fGammaYieldCmd)
        fDetector->SetGammaYieldFactor(fGammaYieldCmd->GetNewDoubleValue(newValue));
}
