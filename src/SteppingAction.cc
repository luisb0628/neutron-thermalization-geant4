#include "SteppingAction.hh"
#include "ModerationConfig.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4VProcess.hh"
#include "G4HadronicProcess.hh"
#include "G4Nucleus.hh"
#include "G4VPhysicalVolume.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"

#include <atomic>

namespace {
    // Neutrones primarios con colisiones guardadas una por una (compartido entre hilos)
    std::atomic<G4int> gStepTracksRecorded{0};
    const G4double kThermal05   = 0.5   * eV;   // corte de cadmio
    const G4double kThermal0025 = 0.025 * eV;   // energia termica
}

SteppingAction::SteppingAction() = default;
SteppingAction::~SteppingAction() = default;

void SteppingAction::ResetState(G4double e0)
{
    fE0 = e0;
    fEnteredBlock = false; fEEntry = -1.; fEntryPos = G4ThreeVector(); fTimeEntry = -1.;
    fNElasticH = fNElasticC = fNElasticOther = 0;
    fNInelastic = fNCapture = fNCollOther = fNCollBlock = 0;
    fPathBlock = 0.;
    fBelow05 = fBelow0025 = false;
    fNColl05 = fNColl0025 = -1;
    fTime05 = fTime0025 = -1.;
    fPath05 = fR05 = fDz05 = -1.;
    fRecordSteps = false; fDecidedSteps = false;
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    if (!ModerationConfig::IsEnabled()) return;

    auto* track = step->GetTrack();
    if (track->GetParentID() != 0) return;
    if (track->GetDefinition()->GetParticleName() != "neutron") return;

    G4StepPoint* pre  = step->GetPreStepPoint();
    G4StepPoint* post = step->GetPostStepPoint();

    if (track->GetCurrentStepNumber() == 1) ResetState(pre->GetKineticEnergy());

    const G4VPhysicalVolume* preVol = pre->GetPhysicalVolume();
    const G4String preName = preVol ? preVol->GetName() : G4String("OutOfWorld");
    const G4bool inBlock = (preName == "Block");

    if (inBlock) {
        if (!fEnteredBlock) {
            fEnteredBlock = true;
            fEEntry   = pre->GetKineticEnergy();
            fEntryPos = pre->GetPosition();
            fTimeEntry = pre->GetGlobalTime();
            if (!fDecidedSteps) {
                fDecidedSteps = true;
                // el contador se incrementa solo si todavia hay cupo
                fRecordSteps = (gStepTracksRecorded.fetch_add(1) < ModerationConfig::GetMaxStepTracks());
            }
        }
        fPathBlock += step->GetStepLength();
    }

    // --- colision hadronica (elastica, inelastica, captura) ---
    const G4VProcess* proc = post->GetProcessDefinedStep();
    const auto* had = dynamic_cast<const G4HadronicProcess*>(proc);
    if (had) {
        const G4String pname = proc->GetProcessName();
        G4int tz = -1, ta = -1;
        if (const G4Nucleus* nuc = had->GetTargetNucleus()) {
            tz = nuc->GetZ_asInt();
            ta = nuc->GetA_asInt();
        }
        if (inBlock) {
            ++fNCollBlock;
            if (pname == "hadElastic") {
                if (tz == 1)      ++fNElasticH;
                else if (tz == 6) ++fNElasticC;
                else              ++fNElasticOther;
            } else if (pname.find("apture") != std::string::npos) {   // nCapture / nCaptureHP
                ++fNCapture;
            } else {
                ++fNInelastic;
            }
            if (fRecordSteps) {
                const G4ThreeVector d0 = pre->GetMomentumDirection();
                const G4ThreeVector d1 = post->GetMomentumDirection();
                auto* am = G4AnalysisManager::Instance();
                am->FillNtupleIColumn(3, 0, G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID());
                am->FillNtupleIColumn(3, 1, fNCollBlock);
                am->FillNtupleSColumn(3, 2, pname);
                am->FillNtupleIColumn(3, 3, tz);
                am->FillNtupleIColumn(3, 4, ta);
                am->FillNtupleDColumn(3, 5, pre->GetKineticEnergy() / eV);
                am->FillNtupleDColumn(3, 6, post->GetKineticEnergy() / eV);
                am->FillNtupleDColumn(3, 7, post->GetGlobalTime() / ns);
                am->FillNtupleDColumn(3, 8, step->GetStepLength() / cm);
                am->FillNtupleDColumn(3, 9, track->GetTrackLength() / cm);
                am->FillNtupleDColumn(3, 10, post->GetPosition().x() / cm);
                am->FillNtupleDColumn(3, 11, post->GetPosition().y() / cm);
                am->FillNtupleDColumn(3, 12, post->GetPosition().z() / cm);
                am->FillNtupleDColumn(3, 13, d0.dot(d1));
                am->AddNtupleRow(3);
            }
        } else {
            ++fNCollOther;
        }
    }

    // --- primer cruce por debajo de 0.5 eV y de 0.025 eV ---
    const G4double ePost = post->GetKineticEnergy();
    if (!fBelow05 && ePost < kThermal05 && ePost > 0.) {
        fBelow05 = true;
        fNColl05 = fNCollBlock;
        fTime05  = post->GetGlobalTime();
        fPath05  = track->GetTrackLength();
        if (fEnteredBlock) {
            const G4ThreeVector dv = post->GetPosition() - fEntryPos;
            fR05  = dv.mag();
            fDz05 = dv.z();
        }
    }
    if (!fBelow0025 && ePost < kThermal0025 && ePost > 0.) {
        fBelow0025 = true;
        fNColl0025 = fNCollBlock;
        fTime0025  = post->GetGlobalTime();
    }

    // --- fin del track primario ---
    if (track->GetTrackStatus() != fAlive) WriteSummary(step);
}

void SteppingAction::WriteSummary(const G4Step* step)
{
    // Solo los neutrones que llegan a la parafina (el resto no se modera ahi);
    // el total de primarios sale de SourceSpectrum.
    if (!fEnteredBlock) return;

    G4StepPoint* post = step->GetPostStepPoint();
    const G4VProcess* proc = post->GetProcessDefinedStep();
    const G4VPhysicalVolume* preVol = step->GetPreStepPoint()->GetPhysicalVolume();

    auto* am = G4AnalysisManager::Instance();
    am->FillNtupleIColumn(2, 0, G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID());
    am->FillNtupleDColumn(2, 1, fE0 / eV);
    am->FillNtupleDColumn(2, 2, fEEntry >= 0. ? fEEntry / eV : -1.);
    am->FillNtupleIColumn(2, 3, fEnteredBlock ? 1 : 0);
    am->FillNtupleIColumn(2, 4, fNElasticH);
    am->FillNtupleIColumn(2, 5, fNElasticC);
    am->FillNtupleIColumn(2, 6, fNElasticOther);
    am->FillNtupleIColumn(2, 7, fNInelastic);
    am->FillNtupleIColumn(2, 8, fNCapture);
    am->FillNtupleIColumn(2, 9, fNCollOther);
    am->FillNtupleDColumn(2, 10, fPathBlock / cm);
    am->FillNtupleIColumn(2, 11, fNColl05);
    am->FillNtupleDColumn(2, 12, fTime05 >= 0. ? fTime05 / ns : -1.);
    am->FillNtupleDColumn(2, 13, fPath05 >= 0. ? fPath05 / cm : -1.);
    am->FillNtupleDColumn(2, 14, fR05 >= 0. ? fR05 / cm : -1.);
    am->FillNtupleDColumn(2, 15, fDz05 >= 0. ? fDz05 / cm : -1.);
    am->FillNtupleIColumn(2, 16, fNColl0025);
    am->FillNtupleDColumn(2, 17, fTime0025 >= 0. ? fTime0025 / ns : -1.);
    am->FillNtupleDColumn(2, 18, post->GetKineticEnergy() / eV);
    am->FillNtupleSColumn(2, 19, preVol ? preVol->GetName() : G4String("OutOfWorld"));
    am->FillNtupleSColumn(2, 20, proc ? proc->GetProcessName() : G4String("none"));
    am->FillNtupleDColumn(2, 21, post->GetGlobalTime() / ns);
    am->FillNtupleDColumn(2, 22, fTimeEntry / ns);
    am->AddNtupleRow(2);
}
