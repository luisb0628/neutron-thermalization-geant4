#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "SourceMessenger.hh"

#include "G4GeneralParticleSource.hh"
#include "G4SPSEneDistribution.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction(const DetectorConstruction* detector)
 : fDetector(detector),
   fSourceDistance(10.*cm),
   fActivity(2.98)
{
    fGPS      = new G4GeneralParticleSource();
    fGammaGun = new G4ParticleGun(1);

    auto* table = G4ParticleTable::GetParticleTable();
    fGammaGun->SetParticleDefinition(table->FindParticle("gamma"));
    fGammaGun->SetParticleEnergy(4.44*MeV);

    BuildAmBeNeutronSource();

    fMessenger = new SourceMessenger(this);

    G4cout << "\n====================================================\n"
           << "  Fuente AmBe configurada\n"
           << "  Actividad     : " << fActivity  << " Ci\n"
           << "  Distancia     : " << fSourceDistance/cm << " cm\n"
           << "  Tasa n (est.) : " << fActivity * 2.2e6 << " n/s\n"
           << "====================================================\n" << G4endl;
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fGammaGun;
    delete fMessenger;
    // GPS es gestionado internamente por Geant4
}

// ------------------------------------------------------------
// Espectro AmBe: ISO 8529-1, rango 0-11 MeV, pico ~4.5 MeV
// ------------------------------------------------------------
void PrimaryGeneratorAction::BuildAmBeNeutronSource()
{
    auto* src = fGPS->GetCurrentSource();

    src->SetParticleDefinition(
        G4ParticleTable::GetParticleTable()->FindParticle("neutron"));

    src->GetAngDist()->SetAngDistType("iso");
    src->GetPosDist()->SetPosDisType("Point");

    auto* eDist = src->GetEneDist();
    eDist->SetEnergyDisType("Arb");

    // Pares (energía en MeV, intensidad relativa) — espectro AmBe ISO 8529-1
    const std::vector<std::pair<G4double,G4double>> spectrum = {
        {0.00, 0.000}, {0.25, 0.026}, {0.50, 0.054},
        {0.75, 0.085}, {1.00, 0.120}, {1.50, 0.180},
        {2.00, 0.240}, {2.50, 0.295}, {3.00, 0.355},
        {3.50, 0.415}, {4.00, 0.470}, {4.50, 0.505},
        {5.00, 0.480}, {5.50, 0.395}, {6.00, 0.280},
        {6.50, 0.175}, {7.00, 0.105}, {7.50, 0.060},
        {8.00, 0.033}, {8.50, 0.018}, {9.00, 0.009},
        {9.50, 0.004}, {10.00, 0.002}, {10.50, 0.001},
        {11.00, 0.000}
    };

    for (const auto& [e, p] : spectrum) {
        eDist->ArbEnergyHisto(G4ThreeVector(e*MeV, p, 0.));
    }
    eDist->ArbInterpolate("Lin");
}

// ------------------------------------------------------------
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // Orden: fuente → plomo → parafina → detector
    // Fuente a fSourceDistance delante de la cara frontal del plomo
    G4double z = -(fDetector->GetParaffinZ() + 2.*fDetector->GetLeadZ() + fSourceDistance);
    G4ThreeVector srcPos(0., 0., z);

    fGPS->GetCurrentSource()->GetPosDist()->SetCentreCoords(srcPos);
    fGPS->GeneratePrimaryVertex(event);

    // ~60% de los neutrones AmBe llevan un gamma 4.44 MeV (C-12* → C-12)
    if (G4UniformRand() < 0.6) {
        G4double cosTheta = 2.*G4UniformRand() - 1.;
        G4double sinTheta = std::sqrt(1. - cosTheta*cosTheta);
        G4double phi      = CLHEP::twopi * G4UniformRand();
        G4ThreeVector dir(sinTheta*std::cos(phi), sinTheta*std::sin(phi), cosTheta);

        fGammaGun->SetParticlePosition(srcPos);
        fGammaGun->SetParticleMomentumDirection(dir);
        fGammaGun->GeneratePrimaryVertex(event);
    }
}
