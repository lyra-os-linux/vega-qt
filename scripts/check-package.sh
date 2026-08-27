#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

test -f packaging/vega.svg
test -f packaging/org.lyraos.Vega.Qt.desktop
grep -q '^Icon=vega$' packaging/org.lyraos.Vega.Qt.desktop
grep -q 'packaging/vega.svg' CMakeLists.txt
grep -q '%{_datadir}/icons/hicolor/scalable/apps/vega.svg' packaging/obs/vega-qt.spec

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

echo "Verificação do Vega Qt concluída com sucesso."
