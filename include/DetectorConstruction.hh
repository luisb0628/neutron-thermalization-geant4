#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {
  public:
    DetectorConstruction();
    ~DetectorConstruction() override;
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;
  private:
    G4double fParaffinThickness; 
};

#endif
