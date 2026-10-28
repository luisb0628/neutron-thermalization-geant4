# Termalización de neutrones de AmBe en parafina (Geant4)

Simulación Monte Carlo en **Geant4** de la moderación de los neutrones de una fuente de **²⁴¹Am-Be** en un bloque de **parafina**, con la geometría de la cabeza de un densímetro nuclear **Troxler 3440**: la fuente de neutrones (AmBe) y la de densidad (¹³⁷Cs) dentro de una caja de blindaje de tungsteno-acero, un bloque de plomo parcial, la parafina y un detector de aire. El programa guarda el *phase space* de las partículas que llegan al detector y, si se pide, cada colisión del neutrón dentro de la parafina.

Es la **primera etapa** de una cadena de tres simulaciones:

| Etapa | Repositorio | Entrada | Salida |
|---|---|---|---|
| 1. Termalización | **neutron-thermalization-geant4** (este) | — | `AmBePhaseSpace.root`, `Cs137PhaseSpace.root` |
| 2. Captura | [neutron-capture-graphene-boron-geant4](https://github.com/luisb0628/neutron-capture-graphene-boron-geant4) | phase space de la etapa 1 | `output.root` |
| 3. Centelleador | [Scintillator_Sipm](https://github.com/luisb0628/Scintillator_Sipm) | `output.root` de la etapa 2 | `output.root` |

---

## Física y geometría

**Lista de física:** `QGSP_BERT_HP` (neutrones de alta precisión por debajo de 20 MeV), la misma en las tres etapas.

**Fuente de ²⁴¹Am-Be.** Emite solo neutrones, isótropos y puntuales, desde el centro de la cavidad activa de su cápsula de acero. La energía se muestrea de tres canales de la reacción ⁹Be(α,n)¹²C:

| Canal | Fracción | Estado final del ¹²C |
|---|---|---|
| `nGND` | 31,6 % | fundamental |
| `n1st` | 46,5 % | primer excitado (4,438 MeV) |
| `n2nd` | 21,9 % | segundo excitado y ruptura en tres α |

El espectro resultante cubre de 0 a 8 MeV y aproxima el de la norma ISO 8529-1. El γ de 4,438 MeV del canal `n1st` **no se emite**: no aparece en el espectro medido, porque el blindaje del densímetro lo atenúa.

**Fuente de ¹³⁷Cs.** Un γ de 0,661657 MeV por evento, isótropo, desde una cápsula igual a la del AmBe y ubicada en la mitad de la caja cubierta por el plomo.

**Las dos fuentes se simulan por separado** (`/source/mode`). El ¹³⁷Cs emite muchos más fotones por segundo que el AmBe, así que se corren aparte y se combinan después en el análisis, con un peso según la actividad de cada una.

**Geometría:**
- Caja de blindaje hueca de aleación tungsteno-acero (60/40 en masa, 12,33 g/cm³), de (33,5 × 20 × 5,4) cm³ con pared de 5 mm.
- Bloque de plomo entre la caja y la parafina que cubre solo la mitad del ancho, del lado del ¹³⁷Cs. El eje del AmBe coincide con el borde del plomo, así que sus neutrones salen por el lado libre.
- Parafina CH₂ de 0,93 g/cm³ con dimensiones configurables.
- Detector: cilindro de aire de 1,3 cm de radio detrás de la parafina, que registra cada partícula que lo cruza.

---

## Requisitos

- **Geant4 11** con datos de neutrones HP, `analysis` y, para visualizar, `ui_all` y `vis_all`.
- **CMake** ≥ 3.16 y un compilador **C++17**.
- **Python 3** con `uproot`, `numpy` y `pandas`, para leer las salidas y correr los barridos.
- Opcional, para los videos: **Ghostscript** (`gs`) y **ffmpeg**.

Las corridas de la tesis se hicieron en Docker con la imagen `luisb28/geant4-clase:11.4-python`, montando la carpeta que contiene los tres repositorios en `/workspace`.

---

## Compilación y ejecución

```bash
source /ruta/a/geant4/bin/geant4.sh
mkdir -p build && cd build
cmake ..
make -j$(nproc)

./Neutron_Thermalization ../macros/ambe_only.mac     # neutrones del AmBe   -> AmBePhaseSpace.root
./Neutron_Thermalization ../macros/cs137_only.mac    # gammas del 137Cs     -> Cs137PhaseSpace.root
./Neutron_Thermalization                             # modo interactivo con visualización
```

La etapa 2 lee los `.root` directamente de `build/`, con una ruta relativa fija (`../../neutron-thermalization-geant4/build/`). Por eso los tres repositorios deben estar uno al lado del otro.

---

## Macros

| Macro | Qué corre |
|---|---|
| `macros/ambe_only.mac` | Solo AmBe, 1×10⁷ neutrones, geometría de referencia |
| `macros/cs137_only.mac` | Solo ¹³⁷Cs, 5×10⁶ gammas, misma geometría |
| `macros/ambe_moderacion.mac` | Solo AmBe con el registro de la moderación activado (`ModerationSummary` y `ModerationSteps`) |
| `macros/ambe_run.mac` | Las dos fuentes en el mismo evento (`/source/mode Both`), 1×10⁶ eventos |
| `macros/run1.mac`, `vis2.mac`, `vis_*.mac` | Pruebas y visualización |
| `macros/geometry.py` | Barrido de las dimensiones de la parafina, con progreso y reanudación |

Las macros usan una parafina de (12 × 12 × 8) cm³ (half-lengths 6, 6 y 4 cm) y un plomo de 10 cm (`/detector/setLeadZ 5 cm`). La configuración final de la tesis usa la misma parafina con un plomo de **5 cm** (`/detector/setLeadZ 2.5 cm`).

### Comandos

| Comando | Descripción |
|---|---|
| `/detector/setParaffinX`, `Y`, `Z` | Half-length de la parafina en cada eje (Z es el espesor, en la dirección del flujo) |
| `/detector/setLeadZ` | Half-length del plomo en Z (espesor total = 2 × valor) |
| `/source/mode AmBe \| Cs137 \| Both` | Fuente(s) que se generan; define también el nombre del archivo de salida |
| `/ambe/distance` | Corrimiento de las fuentes respecto a la cara frontal del plomo |
| `/ambe/activity` | Actividad en Ci, solo para normalizar; no cambia la física |
| `/ambe/gammaYield` | Se conserva por compatibilidad, pero **no tiene efecto**: el generador ya no emite el γ de 4,438 MeV |
| `/moderation/enable 0 \| 1` | Guarda o no las tablas de moderación |
| `/moderation/maxStepTracks N` | Número de neutrones cuyas colisiones se guardan una por una |

---

## Salida

El nombre del archivo depende de `/source/mode`: `AmBePhaseSpace.root`, `Cs137PhaseSpace.root` o `AmBeCs137PhaseSpace.root`.

| Tabla | Contenido |
|---|---|
| `PhaseSpace` | Cada partícula que llega al detector: `Particle`, `KinE_eV`, `PosX/Y/Z_cm`, `DirX/Y/Z`. Es la entrada de la etapa 2. |
| `SourceSpectrum` | Cada primaria tal como sale del generador, una fila por partícula (mismas columnas). |
| `ModerationSummary` | Una fila por neutrón que entra a la parafina: energía inicial y de entrada, colisiones elásticas con H y C, inelásticas y capturas, recorrido, tiempo y número de colisiones hasta 0,5 eV y hasta 0,025 eV, energía, volumen y proceso finales. |
| `ModerationSteps` | Una fila por colisión del neutrón en la parafina: proceso, núcleo blanco (Z, A), energía antes y después, tiempo, longitud del paso, posición y ángulo. |

---

## Estructura del código

| Clase | Responsabilidad |
|---|---|
| `DetectorConstruction` / `DetectorMessenger` | Mundo, caja de blindaje, plomo parcial, cápsulas, parafina y detector; comandos `/detector/` |
| `PrimaryGeneratorAction` | Fuentes AmBe (tres canales) y ¹³⁷Cs; llena `SourceSpectrum` al generar |
| `SourceMode` / `GeneratorMessenger` | Elección de la fuente y del nombre de salida (`/source/mode`) |
| `SourceMessenger` | Comandos `/ambe/` |
| `ModerationConfig` / `SteppingAction` | Registro de la moderación en la parafina (`/moderation/`) |
| `TransmittedSD` | Detector sensible que llena `PhaseSpace` |
| `RunAction` / `EventAction` | Archivo ROOT, tablas y barra de progreso |
| `SourceMonitorSD` | Sin uso: se reemplazó por el registro de `SourceSpectrum` en la generación, que no cuenta dos veces las partículas retrodispersadas |

**Otros archivos:** `python/ambe_scan.py` (barrido de geometría y distancia), `python/accumulate_frames.py` y `make_video.sh` (video de la simulación, ver `guia_video.pdf`).

---

## Autor

**Luis Beltrán** — Maestría en Ingeniería Física, Universidad Antonio Nariño (UAN).
