#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "TransmittedSD.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4RegionStore.hh"
#include "G4ProductionCuts.hh"
#include "G4Region.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"
#include "CLHEP/Units/SystemOfUnits.h"

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   fParaffinX(5*cm/2),
   fParaffinY(5*cm/2),
   fParaffinZ(5*cm/2),
   fLeadZ(1.5*cm),
   fMessenger(nullptr)
{
    fMessenger = new DetectorMessenger(this);
}

// ------------------------------------------------------------
// Destructor
// ------------------------------------------------------------
DetectorConstruction::~DetectorConstruction()
{
    delete fMessenger;
}

// ------------------------------------------------------------
// Construcción de la geometría
// ------------------------------------------------------------
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto nist = G4NistManager::Instance();

    // --- Mundo ---
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_AIR");
    G4double worldHalfXY = std::max({fParaffinX, fParaffinY, 10.*cm}) + 20.*cm;
    G4double worldHalfZ  = fParaffinZ + 2.*fLeadZ + 50.*cm;
    auto solidWorld = new G4Box("World", worldHalfXY, worldHalfXY, worldHalfZ);
    auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    auto physWorld  = new G4PVPlacement(0, {}, logicWorld, "World", 0, false, 0);

    // --- Bloque de parafina (moderador), centrado en z=0 ---
    G4Material* paraffin = G4Material::GetMaterial("Paraffin");
    if (!paraffin) {
        paraffin = new G4Material("Paraffin", 0.93*g/cm3, 2);
        paraffin->AddElement(nist->FindOrBuildElement("C"), 1);
        paraffin->AddElement(nist->FindOrBuildElement("H"), 2);
    }
    auto solidBlock = new G4Box("Block", fParaffinX, fParaffinY, fParaffinZ);
    auto logicBlock = new G4LogicalVolume(solidBlock, paraffin, "Block");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), logicBlock, "Block", logicWorld, false, 0);

    // --- Bloque de plomo (20x20 cm area, espesor variable) ---
    // Cara trasera pegada a cara frontal de parafina (z = -fParaffinZ)
    // Centro del plomo en z = -(fParaffinZ + fLeadZ)
    G4Material* lead = nist->FindOrBuildMaterial("G4_Pb");
    auto solidLead = new G4Box("Lead", 10.*cm, 10.*cm, fLeadZ);
    auto logicLead = new G4LogicalVolume(solidLead, lead, "Lead");
    G4double zLead = -(fParaffinZ + fLeadZ);
    new G4PVPlacement(0, G4ThreeVector(0,0,zLead), logicLead, "Lead", logicWorld, false, 0);

    auto visLead = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.7));
    visLead->SetForceSolid(true);
    logicLead->SetVisAttributes(visLead);

    auto visParaffin = new G4VisAttributes(G4Colour(1.0, 1.0, 0.6, 0.6));
    visParaffin->SetForceSolid(true);
    logicBlock->SetVisAttributes(visParaffin);

    // --- Detector cilíndrico justo después de la parafina ---
    G4double detRadius = 1.3*cm;
    G4double detHalfZ  = 0.5*mm;
    auto detMat   = nist->FindOrBuildMaterial("G4_AIR");
    auto solidDet = new G4Tubs("Detector", 0., detRadius, detHalfZ, 0., CLHEP::twopi);
    auto logicDet = new G4LogicalVolume(solidDet, detMat, "Detector");
    G4double zPos = fParaffinZ + 0.1*cm + detHalfZ;
    new G4PVPlacement(0, G4ThreeVector(0,0,zPos), logicDet, "Detector", logicWorld, false, 0);

    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Cortes de producción ---
    auto cuts = new G4ProductionCuts();
    cuts->SetProductionCut(0.001*mm, "neutron");
    cuts->SetProductionCut(0.001*mm, "gamma");
    cuts->SetProductionCut(0.001*mm, "e-");

    // --- Región del detector ---
    G4Region* region = G4RegionStore::GetInstance()->GetRegion("DetectorRegion", false);
    if (!region) {
        region = new G4Region("DetectorRegion");
        region->SetProductionCuts(cuts);
    }
    region->AddRootLogicalVolume(logicBlock);
    region->AddRootLogicalVolume(logicDet);

    return physWorld;
}

// ------------------------------------------------------------
// Detector sensible
// ------------------------------------------------------------
void DetectorConstruction::ConstructSDandField()
{
    G4SDManager* sdman = G4SDManager::GetSDMpointer();
    TransmittedSD* sd = nullptr;

    auto existingSD = sdman->FindSensitiveDetector("TransmittedSD", false);
    if (!existingSD) {
        sd = new TransmittedSD("TransmittedSD");
        sdman->AddNewDetector(sd);
    } else {
        sd = static_cast<TransmittedSD*>(existingSD);
    }

    auto lv = G4LogicalVolumeStore::GetInstance()->GetVolume("Detector");
    if (lv) lv->SetSensitiveDetector(sd);
}
