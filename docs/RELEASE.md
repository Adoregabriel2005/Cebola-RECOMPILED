# Estado da publicação

A publicação atual é **v0.10.1**, um release normal promovido a **Latest**, com ZIP Windows x64 portátil para baixar, extrair e executar. Publicações anteriores permanecem como histórico. O status do GitHub não certifica estabilidade integral: os limites abaixo continuam declarados.

## Validações e permissões ainda pendentes

1. Percorrer as 12 fases, portas e serviços, chefes, mortes, continuações, segredos relevantes e final sem lacunas AOT, com regressões reproduzíveis.
2. Conferir comportamento contra outra implementação independente ou hardware, além da referência que compartilha partes do runtime.
3. Validar teclado, controles físicos, som, tela cheia, mudança de resolução e salvar/carregar em Windows 10 x64 real, incluindo máquinas de menor desempenho.
4. Resolver a ausência de licença declarada do núcleo e revisar permissões de distribuição do material e do binário derivado do jogo.

Os requisitos publicados são estimativas, com evidências e lacunas em REQUISITOS.md. O número de candidatos AOT não substitui esses critérios.

## Empacotamento

Execute release-source.ps1 a partir de um checkout Git limpo e commitado. O script empacota somente o conteúdo do commit informado, verifica a ausência de ROMs, binários e pastas de geração, e cria ZIP, manifesto com commit e SHA256SUMS.txt. Não baixa dependências nem envia arquivos ao GitHub. O diretório release-dist/ é ignorado pelo Git.

O pacote portátil é criado por release-portable.ps1 com uma lista explícita: aplicativo, SDL3, logo Underplay, licenças, instruções e documentação. Não inclui ROM, ferramentas de diagnóstico, saves, configurações de usuário ou dumps. O executável contém o runtime e traduções nativas geradas no build; a exigência de ROM não significa ausência de código derivado do jogo. A publicação foi solicitada pelo autor e não é uma declaração de regularização das permissões pendentes.
