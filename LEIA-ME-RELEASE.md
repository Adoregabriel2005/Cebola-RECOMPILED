# Mônica Recomp 0.10.0-preview.1

**Pré-release de código para geração local. Não é uma versão estável nem um pacote pronto para jogar.**

O ZIP contém o frontend, ferramentas próprias, patches, documentação e materiais do projeto. Não inclui ROM, executável, DLLs, código C gerado ou uma cópia completa do runtime da dependência.

## Como usar

1. Extraia o ZIP em uma pasta de trabalho.
2. Tenha sua ROM compatível e as ferramentas descritas em docs/REQUISITOS.md instaladas no PATH.
3. Abra o PowerShell na pasta extraída monica-recomp e execute:

```powershell
./build-local.ps1 -RomPath 'C:/caminho/Monica no Castelo do Dragao (Brazil).sms'
./test-local.ps1 -RomPath 'C:/caminho/Monica no Castelo do Dragao (Brazil).sms'
```

4. O aplicativo gerado fica em local-bin/v0.10/MonicaRecomp-v0.10.exe. Mantenha SDL3.dll, assets/ e as licenças dessa pasta juntos. A geração ocorre com a ROM local do usuário.

Saves da versão 0.9 continuam compatíveis com a 0.10 (revisão 3). Saves anteriores não são convertidos automaticamente. Os dados ficam em %LOCALAPPDATA%/MonicaRecompPrototype.

## Estado desta revisão

Os testes até o início da segunda fase e a exploração adicional passaram, mas o jogo não foi validado até o final. 16:9 preserva proporções com barras ou recorte; ainda não mostra cenário extra. O menu usa o slot 1. O carregamento reconstrói a sessão, não restaura um snapshot instantâneo. Windows 10 é o alvo, mas ainda falta validação em uma instalação limpa real.

O núcleo smsggrecomp usado não declara licença. A publicação deste pacote não resolve essa pendência, nem concede direitos sobre o jogo, as marcas ou os materiais visuais. Consulte LICENSE_SCOPE.md e docs/DIREITOS.md. Não há binário derivado da ROM neste release.

Confira os requisitos estimados em docs/REQUISITOS.md e a cobertura em docs/COBERTURA.md. Os hashes de integridade dos arquivos publicados estão em SHA256SUMS.txt nos assets do release.
