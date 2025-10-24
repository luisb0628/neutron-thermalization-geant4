# Termalización de Neutrones en Parafina (Geant4)

Simulación Monte Carlo de la **termalización de neutrones en parafina** usando **Geant4**.  
El objetivo es estudiar cómo los neutrones rápidos pierden energía por colisiones elásticas con hidrógeno y carbono dentro de la parafina, y determinar qué fracción de ellos alcanza el régimen térmico.

---

## 📁 Contenido del repositorio

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

## ⚙️ Requisitos

- **Geant4** (versión ≥ 10.7 recomendada)
- **CMake** (≥ 3.10)
- Compilador C++ (g++ o clang)
- **ROOT** (opcional, para análisis avanzado)
- **Python 3**, con las librerías `pandas`, `matplotlib` y `jupyter` (para el análisis en `analysis/root.ipynb`)

> 🔹 Antes de compilar, asegúrate de haber cargado el entorno de Geant4 con:  
> `source /ruta/a/geant4/bin/geant4.sh`

---

## 🛠️ Compilación

Desde la carpeta raíz del proyecto:

```bash
mkdir -p build
cd build
cmake .. -DGeant4_DIR=/ruta/a/geant4/lib/Geant4-<version>   # Solo si CMake no lo detecta automáticamente
make -j$(nproc)
```

Si Geant4 está correctamente configurado en tu entorno, CMake lo encontrará automáticamente.

---

## 🚀 Ejecución

El proyecto incluye macros de ejemplo en la carpeta `macros/`:

```bash
# Ejecutar con macro por defecto
./neutron-thermalization macros/run.mac

# Ejecutar en modo visual (si tu compilación incluye interfaz gráfica)
./neutron-thermalization macros/vis.mac

# Ejecutar y redirigir la salida
./neutron-thermalization macros/run.mac > output.log
```

### Parámetros configurables
- Número de eventos (`/run/beamOn <N>`)
- Energía inicial del haz (ej. 2 MeV o 14 MeV)
- Geometría y espesor del bloque de parafina
- Posición y dirección del haz
- Formato de salida (ASCII, ROOT, CSV)

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

## 📄 Formato de salida sugerido

- **ROOT:** árbol (`TTree`) con columnas: `eventID, E_initial, E_final, n_collisions, x_final, y_final, z_final`.
- **CSV:** formato simple con columnas como:  
  `event, E_final_eV, n_collisions, z_final_cm`.

---

## 📈 Análisis en Python (`analysis/root.ipynb`)

El notebook incluido permite:
- Leer archivos ROOT o CSV.
- Graficar distribuciones de energía (lineal y logarítmica).
- Calcular la fracción de neutrones termalizados (`E ≤ 0.025 eV`).
- Comparar resultados para distintos espesores o energías iniciales.

Ejemplo básico en Python:

```python
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("analysis/output_neutrons.csv")

plt.hist(df["E_final_eV"], bins=100, log=True)
plt.xlabel("Energía final (eV)")
plt.ylabel("Cuentas")
plt.title("Distribución de energía final de neutrones")
plt.show()

# Fracción de neutrones termalizados
fraction = (df["E_final_eV"] <= 0.025).mean()
print(f"Fracción termalizada: {fraction:.3f}")
```

---

## 🧱 Estructura del código

- `DetectorConstruction` → Define geometría, materiales y volúmenes sensibles.  
- `PrimaryGeneratorAction` → Configura el haz de neutrones inicial.  
- `RunAction`, `EventAction`, `SteppingAction` → Controlan estadísticas, histogramas y salida.  
- `macros/run.mac` → Controla parámetros de ejecución y número de eventos.

---

## ✅ Validación y buenas prácticas

- Comparar resultados con valores teóricos o literatura (secciones eficaces de H y C).  
- Aumentar el número de eventos para reducir incertidumbre estadística.  
- Usar diferentes semillas del generador aleatorio.  
- Revisar la sensibilidad a cortes de energía y tamaños de paso.  
- Definir con claridad el criterio de “termalización” (E ≤ 0.025 eV).

---

## 📘 Ejemplo de macro (`macros/run.mac`)

```
/run/initialize
/run/numberOfThreads 4
/run/beamOn 100000
/control/exit
```

---

## 📂 Resultados y análisis recomendados

Cada ejecución debe incluir:
- Energía inicial del haz (MeV)
- Espesor de la parafina (cm)
- Número de eventos simulados
- Fracción de neutrones termalizados
- Histogramas o figuras resumen (PNG, PDF)
- Breve interpretación de resultados

---

## 💡 Ideas para mejoras futuras

- Script en bash o Python para barridos automáticos de energía y espesor.  
- Inclusión de `GitHub Actions` para comprobar la compilación automáticamente.  
- Añadir ejemplos de salida y notebooks preconfigurados.  
- Implementar detección angular o espacial de neutrones termalizados.  

---

## 🤝 Contribuciones

1. Haz un **fork** del repositorio.  
2. Crea una rama para tus cambios (`git checkout -b mejora-feature`).  
3. Realiza un **pull request** describiendo claramente tus aportes.  
4. Evita subir archivos grandes de datos; usa enlaces externos (Zenodo, OSF, etc.).

---

## 📜 Licencia

Licencia recomendada: **MIT** o **Apache-2.0**.  
Agrega un archivo `LICENSE` con la licencia elegida.

---

## 📧 Contacto

Proyecto mantenido por **Luis Beltrán**.  
Para comentarios, dudas o mejoras, abre un *issue* en el repositorio o envía un *pull request*.

---
