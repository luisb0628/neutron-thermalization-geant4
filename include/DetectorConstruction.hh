#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
class DetectorMessenger;
class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    // Método principal de construcción
    G4VPhysicalVolume* Construct() override;

    // Método para definir detectores sensibles y campos
    void ConstructSDandField() override;

    // --- NUEVOS MÉTODOS: para modificar las dimensiones del bloque de parafina ---
    void SetParaffinX(G4double val) { fParaffinX = val; }
    void SetParaffinY(G4double val) { fParaffinY = val; }
    void SetParaffinZ(G4double val) { fParaffinZ = val; }

    // (Opcional: getters si los necesitas)
    G4double GetParaffinX() const { return fParaffinX; }
    G4double GetParaffinY() const { return fParaffinY; }
    G4double GetParaffinZ() const { return fParaffinZ; }

private:
    // --- NUEVAS VARIABLES: medias longitudes del bloque de parafina ---
    G4double fParaffinX;
    G4double fParaffinY;
    G4double fParaffinZ;
    DetectorMessenger* fMessenger;
};


#endif
