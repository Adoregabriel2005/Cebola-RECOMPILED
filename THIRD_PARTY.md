# Dependências, material histórico e direitos

## Base de recompilação

- smsggrecomp: https://github.com/mstan/smsggrecomp
- Revisão usada: `224d5bb2c150a2c295033d35dec629ef9ee42940`.
- O README da base informa que sua licença ainda não foi declarada. Não há concessão de licença dessa base por este projeto.
- A base e seus submódulos são obtidos diretamente de suas origens durante a preparação local. Os patches deste projeto não incluem a ROM, as traduções geradas ou uma cópia completa do runtime.
- superzazu/Z80, usado pela base para referência, tem licença MIT própria. Consulte os arquivos originais obtidos com a dependência.

## SDL3

- Fonte: https://github.com/libsdl-org/SDL
- Versão: 3.4.18, commit `829a65d769d935c4852f8159e964312c0957260a`.
- Licença zlib: https://github.com/libsdl-org/SDL/blob/release-3.4.18/LICENSE.txt
- O build inclui HIDAPI, Raw Input, DirectInput, Windows Gaming Input e XInput quando disponíveis na toolchain.
- A cópia de SDL3.dll no build local deve permanecer acompanhada de LICENSE-SDL.txt.

## Jogo e imagens

- Mônica no Castelo do Dragão: Tec Toy, 1991; adaptação brasileira de Wonder Boy in Monster Land, Westone/Sega, com personagens de Maurício de Sousa.
- `assets/period-box.png` e `assets/period-advert.png`: imagens de época fornecidas pelo usuário. Exibidas para contextualização histórica, sem alegação de autoria do projeto.
- `docs/images/tela-titulo.png` e `jogabilidade.png`: capturas geradas durante testes com a ROM do usuário. Não substituem nem incluem a ROM inteira.
- `assets/menu-forest.png`: ilustração de fã criada por ImageGen para o menu. Prompt em `assets/menu-art-prompt.txt`. Não é arte oficial ou extraída da ROM; os personagens continuam pertencendo aos titulares.
- O controle preto na tela de configuração é desenhado por código, inspirado no layout do Control Pad do Master System. Não foi incluída uma foto de terceiros.

ROM, personagens, artes, marcas e código original do jogo continuam sujeitos aos seus direitos. Finalidade educacional e exigência de ROM própria não criam uma licença automática. Não foi atribuída uma licença geral que pretenda cobrir os componentes de terceiros.

- Logo do título: title-logo.png é uma nova adaptação de fã feita com ImageGen, inspirada na marca clássica e nas referências fornecidas pelo usuário. Não é uma logo oficial. O prompt está em assets/title-logo-prompt.txt. A marca e os elementos originais continuam pertencendo aos respectivos titulares. app-icon.png/app.ico: nova ilustração de fã feita pelo ImageGen integrado; não é um ícone oficial. O prompt está em assets/icon-art-prompt.txt.

## Textos de licença e escopo

Contribuições próprias: LICENSE (MIT), sujeito a LICENSE_SCOPE.md. SDL3: LICENSES/SDL3-zlib.txt; HIDAPI: alternativa BSD escolhida, LICENSES/HIDAPI-BSD-3-Clause.txt; superzazu/z80: LICENSES/superzazu-z80-MIT.txt. A base smsggrecomp continua com autorização pendente. Consulte docs/DIREITOS.md. Esses documentos não concedem direitos sobre personagens, marcas ou o cartucho.
