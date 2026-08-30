#!/usr/bin/env bash
set -euo pipefail

VEGA_QT_REPO_URL="https://download.opensuse.org/repositories/home:/rodrigosbrito:/vega/openSUSE_Leap_16.0/"
VEGA_QT_REPO_ALIAS="vega-obs"

if [ "$(id -u)" -ne 0 ]; then
  echo "Execute como administrador: pkexec bash scripts/install-obs.sh" >&2
  exit 1
fi

if ! command -v zypper >/dev/null 2>&1; then
  echo "Este instalador requer openSUSE e o gerenciador zypper." >&2
  exit 1
fi

if zypper lr "$VEGA_QT_REPO_ALIAS" >/dev/null 2>&1; then
  echo "Repositório $VEGA_QT_REPO_ALIAS já configurado."
else
  zypper --non-interactive addrepo --refresh "$VEGA_QT_REPO_URL" "$VEGA_QT_REPO_ALIAS"
fi

zypper --non-interactive --gpg-auto-import-keys refresh "$VEGA_QT_REPO_ALIAS"

if ! zypper --xmlout search --match-exact vega-qt 2>/dev/null \
  | grep -q '<solvable.*name="vega-qt"'; then
  echo >&2
  echo "O repositório foi configurado, mas o pacote vega-qt ainda não está disponível." >&2
  echo "A primeira publicação pode estar sendo processada pelo OBS. Tente novamente mais tarde:" >&2
  echo "  pkexec zypper refresh vega-obs" >&2
  echo "  pkexec zypper install vega-qt" >&2
  exit 2
fi

zypper --non-interactive install vega-qt

echo
echo "Vega Qt instalado. Abra 'Vega Qt' no menu do Plasma ou execute: vega-qt"
