#ifndef GeneratorMessenger_h
#define GeneratorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class G4UIdirectory;
class G4UIcmdWithAString;

// Comandos de macro para elegir que fuente(s) se generan por evento, sin
// tocar DetectorConstruction/SourceMessenger (posicion, distancia y
// actividad de las fuentes siguen configurandose igual que antes con
// /ambe/...; esto solo decide si se emite AmBe, Cs-137, o ambas).
class GeneratorMessenger : public G4UImessenger {
public:
    GeneratorMessenger();
    ~GeneratorMessenger() override;
    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    G4UIdirectory*       fSourceDir;
    G4UIcmdWithAString*  fModeCmd; // /source/mode AmBe | Cs137 | Both
};

#endif
