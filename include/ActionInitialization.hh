#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

class DetectorConstruction;
class GeneratorMessenger;
class ModerationMessenger;

class ActionInitialization : public G4VUserActionInitialization {
public:
    ActionInitialization(DetectorConstruction* detector);
    ~ActionInitialization() override;

    void BuildForMaster() const override;
    void Build() const override;

private:
    DetectorConstruction* fDetector;

    // Construido aqui (una sola vez, en el hilo master, antes de /run/initialize)
    // y no dentro de PrimaryGeneratorAction: el comando /source/mode tiene que
    // existir ANTES de que el macro lo use, y PrimaryGeneratorAction recien se
    // construye en Build(), que Geant4 no llama hasta /run/initialize — igual
    // que DetectorConstruction/SourceMessenger, que por eso se crean eager en
    // main.cc en vez de en Build(). Ver SourceMode.hh/GeneratorMessenger.hh.
    GeneratorMessenger*   fGeneratorMessenger;

    // /moderation/...: registro paso a paso de la moderacion (ver SteppingAction)
    ModerationMessenger*  fModerationMessenger;
};

#endif
