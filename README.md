# Termalización de Neutrones en Parafina con Fuente AmBe (Geant4)

Simulación Monte Carlo de la **termalización de neutrones en parafina** usando **Geant4**.
El objetivo es estudiar cómo los neutrones producidos por una fuente AmBe pierden energía
al interactuar con la parafina, y caracterizar las partículas que alcanzan un detector
cilíndrico de grafeno dopado con boro colocado a la salida del moderador.

---

## Ramas del repositorio

| Rama | Descripción |
|------|-------------|
| `main` | Versión base: fuente monoenerética (4.2 MeV), solo neutrones |
| `only_energyk` | Simplificación: solo guarda energía cinética |
| `ambe_source` | **Rama principal de desarrollo**: fuente AmBe realista, phase space completo |

---

## Descripción física

### Fuente AmBe
La fuente de **Americio-Berilio (AmBe)** produce neutrones mediante la reacción:

```
α + ⁹Be → ¹²C* + n    (~60% de eventos → gamma 4.44 MeV)
α + ⁹Be → ¹²C  + n    (~40%)
```

- **Espectro neutrónico:** continuo, 0–11 MeV, pico en ~4.5 MeV (ISO 8529-1)
- **Gamma correlacionado:** 4.44 MeV (de-excitación del C-12), presente en ~60% de eventos
- **Actividad:** 2.98 Ci → ~6.6 × 10⁶ neutrones/s (yield ≈ 2.2 × 10⁶ n/s/Ci)
- **Emisión:** isotrópica (4π sr), modelada como fuente puntual

> Los gammas de 59.5 keV del Am-241 no se simulan: su tasa es ~16,700× mayor que
> la de neutrones, haciendo inviable su inclusión sin técnicas de biasing.

### Moderador
Bloque de parafina (`G4_PARAFFIN`, ρ = 0.93 g/cm³) que modera los neutrones rápidos
mediante colisiones elásticas con hidrógeno y carbono.

### Detector
Cilindro de aire (placeholder de grafeno dopado con boro) colocado en la cara trasera
de la parafina:
- **Diámetro:** 2.6 cm
- **Espesor:** 1 mm
- **Posición:** centrado en (0, 0), justo a la salida del moderador

El detector registra **todas las partículas** que lo atraviesan (neutrones, gammas,
electrones, positrones) con su fase espacial completa.

---

## Estructura del repositorio

```
.
├── CMakeLists.txt
├── main.cc
├── include/
│   ├── ActionInitialization.hh
│   ├── DetectorConstruction.hh
│   ├── DetectorMessenger.hh
│   ├── EventAction.hh
│   ├── PrimaryGeneratorAction.hh
│   ├── RunAction.hh
│   ├── SourceMessenger.hh          # Comandos /ambe/distance y /ambe/activity
│   └── TransmittedSD.hh
├── src/
│   ├── ActionInitialization.cc
│   ├── DetectorConstruction.cc     # Geometría: mundo dinámico + detector cilíndrico
│   ├── DetectorMessenger.cc        # Comandos /detector/setParaffinX/Y/Z
│   ├── EventAction.cc
│   ├── PrimaryGeneratorAction.cc   # Fuente AmBe: GPS + gamma correlacionado
│   ├── RunAction.cc                # Ntuple PhaseSpace (8 columnas)
│   ├── SourceMessenger.cc
│   └── TransmittedSD.cc            # Registra todas las partículas
├── macros/
│   ├── ambe_run.mac                # Macro principal para fuente AmBe
│   ├── run1.mac                    # Macro base (fuente simple)
│   ├── vis1.mac                    # Visualización modo 1
│   └── vis2.mac                    # Visualización modo 2
└── python/
    ├── ambe_scan.py                # Barrido de geometría y distancia
    ├── analisis_ambe.ipynb         # Análisis de geometría (heatmaps, 3D, energías)
    └── analisis_distancia.ipynb    # Análisis de dependencia con la distancia
```

---

## Requisitos

- **Geant4** ≥ 10.7 (compilado con `ui_all`, `vis_all`, `analysis`)
- **CMake** ≥ 3.16
- **C++17** (requerido por Geant4)
- **Python 3** con entorno `mi_entorno` que incluye:
  - `uproot`, `numpy`, `pandas`, `matplotlib`, `seaborn`, `jupyter`

---

## Compilación

```bash
# Cargar entorno de Geant4 (ajusta la ruta a tu instalación)
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

Genera `AmBePhaseSpace.root` en el directorio `build/`.

### Modo interactivo (visualización)

```bash
cd build
./Neutron_Thermalization          # abre interfaz gráfica
```

---

## Comandos del macro

```bash
# Geometría de la parafina (half-lengths)
/detector/setParaffinX 10 cm
/detector/setParaffinY 10 cm
/detector/setParaffinZ 10 cm      # espesor en la dirección del haz

# Fuente AmBe
/ambe/distance 10 cm              # distancia fuente → cara frontal de parafina
/ambe/activity 2.98               # actividad en Ci (solo para normalización)

# Inicialización y simulación
/run/initialize
/run/beamOn 500000                # número de disparos (1 neutrón AmBe por evento)
```

> Cada evento dispara **1 neutrón** del espectro AmBe + **1 gamma de 4.44 MeV** con
> probabilidad del 60% (ambos desde la misma posición, dirección isótropa).

---

## Salida de la simulación

Archivo ROOT: `AmBePhaseSpace.root`

| Ntuple | Descripción |
|--------|-------------|
| `PhaseSpace` | Una fila por partícula que alcanza el detector |

| Columna | Tipo | Descripción |
|---------|------|-------------|
| `Particle` | string | Nombre de la partícula (`neutron`, `gamma`, `e-`, …) |
| `KinE_eV` | double | Energía cinética en eV |
| `PosX_cm` | double | Posición X en cm |
| `PosY_cm` | double | Posición Y en cm |
| `PosZ_cm` | double | Posición Z en cm |
| `DirX` | double | Componente X del vector dirección (unitario) |
| `DirY` | double | Componente Y |
| `DirZ` | double | Componente Z |

Este archivo sirve como **input de phase space** para la siguiente etapa de simulación
(detector de grafeno dopado con boro).

### Normalización de resultados

La actividad de la fuente no afecta la física simulada, solo la normalización:

```
Tasa real [part/s] = (N_detectadas / N_simuladas) × Actividad[Ci] × 2.2×10⁶ [n/s/Ci]
```

---

## Barrido paramétrico (Python)

El script `python/ambe_scan.py` corre múltiples simulaciones variando geometría y distancia:

```bash
# Editar parámetros al inicio del script:
#   X_values, Y_values, Z_values  → half-lengths de la parafina (cm)
#   DIST_values                   → distancias fuente–parafina (cm)
#   BEAM_ON                       → disparos por simulación

/home/luisb28/mi_entorno/bin/python python/ambe_scan.py
```

Genera:
- `build/resultados_ambe.csv` — tabla con todas las configuraciones y resultados
- `build/AmBe_{WxHxD}cm_d{dist}cm.root` — archivo ROOT por configuración

**Tiempo estimado:** ~1 min por 50,000 eventos (varía con el tamaño de la parafina).

---

## Análisis en Python

### Análisis de geometría (`analisis_ambe.ipynb`)

Para cada valor de distancia por separado:

- Composición de partículas detectadas (neutrones, gammas, electrones)
- Mapa de calor: fracción de termalización vs espesor × lado (caras cuadradas)
- Scatter: efecto del área frontal no cuadrada (Ancho × Alto independientes)
- Superficie 3D: área frontal × espesor × fracción térmica
- Distribuciones de energía por tipo de partícula

### Análisis de distancia (`analisis_distancia.ipynb`)

- Fracción de termalización vs distancia (una línea por espesor)
- Eficiencia de detección vs distancia
- Tasa de neutrones totales y térmicos [n/s] vs distancia
- Distribución de energías: misma geometría, distintas distancias
- Tabla resumen: mejor geometría por distancia

```bash
/home/luisb28/mi_entorno/bin/jupyter notebook python/analisis_ambe.ipynb
/home/luisb28/mi_entorno/bin/jupyter notebook python/analisis_distancia.ipynb
```

---

## Estructura del código (rama `ambe_source`)

| Clase | Responsabilidad |
|-------|----------------|
| `DetectorConstruction` | Mundo dinámico, bloque de parafina configurable, detector cilíndrico G4Tubs |
| `DetectorMessenger` | Comandos `/detector/setParaffinX/Y/Z` |
| `PrimaryGeneratorAction` | GPS con espectro AmBe ISO 8529-1 + gamma 4.44 MeV correlacionado |
| `SourceMessenger` | Comandos `/ambe/distance` y `/ambe/activity` |
| `TransmittedSD` | Detector sensible: registra todas las partículas con phase space completo |
| `RunAction` | Abre/cierra ROOT, crea ntuple `PhaseSpace` |
| `EventAction` | Inicialización por evento |
| `ActionInitialization` | Conecta todas las acciones del usuario |

---

## Próximos pasos

- [ ] Simulación del detector de grafeno dopado con boro usando el phase space como input
- [ ] Biasing angular para mejorar eficiencia estadística
- [ ] Modelo geométrico de la cápsula AmBe real
- [ ] Comparación con datos experimentales

---

## Contacto

Proyecto desarrollado por **Luis Beltrán**.
