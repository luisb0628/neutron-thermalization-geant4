#ifndef DetectorMessenger_h
#define DetectorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;

class DetectorMessenger : public G4UImessenger {
public:
    DetectorMessenger(DetectorConstruction* detector);
    ~DetectorMessenger() override;

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    DetectorConstruction* fDetector;  // referencia al detector

    G4UIdirectory* fDetectorDir;
    G4UIcmdWithADoubleAndUnit* fParaffinXCmd;
    G4UIcmdWithADoubleAndUnit* fParaffinYCmd;
    G4UIcmdWithADoubleAndUnit* fParaffinZCmd;
    G4UIcmdWithADoubleAndUnit* fLeadZCmd;
};

#endif
