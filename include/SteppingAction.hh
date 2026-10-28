#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

// Sigue al neutron primario (ParentID = 0) y registra:
//  - ModerationSummary (ntuple 2): una fila por neutron primario, con el numero de
//    colisiones elasticas sobre H y C dentro de la parafina, la energia, tiempo y
//    distancia al llegar a < 0.5 eV y < 0.025 eV, y donde y como termina el track.
//  - ModerationSteps   (ntuple 3): una fila por colision dentro de la parafina,
//    solo para los primeros /moderation/maxStepTracks neutrones.
class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction();
    ~SteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

private:
    void ResetState(G4double e0);
    void WriteSummary(const G4Step* step);

    // --- estado del neutron primario del evento actual (un evento = un primario) ---
    G4double      fE0           = 0.;
    G4bool        fEnteredBlock = false;
    G4double      fEEntry       = -1.;
    G4ThreeVector fEntryPos;
    G4double      fTimeEntry    = -1.;   // tiempo global al entrar a la parafina
    G4int    fNElasticH = 0, fNElasticC = 0, fNElasticOther = 0;
    G4int    fNInelastic = 0, fNCapture = 0, fNCollOther = 0;
    G4int    fNCollBlock = 0;          // colisiones hadronicas dentro de la parafina
    G4double fPathBlock = 0.;
    // primer cruce por debajo de 0.5 eV y 0.025 eV
    G4bool   fBelow05 = false, fBelow0025 = false;
    G4int    fNColl05 = -1, fNColl0025 = -1;
    G4double fTime05 = -1., fTime0025 = -1.;
    G4double fPath05 = -1., fR05 = -1., fDz05 = -1.;
    G4bool   fRecordSteps  = false;    // este evento guarda colisiones una por una
    G4bool   fDecidedSteps = false;
};

#endif
