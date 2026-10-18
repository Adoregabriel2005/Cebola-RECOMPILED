# Mônica Recomp 0.10.0 — Windows x64 portátil

## Baixar e jogar

1. Baixe MonicaRecomp-v0.10.0-windows-x64.zip na página Releases.
2. Extraia o ZIP inteiro. Não execute o programa de dentro do arquivo compactado.
3. Abra MonicaRecomp.exe na pasta MonicaRecomp.
4. Clique em Selecionar ROM e escolha sua cópia compatível de Mônica no Castelo do Dragão (Brazil). Depois clique em Jogar.

Não é necessário instalar compiladores ou usar PowerShell para jogar com o pacote portátil. Mantenha SDL3.dll, assets/ e as licenças juntos do executável. A ROM não acompanha o download.

ROM suportada: 262144 bytes, CRC32 01D67C0B, SHA256 484FDF3CB85C1BFD1CA938CFD59FD582A06EE20F9F3686BB3DFD1F7A70193086.

## Controles padrão

Setas: mover; Z/X: botões 1/2; P: Pause do console; Esc: menu do PC; F5/F9: salvar/carregar; F11: tela cheia. O remapeamento fica em Controles.

Saves da 0.9 e da 0.10 continuam compatíveis (revisão 3). Dados e configurações ficam em %LOCALAPPDATA%/MonicaRecompPrototype. Não são gravados dentro da pasta do jogo.

## Requisitos estimados para 720p

Windows 10 x64 22H2; CPU x86-64/SSE2 de dois núcleos em torno de 2 GHz; 2 GB de RAM (4 GB recomendados); vídeo integrado compatível; tela 1280×720; 250 MB livres. Não são mínimos certificados em computadores antigos.

## Limitações conhecidas

O jogo ainda não foi validado até o final. 16:9 usa barras ou recorte preservando proporções; ainda não mostra cenário extra. O menu usa o slot 1. O carregamento reconstrói a sessão e pode demorar em partidas longas. Windows 10 é o alvo, mas ainda falta validação em uma instalação limpa real.

A publicação é um release normal/Latest no GitHub. Esse status não certifica ausência de bugs nem resolve as pendências de licenciamento documentadas. O executável contém traduções nativas geradas durante o build e componentes do núcleo smsggrecomp, cuja revisão não declara licença. Exigir ROM própria não elimina os direitos sobre o jogo. Consulte LICENSE_SCOPE.md, THIRD_PARTY.md e docs/DIREITOS.md. Os materiais visuais continuam sujeitos aos direitos dos titulares.

O pacote não é assinado digitalmente. Consulte os hashes SHA-256 publicados para conferir a integridade do download. Código e instruções de build permanecem no repositório.
