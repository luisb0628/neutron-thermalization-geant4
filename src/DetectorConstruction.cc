#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "TransmittedSD.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4RegionStore.hh"
#include "G4ProductionCuts.hh"
#include "G4Region.hh"
#include "G4VisAttributes.hh"
#include "G4SystemOfUnits.hh"

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   fParaffinX(5*cm/2),
   fParaffinY(5*cm/2),
   fParaffinZ(5*cm/2),
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

    // --- Mundo (suficientemente grande para parafina + fuente a distancia) ---
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_AIR");
    G4double worldHalf = std::max({fParaffinX, fParaffinY, fParaffinZ}) * 4 + 30.*cm;
    auto solidWorld = new G4Box("World", worldHalf, worldHalf, worldHalf);
    auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    auto physWorld  = new G4PVPlacement(0, {}, logicWorld, "World", 0, false, 0);

    // --- Bloque de parafina (moderador) ---
    G4Material* paraffin = G4Material::GetMaterial("Paraffin");
    if (!paraffin) {
        paraffin = new G4Material("Paraffin", 0.93*g/cm3, 2);
        paraffin->AddElement(nist->FindOrBuildElement("C"), 1);
        paraffin->AddElement(nist->FindOrBuildElement("H"), 2);
    }

    auto solidBlock = new G4Box("Block", fParaffinX, fParaffinY, fParaffinZ);
    auto logicBlock = new G4LogicalVolume(solidBlock, paraffin, "Block");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), logicBlock, "Block", logicWorld, false, 0);

    // --- Detector plano (cubre toda la cara trasera de la parafina) ---
    G4double detHalfX = fParaffinX, detHalfY = fParaffinY;
    G4double detHalfZ = 0.5*mm;
    auto detMat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidDet = new G4Box("Detector", detHalfX, detHalfY, detHalfZ);
    auto logicDet = new G4LogicalVolume(solidDet, detMat, "Detector");

    // Posición del detector justo después de la parafina
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
