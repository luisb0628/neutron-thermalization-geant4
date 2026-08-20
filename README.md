# Termalización de Neutrones en Parafina con Fuentes AmBe + Cs-137 (Geant4)

Simulación Monte Carlo de la **termalización de neutrones en parafina** usando **Geant4**, modelando la cabeza sensora de un **densímetro nuclear tipo Troxler**: una fuente de neutrones **AmBe** (humedad) y una fuente gamma **Cs-137** (densidad), ambas alojadas dentro de una caja blindaje de tungsteno-acero, irradiando un moderador de parafina hacia un detector.

Este proyecto es la **primera etapa** de una cadena de simulación:

1. **Neutron_Thermalization** (este repo) — genera `AmBePhaseSpace.root`
2. **Film_graphene** — captura neutrones en el film grafeno/Kapton
3. **Scintillator_Sipm** — detecta partículas secundarias en un centellador + SiPM

---

## Ramas del repositorio

| Rama | Descripción |
|------|-------------|
| `main` | Versión base: fuente monoenergética 4.2 MeV |
| `ambe_source` | Fuente AmBe realista con phase space completo |
| `Troxler-3440` | **Rama actual**: geometría de densímetro Troxler (caja W-acero + plomo parcial + AmBe/Cs-137) |

---

## Descripción física

### Fuente AmBe (neutrones)

La fuente **Americio-Berilio (AmBe)** produce neutrones por la reacción ⁹Be(α,n)¹²C, repartidos en 3 canales según el estado final del ¹²C (fracciones de De Guarrini & Malaroda 1971 / Geiger & Van der Zwan 1975):

| Canal | Fracción | Descripción |
|-------|----------|-------------|
| `nGND` | 31.6 % | Transición al estado fundamental del ¹²C |
| `n1st` | 46.5 % | Transición al 1er estado excitado — **acompañado del gamma de 4.438 MeV** (Doppler-ensanchado, σ/E≈0.9%) |
| `n2nd` | 21.9 % | Canal blando, incluye breakup en 3 alfas por encima del umbral (~7.65 MeV, estado de Hoyle) |

- **Espectro neutrónico:** continuo, cada canal con su propia forma (ver `PrimaryGeneratorAction::BuildSpectrum_*`, actualmente formas aproximadas — placeholder pendiente de reemplazar por valores digitalizados)
- **Actividad:** configurable vía `/ambe/activity` (Ci) — usada para normalización de tasas, no cambia la física del generador
- **Emisión:** isotrópica (4π sr), puntual, dentro de una cápsula sellada de doble encapsulado en acero inoxidable

### Fuente Cs-137 (calibración, gamma)

Gamma monoenergético de **661.657 keV** (transición del Ba-137m), emisión isotrópica desde una cápsula sellada idéntica en construcción a la de AmBe, ubicada en la mitad de la caja blindaje cubierta por el plomo.

> ⚠️ **Limitación conocida:** en el generador actual, el gamma de Cs-137 se emite **en cada evento**, junto con el neutrón de AmBe — esto simplifica el código pero no representa la física real (son decaimientos radiactivos independientes, con tasas de emisión muy distintas). Si necesitás resultados temporales realistas (coincidencias, tasa de cuentas), hay que desacoplar ambas fuentes — ver `PrimaryGeneratorAction::GeneratePrimaries`.

### Moderador

Bloque de parafina (`ρ = 0.93 g/cm³`, C:H = 1:2) configurable en dimensiones desde macro.

### Blindaje

- **Caja blindaje** de aleación tungsteno-acero (60/40 en masa, ρ≈12.33 g/cm³ por regla de mezcla), hueca (pared 5 mm), dimensiones fijas **Ancho(X)=33.5 cm × Alto(Y)=20 cm × Espesor(Z)=5.4 cm**. Ambas fuentes selladas están alojadas en su cavidad de aire: AmBe centrada en el ancho, Cs-137 desplazada hacia la mitad cubierta por el plomo.
- **Bloque de plomo parcial**, entre la caja blindaje y la parafina: cubre solo la **mitad del ancho** de la caja (16.75 cm de los 33.5 cm), del lado de la fuente Cs-137 — el lado de AmBe queda sin plomo delante (solo aire). Espesor configurable vía `/detector/setLeadZ` (half-length; por defecto 1 cm → 2 cm de espesor total).

### Detector

Cilindro de aire (placeholder de grafeno dopado con boro):
- Diámetro: 2.6 cm, Espesor: 1 mm
- Registra todas las partículas que llegan con su phase space completo (tipo, energía, posición, dirección)

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
│   ├── DetectorConstruction.hh      # Mundo, parafina configurable, caja W-acero + plomo parcial, fuentes, detector
│   ├── DetectorMessenger.hh         # Comandos /detector/setParaffinX/Y/Z, /detector/setLeadZ
│   ├── PrimaryGeneratorAction.hh    # Fuentes AmBe (3 canales) + Cs-137, registro en SourceSpectrum
│   ├── SourceMessenger.hh           # Comandos /ambe/distance y /ambe/activity
│   ├── SourceMonitorSD.hh           # DESACTIVADO (ver nota abajo) — reemplazado por registro en generación
│   ├── TransmittedSD.hh             # Sensible del detector -> ntuple PhaseSpace
│   ├── RunAction.hh
│   ├── EventAction.hh               # Barra de progreso con ETA durante la corrida
│   └── ActionInitialization.hh
├── src/                             # Implementación de cada clase de include/
├── macros/
│   ├── ambe_run.mac                 # Macro principal (batch, ambas fuentes)
│   ├── geometry.py                  # Barrido paramétrico standalone (progreso, ETA, reanudar)
│   ├── vis1.mac / vis2.mac          # Visualización
│   ├── vis_video.mac                # Exporta frames EPS por evento
│   ├── vis_box_frame.mac            # Frame de geometría base para overlay de video
│   ├── vis_box_frame_geo1.mac  …    # Frames de geometría para cada configuración
│   ├── vis_video_geo1.mac  …        # Macros de video para cada configuración
│   └── vis_rotate_frame.mac
└── python/
    ├── ambe_scan.py                 # Barrido paramétrico de geometría y distancia
    ├── accumulate_frames.py         # Acumula frames para el video
    ├── Termalizacion.csv            # Resultados de un barrido previo
    ├── analisis_ambe.ipynb          # Análisis de geometría (heatmaps, 3D, energías)
    └── analisis_distancia.ipynb     # Análisis vs distancia fuente–parafina
```

**Nota sobre `SourceMonitorSD`:** originalmente registraba el espectro "de fuente" contando cruces de partículas hacia un volumen que envuelve cada cápsula — pero eso duplicaba partículas que retrodispersaban y volvían a cruzar, con energía ya alterada por esa dispersión. Se reemplazó por `PrimaryGeneratorAction::RecordSourceSpectrum`, que registra cada partícula primaria **exactamente una vez**, con la energía/dirección tal como fue generada. La clase sigue en el código (no-operativa) para no tocar la geometría de los volúmenes monitor.

---

## Requisitos

- **Geant4** ≥ 10.7 compilado con `ui_all`, `vis_all`, `analysis` y física HP
- **CMake** ≥ 3.16, **C++17**
- **Python 3** con: `uproot`, `numpy`, `pandas`, `matplotlib`, `jupyter`
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

# Espesor del plomo parcial (half-length; espesor_total = 2x valor)
/detector/setLeadZ 1 cm

# Fuentes AmBe + Cs-137 (comparten distancia/actividad de normalización)
/ambe/distance 0 cm       # offset extra en Z respecto al centro de la caja blindaje
/ambe/activity 2.98       # actividad en Ci (solo para normalización)

/run/initialize
/run/beamOn 500000
```

---

## Salida

Archivo ROOT: `AmBePhaseSpace.root`, con **dos** ntuples (mismo esquema de columnas):

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

- **`PhaseSpace`** — partículas que llegan al detector (todas, después de moderador/blindaje). Es el **input de phase space** para Film_graphene.
- **`SourceSpectrum`** — partículas primarias tal como las emite el generador (antes de cualquier transporte), una fila exacta por partícula — ver nota sobre `SourceMonitorSD` arriba.

---

## Barrido paramétrico

Dos scripts equivalentes, con barra de progreso, ETA en vivo, reanudar sin repetir combinaciones, y guardado incremental:

```bash
python macros/geometry.py       # standalone, corre desde macros/
python python/ambe_scan.py      # variante con barrido de plomo y distancia también
```

Genera:
- Un CSV de resultados por configuración (nombre según el script)
- Un `.root` por configuración en `build/`

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
| `DetectorConstruction` | Mundo, parafina configurable, caja blindaje W-acero + plomo parcial, cápsulas de fuente, detector `G4Tubs` |
| `DetectorMessenger` | Comandos `/detector/setParaffinX/Y/Z`, `/detector/setLeadZ` |
| `PrimaryGeneratorAction` | GPS con 3 canales AmBe + gamma correlacionado 4.438 MeV + Cs-137; registra `SourceSpectrum` en generación |
| `SourceMessenger` | Comandos `/ambe/distance` y `/ambe/activity` |
| `SourceMonitorSD` | Desactivado (ver nota arriba) |
| `TransmittedSD` | Registra todas las partículas del detector con phase space completo |
| `EventAction` | Barra de progreso con ETA durante la corrida |
| `RunAction` | Abre/cierra ROOT, crea ntuples `PhaseSpace` y `SourceSpectrum` |

---

## Próximos pasos

- [ ] Desacoplar la emisión de Cs-137 de AmBe (son fuentes independientes, no deberían coincidir en cada evento)
- [ ] Reemplazar los espectros AmBe placeholder por valores digitalizados de literatura
- [ ] Biasing angular para mejorar eficiencia estadística
- [ ] Modelo geométrico de la cápsula AmBe real
- [ ] Comparación con datos experimentales

---

## Contacto

Proyecto desarrollado por **Luis Beltrán**
Maestría en Ingeniería Física — UAN
