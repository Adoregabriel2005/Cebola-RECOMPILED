# Cebola - RECOMPILED

Projeto experimental de recompilação estática de Z80 para C, com execução nativa no Windows e interface Win32/GDI. A base técnica é [smsggrecomp, de mstan](https://github.com/mstan/smsggrecomp).

**A ROM não acompanha o projeto. Quem joga fornece seu próprio arquivo compatível.** Esta versão valida 256 KiB e CRC32 `01D67C0B`; outras revisões precisam de recompilação e validação próprias.

## Recursos

- Código recompilado por instrução e banco, com diagnóstico de caminhos ainda sem cobertura.
- Controles modernos por SDL3, configurações de vídeo e áudio e navegação por teclado ou controle.
- Salvamento por reconstrução das entradas, com verificação do estado.
- Vídeo e som do console pelo runtime da base; saída de áudio por WinMM.

## Baixar

O pacote Windows x64 está em [Releases](https://github.com/Adoregabriel2005/Cebola-RECOMPILED/releases/latest). Extraia o ZIP completo, abra `CebolaRecompiled.exe` e selecione sua ROM compatível.

## Compilar

No Windows, instale Git, CMake, Clang e mingw32-make. Execute no PowerShell:

```powershell
./build-local.ps1 -RomPath 'C:/roms/jogo.sms'
```

O script prepara as dependências fixadas por revisão, gera o código a partir da ROM local e compila o programa. A ROM, o código gerado e os binários não são rastreados pelo Git. Consulte [requisitos](docs/REQUISITOS.md), [validação](docs/VALIDATION.md) e os scripts de distribuição.

## Estado e créditos

A execução completa permanece em validação: não se declara cobertura integral. SDL3 fornece controles; Win32/GDI fornece a interface; WinMM fornece saída de áudio; superzazu/z80 participa das ferramentas de referência e testes.

As contribuições próprias seguem [LICENSE](LICENSE), com os limites descritos em [LICENSE_SCOPE.md](LICENSE_SCOPE.md) e as atribuições em [THIRD_PARTY.md](THIRD_PARTY.md). A revisão utilizada do smsggrecomp continua sem licença declarada; essa pendência não é resolvida pela licença do projeto.

A interface é desenhada por código. A logo abaixo foi fornecida pelo responsável pelo projeto e está preservada sem alterações.

[![Powered by Underplay](assets/underplay.png)](https://underplay.com.br)
