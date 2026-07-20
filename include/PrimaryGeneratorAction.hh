#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class DetectorConstruction;
class SourceMessenger;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction(const DetectorConstruction* detector);
    ~PrimaryGeneratorAction() override;
    void GeneratePrimaries(G4Event* event) override;

    void SetSourceDistance(G4double d) { fSourceDistance = d; }
    void SetActivity(G4double a)       { fActivity = a; }
    G4double GetActivity()        const { return fActivity; }
    G4double GetSourceDistance()  const { return fSourceDistance; }

private:
    void BuildSpectrum_nGND(G4GeneralParticleSource* gps);
    void BuildSpectrum_n1st(G4GeneralParticleSource* gps);
    void BuildSpectrum_n2nd(G4GeneralParticleSource* gps);
    void EmitCoincidentGamma(G4Event* event, const G4ThreeVector& pos);

    G4GeneralParticleSource* fGPS_nGND;
    G4GeneralParticleSource* fGPS_n1st;
    G4GeneralParticleSource* fGPS_n2nd;
    const DetectorConstruction* fDetector;
    G4double                 fSourceDistance; // distancia fuente → cara frontal parafina
    G4double                 fActivity;       // en Ci, solo para normalización

    // Fracciones de rama AmBe (De Guarrini & Malaroda 1971 / Geiger & Van der Zwan 1975)
    G4double                 fFrac_nGND; // ~31.6% - transición al estado fundamental del 12C
    G4double                 fFrac_n1st; // ~46.5% - transición al 1er estado excitado (+ gamma 4.438 MeV)
    // fFrac_n2nd (~21.9%) es el remanente: 1 - fFrac_nGND - fFrac_n1st

    SourceMessenger*         fMessenger;
};

#endif
