#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "SourceMessenger.hh"

#include "G4GeneralParticleSource.hh"
#include "G4SPSEneDistribution.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Gamma.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include "Randomize.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction(const DetectorConstruction* detector)
 : fDetector(detector),
   fSourceDistance(0.*cm),
   fActivity(2.98),
   fFrac_nGND(0.316),
   fFrac_n1st(0.465)
   // fFrac_n2nd = 1 - fFrac_nGND - fFrac_n1st ~= 0.219
{
    fGPS_nGND = new G4GeneralParticleSource();
    fGPS_n1st = new G4GeneralParticleSource();
    fGPS_n2nd = new G4GeneralParticleSource();

    for (auto* gps : {fGPS_nGND, fGPS_n1st, fGPS_n2nd}) {
        gps->GetCurrentSource()->SetParticleDefinition(
            G4ParticleTable::GetParticleTable()->FindParticle("neutron"));
        gps->GetCurrentSource()->GetAngDist()->SetAngDistType("iso");
        gps->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
    }

    BuildSpectrum_nGND(fGPS_nGND);
    BuildSpectrum_n1st(fGPS_n1st);
    BuildSpectrum_n2nd(fGPS_n2nd);

    fMessenger = new SourceMessenger(this);

    G4cout << "\n====================================================\n"
           << "  Fuente AmBe configurada (3 canales: nGND/n1st/n2nd)\n"
           << "  Fracciones    : nGND=" << fFrac_nGND*100. << "%  n1st=" << fFrac_n1st*100.
           << "%  n2nd=" << (1.-fFrac_nGND-fFrac_n1st)*100. << "%\n"
           << "  Actividad     : " << fActivity  << " Ci\n"
           << "  Distancia     : " << fSourceDistance/cm << " cm\n"
           << "  Tasa n (est.) : " << fActivity * 2.2e6 << " n/s\n"
           << "====================================================\n" << G4endl;
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fMessenger;
    delete fGPS_nGND;
    delete fGPS_n1st;
    delete fGPS_n2nd;
}

// ------------------------------------------------------------
// PLACEHOLDER: reemplazar por los valores digitalizados de
// De Guarrini & Malaroda (1971) / Geiger & Van der Zwan (1975)
// para cada canal — de momento son formas aproximadas, no reales
// ------------------------------------------------------------
void PrimaryGeneratorAction::BuildSpectrum_nGND(G4GeneralParticleSource* gps)
{
    // Canal al estado fundamental del 12C: grupo de neutrones más energético
    auto* eDist = gps->GetCurrentSource()->GetEneDist();
    eDist->SetEnergyDisType("Arb");
    const std::vector<std::pair<G4double,G4double>> spectrum = {
        {3.00, 0.00}, {4.00, 0.15}, {5.00, 0.55},
        {6.00, 1.00}, {6.50, 0.85}, {7.00, 0.45},
        {7.50, 0.15}, {8.00, 0.00}
    };
    for (const auto& [e, p] : spectrum) eDist->ArbEnergyHisto(G4ThreeVector(e*MeV, p, 0.));
    eDist->ArbInterpolate("Lin");
}

void PrimaryGeneratorAction::BuildSpectrum_n1st(G4GeneralParticleSource* gps)
{
    // Canal dominante: transición al 1er estado excitado (4.438 MeV) del 12C
    auto* eDist = gps->GetCurrentSource()->GetEneDist();
    eDist->SetEnergyDisType("Arb");
    const std::vector<std::pair<G4double,G4double>> spectrum = {
        {0.50, 0.00}, {1.50, 0.35}, {2.50, 0.75},
        {3.50, 1.00}, {4.50, 0.80}, {5.00, 0.45},
        {5.50, 0.15}, {6.00, 0.00}
    };
    for (const auto& [e, p] : spectrum) eDist->ArbEnergyHisto(G4ThreeVector(e*MeV, p, 0.));
    eDist->ArbInterpolate("Lin");
}

void PrimaryGeneratorAction::BuildSpectrum_n2nd(G4GeneralParticleSource* gps)
{
    // Canal blando: incluye breakup en 3 alfas (por encima del umbral ~7.65 MeV, estado de Hoyle)
    auto* eDist = gps->GetCurrentSource()->GetEneDist();
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
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // Orden: fuente → plomo → parafina → detector
    // Fuente a fSourceDistance delante de la cara frontal del plomo
    G4double z = -(fDetector->GetParaffinZ() + 2.*fDetector->GetLeadZ() + fSourceDistance);
    G4ThreeVector srcPos(0., 0., z);

    // Selección del canal según las fracciones medidas
    G4double r = G4UniformRand();
    G4GeneralParticleSource* activeGPS = nullptr;
    G4bool emitGamma = false;

    if (r < fFrac_nGND) {
        activeGPS = fGPS_nGND;
    } else if (r < fFrac_nGND + fFrac_n1st) {
        activeGPS = fGPS_n1st;
        emitGamma = true;   // n1st siempre va acompañado del gamma de 4438 keV
    } else {
        activeGPS = fGPS_n2nd;
    }

    activeGPS->GetCurrentSource()->GetPosDist()->SetCentreCoords(srcPos);
    activeGPS->GeneratePrimaryVertex(event);

    if (emitGamma) EmitCoincidentGamma(event, srcPos);
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
}
