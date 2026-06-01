"""
Crea frames acumulados a partir de PNGs individuales de Geant4.
Cada frame N muestra las trayectorias de los eventos 1..N superpuestas.
Opcionalmente superpone un frame de geometría semitransparente (caja de parafina).
Uso: python3 accumulate_frames.py --n 60 [--box box_frame.png --box-alpha 0.25]
"""
import argparse
import os
import numpy as np
from PIL import Image

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--n", type=int, default=60, help="Número de frames")
    parser.add_argument("--indir", default=".", help="Directorio con los PNGs frame_N.png")
    parser.add_argument("--outdir", default=".", help="Directorio de salida")
    parser.add_argument("--box", default=None, help="PNG del cubo de referencia (geometría sólida)")
    parser.add_argument("--box-alpha", type=float, default=0.25, help="Opacidad del overlay del cubo (0-1)")
    args = parser.parse_args()

    os.makedirs(args.outdir, exist_ok=True)

    # Cargar overlay del cubo si se especificó
    box_overlay = None
    if args.box and os.path.exists(args.box):
        box_img = np.array(Image.open(args.box).convert("RGB"), dtype=np.float32)
        box_overlay = (box_img * args.box_alpha).astype(np.uint8)
        print(f"  Overlay del cubo: {args.box} (alpha={args.box_alpha})")

    accumulated = None

    for i in range(1, args.n + 1):
        path = os.path.join(args.indir, f"frame_{i:04d}.png")
        if not os.path.exists(path):
            print(f"Advertencia: no se encontró {path}")
            continue

        frame = np.array(Image.open(path).convert("RGB"), dtype=np.uint16)

        if accumulated is None:
            accumulated = frame.copy()
        else:
            np.maximum(accumulated, frame, out=accumulated)

        # Componer con el overlay del cubo
        if box_overlay is not None:
            result = accumulated.copy()
            np.maximum(result, box_overlay, out=result)
        else:
            result = accumulated

        out_path = os.path.join(args.outdir, f"accum_{i:04d}.png")
        Image.fromarray(result.astype(np.uint8)).save(out_path)
        print(f"  {i:3d}/{args.n}  →  {out_path}")

    print("Listo.")

if __name__ == "__main__":
    main()
