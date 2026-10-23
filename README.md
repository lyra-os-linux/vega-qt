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
`qt6-linguist-devel`, `kf6-kirigami-devel`, `cmake` e `ninja`.

## Traduções

O idioma-fonte e o fallback são o inglês (en-US). Os catálogos pt-BR e es-ES
ficam em `translations/` e são embutidos no `vega-qt` e no notificador de
atualizações. Qualquer variante de português usa pt-BR, e qualquer variante de
espanhol usa es-ES. Toda string visível usa `qsTr()` no QML ou `tr()` no C++.

Depois de alterar textos, atualize os catálogos e confira se nada ficou sem
tradução:

```sh
lupdate -locations relative -no-obsolete src qml \
  -ts translations/vega-qt_pt_BR.ts translations/vega-qt_es_ES.ts
scripts/check-translations.sh
```

## Instalação no openSUSE

O pacote `vega-qt` é publicado no mesmo repositório OBS das demais interfaces
do Vega: `home:rodrigosbrito:vega`. Ele pode coexistir com `vega-gtk` e
`vega-xfce`.

```sh
pkexec zypper ar -f \
  https://download.opensuse.org/repositories/home:/rodrigosbrito:/vega/openSUSE_Leap_16.0/ \
  vega-obs
pkexec zypper --gpg-auto-import-keys refresh vega-obs
pkexec zypper install vega-qt
```

Também é possível usar o instalador do repositório:

```sh
pkexec bash scripts/install-obs.sh
```

Esta é uma versão inicial para testes no openSUSE. Operações administrativas
continuam protegidas pelo Polkit através do `vegad`.

Consulte [docs/testando.md](docs/testando.md) para o roteiro de validação e
para saber quais informações incluir ao abrir um relato.

### Prazo e cancelamento do assistente

Cada chamada ao provedor tem um prazo total de 30 segundos, incluindo conexão,
envio e resposta completa. Receber pequenos trechos não renova esse prazo.
**Cancelar** interrompe a chamada; **Limpar** também interrompe uma chamada ativa
antes de apagar a conversa. Sucesso, erro, timeout e cancelamento liberam o estado
ocupado e os recursos de rede. Alterações de configuração ficam indisponíveis
durante a chamada. Não há repetição automática da mensagem.

A suíte abaixo usa HTTP real em loopback e um `secret-tool` fictício, sem acessar
provedores ou chaves pessoais. Ela precisa apenas de Qt Core, Network e Test:

```sh
cmake -S tests -B build-tests -G Ninja
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

O CI também compila a aplicação Qt/Kirigami completa. Os testes cobrem respostas
dos três provedores, servidor silencioso, resposta parcial, envio contínuo de
pequenos trechos, cancelamento, limpeza, erros e destruição durante uma chamada.
