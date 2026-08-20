#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"
class DetectorMessenger;
class SourceMessenger;
class G4LogicalVolume;
class G4Material;
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
    void SetLeadZ(G4double val)     { fLeadZ = val; }

    G4double GetParaffinX() const { return fParaffinX; }
    G4double GetParaffinY() const { return fParaffinY; }
    G4double GetParaffinZ() const { return fParaffinZ; }
    G4double GetLeadZ()     const { return fLeadZ; }

    // --- Posición de las fuentes (dueño único: la geometría las coloca
    //     y PrimaryGeneratorAction consulta estos mismos valores) ---
    void SetSourceDistance(G4double val) { fSourceDistance = val; }
    void SetCs137Offset(const G4ThreeVector& v) { fCs137Offset = v; }
    void SetActivity(G4double val) { fActivity = val; }

    G4double GetSourceDistance() const { return fSourceDistance; }
    const G4ThreeVector& GetCs137Offset() const { return fCs137Offset; }
    G4double GetActivity() const { return fActivity; }

    // Posiciones reales de emisión (centro de la cavidad activa de cada
    // cápsula) — usadas tanto para colocar la geometría como por el GPS,
    // así ambas cosas quedan garantizadas en el mismo punto.
    G4ThreeVector GetAmBePosition() const;
    G4ThreeVector GetCs137Position() const;

private:
    G4LogicalVolume* BuildSealedSource(const G4String& name);
    G4LogicalVolume* BuildSourceMonitor(const G4String& name, G4LogicalVolume* capsuleLV);

    G4double fParaffinX;
    G4double fParaffinY;
    G4double fParaffinZ;
    G4double fLeadZ;       // media longitud (Z) del bloque de plomo parcial, entre la caja
                            // blindaje W-acero y la parafina — default 1 cm (2 cm de espesor total)

    G4double fSourceDistance; // offset extra en Z de las fuentes respecto al centro de la
                              // caja blindaje (0 = centradas en la cavidad, por defecto)
    G4ThreeVector fCs137Offset; // offset fijo de la fuente Cs-137 respecto a la AmBe
                                // (por defecto las deja en mitades opuestas del ancho de la caja:
                                //  AmBe del lado sin plomo, Cs-137 del lado cubierto por el plomo)
    G4double fActivity;       // en Ci, solo para normalización

    DetectorMessenger* fMessenger;
    SourceMessenger*   fSourceMessenger;
};


#endif
