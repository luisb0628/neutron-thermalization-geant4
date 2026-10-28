#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4SingleParticleSource.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class DetectorConstruction;
class G4Event;
class G4PrimaryVertex;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction(DetectorConstruction* detector);
    ~PrimaryGeneratorAction() override;
    void GeneratePrimaries(G4Event* event) override;

    G4double GetActivity()        const;
    G4double GetSourceDistance()  const;

private:
    void BuildSpectrum_nGND(G4SingleParticleSource* gps);
    void BuildSpectrum_n1st(G4SingleParticleSource* gps);
    void BuildSpectrum_n2nd(G4SingleParticleSource* gps);
    void GenerateAmBePrimary(G4Event* event);
    void GenerateCs137Primary(G4Event* event);
    void BuildCs137Source();

    // Registra en el ntuple "SourceSpectrum" la partícula tal como fue
    // generada (energía/dirección/posición reales del vértice primario),
    // exactamente una vez por partícula primaria — sin recruces de
    // geometría (antes esto lo hacía SourceMonitorSD contando cruces del
    // volumen monitor, lo que duplicaba neutrones que retrodispersaban en
    // la cápsula y volvían a cruzarlo).
    void RecordSourceSpectrum(const G4PrimaryVertex* vertex);

    // G4SingleParticleSource en vez de G4GeneralParticleSource: cada GPS
    // completo comparte un único G4GeneralParticleSourceData por hilo (es
    // un singleton interno de Geant4), así que 4 G4GeneralParticleSource
    // terminan pisándose el "current source" entre sí. G4SingleParticleSource
    // es la clase que GPS envuelve internamente y sí es independiente.
    G4SingleParticleSource* fGPS_nGND;
    G4SingleParticleSource* fGPS_n1st;
    G4SingleParticleSource* fGPS_n2nd;
    G4SingleParticleSource* fGPS_Cs137;
    DetectorConstruction*   fDetector; // dueño de la posición/actividad de las fuentes (geometría)
    // El comando /source/mode (ver SourceMode.hh) se registra en
    // ActionInitialization, no aquí: tiene que existir antes de /run/initialize,
    // y PrimaryGeneratorAction recién se construye en ese momento.

    // Fracciones de rama AmBe (De Guarrini & Malaroda 1971 / Geiger & Van der Zwan 1975).
    // El gamma de 4.438 MeV coincidente con n1st se quitó: no se observa en el
    // espectro experimental, así que ya no se emite (solo se sigue usando la
    // fracción para muestrear el espectro de energía correcto del canal n1st).
    G4double                 fFrac_nGND; // ~31.6% - transición al estado fundamental del 12C
    G4double                 fFrac_n1st; // ~46.5% - transición al 1er estado excitado del 12C
    // fFrac_n2nd (~21.9%) es el remanente: 1 - fFrac_nGND - fFrac_n1st
};

#endif
