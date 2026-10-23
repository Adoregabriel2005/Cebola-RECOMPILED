# Validação

## Identidade Cebola - RECOMPILED — 0.10.1

Em 23/10/2026, os patches foram aplicados sobre a revisão limpa fixada do smsggrecomp. O recompilador gerou novamente as 200.067 entradas AOT com o prefixo `cebola`, e todos os alvos foram compilados.

A suíte `test-local.ps1` passou: saves em processos diferentes, abertura, seis percursos até 18.500 quadros, comparação de imagem/áudio com a referência, 12 roteiros de botões, Pause, rejeição de save corrompido, controles, configurações e navegação. Após a atualização visual final, os testes de controles/configurações e navegação passaram novamente: 24 trocas de aba, 28 trocas de página, isolamento e retorno do menu.

As artes anteriores foram retiradas da árvore atual. A interface funciona sem arquivos dessas artes. A única imagem incluída é a logo fornecida para o rodapé do README, preservada byte a byte: SHA256 `8d93108033b58bb2099fe18250137ec7d4aeff9463618f1b0a81505b3b4e827d`.

A mudança de identidade não comprova cobertura integral do jogo. Os limites dos testes anteriores e da referência compartilhada continuam válidos.

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

## Revisão 0.8 — Pause original do console

Novo evento de aperto para NMI 0x0066, separado do menu do PC. Dois apertos nos quadros 700/1000, cada um segurado por dez quadros: exatamente duas NMIs em ambos os caminhos. Até o quadro 1800, framebuffer e áudio nativos coincidiram com a referência; zero misses. Save no quadro 850 (durante o Pause), restaurado em outro processo até 1800: mesmo estado, imagem e áudio. Save anterior da versão 0.7 no quadro 3500 também foi restaurado até 6000. O perfil contém agora 5.026 endereços. Patch do runtime aplicado a originais e comparado com a implementação testada. A API mantém os inputs antigos; o bit novo registra Pause para reprodução determinística.


## Revisão 0.9 — porta, região e avanço de fase

Reproduzidos misses 0x1950 na primeira porta e 0x1631 na abertura sem atalhos. O perfil passou de 5.026 para 7.115 pares banco/endereço observados, com fechamento estático de ramos diretos até 9.670 candidatos. A ferramenta tools/aot_reachability.c usa mapeamentos de banco observados; não demonstra cobertura de destinos indiretos desconhecidos nem de todas as fases.

Corrigida a porta de controle $3F e seus espelhos para TR/TH na máquina export. A ROM passa a selecionar português na primeira casa. Testes diretos exercitam as duas respostas TH da detecção de região, espelho de porta e saídas TR. Referências técnicas: https://www.smspower.org/Development/RegionDetection e https://www.smspower.org/Development/PeripheralPorts .

Roteiros em tests/routes/: first-door até 1300 (save 1000); first-dialogue até 3500 (save 1300); first-road até 5000 (save 3500); first-boss até 5500 (save 5100); round-two até 8500 (save 7900). Comparam framebuffer e áudio à referência; saves comparam também hash do estado na restauração em outro processo. Não há interpretador de fallback na execução nativa. Os roteiros representam a primeira casa, sua saída, combate inicial, primeiro chefe e chegada à segunda fase. Lojas e fases posteriores ainda não foram integralmente verificadas.

Saves passaram para REPLAY_BUILD=3: estado da porta de I/O incluído no hash e comportamento da região corrigido. Arquivos anteriores são preservados, mas precisam de build antigo para reprodução; não são migrados por adivinhação.


## Revisão 0.10 — entradas conservadoras e acervo histórico

O perfil parte de 7.562 pares observados e passa a 200.067 candidatos: todos os offsets das janelas/bancos observados e fechamento de ramos diretos considerando todos os bancos da página 2. Dados também podem ser candidatos; esse número não mede porcentagem de código nem prova que todas as fases foram percorridas. Casos nativos continuam protegidos por comparação de bytes e não há fallback de execução. Detalhes e limites em COBERTURA.md.

Os switches gerados foram divididos por faixas de 1 KiB, com helpers sem inline e otimização O1 nesses arquivos; o runtime mantém a otimização normal de Release. A geração ampla expôs uma classificação incorreta de HALT precedido por DD/FD: ela foi corrigida no decoder para não gerar uma referência inexistente a endereço indexado. Regressão com programas originais de 1/2 bytes confere flag HALT, comprimento, ausência de deslocamento e PC/ciclos contra o executor Z80 de referência. Patches completos aplicados a originais e migração incremental do patch publicado em 0.9 conferida.

Abertura sem atalhos até 5.000 quadros e seis roteiros: primeira porta, diálogo, estrada, sala de chefe, segunda fase e exploração até 18.500 quadros. Imagem e áudio coincidem com a referência; saves nos seis roteiros restauram o mesmo hash de estado. Regressões de 12 entradas e Pause original mantidas. Save criado pela versão 0.9 no quadro 7.900 foi carregado pela 0.10 até 8.500 com RESULT idêntico (estado/imagem/áudio). Uma medição local dessa reconstrução e avanço levou 5,59 s; é execução de desenvolvimento sem apresentação, não um benchmark de FPS da interface.

Sete capítulos de história renderizados e inspecionados em 1280×720; comparação também inspecionada em 960×720, sem corte do texto. Fontes, publicidade e ausência de cifra auditável de vendas ficam visíveis no próprio acervo. A navegação automática percorre os capítulos sem abrir janelas de configuração adicionais. O teste de exportação de cobertura usa entradas repetidas, mudanças, Pause e último intervalo para conferir o formato inclusivo do roteiro local.
