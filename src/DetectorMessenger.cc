#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"

#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIdirectory.hh"
#include "G4SystemOfUnits.hh"

// ------------------------------------------------------------
// Constructor: define los comandos accesibles desde el macro
// ------------------------------------------------------------
DetectorMessenger::DetectorMessenger(DetectorConstruction* detector)
 : fDetector(detector)
{
    // Directorio principal de comandos
    fDetectorDir = new G4UIdirectory("/detector/");
    fDetectorDir->SetGuidance("Comandos para configurar la geometría del detector.");

    // --- Comando para X ---
    fParaffinXCmd = new G4UIcmdWithADoubleAndUnit("/detector/setParaffinX", this);
    fParaffinXCmd->SetGuidance("Define el tamaño medio (half-length) en X de la parafina.");
    fParaffinXCmd->SetParameterName("X", false);
    fParaffinXCmd->SetUnitCategory("Length");

    // --- Comando para Y ---
    fParaffinYCmd = new G4UIcmdWithADoubleAndUnit("/detector/setParaffinY", this);
    fParaffinYCmd->SetGuidance("Define el tamaño medio (half-length) en Y de la parafina.");
    fParaffinYCmd->SetParameterName("Y", false);
    fParaffinYCmd->SetUnitCategory("Length");

    // --- Comando para Z (parafina) ---
    fParaffinZCmd = new G4UIcmdWithADoubleAndUnit("/detector/setParaffinZ", this);
    fParaffinZCmd->SetGuidance("Define el tamaño medio (half-length) en Z (espesor) de la parafina.");
    fParaffinZCmd->SetParameterName("Z", false);
    fParaffinZCmd->SetUnitCategory("Length");

    // --- Comando para espesor (half-length) del bloque de plomo adicional ---
    // (lado -X, donde está la cápsula de Cs-137; la caja de tungsteno es de tamaño fijo)
    fLeadZCmd = new G4UIcmdWithADoubleAndUnit("/detector/setLeadZ", this);
    fLeadZCmd->SetGuidance("Define la media longitud en Z del bloque de plomo adicional (espesor_total/2), ubicado entre la caja de tungsteno y la parafina, del lado de la fuente Cs-137.");
    fLeadZCmd->SetParameterName("LeadZ", false);
    fLeadZCmd->SetDefaultUnit("cm");
    fLeadZCmd->SetUnitCategory("Length");
}

// ------------------------------------------------------------
// Destructor
// ------------------------------------------------------------
DetectorMessenger::~DetectorMessenger()
{
    delete fParaffinXCmd;
    delete fParaffinYCmd;
    delete fParaffinZCmd;
    delete fLeadZCmd;
    delete fDetectorDir;
}

// ------------------------------------------------------------
// Conecta los comandos con los setters del DetectorConstruction
// ------------------------------------------------------------
void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fParaffinXCmd) {
        fDetector->SetParaffinX(fParaffinXCmd->GetNewDoubleValue(newValue));
    }
    else if (command == fParaffinYCmd) {
        fDetector->SetParaffinY(fParaffinYCmd->GetNewDoubleValue(newValue));
    }
    else if (command == fParaffinZCmd) {
        fDetector->SetParaffinZ(fParaffinZCmd->GetNewDoubleValue(newValue));
    }
    else if (command == fLeadZCmd) {
        fDetector->SetLeadZ(fLeadZCmd->GetNewDoubleValue(newValue));
    }
}
