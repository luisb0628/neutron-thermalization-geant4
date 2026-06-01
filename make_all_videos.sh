#!/usr/bin/env bash
# make_all_videos.sh — genera 5 videos con diferentes geometrías de parafina
#
# Uso: ./make_all_videos.sh [-n FRAMES] [-f FPS] [-a ALPHA] [-r DPI]
#
# Geometrías:
#   1. Thin Slab       — 20x20x4  cm
#   2. Compact Cube    — 10x10x10 cm
#   3. Deep Block      — 10x10x30 cm
#   4. Wide Plate      — 30x30x8  cm
#   5. Asymmetric      — 20x6x16  cm

set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

BOLD="\033[1m"; GREEN="\033[1;32m"; CYAN="\033[1;36m"; RESET="\033[0m"
sep() { echo -e "${BOLD}══════════════════════════════════════════════════════${RESET}"; }

# ── Defaults (pasan directo a make_video.sh) ──────────────────────────────────
N_FRAMES=120
FPS=60
BOX_ALPHA=0.25
DPI=72

while [[ $# -gt 0 ]]; do
    case "$1" in
        -n) N_FRAMES="$2"; shift 2 ;;
        -f) FPS="$2";      shift 2 ;;
        -a) BOX_ALPHA="$2"; shift 2 ;;
        -r) DPI="$2";      shift 2 ;;
        -h|--help)
            sed -n '2,12p' "$0" | sed 's/^# \?//'
            exit 0 ;;
        *) echo "Argumento desconocido: $1"; exit 1 ;;
    esac
done

# ── Definición de las 5 geometrías ───────────────────────────────────────────
# Formato: "vis_macro:box_macro:output_mp4"
declare -a GEOS=(
    "vis_video_geo1.mac:vis_box_frame_geo1.mac:video_geo1_losa_delgada.mp4"
    "vis_video_geo2.mac:vis_box_frame_geo2.mac:video_geo2_cubo_compacto.mp4"
    "vis_video_geo3.mac:vis_box_frame_geo3.mac:video_geo3_bloque_profundo.mp4"
    "vis_video_geo4.mac:vis_box_frame_geo4.mac:video_geo4_placa_ancha.mp4"
    "vis_video_geo5.mac:vis_box_frame_geo5.mac:video_geo5_asimetrico.mp4"
)

declare -a LABELS=(
    "Geo 1 — Thin Slab       20x20x4  cm"
    "Geo 2 — Compact Cube    10x10x10 cm"
    "Geo 3 — Deep Block      10x10x30 cm"
    "Geo 4 — Wide Plate      30x30x8  cm"
    "Geo 5 — Asymmetric      20x6x16  cm"
)

TOTAL=${#GEOS[@]}
FAILED=()
FIRST=1

sep
echo -e "${BOLD}  make_all_videos.sh — 5 geometrías de parafina${RESET}"
echo -e "  Frames: $N_FRAMES  |  FPS: $FPS  |  DPI: $DPI  |  Alpha: $BOX_ALPHA"
sep

for (( idx=0; idx<TOTAL; idx++ )); do
    IFS=: read -r VIS_MAC BOX_MAC OUTPUT <<< "${GEOS[$idx]}"
    LABEL="${LABELS[$idx]}"

    echo ""
    sep
    echo -e "${CYAN}  [$((idx+1))/$TOTAL]  $LABEL${RESET}"
    sep

    REBUILD_FLAG=""
    [[ $FIRST -eq 0 ]] && REBUILD_FLAG="--no-rebuild"
    FIRST=0

    if bash "$ROOT/make_video.sh" \
            -m "macros/$VIS_MAC" \
            -b "$ROOT/macros/$BOX_MAC" \
            -o "$OUTPUT" \
            -n "$N_FRAMES" \
            -f "$FPS" \
            -a "$BOX_ALPHA" \
            -r "$DPI" \
            $REBUILD_FLAG; then
        echo -e "${GREEN}  ✓ $OUTPUT generado${RESET}"
    else
        echo -e "\033[1;31m  ✗ Falló: $OUTPUT\033[0m"
        FAILED+=("$OUTPUT")
    fi
done

# ── Resumen final ─────────────────────────────────────────────────────────────
echo ""
sep
echo -e "${BOLD}  Resumen${RESET}"
sep
echo -e "  Videos generados en: $ROOT/build/"
echo ""
for (( idx=0; idx<TOTAL; idx++ )); do
    IFS=: read -r _ _ OUTPUT <<< "${GEOS[$idx]}"
    LABEL="${LABELS[$idx]}"
    FILE="$ROOT/build/$OUTPUT"
    if [[ -f "$FILE" ]]; then
        SIZE=$(du -sh "$FILE" | cut -f1)
        echo -e "  ${GREEN}✓${RESET}  $LABEL  →  $OUTPUT  ($SIZE)"
    else
        echo -e "  \033[1;31m✗\033[0m  $LABEL  →  $OUTPUT  (no generado)"
    fi
done

if [[ ${#FAILED[@]} -gt 0 ]]; then
    echo ""
    echo -e "\033[1;31m  ${#FAILED[@]} video(s) fallaron.\033[0m"
    exit 1
fi

echo ""
sep
echo -e "${GREEN}${BOLD}  ¡Todos los videos generados exitosamente!${RESET}"
sep
