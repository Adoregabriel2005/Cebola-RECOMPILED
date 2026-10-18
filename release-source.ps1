param(
    [string]$Version = '0.10.0-preview.1',
    [string]$OutputDirectory = (Join-Path $PSScriptRoot 'release-dist')
)
$ErrorActionPreference='Stop'
if($Version -notmatch '^\d+\.\d+\.\d+-preview\.\d+$'){throw 'Este empacotador publica somente versões preview, sem afirmar estabilidade.'}
& git -C $PSScriptRoot rev-parse --show-toplevel *> $null
if($LASTEXITCODE){throw 'Execute a partir de um checkout Git do projeto.'}
$repoRoot=(& git -C $PSScriptRoot rev-parse --show-toplevel).Trim()
if([IO.Path]::GetFullPath($repoRoot) -ne [IO.Path]::GetFullPath($PSScriptRoot)){throw 'A pasta do projeto deve ser a raiz do checkout.'}
$dirty=& git -C $PSScriptRoot status --porcelain
if($LASTEXITCODE -or $dirty){throw 'Faça commit das alterações antes de empacotar; somente HEAD será incluído.'}
$commit=(& git -C $PSScriptRoot rev-parse HEAD).Trim()
$tracked=& git -C $PSScriptRoot ls-tree -r --name-only HEAD
if($LASTEXITCODE){throw 'Não foi possível ler os arquivos do commit.'}
foreach($name in $tracked){
    if($name -match '(^|/)(generated|runtime|work|build|local-bin|release-dist)/' -or $name -match '\.(sms|gg|bin|exe|dll|msave|sav|state|ram|vram|cram|cpu|regs|psg)$' -or $name -eq 'coverage-last-route.txt'){
        throw "Arquivo incompatível com o release de fontes: $name"
    }
}
$destination=[IO.Path]::GetFullPath($OutputDirectory)
$packageName="MonicaRecomp-v$Version-fontes.zip"
$package=Join-Path $destination $packageName
$manifestPath=Join-Path $destination 'release-manifest.json'
$hashPath=Join-Path $destination 'SHA256SUMS.txt'
foreach($file in @($package,$manifestPath,$hashPath)){if(Test-Path -LiteralPath $file){throw "O arquivo já existe e não será sobrescrito: $file"}}
New-Item -ItemType Directory -Force -Path $destination | Out-Null
& git -C $PSScriptRoot archive --format=zip --prefix=monica-recomp/ "--output=$package" HEAD
if($LASTEXITCODE){throw 'Falha ao gerar o arquivo do commit.'}
$packageHash=(Get-FileHash -LiteralPath $package -Algorithm SHA256).Hash.ToLowerInvariant()
$manifest=[ordered]@{
    version=$Version
    commit=$commit
    package=$packageName
    package_sha256=$packageHash
    package_bytes=(Get-Item -LiteralPath $package).Length
    kind='source-preview'
    executable_included=$false
    rom_included=$false
    target='Windows x64; local build required'
    requirements='Estimates only; see docs/REQUISITOS.md'
}
$utf8=New-Object Text.UTF8Encoding($false)
[IO.File]::WriteAllText($manifestPath,(ConvertTo-Json $manifest -Depth 3)+"`n",$utf8)
$manifestHash=(Get-FileHash -LiteralPath $manifestPath -Algorithm SHA256).Hash.ToLowerInvariant()
[IO.File]::WriteAllText($hashPath,"$packageHash  $packageName`n$manifestHash  release-manifest.json`n",$utf8)
Write-Host "Release de fontes: $package"
Write-Host "Commit: $commit"
Write-Host "SHA256: $packageHash"
