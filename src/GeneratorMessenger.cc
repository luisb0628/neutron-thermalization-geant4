#include "GeneratorMessenger.hh"
#include "SourceMode.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"

GeneratorMessenger::GeneratorMessenger() : G4UImessenger()
{
    fSourceDir = new G4UIdirectory("/source/");
    fSourceDir->SetGuidance("Comandos para elegir que fuente(s) se simulan.");

    fModeCmd = new G4UIcmdWithAString("/source/mode", this);
    fModeCmd->SetGuidance(
        "Fuente(s) a generar por evento: AmBe (solo neutrones AmBe), "
        "Cs137 (solo gamma de 661.657 keV) o Both (las dos, como antes). "
        "Default: Both. Usar AmBe/Cs137 por separado y combinar los .root "
        "despues en el analisis, ya que Cs-137 produce muchos mas fotones "
        "por evento que la cadena de neutrones del AmBe.");
    fModeCmd->SetParameterName("mode", false);
    fModeCmd->SetCandidates("AmBe Cs137 Both");
    fModeCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

GeneratorMessenger::~GeneratorMessenger()
{
    delete fModeCmd;
    delete fSourceDir;
}

void GeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fModeCmd) {
        if (newValue == "AmBe")       SourceModeConfig::SetMode(SourceMode::AmBeOnly);
        else if (newValue == "Cs137") SourceModeConfig::SetMode(SourceMode::Cs137Only);
        else                          SourceModeConfig::SetMode(SourceMode::Both);
    }
}
