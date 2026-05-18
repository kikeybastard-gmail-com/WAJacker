#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
ARTEFACTS="$BUILD_DIR/GrooveDigger_artefacts"

echo "==> Configuring..."
cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" -Wno-dev

echo "==> Building..."
cmake --build "$BUILD_DIR" --config Release

echo "==> Installing..."

# AU (AudioUnit) — preferred by Ableton on macOS
AU_SRC="$ARTEFACTS/AU/Groove Digger.component"
AU_DEST="$HOME/Library/Audio/Plug-Ins/Components"
if [ -d "$AU_SRC" ]; then
    mkdir -p "$AU_DEST"
    cp -r "$AU_SRC" "$AU_DEST/"
    echo "    AU  → $AU_DEST"
else
    echo "    AU not found, skipping"
fi

# VST3
VST3_SRC="$ARTEFACTS/VST3/GrooveDigger.vst3"
VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3"
if [ -d "$VST3_SRC" ]; then
    mkdir -p "$VST3_DEST"
    cp -r "$VST3_SRC" "$VST3_DEST/"
    echo "    VST3 → $VST3_DEST"
else
    echo "    VST3 not found, skipping"
fi

# Standalone app
APP_SRC="$ARTEFACTS/Standalone/Groove Digger.app"
APP_DEST="/Applications"
if [ -d "$APP_SRC" ]; then
    cp -r "$APP_SRC" "$APP_DEST/"
    echo "    App  → $APP_DEST"
else
    echo "    Standalone app not found, skipping"
fi

echo ""
echo "Done. In Ableton: Preferences → Plug-Ins → Rescan."
echo "The standalone app is in /Applications/Groove Digger.app"
