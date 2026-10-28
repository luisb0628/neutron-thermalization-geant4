#ifndef SourceMode_h
#define SourceMode_h 1

#include "globals.hh"

// Modo de fuente activo para esta corrida: separa AmBe (neutrones) y Cs-137
// (gammas) en simulaciones independientes, porque el Cs-137 produce muchos
// mas fotones por evento que la cadena de neutrones del AmBe, y mezclar
// ambas fuentes 1 evento = 1 neutron + 1 gamma no refleja la estadistica
// real de cada una. Los dos .root resultantes se combinan despues en el
// analisis (no aqui).
enum class SourceMode { AmBeOnly, Cs137Only, Both };

// Estado simple de proceso (no por-hilo): se fija una sola vez, desde el
// macro, antes de /run/initialize y /run/beamOn, igual que el resto de los
// parametros de fuente de este proyecto (ver SourceMessenger). No toca
// DetectorConstruction para no mezclar configuracion de fuente con geometria.
namespace SourceModeConfig {
    void SetMode(SourceMode mode);
    SourceMode GetMode();

    // Nombre de archivo .root de salida segun el modo activo.
    G4String GetOutputFileName();
}

#endif
