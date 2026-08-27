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
