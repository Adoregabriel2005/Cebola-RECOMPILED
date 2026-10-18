# Estado da publicação

A primeira publicação é **v0.10.0-preview.1**, marcada como pré-release e sem promoção a “Latest” estável. O pacote disponibiliza código e scripts para geração local. Não é anunciado como download de executável pronto.

## Critérios pendentes para versão estável

1. Percorrer as 12 fases, portas e serviços, chefes, mortes, continuações, segredos relevantes e final sem lacunas AOT, com regressões reproduzíveis.
2. Conferir comportamento contra outra implementação independente ou hardware, além da referência que compartilha partes do runtime.
3. Validar teclado, controles físicos, som, tela cheia, mudança de resolução e salvar/carregar em Windows 10 x64 real, incluindo máquinas de menor desempenho.
4. Resolver a ausência de licença declarada do núcleo e revisar permissões de distribuição do material e do binário derivado do jogo.

Os requisitos publicados são estimativas, com evidências e lacunas em REQUISITOS.md. O número de candidatos AOT não substitui esses critérios.

## Empacotamento

Execute release-source.ps1 a partir de um checkout Git limpo e commitado. O script empacota somente o conteúdo do commit informado, verifica a ausência de ROMs, binários e pastas de geração, e cria ZIP, manifesto com commit e SHA256SUMS.txt. Não baixa dependências nem envia arquivos ao GitHub. O diretório release-dist/ é ignorado pelo Git.
