# Roteiro de desenvolvimento

1. Ampliar os roteiros e os endereços AOT para cobrir fases, portas, lojas, chefes, mortes, transições e final do jogo.
2. Comparar CPU, vídeo e áudio com uma implementação independente, além da referência que compartilha o runtime.
3. Testar Xbox, DualShock/DualSense, Switch, 8BitDo e adaptadores USB com/sem fio, usando seleção e remapeamento SDL quando necessário.
4. Identificar câmera, mapas, metatiles, criação de objetos e HUD para uma visão 16:9 que mostre mais cenário, preservando as regras do jogo.
5. Evoluir os saves para snapshots instantâneos de CPU, RAM, mapper, VDP, PSG e temporização, mantendo formato versionado e testes de retomada.
6. Validar Windows 10 em instalação real e definir um instalador que gere o build local a partir da ROM, sem exigir configuração manual do compilador.
7. Revisar licenças e distribuição antes de disponibilizar qualquer binário que contenha código derivado do jogo.

## Estrutura

- `src/app.c`: aplicativo Win32, menus, sessão e apresentação.
- `src/theme.c`: cenário, tipografia e botões do menu.
- `src/preferences.c`: controles, vídeo, áudio e história.
- `src/input.c`: gamepads e joysticks via SDL3.
- `src/replay.c`: formato de save por reconstrução e integridade.
- `patches/banked-aot.patch`: emissor C por instrução/banco e opção da ferramenta.
- `patches/runtime-aot.patch`: scheduler AOT, validação da ROM, digest do estado e instrumentação.
- `instruction-trace.txt`: endereços observados, sem bytes da ROM.
- `generated/`, `work/`, `local-bin/`: conteúdo local ignorado pelo Git.
