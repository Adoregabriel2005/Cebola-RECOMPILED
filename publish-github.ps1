param([string]$Repository = 'Cebola-RECOMPILED')
$ErrorActionPreference='Stop'
Get-Command gh,git -ErrorAction Stop | Out-Null
Push-Location $PSScriptRoot
try {
 & gh auth status
 if($LASTEXITCODE){throw 'Entre na sua conta com gh auth login antes de publicar.'}
 $owner=& gh api user --jq '.login'
 if($LASTEXITCODE){throw 'Não foi possível verificar a conta.'}
 $accountId=& gh api user --jq '.id'
 if($Repository -notmatch '^[a-zA-Z0-9_.-]+$'){throw 'Use somente o nome do repositório; a conta vem da autenticação atual.'}
 if(-not (Test-Path -LiteralPath (Join-Path $PSScriptRoot '.git'))){& git init -b main;if($LASTEXITCODE){throw 'Falha ao iniciar o repositório.'}}
 & git add -- .
 if($LASTEXITCODE){throw 'Falha ao preparar os arquivos.'}
 $tracked=& git ls-files
 foreach($path in $tracked){
   if($path -match '(^|/)(generated|work|local-bin|runtime)/' -or $path -match '\.(sms|gg|bin|exe|dll|msave|ram|vram|cram|cpu|regs|psg|sav|state)$'){throw "Arquivo incompatível com publicação: $path"}
 }
 $existing=& gh repo view "$owner/$Repository" --json name --jq '.name' 2>$null
 if($LASTEXITCODE -eq 0){throw 'Esse repositório já existe. Inspecione-o antes de atualizar; este script só cria repositórios novos.'}
 & git diff --cached --quiet
 if($LASTEXITCODE -eq 1){
  & git -c "user.name=$owner" -c "user.email=$accountId+$owner@users.noreply.github.com" commit -m 'Add educational Cebola Master System recomp project'
  if($LASTEXITCODE){throw 'Falha ao criar o commit.'}
 }
 & gh repo create "$owner/$Repository" --public --description 'Estudo de recompilação de Cebola - RECOMPILED para Windows; requer ROM própria.' --source . --remote origin --push
 if($LASTEXITCODE){throw 'A publicação não foi concluída. Nenhuma ROM foi enviada por este script.'}
} finally {Pop-Location}

