#!/usr/bin/env bash
# Falha quando uma string visível não tem tradução pt-BR ou es-ES nos catálogos
# de translations/. O inglês é o idioma-fonte e o fallback.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

lupdate="${LUPDATE:-}"
if [ -z "$lupdate" ]; then
  for candidate in lupdate6 lupdate-qt6 /usr/lib/qt6/bin/lupdate /usr/lib64/qt6/bin/lupdate lupdate; do
    if command -v "$candidate" >/dev/null 2>&1; then
      lupdate="$(command -v "$candidate")"
      break
    fi
  done
fi
[ -n "$lupdate" ] || { echo "lupdate do Qt 6 não encontrado (defina LUPDATE)" >&2; exit 1; }

tmpdir="$(mktemp -d)"
trap 'rm -rf -- "$tmpdir"' EXIT
mkdir -p "$tmpdir/translations"
cp translations/vega-qt_pt_BR.ts translations/vega-qt_es_ES.ts "$tmpdir/translations/"
cp -r src qml "$tmpdir/"
(cd "$tmpdir" && "$lupdate" -no-obsolete src qml \
  -ts translations/vega-qt_pt_BR.ts translations/vega-qt_es_ES.ts >/dev/null)

status=0
for catalog in "$tmpdir"/translations/*.ts; do
  if grep -q 'type="unfinished"' "$catalog"; then
    echo "$(basename "$catalog"): strings sem tradução:" >&2
    grep -B4 'type="unfinished"' "$catalog" | sed -n 's/.*<source>\(.*\)<\/source>.*/  \1/p' >&2
    status=1
  fi
done
[ "$status" = 0 ] && echo "Traduções pt-BR e es-ES completas."
exit "$status"
