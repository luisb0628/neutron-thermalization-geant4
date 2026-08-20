#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "SourceMessenger.hh"
#include "TransmittedSD.hh"
#include "SourceMonitorSD.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Orb.hh"
#include "G4SubtractionSolid.hh"
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

namespace {
    // Dimensiones de la cápsula sellada (ver BuildSealedSource).
    constexpr G4double kCapsuleOuterR  = 3.0*mm;
    constexpr G4double kCapsuleOuterHz = 5.0*mm;
    // Radio del volumen "monitor" que envuelve cada cápsula (debe ser mayor
    // que su radio de esquina, sqrt(3^2+5^2) ~= 5.83 mm).
    constexpr G4double kSourceMonitorR = 7.0*mm;

    // Caja blindaje de aleación tungsteno-acero (60/40 en masa), hueca
    // (pared 5 mm), que aloja ambas fuentes selladas en su cavidad de aire.
    // Dimensiones externas fijas: Ancho(X)=33.5 cm, Alto(Y)=20 cm, Espesor(Z)=5.4 cm.
    constexpr G4double kShieldHalfX = 16.75*cm;   // 33.5 cm (ancho)
    constexpr G4double kShieldHalfY = 10.00*cm;   // 20 cm (alto)
    constexpr G4double kShieldHalfZ = 2.70*cm;    // 5.4 cm (espesor, dirección del haz)
    constexpr G4double kShieldWall  = 5.0*mm;

    // Bloque de plomo adicional, entre la caja blindaje y la parafina.
    // Cubre solo la MITAD del ancho (X) de la caja — el lado donde está la
    // fuente Cs-137 — y el alto (Y) completo. El lado sin plomo (AmBe) queda
    // como un hueco de aire del mismo espesor.
    constexpr G4double kLeadHalfX = kShieldHalfX / 2.;  // 8.375 cm -> 16.75 cm de ancho (mitad de 33.5 cm)
    constexpr G4double kLeadHalfY = kShieldHalfY;       // 10 cm -> 20 cm (igual que la caja)
}

// ------------------------------------------------------------
// Posiciones de emisión de las fuentes: centradas en la cavidad de la caja
// blindaje (en Z). AmBe queda en el centro del ancho (X=0) de la caja;
// Cs-137 se desplaza (fCs137Offset) hacia el centro de la mitad cubierta
// por el plomo. fSourceDistance es un offset extra opcional en Z
// (0 = centradas).
// ------------------------------------------------------------
G4ThreeVector DetectorConstruction::GetAmBePosition() const
{
    G4double z = -(fParaffinZ + 2.*fLeadZ + kShieldHalfZ + fSourceDistance);
    G4double x = 0.;   // centro del ancho de la caja blindaje
    return G4ThreeVector(x, 0., z);
}

G4ThreeVector DetectorConstruction::GetCs137Position() const
{
    return GetAmBePosition() + fCs137Offset;
}

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   fParaffinX(5*cm/2),
   fParaffinY(5*cm/2),
   fParaffinZ(5*cm/2),
   fLeadZ(1.*cm),
   fSourceDistance(0.*cm),
   fCs137Offset(-kShieldHalfX / 2., 0., 0.),
   fActivity(2.98),
   fMessenger(nullptr),
   fSourceMessenger(nullptr)
{
    fMessenger = new DetectorMessenger(this);
    fSourceMessenger = new SourceMessenger(this);
}

// ------------------------------------------------------------
// Destructor
// ------------------------------------------------------------
DetectorConstruction::~DetectorConstruction()
{
    delete fMessenger;
    delete fSourceMessenger;
}

// ------------------------------------------------------------
// Construcción de la geometría
// ------------------------------------------------------------
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto nist = G4NistManager::Instance();

    // --- Mundo ---
    G4Material* worldMat = nist->FindOrBuildMaterial("G4_AIR");
    G4double worldHalfXY = std::max({fParaffinX, fParaffinY, kShieldHalfX, kShieldHalfY}) + 20.*cm;
    G4double worldHalfZ  = fParaffinZ + 2.*fLeadZ + 2.*kShieldHalfZ + 50.*cm;
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

    // --- Bloque de plomo parcial (lado -X, donde está la fuente Cs-137) ---
    // Entre la caja blindaje y la parafina. Cara frontal pegada a cara
    // trasera de la parafina (z = -fParaffinZ). Cubre x ∈ [-kShieldHalfX, 0]
    // (mitad del ancho de la caja); del lado AmBe (x>0) queda aire.
    G4Material* lead = nist->FindOrBuildMaterial("G4_Pb");
    auto solidLead = new G4Box("Lead", kLeadHalfX, kLeadHalfY, fLeadZ);
    auto logicLead = new G4LogicalVolume(solidLead, lead, "Lead");
    G4double xLead = -kLeadHalfX;
    G4double zLead = -(fParaffinZ + fLeadZ);
    new G4PVPlacement(0, G4ThreeVector(xLead,0,zLead), logicLead, "Lead", logicWorld, false, 0);

    auto visLead = new G4VisAttributes(G4Colour(0.4, 0.4, 0.45, 0.7));
    visLead->SetForceSolid(true);
    logicLead->SetVisAttributes(visLead);

    auto visParaffin = new G4VisAttributes(G4Colour(1.0, 1.0, 0.6, 0.6));
    visParaffin->SetForceSolid(true);
    logicBlock->SetVisAttributes(visParaffin);

    // --- Caja blindaje de aleación tungsteno-acero (60/40 en masa) ---
    // Carcasa hueca (paredes de 5 mm) que aloja ambas fuentes selladas en su
    // cavidad de aire. Cara frontal pegada a cara trasera del plomo
    // (z = -(fParaffinZ + 2·fLeadZ)). Centro de la caja en
    // z = -(fParaffinZ + 2·fLeadZ + kShieldHalfZ).
    G4Material* wSteelAlloy = G4Material::GetMaterial("WSteelAlloy");
    if (!wSteelAlloy) {
        G4Material* tungsten = nist->FindOrBuildMaterial("G4_W");
        G4Material* steel    = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
        // Densidad por regla de mezcla en masa (1/rho = Σ w_i/rho_i)
        // con rho_W = 19.3 g/cm3, rho_acero = 8.0 g/cm3 → rho ≈ 12.33 g/cm3
        wSteelAlloy = new G4Material("WSteelAlloy", 12.33*g/cm3, 2);
        wSteelAlloy->AddMaterial(tungsten, 0.60);
        wSteelAlloy->AddMaterial(steel, 0.40);
    }

    auto solidShieldOuter = new G4Box("ShieldOuter", kShieldHalfX, kShieldHalfY, kShieldHalfZ);
    auto solidShieldInner = new G4Box("ShieldInner",
                                       kShieldHalfX - kShieldWall,
                                       kShieldHalfY - kShieldWall,
                                       kShieldHalfZ - kShieldWall);
    auto solidShield = new G4SubtractionSolid("Shield", solidShieldOuter, solidShieldInner);
    auto logicShield = new G4LogicalVolume(solidShield, wSteelAlloy, "Shield");
    G4double zShield = -(fParaffinZ + 2.*fLeadZ + kShieldHalfZ);
    new G4PVPlacement(0, G4ThreeVector(0,0,zShield), logicShield, "Shield", logicWorld, false, 0);

    auto visShield = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.7));
    visShield->SetForceSolid(true);
    logicShield->SetVisAttributes(visShield);

    // --- Fuentes selladas (AmBe + Cs-137), doble encapsulado en acero inoxidable,
    // dentro de la cavidad de aire de la caja blindaje (repartidas en mitades
    // opuestas del ancho, ver GetAmBePosition/GetCs137Position — única fuente
    // de verdad, también usada por PrimaryGeneratorAction). Colocadas como
    // hijas directas de World: caen geométricamente dentro de la cavidad de
    // "Shield" (que ahí es aire, por la resta), sin solape.
    auto logicAmBeCapsule = BuildSealedSource("AmBeSource");
    auto logicAmBeMonitor = BuildSourceMonitor("AmBeMonitor", logicAmBeCapsule);
    new G4PVPlacement(0, GetAmBePosition(), logicAmBeMonitor, "AmBeMonitor", logicWorld, false, 0);

    auto logicCs137Capsule = BuildSealedSource("Cs137Source");
    auto logicCs137Monitor = BuildSourceMonitor("Cs137Monitor", logicCs137Capsule);
    new G4PVPlacement(0, GetCs137Position(), logicCs137Monitor, "Cs137Monitor", logicWorld, false, 0);

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
// Fuente sellada con doble encapsulado de acero inoxidable
// (construcción típica de fuentes selladas industriales: capsula
// interna + espacio de aire + cápsula externa soldada).
// El GPS de PrimaryGeneratorAction emite exactamente en el centro
// (posición del PVPlacement de esta LV), por lo que las partículas
// deben atravesar ambas paredes de acero para salir al mundo.
// ------------------------------------------------------------
G4LogicalVolume* DetectorConstruction::BuildSealedSource(const G4String& name)
{
    auto nist = G4NistManager::Instance();
    G4Material* steel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    G4Material* air   = nist->FindOrBuildMaterial("G4_AIR");

    // --- Cápsula externa (acero, sólida) ---
    G4double rOuter = kCapsuleOuterR, hzOuter = kCapsuleOuterHz;
    auto solidOuter = new G4Tubs(name+"_Outer", 0., rOuter, hzOuter, 0., CLHEP::twopi);
    auto logicOuter = new G4LogicalVolume(solidOuter, steel, name+"_Outer");

    // --- Espacio de aire entre cápsulas (pared externa = 0.5 mm) ---
    G4double rGap = 2.5*mm, hzGap = 4.5*mm;
    auto solidGap = new G4Tubs(name+"_Gap", 0., rGap, hzGap, 0., CLHEP::twopi);
    auto logicGap = new G4LogicalVolume(solidGap, air, name+"_Gap");
    new G4PVPlacement(0, G4ThreeVector(), logicGap, name+"_Gap", logicOuter, false, 0);

    // --- Cápsula interna (acero, pared = 0.3 mm respecto al espacio de aire) ---
    G4double rInner = 2.2*mm, hzInner = 4.2*mm;
    auto solidInner = new G4Tubs(name+"_Inner", 0., rInner, hzInner, 0., CLHEP::twopi);
    auto logicInner = new G4LogicalVolume(solidInner, steel, name+"_Inner");
    new G4PVPlacement(0, G4ThreeVector(), logicInner, name+"_Inner", logicGap, false, 0);

    // --- Cavidad activa (pared interna = 0.5 mm); el GPS emite desde su centro ---
    G4double rCore = 1.7*mm, hzCore = 3.7*mm;
    auto solidCore = new G4Tubs(name+"_Core", 0., rCore, hzCore, 0., CLHEP::twopi);
    auto logicCore = new G4LogicalVolume(solidCore, air, name+"_Core");
    new G4PVPlacement(0, G4ThreeVector(), logicCore, name+"_Core", logicInner, false, 0);

    auto visSteel = new G4VisAttributes(G4Colour(0.75, 0.75, 0.8, 1.0));
    visSteel->SetForceSolid(true);
    logicOuter->SetVisAttributes(visSteel);
    logicInner->SetVisAttributes(visSteel);
    logicGap->SetVisAttributes(G4VisAttributes::GetInvisible());
    logicCore->SetVisAttributes(G4VisAttributes::GetInvisible());

    return logicOuter;
}

// ------------------------------------------------------------
// Volumen "monitor": esfera de aire que envuelve por completo la cápsula
// sellada (con margen). Como es la madre directa de la cápsula, cualquier
// partícula que sale del acero entra a este volumen.
// ------------------------------------------------------------
G4LogicalVolume* DetectorConstruction::BuildSourceMonitor(const G4String& name, G4LogicalVolume* capsuleLV)
{
    auto nist = G4NistManager::Instance();
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR");

    auto solidMonitor = new G4Orb(name, kSourceMonitorR);
    auto logicMonitor = new G4LogicalVolume(solidMonitor, air, name);
    new G4PVPlacement(0, G4ThreeVector(), capsuleLV, capsuleLV->GetName(), logicMonitor, false, 0);
    logicMonitor->SetVisAttributes(G4VisAttributes::GetInvisible());

    return logicMonitor;
}

// ------------------------------------------------------------
// Detectores sensibles
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

    SourceMonitorSD* monitorSD = nullptr;
    auto existingMonitorSD = sdman->FindSensitiveDetector("SourceMonitorSD", false);
    if (!existingMonitorSD) {
        monitorSD = new SourceMonitorSD("SourceMonitorSD");
        sdman->AddNewDetector(monitorSD);
    } else {
        monitorSD = static_cast<SourceMonitorSD*>(existingMonitorSD);
    }
    for (const auto& monitorName : {"AmBeMonitor", "Cs137Monitor"}) {
        auto monitorLV = G4LogicalVolumeStore::GetInstance()->GetVolume(monitorName);
        if (monitorLV) monitorLV->SetSensitiveDetector(monitorSD);
    }
}
