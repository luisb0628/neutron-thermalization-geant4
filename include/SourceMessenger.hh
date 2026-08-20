#ifndef SourceMessenger_h
#define SourceMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithADouble;

class SourceMessenger : public G4UImessenger {
public:
    SourceMessenger(DetectorConstruction* detector);
    ~SourceMessenger() override;
    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    DetectorConstruction*        fDetector;
    G4UIdirectory*               fAmBeDir;
    G4UIcmdWithADoubleAndUnit*   fDistanceCmd;
    G4UIcmdWithADouble*          fActivityCmd;
};

#endif
