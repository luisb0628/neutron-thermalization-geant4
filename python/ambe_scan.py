import os
import time
import subprocess
import numpy as np
import pandas as pd
import uproot

# =============================================================
#  Parámetros del barrido — edita aquí
# =============================================================
BEAM_ON   = 100000          # disparos por simulación
ACTIVITY  = 2.98           # Ci (solo para normalización)

# Geometría de la parafina (half-lengths en cm)
X_values = [0.5,1.0,1.5,2.0,2.5,3.0,3.5,4.0,4.5,5.0,5.5,6.0,6.5,7.0,7.5,8.0,8.5,9.0,9.5,10.0]
Y_values = [0.5,1.0,1.5,2.0,2.5,3.0,3.5,4.0,4.5,5.0,5.5,6.0,6.5,7.0,7.5,8.0,8.5,9.0,9.5,10.0]
Z_values = [0.5,1.0,1.5,2.0,2.5,3.0,3.5,4.0,4.5,5.0,5.5,6.0,6.5,7.0,7.5,8.0,8.5,9.0,9.5,10.0]

# Espesor del bloque de plomo adicional (half-length en cm; espesor_total = 2 × valor)
# Lado -X (donde está la fuente Cs-137), area fija 16.75x20 cm.
# La caja de tungsteno (33.5x20x5.4 cm, rotada) es de tamaño fijo, no se barre aquí.
# Ejemplo: 10 cm → bloque de 20 cm de espesor
LEAD_Z_values = [10.0]  # cm

# Distancias fuente → pared interna frontal de la CAJA BLINDAJE
DIST_values = [0]  # cm

# =============================================================
#  Rutas
# =============================================================
BUILD_DIR = os.path.join(os.path.dirname(__file__), "../build")
EXE       = os.path.join(BUILD_DIR, "Neutron_Thermalization")
ROOT_FILE = os.path.join(BUILD_DIR, "AmBePhaseSpace.root")
MACRO_TMP = os.path.join(BUILD_DIR, "auto_ambe.mac")

results = []
total_start = time.time()

combos = [(x, y, z, lz, d)
          for x in X_values
          for y in Y_values
          for z in Z_values
          for lz in LEAD_Z_values
          for d in DIST_values]

print(f"Simulaciones a correr: {len(combos)}")
print(f"Disparos por sim     : {BEAM_ON:,}")
print(f"Actividad fuente     : {ACTIVITY} Ci")
print("=" * 60)

for i, (X, Y, Z, LZ, dist) in enumerate(combos, 1):

    tag  = f"{2*X:.0f}x{2*Y:.0f}x{2*Z:.0f}cm_pb{2*LZ:.0f}cm_d{dist:.0f}cm"
    dest = os.path.join(BUILD_DIR, f"AmBe_{tag}.root")
    print(f"\n[{i}/{len(combos)}] Parafina {2*X}x{2*Y}x{2*Z} cm  |  Pb {2*LZ:.0f} cm  |  dist {dist} cm", flush=True)

    if os.path.exists(dest):
        print("  SKIP: ya existe, leyendo resultado guardado.", flush=True)
        elapsed = 0.0
    else:
        # --- Macro temporal ---
        macro = f"""\
/control/verbose 0
/run/verbose 0
/event/verbose 0
/tracking/verbose 0

/detector/setParaffinX {X} cm
/detector/setParaffinY {Y} cm
/detector/setParaffinZ {Z} cm
/detector/setLeadZ {LZ} cm

/ambe/distance {dist} cm
/ambe/activity {ACTIVITY}

/run/initialize
/run/beamOn {BEAM_ON}
"""
        with open(MACRO_TMP, "w") as f:
            f.write(macro)

        # --- Correr Geant4 ---
        t0 = time.time()
        subprocess.run([EXE, MACRO_TMP],
                       stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL,
                       cwd=BUILD_DIR)
        elapsed = time.time() - t0
        mins, secs = divmod(elapsed, 60)
        print(f"  Duracion: {int(mins)} min {secs:.1f} s", flush=True)

        if not os.path.exists(ROOT_FILE):
            print("  AVISO: no se encontro AmBePhaseSpace.root")
            continue

        os.replace(ROOT_FILE, dest)

    # --- Leer ROOT ---
    try:
        with uproot.open(dest) as froot:
            tree = froot["PhaseSpace"]
            arr  = tree.arrays(["Particle", "KinE_eV"], library="np")

        particle = arr["Particle"]
        energy   = arr["KinE_eV"]

        n_total    = len(particle)
        n_neutron  = int((particle == "neutron").sum())
        n_gamma    = int((particle == "gamma").sum())
        n_electron = int((particle == "e-").sum())
        n_other    = int(n_total - n_neutron - n_gamma - n_electron)

        mask_n    = particle == "neutron"
        en        = energy[mask_n]
        n_thermal    = int((en <= 0.025).sum())
        n_epithermal = int(((en > 0.025) & (en < 0.5e6)).sum())
        n_fast       = int((en >= 0.5e6).sum())

        frac_thermal = (n_thermal / n_neutron * 100) if n_neutron > 0 else 0.0

        # Tasa estimada en detector [part/s] usando actividad
        neutron_rate_source = ACTIVITY * 2.2e6   # n/s de la fuente
        tasa_n_detector     = (n_neutron / BEAM_ON) * neutron_rate_source

        print(f"  Detectados : {n_total}  "
              f"(n={n_neutron}, g={n_gamma}, e-={n_electron}, otros={n_other})")
        print(f"  Neutrones  : termicos={n_thermal} ({frac_thermal:.1f}%)  "
              f"epitermicos={n_epithermal}  rapidos={n_fast}")
        print(f"  Tasa n det.: {tasa_n_detector:.2f} n/s  (estimada con {ACTIVITY} Ci)")

        results.append({
            "Ancho_cm":        2 * X,
            "Alto_cm":         2 * Y,
            "Espesor_cm":      2 * Z,
            "Plomo_cm":        2 * LZ,
            "Distancia_cm":    dist,
            "BeamOn":          BEAM_ON,
            "Actividad_Ci":    ACTIVITY,
            "Duracion_s":      round(elapsed, 1),
            "Total_detectados":n_total,
            "Neutrones":       n_neutron,
            "Gammas":          n_gamma,
            "Electrones":      n_electron,
            "Otros":           n_other,
            "N_termicos":      n_thermal,
            "N_epitermicos":   n_epithermal,
            "N_rapidos":       n_fast,
            "Frac_termica_%":  round(frac_thermal, 2),
            "Tasa_n_det_ps":   round(tasa_n_detector, 2),
        })


    except Exception as e:
        print(f"  ERROR leyendo ROOT: {e}")

# --- CSV final ---
if results:
    df = pd.DataFrame(results)
    csv_path = os.path.join(BUILD_DIR, "resultados_ambe.csv")
    df.to_csv(csv_path, index=False)
    print(f"\nResultados guardados en: {csv_path}")

total = time.time() - total_start
mins, secs = divmod(total, 60)
print(f"Tiempo total: {int(mins)} min {secs:.1f} s")
