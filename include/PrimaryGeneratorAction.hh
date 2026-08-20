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
    void EmitCoincidentGamma(G4Event* event, const G4ThreeVector& pos);
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

    // Fracciones de rama AmBe (De Guarrini & Malaroda 1971 / Geiger & Van der Zwan 1975)
    G4double                 fFrac_nGND; // ~31.6% - transición al estado fundamental del 12C
    G4double                 fFrac_n1st; // ~46.5% - transición al 1er estado excitado (+ gamma 4.438 MeV)
    // fFrac_n2nd (~21.9%) es el remanente: 1 - fFrac_nGND - fFrac_n1st
};

#endif
