param(
    [string]$BuildDirectory=(Join-Path $PSScriptRoot 'local-bin/v0.10'),
    [string]$OutputDirectory=(Join-Path $PSScriptRoot 'release-dist/v0.10.1'),
    [string]$Version='0.10.1'
)
$ErrorActionPreference='Stop'
if($Version -notmatch '^\d+\.\d+\.\d+$'){throw 'Versão inválida.'}
$build=[IO.Path]::GetFullPath($BuildDirectory)
$destination=[IO.Path]::GetFullPath($OutputDirectory)
$stage=Join-Path $destination 'CebolaRecompiled'
$zip=Join-Path $destination "CebolaRecompiled-v$Version-windows-x64.zip"
if((Test-Path -LiteralPath $stage) -or (Test-Path -LiteralPath $zip)){throw 'Escolha uma pasta de saída nova; nada será sobrescrito.'}
$files=@(
    @('CebolaRecompiled-v0.10.exe','CebolaRecompiled.exe'),@('SDL3.dll','SDL3.dll'),
    @('assets/underplay.png','assets/underplay.png'),
    @('LICENSE','LICENSE'),@('LICENSE_SCOPE.md','LICENSE_SCOPE.md'),@('THIRD_PARTY.md','THIRD_PARTY.md'),
    @('LICENSES/SDL3-zlib.txt','LICENSES/SDL3-zlib.txt'),@('LICENSES/HIDAPI-BSD-3-Clause.txt','LICENSES/HIDAPI-BSD-3-Clause.txt'),
    @('LICENSES/superzazu-z80-MIT.txt','LICENSES/superzazu-z80-MIT.txt')
)
foreach($pair in $files){if(-not(Test-Path -LiteralPath (Join-Path $build $pair[0]) -PathType Leaf)){throw "Falta arquivo do build: $($pair[0])"}}
New-Item -ItemType Directory -Path $stage -Force | Out-Null
foreach($pair in $files){$target=Join-Path $stage $pair[1];New-Item -ItemType Directory -Force -Path (Split-Path $target) | Out-Null;Copy-Item -LiteralPath (Join-Path $build $pair[0]) -Destination $target}
foreach($name in @('LEIA-ME-RELEASE.md','LICENSE_SCOPE.md','THIRD_PARTY.md','docs/REQUISITOS.md','docs/DIREITOS.md','docs/RELEASE.md')){
    $target=Join-Path $stage $name;New-Item -ItemType Directory -Force -Path (Split-Path $target) | Out-Null;Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination $target -Force
}
Compress-Archive -LiteralPath $stage -DestinationPath $zip -CompressionLevel Optimal
$hash=(Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
$utf8=New-Object Text.UTF8Encoding($false)
[IO.File]::WriteAllText((Join-Path $destination 'SHA256SUMS.txt'),"$hash  $([IO.Path]::GetFileName($zip))`n",$utf8)
Write-Host "ZIP portátil: $zip"
Write-Host "SHA256: $hash"
