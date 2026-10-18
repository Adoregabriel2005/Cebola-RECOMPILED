# Requisitos estimados — Windows x64

São **estimativas de engenharia para 720p**, não mínimos certificados por testes em máquinas antigas. A versão atual é experimental. Não há garantia de 60 FPS em todos os computadores ou em todas as fases. “Mínimo estimado” não significa máquina efetivamente testada.

## Para executar o aplicativo gerado localmente

| Componente | Mínimo estimado | Recomendado |
|---|---|---|
| Sistema | Windows 10 de 64 bits, versão 22H2 | Windows 10/11 de 64 bits |
| Processador | x86-64 com SSE2, dois núcleos, cerca de 2 GHz | Core i3 / Ryzen 3 ou equivalente, dois ou mais núcleos com bom desempenho por núcleo |
| Memória do sistema | 2 GB de RAM total | 4 GB ou mais |
| Vídeo | GPU integrada com driver compatível com o Windows; apresentação GDI | GPU integrada recente; não é exigida placa dedicada |
| Tela | 1280×720, escala de 100%, usando tela cheia se necessário | 1920×1080 |
| Armazenamento | 250 MB livres para aplicativo, dados e margem de saves | 500 MB livres ou mais |
| Som | Saída de áudio reconhecida pelo Windows | Dispositivo com driver atualizado |
| Entrada | Teclado | Teclado ou controle USB/Bluetooth reconhecido pelo Windows e SDL3 |
| Jogo | ROM própria compatível, 256 KiB, CRC32 01D67C0B | A mesma ROM suportada |

Os requisitos de CPU e RAM acima são estimados a partir da arquitetura e do escopo do aplicativo, não de uma bateria de benchmarks nesses modelos. O vídeo atual usa GDI/AlphaBlend/BitBlt, não um renderizador 3D que exija uma GPU dedicada. Resoluções altas e filtros aumentam o custo de apresentação. Drivers e controles físicos precisam de validação individual.

O aplicativo x64 não roda em Windows de 32 bits. ARM64 não foi validado. Mantenha SDL3.dll, assets/ e os textos de licença junto do executável. A ROM não acompanha o pacote. O save registra entradas por quadro e reconstrói a sessão; partidas longas aumentam o tempo de carregamento e o tamanho dos arquivos.

## Para gerar o build a partir deste release de código

Além da ROM e da conexão para obter as dependências:

- 8 GB de RAM e 5 GB livres de disco são a estimativa recomendada para compilar; 16 GB e SSD dão margem para compilação paralela.
- PowerShell, Git, CMake e uma toolchain Clang/LLVM-MinGW com cc, clang, windres e mingw32-make acessíveis pelo PATH.
- O build atual usa o gerador “MinGW Makefiles”; instalar somente o compilador MSVC não satisfaz esse script.
- A primeira preparação baixa fontes das dependências e pode demorar vários minutos. O tempo depende de rede, CPU e disco.

Esses requisitos de compilação são distintos dos requisitos para jogar. O ZIP Windows x64 da página Releases já inclui o executável e a SDL3; essas ferramentas adicionais só são necessárias para quem optar por compilar.

## Evidência disponível

A revisão 0.10 passou pelos roteiros e verificações descritos em [VALIDATION.md](VALIDATION.md), incluindo introdução, primeira porta, primeiro chefe, chegada à segunda fase, exploração de 18.500 quadros e reconstrução de saves. Uma medição anterior de 8.500 quadros sem apresentação levou 5,59 s na máquina de desenvolvimento; isso não certifica uma CPU mínima nem mede o FPS da interface.

Não foi concluído um teste do jogo inteiro, uma matriz de computadores de baixo desempenho ou uma sessão em instalação física limpa de Windows 10. Essas validações permanecem necessárias para declarar mínimos certificados e uma versão estável.
