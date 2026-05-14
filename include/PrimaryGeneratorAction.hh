#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleGun.hh"
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
    void BuildAmBeNeutronSource();

    G4GeneralParticleSource* fGPS;
    G4ParticleGun*           fGammaGun;
    const DetectorConstruction* fDetector;
    G4double                 fSourceDistance; // distancia fuente → cara frontal parafina
    G4double                 fActivity;       // en Ci, solo para normalización
    SourceMessenger*         fMessenger;
};

#endif
