# Termalización de Neutrones en Parafina (Geant4)

Simulación Monte Carlo de la **termalización de neutrones en parafina** usando **Geant4**.  
El objetivo es estudiar cómo los neutrones rápidos pierden energía por colisiones elásticas con hidrógeno y carbono dentro de la parafina, y determinar qué fracción de ellos alcanza el régimen térmico.

---

## Contenido del repositorio

```
.
├── CMakeLists.txt
├── main.cc
├── include/           # Archivos de cabecera (DetectorConstruction, PrimaryGeneratorAction, RunAction, EventAction, etc.)
├── src/               # Código fuente principal
├── macros/            # Macros de ejemplo para ejecución
├── data/              # (Opcional) Archivos de salida o parámetros
├── analysis/          # Notebooks y scripts de análisis (root.ipynb, scripts en Python)
└── README.md
```

---

## Requisitos

- **Geant4** (versión ≥ 10.7 recomendada)
- **CMake** (≥ 3.10)
- Compilador C++ (g++ o clang)
- **ROOT** (opcional, para análisis avanzado)
- **Python 3**, con las librerías `pandas`, `matplotlib` y `jupyter` (para el análisis en `analysis/root.ipynb`)

>  Antes de compilar, asegúrate de haber cargado el entorno de Geant4 con:  
> `source ../geant4/bin/geant4.sh`

---

## 🛠️ Compilación

Desde la carpeta raíz del proyecto:

```bash
mkdir -p build
cd build
cmake .. 
make 
```

Si Geant4 está correctamente configurado en tu entorno, CMake lo encontrará automáticamente.

---

## Ejecución

El proyecto incluye macros de ejemplo en la carpeta `macros/`:

```bash
# Ejecutar con macro por defecto
./neutron-thermalization macros/run.mac

# Ejecutar en modo visual 
./neutron-thermalization macros/vis.mac

```

---

## 📊 Resultados esperados

- **Distribución de energía** de los neutrones después de atravesar la parafina.  
- **Fracción de neutrones termalizados**, definida como aquellos con energía ≤ 0.025 eV.  
- **Número promedio de colisiones** necesarias para alcanzar energía térmica.  
- **Perfil espacial** de termalización dentro del bloque.  

Los resultados pueden representarse en histogramas de:
- Energía inicial vs energía final.
- Número de colisiones por neutrón.
- Fracción de termalizados en función del espesor del bloque.

---

## Análisis en Python (`analysis/root.ipynb`)

El notebook incluido permite:
- Leer archivos ROOT o CSV.
- Graficar distribuciones de energía (lineal y logarítmica).
- Calcular la fracción de neutrones termalizados (`E ≤ 0.025 eV`).
- Comparar resultados para distintos espesores o energías iniciales.


---

## Estructura del código

- `DetectorConstruction` → Define geometría, materiales y volúmenes sensibles.  
- `PrimaryGeneratorAction` → Configura el haz de neutrones inicial.  
- `RunAction`, `EventAction`, `SteppingAction` → Controlan estadísticas, histogramas y salida.  
- `macros/run.mac` → Controla parámetros de ejecución y número de eventos.

---

## Contacto

Proyecto mantenido por **Luis Beltrán**.  

