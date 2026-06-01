# Termalización de Neutrones en Parafina con Fuente AmBe (Geant4)

Simulación Monte Carlo de la **termalización de neutrones en parafina** usando **Geant4**.  
Estudia cómo los neutrones de una fuente AmBe pierden energía al atravesar el moderador y caracteriza las partículas que alcanzan un detector cilíndrico de grafeno dopado con boro.

Este proyecto es la **primera etapa** de una cadena de simulación:

1. **Neutron_Thermalization** (este repo) — genera `AmBePhaseSpace.root`
2. **Film_graphene** — captura neutrones en el film grafeno/Kapton
3. **Scintillator_Sipm** — detecta partículas secundarias en un centellador + SiPM

---

## Ramas del repositorio

| Rama | Descripción |
|------|-------------|
| `main` | Versión base: fuente monoenerética 4.2 MeV |
| `ambe_source` | **Rama principal**: fuente AmBe realista con phase space completo |

---

## Descripción física

### Fuente AmBe

La fuente **Americio-Berilio (AmBe)** produce neutrones por la reacción:

```
α + ⁹Be → ¹²C* + n    (~60% → gamma 4.44 MeV)
α + ⁹Be → ¹²C  + n    (~40%)
```

- **Espectro neutrónico:** continuo 0–11 MeV, pico en ~4.5 MeV (ISO 8529-1)
- **Gamma correlacionado:** 4.44 MeV (~60% de eventos)
- **Actividad:** 2.98 Ci → ~6.6 × 10⁶ neutrones/s
- **Emisión:** isotrópica (4π sr), fuente puntual

### Moderador

Bloque de parafina (`G4_PARAFFIN`, ρ = 0.93 g/cm³) configurable en dimensiones desde macro.

### Detector

Cilindro de aire (placeholder de grafeno dopado con boro):
- Diámetro: 2.6 cm, Espesor: 1 mm
- Registra todas las partículas con su phase space completo (tipo, energía, posición, dirección)

---

## Contenido del repositorio

```
.
├── CMakeLists.txt
├── main.cc
├── make_video.sh                    # Pipeline simulación → video MP4 (una geometría)
├── make_all_videos.sh               # Pipeline para las 5 geometrías predefinidas
├── guia_video.pdf                   # Guía para generar los videos de visualización
├── include/
│   ├── DetectorConstruction.hh      # Mundo dinámico, parafina configurable, detector cilíndrico
│   ├── DetectorMessenger.hh         # Comandos /detector/setParaffinX/Y/Z
│   ├── PrimaryGeneratorAction.hh    # Fuente AmBe: GPS + gamma 4.44 MeV correlacionado
│   ├── SourceMessenger.hh           # Comandos /ambe/distance y /ambe/activity
│   ├── TransmittedSD.hh
│   ├── RunAction.hh
│   ├── EventAction.hh
│   └── ActionInitialization.hh
├── src/
├── macros/
│   ├── ambe_run.mac                 # Macro principal (batch, fuente AmBe)
│   ├── vis1.mac                     # Visualización modo 1
│   ├── vis2.mac                     # Visualización modo 2
│   ├── vis_video.mac                # Exporta frames EPS por evento
│   ├── vis_box_frame.mac            # Frame de geometría base para overlay de video
│   ├── vis_box_frame_geo1.mac  …    # Frames de geometría para cada configuración
│   ├── vis_video_geo1.mac  …        # Macros de video para cada configuración
│   └── vis_rotate_frame.mac
└── python/
    ├── ambe_scan.py                 # Barrido paramétrico de geometría y distancia
    ├── accumulate_frames.py         # Acumula frames para el video
    ├── Termalizacion.csv            # Resultados del barrido
    ├── analisis_ambe.ipynb          # Análisis de geometría (heatmaps, 3D, energías)
    └── analisis_distancia.ipynb     # Análisis vs distancia fuente–parafina
```

---

## Requisitos

- **Geant4** ≥ 10.7 compilado con `ui_all`, `vis_all`, `analysis` y física HP
- **CMake** ≥ 3.16, **C++17**
- **Python 3** con: `uproot`, `numpy`, `pandas`, `matplotlib`, `seaborn`, `jupyter`
- **Ghostscript** (`gs`) y **ffmpeg** — solo para generación de video

---

## Compilación

```bash
source /path/to/geant4/install/bin/geant4.sh
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

---

## Ejecución

### Modo batch (recomendado)

```bash
cd build
./Neutron_Thermalization ../macros/ambe_run.mac
```

Genera `AmBePhaseSpace.root` en `build/`.

### Modo interactivo

```bash
cd build
./Neutron_Thermalization
```

---

## Comandos del macro

```bash
# Geometría de la parafina (half-lengths)
/detector/setParaffinX 10 cm
/detector/setParaffinY 10 cm
/detector/setParaffinZ 10 cm

# Fuente AmBe
/ambe/distance 10 cm      # distancia fuente → cara frontal de parafina
/ambe/activity 2.98       # actividad en Ci (solo para normalización)

/run/initialize
/run/beamOn 500000
```

---

## Salida

Archivo ROOT: `AmBePhaseSpace.root` — ntuple `PhaseSpace`

| Columna | Tipo | Descripción |
|---------|------|-------------|
| `Particle` | string | Nombre Geant4 (`neutron`, `gamma`, `e-`, …) |
| `KinE_eV` | double | Energía cinética en eV |
| `PosX_cm` | double | Posición X en cm |
| `PosY_cm` | double | Posición Y en cm |
| `PosZ_cm` | double | Posición Z en cm |
| `DirX` | double | Dirección X (unitario) |
| `DirY` | double | Dirección Y |
| `DirZ` | double | Dirección Z |

Este archivo es el **input de phase space** para Film_graphene.

---

## Barrido paramétrico (`python/ambe_scan.py`)

Corre múltiples simulaciones variando geometría y distancia:

```bash
python python/ambe_scan.py
```

Genera:
- `build/resultados_ambe.csv` — tabla de resultados por configuración
- `build/AmBe_{WxHxD}cm_d{dist}cm.root` — ROOT por configuración

---

## Generación de video

```bash
# Una geometría
./make_video.sh -n 120 -f 60 -o video_termalizacion.mp4

# Las 5 geometrías predefinidas
./make_all_videos.sh
```

Consulta `guia_video.pdf` para instrucciones detalladas.

---

## Estructura del código

| Clase | Responsabilidad |
|-------|----------------|
| `DetectorConstruction` | Mundo dinámico, bloque de parafina configurable, detector G4Tubs |
| `DetectorMessenger` | Comandos `/detector/setParaffinX/Y/Z` |
| `PrimaryGeneratorAction` | GPS con espectro AmBe ISO 8529-1 + gamma 4.44 MeV correlacionado |
| `SourceMessenger` | Comandos `/ambe/distance` y `/ambe/activity` |
| `TransmittedSD` | Registra todas las partículas con phase space completo |
| `RunAction` | Abre/cierra ROOT, crea ntuple `PhaseSpace` |

---

## Próximos pasos

- [ ] Biasing angular para mejorar eficiencia estadística
- [ ] Modelo geométrico de la cápsula AmBe real
- [ ] Comparación con datos experimentales

---

## Contacto

Proyecto desarrollado por **Luis Beltrán**  
Maestría en Ingeniería Física — UAN
