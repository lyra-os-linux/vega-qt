# Handoff — Vega Qt assíncrono e ISO KDE

Data: 2026-08-30

## Decisões mantidas

- Usar `pkexec` nas operações privilegiadas do host; não depender de `sudo`
  interativo.
- Não publicar nem sobrescrever a ISO GNOME.
- Publicar a imagem KDE somente em um destino de teste próprio, depois de
  validar o Vega Qt, a instalação e o primeiro boot.

## Estado concluído

- A ISO KDE com o instalador neutro e o Vega Qt OBS `0.1.4-lp161.15.1` foi
  construída com `LYRA_PRIVILEGE_TOOL=pkexec`.
- A instalação em disco virtual terminou e o primeiro desktop Plasma em
  português foi observado.
- O instalador não menciona GNOME/KDE e usa o destaque Segurança com escudo.
- Commits do instalador já publicados no `main` do `lyraos-desktop-kde`:
  - `ed18f8b Keep installer welcome desktop-neutral`
  - `f853164 Use neutral shield for security highlight`
- A ISO anterior à correção de fluidez tem SHA-256
  `91f227674a1809dfba9908577ccc511a27c31838160c06692c713a767292af82`.
  Ela não deve ser publicada, pois ainda contém o Vega Qt 0.1.4.

## Correções locais no Vega Qt

O travamento ao abrir e trocar abas foi localizado: todas as consultas ao
`vegad` usavam chamadas D-Bus bloqueantes na thread gráfica.

Em `src/systembackend.cpp`:

- Painel, Software, busca, transações, Serviços, Hardware, Armazenamento,
  Usuários, Rede, Firewall, Bluetooth, Logs, Monitor, Data/Hora e Backup agora
  usam `asyncCall` com `QDBusPendingCallWatcher`.
- Ações de software, repositórios, data/hora e backup também não bloqueiam
  mais a interface.
- Respostas antigas do Painel/Software/busca recebem identificadores para não
  sobrescrever uma consulta mais recente.
- Não restou `.call`, `callWithArgumentList` ou `QDBus::Block` em
  `src/systembackend.cpp`.

Em QML:

- o logotipo foi corrigido de `qrc:/vega.svg` para
  `qrc:/packaging/vega.svg`;
- a página Personalização usa `Carregando…` enquanto o mapa de aparência não
  contém seus campos, eliminando atribuições `undefined`.

Validações já executadas:

```text
cmake --build build -j4                     PASS
./scripts/check-package.sh                  PASS
git diff --check                            PASS
timeout 8 ./build/bin/vega-qt               abriu sem avisos QML no host
```

Os avisos `libEGL ... failed to create dri2 screen` vistos na VM pertencem à
aceleração gráfica virtual. Os loops de binding mostrados por
`StaticLookAndFeel.qml`/`Kirigami Dialog.qml` vieram do KCM externo aberto por
`kcmshell6`, não do carregamento das páginas do Vega.

## Ponto exato da VM

- Uma primeira mídia `/tmp/vega-qt-vm-update.iso` chegou a ser anexada, mas
  continha uma build intermediária.
- A build final foi copiada para `/tmp/vega-qt-vm-update/vega-qt` e empacotada
  em `/tmp/vega-qt-vm-update2.iso` (`VEGA_QT_UPDATE`).
- Antes de anexar a segunda mídia, a VM foi fechada. O socket
  `/var/tmp/lyraos-desktop-test-1001/vm/qemu-monitor.sock` não existe mais.
- Portanto, a build final ainda não foi instalada/testada dentro da VM.

## Próximos passos obrigatórios

1. Abrir novamente a VM KDE instalada sem apagar o disco virtual.
2. Anexar `/tmp/vega-qt-vm-update2.iso` ao CD virtual.
3. Na VM, instalar com `pkexec`:

   ```bash
   pkexec install -m 0755 /run/media/$USER/VEGA_QT_UPDATE/vega-qt /usr/bin/vega-qt
   ```

4. Abrir `vega-qt` pelo Konsole e navegar por todas as abas. Confirmar que a
   janela troca imediatamente de página e que os dados chegam depois, sem os
   erros de QRC/`undefined`.
5. Revisar e versionar a correção (provável 0.1.5), atualizar changelog/spec,
   commit, push/tag e publicar no OBS.
6. Confirmar que o novo RPM do OBS está disponível e reconstruir a ISO KDE
   com:

   ```bash
   LYRA_PRIVILEGE_TOOL=pkexec \
     ./kiwi/test/build-and-run-vm.sh --build-only --published-vega-qt
   ```

7. Repetir instalação e primeiro boot com a ISO final.
8. Publicar com nome explicitamente KDE/teste em um caminho SourceForge
   separado (planejado: `releases/27.02/kde/alpha7/`). Não usar o uploader
   Desktop/GNOME existente.
9. Baixar novamente a ISO publicada e comparar o SHA-256.

## Bloqueio externo conhecido

A chave SSH atual não autenticou no SourceForge em modo não interativo e o
agente SSH não estava acessível. Antes do upload, desbloquear/configurar a
chave do SourceForge no Konsole e testar a conexão. Nenhum arquivo foi enviado
ao SourceForge até aqui.
