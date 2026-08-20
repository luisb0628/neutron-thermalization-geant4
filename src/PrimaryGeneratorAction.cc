#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"

#include "G4SingleParticleSource.hh"
#include "G4SPSEneDistribution.hh"
#include "G4ParticleTable.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"
#include "G4Gamma.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "G4AnalysisManager.hh"
#include "Randomize.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction(DetectorConstruction* detector)
 : fDetector(detector),
   fFrac_nGND(0.316),
   fFrac_n1st(0.465)
   // fFrac_n2nd = 1 - fFrac_nGND - fFrac_n1st ~= 0.219
{
    fGPS_nGND = new G4SingleParticleSource();
    fGPS_n1st = new G4SingleParticleSource();
    fGPS_n2nd = new G4SingleParticleSource();
    fGPS_Cs137 = new G4SingleParticleSource();

    for (auto* gps : {fGPS_nGND, fGPS_n1st, fGPS_n2nd}) {
        gps->SetParticleDefinition(
            G4ParticleTable::GetParticleTable()->FindParticle("neutron"));
        gps->GetAngDist()->SetAngDistType("iso");
        gps->GetPosDist()->SetPosDisType("Point");
    }

    BuildSpectrum_nGND(fGPS_nGND);
    BuildSpectrum_n1st(fGPS_n1st);
    BuildSpectrum_n2nd(fGPS_n2nd);
    BuildCs137Source();

    G4cout << "\n====================================================\n"
           << "  Fuente AmBe configurada (3 canales: nGND/n1st/n2nd)\n"
           << "  Fracciones    : nGND=" << fFrac_nGND*100. << "%  n1st=" << fFrac_n1st*100.
           << "%  n2nd=" << (1.-fFrac_nGND-fFrac_n1st)*100. << "%\n"
           << "  Actividad     : " << fDetector->GetActivity()  << " Ci\n"
           << "  Distancia     : " << fDetector->GetSourceDistance()/cm << " cm\n"
           << "  Tasa n (est.) : " << fDetector->GetActivity() * 2.2e6 << " n/s\n"
           << "  Fuente Cs-137 : 661.657 keV, offset " << fDetector->GetCs137Offset()/cm << " cm respecto a AmBe\n"
           << "====================================================\n" << G4endl;
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fGPS_nGND;
    delete fGPS_n1st;
    delete fGPS_n2nd;
    delete fGPS_Cs137;
}

G4double PrimaryGeneratorAction::GetActivity()       const { return fDetector->GetActivity(); }
G4double PrimaryGeneratorAction::GetSourceDistance() const { return fDetector->GetSourceDistance(); }

// ------------------------------------------------------------
// PLACEHOLDER: reemplazar por los valores digitalizados de
// De Guarrini & Malaroda (1971) / Geiger & Van der Zwan (1975)
// para cada canal — de momento son formas aproximadas, no reales
// ------------------------------------------------------------
void PrimaryGeneratorAction::BuildSpectrum_nGND(G4SingleParticleSource* gps)
{
    // Canal al estado fundamental del 12C: grupo de neutrones más energético
    auto* eDist = gps->GetEneDist();
    eDist->SetEnergyDisType("Arb");
    const std::vector<std::pair<G4double,G4double>> spectrum = {
        {3.00, 0.00}, {4.00, 0.15}, {5.00, 0.55},
        {6.00, 1.00}, {6.50, 0.85}, {7.00, 0.45},
        {7.50, 0.15}, {8.00, 0.00}
    };
    for (const auto& [e, p] : spectrum) eDist->ArbEnergyHisto(G4ThreeVector(e*MeV, p, 0.));
    eDist->ArbInterpolate("Lin");
}

void PrimaryGeneratorAction::BuildSpectrum_n1st(G4SingleParticleSource* gps)
{
    // Canal dominante: transición al 1er estado excitado (4.438 MeV) del 12C
    auto* eDist = gps->GetEneDist();
    eDist->SetEnergyDisType("Arb");
    const std::vector<std::pair<G4double,G4double>> spectrum = {
        {0.50, 0.00}, {1.50, 0.35}, {2.50, 0.75},
        {3.50, 1.00}, {4.50, 0.80}, {5.00, 0.45},
        {5.50, 0.15}, {6.00, 0.00}
    };
    for (const auto& [e, p] : spectrum) eDist->ArbEnergyHisto(G4ThreeVector(e*MeV, p, 0.));
    eDist->ArbInterpolate("Lin");
}

void PrimaryGeneratorAction::BuildSpectrum_n2nd(G4SingleParticleSource* gps)
{
    // Canal blando: incluye breakup en 3 alfas (por encima del umbral ~7.65 MeV, estado de Hoyle)
    auto* eDist = gps->GetEneDist();
    eDist->SetEnergyDisType("Arb");
    const std::vector<std::pair<G4double,G4double>> spectrum = {
        {0.00, 0.00}, {0.50, 0.60}, {1.00, 1.00},
        {1.50, 0.75}, {2.00, 0.40}, {2.50, 0.10},
        {3.00, 0.00}
    };
    for (const auto& [e, p] : spectrum) eDist->ArbEnergyHisto(G4ThreeVector(e*MeV, p, 0.));
    eDist->ArbInterpolate("Lin");
}

// ------------------------------------------------------------
// Fuente Cs-137: gamma monoenergético de 661.657 keV (transición del
// Ba-137m), emisión isotrópica desde un punto fijo junto a la AmBe
// ------------------------------------------------------------
void PrimaryGeneratorAction::BuildCs137Source()
{
    fGPS_Cs137->SetParticleDefinition(
        G4ParticleTable::GetParticleTable()->FindParticle("gamma"));
    fGPS_Cs137->GetAngDist()->SetAngDistType("iso");
    fGPS_Cs137->GetPosDist()->SetPosDisType("Point");

    auto* eDist = fGPS_Cs137->GetEneDist();
    eDist->SetEnergyDisType("Mono");
    eDist->SetMonoEnergy(0.661657*MeV);
}

// ------------------------------------------------------------
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // Posición real de la cápsula AmBe (misma fuente de verdad que la geometría)
    G4ThreeVector srcPos = fDetector->GetAmBePosition();

    // Selección del canal según las fracciones medidas
    G4double r = G4UniformRand();
    G4SingleParticleSource* activeGPS = nullptr;
    G4bool emitGamma = false;

    if (r < fFrac_nGND) {
        activeGPS = fGPS_nGND;
    } else if (r < fFrac_nGND + fFrac_n1st) {
        activeGPS = fGPS_n1st;
        emitGamma = true;   // n1st siempre va acompañado del gamma de 4438 keV
    } else {
        activeGPS = fGPS_n2nd;
    }

    activeGPS->GetPosDist()->SetCentreCoords(srcPos);
    activeGPS->GeneratePrimaryVertex(event);
    RecordSourceSpectrum(event->GetPrimaryVertex(event->GetNumberOfPrimaryVertex() - 1));

    if (emitGamma) EmitCoincidentGamma(event, srcPos);

    // Fuente Cs-137, punto fijo separado de la AmBe (decae independiente,
    // un gamma de 661.657 keV por evento junto con el canal de neutrones)
    fGPS_Cs137->GetPosDist()->SetCentreCoords(fDetector->GetCs137Position());
    fGPS_Cs137->GeneratePrimaryVertex(event);
    RecordSourceSpectrum(event->GetPrimaryVertex(event->GetNumberOfPrimaryVertex() - 1));
}

// ------------------------------------------------------------
void PrimaryGeneratorAction::EmitCoincidentGamma(G4Event* event, const G4ThreeVector& pos)
{
    // Emisión isotrópica, independiente de la dirección del neutrón.
    // Ensanchamiento Doppler aproximado (Ito et al. reportan FWHM ~115-120 keV
    // a 4438 keV, equivalente a sigma/E ~ 0.9%)
    G4double cosTheta = 2.*G4UniformRand() - 1.;
    G4double sinTheta = std::sqrt(1. - cosTheta*cosTheta);
    G4double phi = CLHEP::twopi * G4UniformRand();
    G4ThreeVector dir(sinTheta*std::cos(phi), sinTheta*std::sin(phi), cosTheta);

    auto* vertex = new G4PrimaryVertex(pos, 0.);
    auto* gamma  = new G4PrimaryParticle(G4Gamma::Gamma());
    G4double E = G4RandGauss::shoot(4.438*MeV, 0.009*4.438*MeV);
    gamma->SetMomentumDirection(dir);
    gamma->SetKineticEnergy(E);
    vertex->SetPrimary(gamma);
    event->AddPrimaryVertex(vertex);
    RecordSourceSpectrum(vertex);
}

// ------------------------------------------------------------
void PrimaryGeneratorAction::RecordSourceSpectrum(const G4PrimaryVertex* vertex)
{
    auto* primary = vertex->GetPrimary();
    auto* am = G4AnalysisManager::Instance();

    am->FillNtupleSColumn(1, 0, primary->GetParticleDefinition()->GetParticleName());
    am->FillNtupleDColumn(1, 1, primary->GetKineticEnergy() / eV);
    am->FillNtupleDColumn(1, 2, vertex->GetX0() / cm);
    am->FillNtupleDColumn(1, 3, vertex->GetY0() / cm);
    am->FillNtupleDColumn(1, 4, vertex->GetZ0() / cm);
    am->FillNtupleDColumn(1, 5, primary->GetMomentumDirection().x());
    am->FillNtupleDColumn(1, 6, primary->GetMomentumDirection().y());
    am->FillNtupleDColumn(1, 7, primary->GetMomentumDirection().z());
    am->AddNtupleRow(1);
}
