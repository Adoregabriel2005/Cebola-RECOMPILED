# Validação da versão 0.5

## Testes automatizados aprovados

- AOT: roteiro de 6.000 quadros com zero dispatch ausente e zero fallback de interpretação.
- No roteiro de referência, framebuffer final e áudio acumulado coincidiram entre AOT e intérprete Z80 sobre o mesmo runtime.
- Save 650 -> 800 e save 3.500 -> 6.000, em processos diferentes: digest do estado, imagem e áudio iguais à execução contínua.
- Save com conteúdo corrompido rejeitado antes da execução.
- Dois joysticks virtuais: seleção do dispositivo correto, botões remapeados, direcional por hat, eixos e zona morta, desconexão sem seleção indevida de outro controle.
- Configurações: gravação/leitura e limitação de valores inválidos.
- Geometria: 16:9 preservando 4:3, recorte sem deformação, escala inteira e enquadramento 4:3/5:4/16:9.
- Teste da interface nativa: sem ROM válida, Jogar e Continuar desabilitados; com ROM válida, ambos habilitados. Selecionar ROM e Sobre o jogo disponíveis nos dois estados.

## Inspeção da interface

As imagens atuais de menu e configurações são renderizações do próprio código da interface, feitas por ferramentas locais em processos isolados. A renderização de controles não pressupõe um dispositivo físico conectado.

Regressão da sobreposição: quatro ciclos de abrir e fechar as páginas, alternando estado inicial/pausado, com 24 trocas de aba. O teste força notificações de foco e redimensionamento e verifica que os sete botões permanecem ocultos, que reaparecem ao voltar e que as páginas são filhas da janela principal sem barra de menus. Todos os critérios passaram. Isso não substitui a inspeção de movimento e latência em máquinas diferentes.

O teste com joysticks virtuais não comprova todos os controles físicos. Foi observada a identificação de um Xbox 360 Controller, sem uma bateria de testes físicos de botões, pareamento Bluetooth ou adaptadores.

## Limites da referência

O intérprete superzazu e o AOT compartilham barramento, VDP e PSG. A comparação verifica diferenças da execução da CPU no roteiro; ela não comprova exatidão do runtime em relação ao console. Ainda faltam comparação com emulador independente/hardware e validação do jogo completo, do Windows 10 real e de mais controles físicos.

A interface oferece enquadramento sem esticar, mas não câmera panorâmica com novos mapas visíveis. Não há snapshot instantâneo ou suporte FM implementado nesta versão.

## Seletores integrados

O componente próprio foi testado para mudança por teclado, passagem entre o primeiro e o último valor, leitura Unicode, limpeza de opções e ausência do estilo de janela popup. As seis páginas foram renderizadas pelo próprio aplicativo; Controles também foi conferida em 1024×640. Os testes de navegação e salvamento foram repetidos na revisão 0.5.

## Logo 0.5.1

Nova logo transparente renderizada pelo WIC em BGRA pré-multiplicado, composta com AlphaBlend e ajustada mantendo a proporção. Verificados texto, composição no menu e estados com/sem ROM. PNG RGBA de 2060×763 com alpha de 0 a 255. Compilação concluída; o núcleo do jogo não mudou nesta revisão.

## Revisão 0.6 — saída audiovisual

Teste silencioso contra a saída WinMM real: 100 lotes de 3.733 frames de origem, entregues em dois fragmentos por lote, sem espera artificial do produtor. O caminho de liberação dos buffers foi exercitado por pressão da fila. Produção e envio coincidiram em 44.100 Hz (73.585), 48.000 Hz (80.092) e 96.000 Hz (160.185), sem falha. Esse teste verifica contagem e envio; não comprova qualidade audível da música nem ausência de underruns em todos os drivers. Saves, controles e navegação integrada também passaram novamente. A apresentação do jogo agora compõe o fundo e a imagem em um buffer persistente e faz uma única cópia para a janela; falta avaliação perceptiva durante uma partida na máquina do usuário.

## Revisão 0.7 — direções e botões

Reproduzido no build anterior: pressionar cima entre os quadros 600 e 1200 causava dispatch ausente em 0x1942 no quadro 600. O perfil nativo passou de 4.986 para 5.010 endereços por banco. Foram testadas as máscaras 1, 2, 4, 8, 16, 32, 17, 33, 5, 9, 18 e 40 até o quadro 2400. Todos os roteiros nativos completaram com zero misses; framebuffer final e áudio acumulado coincidiram com o intérprete sobre o mesmo runtime. Isso não comprova cobertura de todas as fases. O aplicativo agora retorna ao menu se outra rota ainda não traduzida interromper a partida. Saves, controles e navegação integrada também passaram novamente.
