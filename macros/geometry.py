import os
import numpy as np
import pandas as pd
import uproot
import subprocess
import time

# --- Puntos a correr: dos sub-mallas (Ancho x Alto x Espesor), en vez del
# cubo completo 20x20x20 --------------------------------------------------
#   Malla A: Ancho 1-15 cm x Alto 1-15 cm x Espesor 5-9 cm    (1,125 puntos)
#   Malla B: Ancho 1-15 cm x Alto 1-15 cm x Espesor 12-15 cm    (900 puntos)
#   Total: 2,025 puntos (25% del cubo completo)
# Valores en cm de largo completo, paso 1 cm (mismo grid que resultados_ambe.csv).
Width_full  = list(range(1, 16))   # 1..15
Height_full = list(range(1, 16))   # 1..15
Thick_A     = list(range(5, 10))   # 5..9
Thick_B     = list(range(12, 16))  # 12..15

combos_cm = ([(w, h, t) for w in Width_full for h in Height_full for t in Thick_A] +
             [(w, h, t) for w in Width_full for h in Height_full for t in Thick_B])
# half-length (lo que usan los comandos /detector/setParaffin...)
combos = [(w / 2, h / 2, t / 2) for (w, h, t) in combos_cm]
n_combos = len(combos)

# --- Parámetros ---
BEAM_ON  = 1_000_000  # eventos por combo (~1 dia para las 2,025 combinaciones)
ACTIVITY = 2.98        # Ci (fuente AmBe, solo para normalización)
LEAD_Z   = 1.5         # cm, half-length del bloque de plomo adicional
DISTANCE = 0           # cm, fuente -> pared interna frontal de la caja blindaje

# --- Rutas (relativas a este script, no al cwd desde donde se invoque) ---
SCRIPT_DIR = os.path.dirname(__file__)
BUILD_DIR  = os.path.join(SCRIPT_DIR, "../build")
exe        = os.path.join(BUILD_DIR, "Neutron_Thermalization")
root_file  = os.path.join(BUILD_DIR, "AmBePhaseSpace.root")
macro_file = os.path.join(BUILD_DIR, "auto_geometry.mac")
csv_path   = os.path.join(SCRIPT_DIR, "resultados_submalla.csv")

# --- Reanudar: si ya hay un CSV de una corrida anterior, no repetir combos ---
results = []
done_keys = set()
if os.path.exists(csv_path):
    prev = pd.read_csv(csv_path)
    results = prev.to_dict("records")
    done_keys = {(round(r["Ancho_cm"], 4), round(r["Alto_cm"], 4), round(r["Espesor_cm"], 4))
                 for r in results}
    print(f"Reanudando desde '{os.path.basename(csv_path)}': "
          f"{len(done_keys)} combinaciones ya hechas, se omiten.")

print(f"Combinaciones totales : {n_combos:,}  "
      f"(Ancho/Alto 1-15 cm x Espesor 5-9 cm, y x Espesor 12-15 cm)")
print(f"Pendientes            : {n_combos - len(done_keys):,}")
print(f"Eventos por combo     : {BEAM_ON:,}")
print("(overhead fijo de arranque de Geant4 es ~3-4s por combo, aparte del "
      "tiempo de simulación — el ETA de abajo ya lo incluye porque se mide en vivo. "
      "Con 1,000,000 eventos/combo, esta submalla toma ~1 dia.)\n")


def format_hms(seconds):
    seconds = max(0, int(seconds))
    h, rem = divmod(seconds, 3600)
    m, s = divmod(rem, 60)
    return f"{h:02d}h{m:02d}m{s:02d}s"


def print_progress(done, total, elapsed_session, n_run_session):
    pct = 100.0 * done / total
    filled = min(20, int(pct / 5))
    bar = "[" + "=" * filled + " " * (20 - filled) + "]"
    avg = elapsed_session / n_run_session if n_run_session > 0 else 0.0
    eta = avg * (total - done)
    print(f"{bar} {pct:5.1f}%  Combo {done:>5}/{total}  "
          f"Transcurrido: {format_hms(elapsed_session)}  "
          f"ETA: {format_hms(eta)}  (~{avg:.1f}s/combo)", flush=True)


start_time = time.time()
n_done = len(done_keys)
n_run_session = 0

for (X, Y, Z) in combos:
    key = (round(2 * X, 4), round(2 * Y, 4), round(2 * Z, 4))
    if key in done_keys:
        continue

    # 1) Generar macro temporal — comandos de la fuente AmBe actual
    macro_content = f"""\
/control/verbose 0
/run/verbose 0
/event/verbose 0
/tracking/verbose 0

/detector/setParaffinX {X} cm
/detector/setParaffinY {Y} cm
/detector/setParaffinZ {Z} cm
/detector/setLeadZ {LEAD_Z} cm

/ambe/distance {DISTANCE} cm
/ambe/activity {ACTIVITY}

/run/initialize
/run/beamOn {BEAM_ON}
"""
    with open(macro_file, "w") as f:
        f.write(macro_content)

    # 2) Ejecutar Geant4 (cwd=BUILD_DIR: ahí vive el ejecutable y ahí escribe AmBePhaseSpace.root)
    subprocess.run([exe, macro_file], stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL, cwd=BUILD_DIR)

    # 3) Leer el archivo ROOT
    if not os.path.exists(root_file):
        print("  AVISO: no se encontró AmBePhaseSpace.root")
        continue

    try:
        with uproot.open(root_file) as froot:
            tree = froot["PhaseSpace"]
            arr  = tree.arrays(["Particle", "KinE_eV"], library="np")

        particle = arr["Particle"]
        energy   = arr["KinE_eV"]          # eV
        total_detected = len(particle)

        en = energy[particle == "neutron"]
        thermal    = int(np.sum(en <= 0.025))                    # E <= 0.025 eV
        epithermal = int(np.sum((en > 0.025) & (en < 0.5e6)))    # 0.025 eV - 0.5 MeV
        fast       = int(np.sum(en >= 0.5e6))                    # >= 0.5 MeV

        results.append({
            "Ancho_cm":    2 * X,
            "Alto_cm":     2 * Y,
            "Espesor_cm":  2 * Z,
            "BeamOn":      BEAM_ON,
            "Detectados":  total_detected,
            "Neutrones":   len(en),
            "Termicos":    thermal,
            "Epitermicos": epithermal,
            "Rapidos":     fast,
        })

        new_root = os.path.join(
            BUILD_DIR, f"NeutronData_{2*X:.1f}x{2*Y:.1f}x{2*Z:.1f}.root")
        os.replace(root_file, new_root)

        # Guardado incremental: si se corta a medio camino, no se pierde nada
        pd.DataFrame(results).to_csv(csv_path, index=False)

    except Exception as e:
        print(f"  ERROR leyendo ROOT: {e}")

    n_done += 1
    n_run_session += 1
    print_progress(n_done, n_combos, time.time() - start_time, n_run_session)

print(f"\nResultados guardados en '{csv_path}'")
elapsed = time.time() - start_time
print(f"Tiempo de esta sesión: {format_hms(elapsed)}")
