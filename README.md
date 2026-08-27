# Vega Qt

Interface Qt 6/Kirigami do centro de controle Vega para o flavor KDE do
LyraOS. Este repositório é independente das interfaces `vega-gtk` e
`vega-xfce`.

O frontend reutiliza o daemon `vegad` e o serviço D-Bus `org.lyraos.Vega1`.

## Desenvolvimento

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/bin/vega-qt
```

Dependências: `qt6-base-devel`, `qt6-declarative-devel`,
`kf6-kirigami-devel`, `cmake` e `ninja`.

## Instalação no openSUSE

O pacote `vega-qt` é publicado no mesmo repositório OBS das demais interfaces
do Vega: `home:rodrigosbrito:vega`. Ele pode coexistir com `vega-gtk` e
`vega-xfce`.

```sh
sudo zypper ar -f \
  https://download.opensuse.org/repositories/home:/rodrigosbrito:/vega/openSUSE_Leap_16.0/ \
  vega-obs
sudo zypper --gpg-auto-import-keys refresh vega-obs
sudo zypper install vega-qt
```

Também é possível usar o instalador do repositório:

```sh
sudo bash scripts/install-obs.sh
```

Esta é uma versão inicial para testes no openSUSE. Operações administrativas
continuam protegidas pelo Polkit através do `vegad`.

Consulte [docs/testando.md](docs/testando.md) para o roteiro de validação e
para saber quais informações incluir ao abrir um relato.
