#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 3 ]]; then
  echo "Usage: $0 <input.md> [output.pptx] [reference.pptx]"
  echo "Example: $0 pgtbl-chat-presentation-mixed-v2.md"
  echo "Example: $0 pgtbl-chat-presentation-mixed-v2.md pgtbl-slides.pptx"
  echo "Example: $0 notes.md slides.pptx template.pptx"
  exit 1
fi

INPUT_MD="$1"
OUTPUT_PPTX="${2:-}"
REFERENCE_PPTX="${3:-}"

if [[ ! -f "$INPUT_MD" ]]; then
  echo "Error: input file not found: $INPUT_MD"
  exit 1
fi

if [[ -z "$OUTPUT_PPTX" ]]; then
  OUTPUT_PPTX="${INPUT_MD%.md}.pptx"
fi

if ! command -v pandoc >/dev/null 2>&1; then
  echo "Error: pandoc is not installed or not in PATH."
  echo "Arch install command: sudo pacman -S --needed pandoc"
  exit 1
fi

PANDOC_ARGS=(
  --from=gfm
  --to=pptx
  --slide-level=2
  --standalone
)

if [[ -n "$REFERENCE_PPTX" ]]; then
  if [[ ! -f "$REFERENCE_PPTX" ]]; then
    echo "Error: reference template not found: $REFERENCE_PPTX"
    exit 1
  fi
  PANDOC_ARGS+=(--reference-doc="$REFERENCE_PPTX")
fi

pandoc "${PANDOC_ARGS[@]}" "$INPUT_MD" -o "$OUTPUT_PPTX"

echo "Done: $OUTPUT_PPTX"
