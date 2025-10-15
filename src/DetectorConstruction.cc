#include "DetectorConstruction.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "TransmittedSD.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4UserLimits.hh"
#include "G4ProductionCuts.hh"
#include "G4Region.hh"
#include "G4VisAttributes.hh"


DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() = default;

G4VPhysicalVolume* DetectorConstruction::Construct() {
    auto nist = G4NistManager::Instance();

    // --- Mundo ---
    auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidWorld = new G4Box("World", 20*cm, 20*cm, 20*cm);
    auto logicWorld = new G4LogicalVolume(solidWorld, worldMat, "World");
    auto physWorld  = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    // --- Bloque de parafina (moderador) ---
    //Definir los parámetros de control
    fParaffinThickness = 12.0 * cm;
    G4double halfParaffinZ = fParaffinThickness / 2.0;
    
    auto paraffin = new G4Material("Paraffin", 0.93*g/cm3, 2);
    paraffin->AddElement(nist->FindOrBuildElement("C"), 1);
    paraffin->AddElement(nist->FindOrBuildElement("H"), 2);

    auto solidBlock = new G4Box("Block", 3*cm, 3*cm, halfParaffinZ);
    auto logicBlock = new G4LogicalVolume(solidBlock, paraffin, "Block");
    new G4PVPlacement(0, G4ThreeVector(0,0,0), logicBlock, "Block", logicWorld, false, 0);

    // --- Detector plano ---
    G4double detHalfX = 2*cm, detHalfY = 2*cm;
    G4double detHalfZ = 0.5*mm;
    auto solidDet = new G4Box("Detector", detHalfX, detHalfY, detHalfZ);
    auto detMat = nist->FindOrBuildMaterial("G4_AIR");
    auto logicDet = new G4LogicalVolume(solidDet, detMat, "Detector");
    G4double zPos = halfParaffinZ + 0.1*cm + detHalfZ;
    new G4PVPlacement(0, G4ThreeVector(0,0,zPos), logicDet, "Detector", logicWorld, false, 0);

    // --- Límites de paso para mayor resolución ---
    G4double maxStep = 0.01*mm;
    logicWorld->SetUserLimits(new G4UserLimits(maxStep));
    logicBlock->SetUserLimits(new G4UserLimits(maxStep));
    logicDet->SetUserLimits(new G4UserLimits(maxStep));
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Cortes de producción ---
    auto cuts = new G4ProductionCuts();
    cuts->SetProductionCut(0.001*mm, "neutron");   // permitir neutrones térmicos
    cuts->SetProductionCut(0.001*mm, "gamma");
    cuts->SetProductionCut(0.001*mm, "e-");

    // Asignar cortes a una región
    auto region = new G4Region("DetectorRegion");
    region->AddRootLogicalVolume(logicBlock);
    region->AddRootLogicalVolume(logicDet);
    region->SetProductionCuts(cuts);

    return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdman = G4SDManager::GetSDMpointer();
    auto sd = new TransmittedSD("TransmittedSD");
    sdman->AddNewDetector(sd);

    auto lv = G4LogicalVolumeStore::GetInstance()->GetVolume("Detector");
    lv->SetSensitiveDetector(sd);
}


