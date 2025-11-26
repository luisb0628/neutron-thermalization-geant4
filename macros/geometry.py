import os
import numpy as np
import pandas as pd
import uproot
import subprocess
import time

# --- Parámetros del barrido ---
X_values = np.arange(0.5, 10.1, 0.5)  # cm
Y_values = np.arange(0.5, 10.1, 0.5)   # cm
Z_values = np.arange(0.5, 10.1, 0.5)   # cm (espesor)

start_time = time.time() 
# --- Ruta del ejecutable ---
exe = "./Neutron_Thermalization"

# --- Archivo ROOT de salida ---
root_file = "NeutronData.root"

# --- Lista de resultados ---
results = []

for X in X_values:
    for Y in Y_values:
        for Z in Z_values:
            # 1️⃣ Generar macro temporal
            macro_content = f"""\
/control/verbose 2
/run/verbose 1
/event/verbose 0
/tracking/verbose 0

/detector/setParaffinX {X} cm
/detector/setParaffinY {Y} cm
/detector/setParaffinZ {Z} cm

/run/initialize

# Configuración del haz
/gun/particle neutron
/gun/energy 4.2 MeV
/gun/position 0 0 -{Z+0.1} cm
/gun/direction 0 0 1
/gun/number 1

# Simulación
/run/beamOn 1000000
"""
            macro_file = "auto.mac"
            with open(macro_file, "w") as f:
                f.write(macro_content)

            print(f"\n🔹 Simulando bloque {2*X}×{2*Y}×{2*Z} cm...")

            # 2️⃣ Ejecutar Geant4
            subprocess.run([exe, macro_file], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

            # 3️⃣ Leer el archivo ROOT
            if not os.path.exists(root_file):
                print("⚠️ No se encontró el archivo ROOT.")
                continue

            try:
                with uproot.open(root_file) as froot:
                    tree = froot["NeutronTracks"]
                    energy = tree["KineticEnergy_eV"].array(library="np")

                total_detected = len(energy)
                thermal = np.sum(energy < 0.025)         # E < 0.025 eV
                epithermal = np.sum((energy >= 0.025) & (energy < 0.5))
                fast = np.sum(energy >= 0.5)

                results.append({
                    "Ancho_(cm)": 2*X,
                    "Alto_(cm)": 2*Y,
                    "Espesor_(cm)": 2*Z,
                    "Detectados": total_detected,
                    "Termicos": int(thermal),
                    "Epitermicos": int(epithermal),
                    "Rapidos": int(fast)
                })

                # Guarda una copia del ROOT para esta geometría
                new_root = f"NeutronData_{2*X}x{2*Y}x{2*Z}.root"
                os.rename(root_file, new_root)

            except Exception as e:
                print(f"❌ Error leyendo ROOT: {e}")

# --- 4️⃣ Guardar CSV final ---
df = pd.DataFrame(results)
df.to_csv("resultados_parafina.csv", index=False)
elapsed = time.time() - start_time
mins, secs = divmod(elapsed, 60)
print(f"⏱️ Tiempo total de ejecución: {int(mins)} min {secs:.1f} s")
print("\n✅ Resultados guardados en 'resultados_parafina.csv'")
