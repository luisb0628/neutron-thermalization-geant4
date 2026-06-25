#!/usr/bin/env bash
# make_video.sh — pipeline completo: simulación → video
# Uso: ./make_video.sh [opciones]
#
#   -n  FRAMES     número de eventos/frames (default: 120)
#   -f  FPS        framerate del video (default: 60)
#   -a  ALPHA      opacidad del overlay del cubo (default: 0.25)
#   -m  MACRO      macro de visualización (default: macros/vis_video.mac)
#   -b  BOX_MACRO  macro de overlay de geometría (default: macros/vis_box_frame.mac)
#   -o  OUTPUT     nombre del MP4 de salida (default: video_simulacion.mp4)
#   -r  DPI        resolución Ghostscript (default: 72)
#   --no-sim       omitir la simulación Geant4 (reusar EPS existentes)
#   --no-convert   omitir conversión EPS→PNG (reusar PNGs existentes)
#   --no-rebuild   no recompilar aunque el código haya cambiado
#   -h             mostrar esta ayuda

set -euo pipefail

# ── Directorio base ───────────────────────────────────────────────────────────
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$ROOT/build"
PYTHON_SCRIPT="$ROOT/python/accumulate_frames.py"

# ── Defaults ──────────────────────────────────────────────────────────────────
N_FRAMES=120
FPS=60
BOX_ALPHA=0.25
MACRO="macros/vis_video.mac"
BOX_MAC_ARG=""
OUTPUT="video_simulacion.mp4"
DPI=72
RUN_SIM=1
RUN_CONVERT=1
RUN_REBUILD=1

# ── Colores ───────────────────────────────────────────────────────────────────
BOLD="\033[1m"; GREEN="\033[1;32m"; YELLOW="\033[1;33m"
RED="\033[1;31m"; CYAN="\033[1;36m"; RESET="\033[0m"

log()  { echo -e "${CYAN}[$(date +%H:%M:%S)]${RESET} $*"; }
ok()   { echo -e "${GREEN}[✓]${RESET} $*"; }
warn() { echo -e "${YELLOW}[!]${RESET} $*"; }
die()  { echo -e "${RED}[✗]${RESET} $*" >&2; exit 1; }
sep()  { echo -e "${BOLD}────────────────────────────────────────────────────────${RESET}"; }

# ── Parsear argumentos ────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        -n) N_FRAMES="$2"; shift 2 ;;
        -f) FPS="$2";      shift 2 ;;
        -a) BOX_ALPHA="$2"; shift 2 ;;
        -m) MACRO="$2";       shift 2 ;;
        -b) BOX_MAC_ARG="$2"; shift 2 ;;
        -o) OUTPUT="$2";      shift 2 ;;
        -r) DPI="$2";      shift 2 ;;
        --no-sim)     RUN_SIM=0;     shift ;;
        --no-convert) RUN_CONVERT=0; shift ;;
        --no-rebuild) RUN_REBUILD=0; shift ;;
        -h|--help)
            sed -n '2,20p' "$0" | sed 's/^# \?//'
            exit 0 ;;
        *) die "Argumento desconocido: $1" ;;
    esac
done

MACRO_PATH="$ROOT/$MACRO"
if [[ -n "$BOX_MAC_ARG" ]]; then
    BOX_MAC="$BOX_MAC_ARG"
else
    BOX_MAC="$ROOT/macros/vis_box_frame.mac"
fi
OUTPUT_PATH="$BUILD/$OUTPUT"

# ── Verificaciones previas ────────────────────────────────────────────────────
sep
echo -e "${BOLD}  make_video.sh — Neutron Thermalization${RESET}"
sep
log "Frames: $N_FRAMES  |  FPS: $FPS  |  Alpha cubo: $BOX_ALPHA"
log "Macro:  $MACRO_PATH"
log "Salida: $OUTPUT_PATH"
sep

[[ -f "$MACRO_PATH" ]]   || die "No se encontró el macro: $MACRO_PATH"
[[ -f "$PYTHON_SCRIPT" ]] || die "No se encontró: $PYTHON_SCRIPT"
command -v python3 >/dev/null 2>&1 || die "python3 no está disponible en el contenedor"
command -v gs     >/dev/null 2>&1 || die "Ghostscript (gs) no está instalado"
command -v ffmpeg >/dev/null 2>&1 || die "ffmpeg no está instalado"

# ── Display virtual (Docker/headless) ─────────────────────────────────────────
if [[ -z "${DISPLAY:-}" ]]; then
    command -v Xvfb >/dev/null 2>&1 || die "No hay DISPLAY y Xvfb no está instalado (apt-get install xvfb)"
    Xvfb :99 -screen 0 1280x1024x24 -nolisten tcp &
    XVFB_PID=$!
    export DISPLAY=:99
    sleep 1
    ok "Xvfb iniciado en :99 (PID $XVFB_PID)"
    trap 'kill $XVFB_PID 2>/dev/null' EXIT
fi

# ── PASO 0: (Re)compilar si hay cambios ───────────────────────────────────────
EXECUTABLE="$BUILD/Neutron_Thermalization"

if [[ $RUN_REBUILD -eq 1 ]]; then
    sep; log "Paso 0 — Verificar compilación"
    NEED_BUILD=0
    if [[ ! -f "$EXECUTABLE" ]]; then
        warn "Ejecutable no encontrado, compilando..."
        NEED_BUILD=1
    else
        if find "$ROOT/src" "$ROOT/include" "$ROOT/main.cc" \
                -newer "$EXECUTABLE" 2>/dev/null | grep -q .; then
            warn "Código fuente modificado, recompilando..."
            NEED_BUILD=1
        fi
    fi

    if [[ $NEED_BUILD -eq 1 ]]; then
        (cd "$BUILD" && cmake .. -DCMAKE_BUILD_TYPE=Release > /dev/null 2>&1 && \
         make -j"$(nproc)" 2>&1 | tail -3)
        ok "Compilación exitosa"
    else
        ok "Ejecutable actualizado, no es necesario recompilar"
    fi
fi

[[ -f "$EXECUTABLE" ]] || die "No se encontró el ejecutable: $EXECUTABLE"

# ── PASO 1: Simulación Geant4 ─────────────────────────────────────────────────
if [[ $RUN_SIM -eq 1 ]]; then
    sep; log "Paso 1 — Simulación Geant4 ($N_FRAMES eventos)"
    cd "$BUILD"
    rm -f frame_*.eps box_frame_*.eps

    # Frame de referencia del cubo (geometría sólida, sin trayectorias)
    if [[ -f "$BOX_MAC" ]]; then
        log "  Generando frame de referencia del cubo..."
        "$EXECUTABLE" "$BOX_MAC" >/dev/null 2>&1
        [[ -f box_frame_0000.eps ]] && ok "  box_frame_0000.eps generado" \
                                    || warn "  box_frame_0000.eps no apareció; overlay desactivado"
    fi

    # Simulación principal: 120 frames de trayectorias
    "$EXECUTABLE" "$MACRO_PATH" >/dev/null 2>&1
    N_EPS=$(ls frame_*.eps 2>/dev/null | wc -l)
    ok "Generados $N_EPS archivos EPS de trayectorias"
    [[ $N_EPS -gt 0 ]] || die "No se exportaron frames EPS — revisa el macro"
else
    warn "Simulación omitida (--no-sim)"
    cd "$BUILD"
fi

# ── PASO 2: Conversión EPS → PNG ──────────────────────────────────────────────
if [[ $RUN_CONVERT -eq 1 ]]; then
    sep; log "Paso 2 — Conversión EPS → PNG (${DPI} dpi)"
    cd "$BUILD"

    # Geant4 OGLSX añade _0000 al nombre base: box_frame.eps → box_frame_0000.eps
    if [[ -f box_frame_0000.eps ]]; then
        gs -dNOPAUSE -dBATCH -sDEVICE=png16m -r"$DPI" -dEPSCrop \
           -sOutputFile=box_frame.png box_frame_0000.eps >/dev/null 2>&1
        ok "box_frame.png generado"
    fi

    # Los EPS se llaman frame_N_0000.eps (N sin ceros). Convertir a frame_NNNN.png
    NCPU=$(nproc)
    COUNT=0
    for eps in frame_*_0000.eps; do
        [[ -f "$eps" ]] || continue
        # Extraer N de frame_N_0000.eps
        num="${eps#frame_}"; num="${num%_0000.eps}"
        png=$(printf "frame_%04d.png" "$num")
        gs -dNOPAUSE -dBATCH -sDEVICE=png16m -r"$DPI" -dEPSCrop \
           -sOutputFile="$png" "$eps" >/dev/null 2>&1 &
        COUNT=$((COUNT+1))
        if (( COUNT % NCPU == 0 )); then wait; fi
    done
    wait
    N_PNG=$(ls frame_[0-9][0-9][0-9][0-9].png 2>/dev/null | wc -l)
    ok "Convertidos $N_PNG frames PNG"
else
    warn "Conversión omitida (--no-convert)"
    cd "$BUILD"
fi

# ── PASO 3: Acumulación con Python ────────────────────────────────────────────
sep; log "Paso 3 — Acumulación de frames (overlay alpha=$BOX_ALPHA)"
cd "$BUILD"

BOX_ARG=""
[[ -f box_frame.png ]] && BOX_ARG="--box box_frame.png --box-alpha $BOX_ALPHA"

# shellcheck disable=SC2086
python3 "$PYTHON_SCRIPT" --n "$N_FRAMES" $BOX_ARG 2>&1 | tail -4

N_ACCUM=$(ls accum_*.png 2>/dev/null | wc -l)
[[ $N_ACCUM -gt 0 ]] || die "No se generaron frames acumulados"
ok "Frames acumulados: $N_ACCUM"

# ── PASO 4: Compilar video ────────────────────────────────────────────────────
sep; log "Paso 4 — Compilar video MP4 ($FPS fps)"
cd "$BUILD"

DURATION=$(echo "scale=1; $N_FRAMES / $FPS" | bc)

# ffmpeg escribe en stderr → redirigir 2>&1 para capturar progreso
ffmpeg -y \
    -framerate "$FPS" \
    -start_number 1 \
    -i accum_%04d.png \
    -c:v libx264 \
    -pix_fmt yuv420p \
    -crf 18 \
    "$OUTPUT" 2>&1 | grep -E "frame=|kb/s" | tail -2 || true

[[ -f "$OUTPUT" ]] || die "ffmpeg no generó el video — revisa los frames PNG"
ok "Video compilado: $OUTPUT (${DURATION}s a ${FPS}fps)"

# ── PASO 5: Limpieza ──────────────────────────────────────────────────────────
sep; log "Paso 5 — Limpieza de archivos intermedios"
rm -f frame_*.eps frame_*_0000.png frame_[0-9][0-9][0-9][0-9].png accum_*.png box_frame*.eps
ok "Limpieza completa"

# ── Resumen ───────────────────────────────────────────────────────────────────
sep
SIZE=$(du -sh "$OUTPUT_PATH" 2>/dev/null | cut -f1)
echo -e "${GREEN}${BOLD}  Video listo: $OUTPUT  ($SIZE, ${DURATION}s @ ${FPS}fps)${RESET}"
sep
