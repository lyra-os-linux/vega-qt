# Testando o Vega Qt

O Vega Qt pode ser instalado ao lado de `vega-gtk` e `vega-xfce`. Cada
interface possui executável e entrada de menu próprios.

## Instalação e atualização

```sh
sudo bash scripts/install-obs.sh
```

Depois da primeira instalação, as atualizações chegam pelo fluxo normal:

```sh
sudo zypper refresh
sudo zypper update vega-qt
```

## Roteiro rápido

1. Abra o Vega Qt pelo menu do Plasma.
2. Alterne entre os temas claro e escuro pelo botão no cabeçalho.
3. Visite todos os itens do menu lateral e confirme ícones e textos.
4. Confira os dados exibidos por Hardware, Armazenamento, Rede, Bluetooth,
   Serviços, Usuários e Monitor.
5. Em Log do Sistema, confirme que a autenticação administrativa é pedida
   antes de qualquer registro ser mostrado.
6. Em Software, teste primeiro uma busca e a consulta de atualizações.
7. Se usar Backup, faça o primeiro teste com uma pasta e um destino sem dados
   importantes.

## Escopo atual

Software, Backup, Data/Hora, Logs e Assistente possuem operações próprias.
Personalização encaminha para os módulos oficiais do Plasma. Hardware,
Armazenamento, Rede, Bluetooth, Serviços, Usuários e Monitor priorizam neste
momento consulta e diagnóstico; novos controles serão adicionados em versões
seguintes.

O Assistente usa uma chave fornecida pelo próprio usuário. A chave é mantida
no Secret Service da sessão e as mensagens são enviadas diretamente ao
provedor selecionado.

## Como relatar um problema

Abra uma issue em <https://github.com/lyra-os-linux/vega-qt/issues> incluindo:

- versão do pacote: `rpm -q vega-qt`;
- versão do openSUSE e do Plasma;
- módulo onde ocorreu o problema;
- passos para reproduzir;
- captura de tela, quando o problema for visual;
- saída de `vega-qt` no terminal, sem chaves, senhas ou outros dados pessoais.
