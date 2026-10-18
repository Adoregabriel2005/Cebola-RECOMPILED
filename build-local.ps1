param(
 [Parameter(Mandatory=$true)][string]$RomPath,
 [string]$WorkRoot = (Join-Path $PSScriptRoot 'work'),
 [switch]$SkipDependencies
)
$ErrorActionPreference='Stop'
Push-Location $PSScriptRoot
try {
$rom=(Resolve-Path -LiteralPath $RomPath).Path
if((Get-FileHash -LiteralPath $rom -Algorithm SHA256).Hash -ne '484FDF3CB85C1BFD1CA938CFD59FD582A06EE20F9F3686BB3DFD1F7A70193086'){throw 'ROM diferente da versão suportada.'}
$work=[IO.Path]::GetFullPath($WorkRoot)
if(-not $SkipDependencies){& (Join-Path $PSScriptRoot 'prepare-deps.ps1') -WorkRoot $work}
$engine=Join-Path $work 'deps/smsggrecomp'
$sdlBuild=Join-Path $work 'deps/sdl-build'
$toolBuild=Join-Path $work 'recompiler-build'
$appBuild=Join-Path $work 'app-build'
$destination=Join-Path $PSScriptRoot 'local-bin/v0.9'
& cmake -S (Join-Path $engine 'recompiler') -B $toolBuild -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release
if($LASTEXITCODE){throw 'Falha na configuração do recompilador.'}
& cmake --build $toolBuild -j 4
if($LASTEXITCODE){throw 'Falha na compilação do recompilador.'}
& (Join-Path $toolBuild 'SmsRecomp.exe') $rom --game (Join-Path $PSScriptRoot 'game.toml')
if($LASTEXITCODE){throw 'Falha na tradução por funções para as ferramentas de diagnóstico.'}
$reach=Join-Path $toolBuild 'aot-reachability.exe'
& clang -O2 -municode "-I$engine/recompiler/src" (Join-Path $PSScriptRoot 'tools/aot_reachability.c') (Join-Path $engine 'recompiler/src/z80_decoder.c') -o $reach
if($LASTEXITCODE){throw 'Falha na ferramenta de alcance estático.'}
& $reach $rom (Join-Path $PSScriptRoot 'instruction-seeds.txt') (Join-Path $PSScriptRoot 'instruction-trace.txt')
if($LASTEXITCODE){throw 'Falha na análise dos caminhos AOT.'}
& (Join-Path $toolBuild 'SmsRecomp.exe') $rom --game (Join-Path $PSScriptRoot 'game.toml') --bank-step-trace (Join-Path $PSScriptRoot 'instruction-trace.txt')
if($LASTEXITCODE){throw 'Falha na geração AOT por instrução.'}
& cmake -S $PSScriptRoot -B $appBuild -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release "-DENGINE=$engine" "-DSDL3_DIR=$sdlBuild" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$destination"
if($LASTEXITCODE){throw 'Falha na configuração do aplicativo.'}
& cmake --build $appBuild -j 4
if($LASTEXITCODE){throw 'Falha na compilação. Feche a mesma versão do aplicativo se estiver em uso.'}
Copy-Item -LiteralPath (Join-Path $work 'deps/SDL3/LICENSE.txt') -Destination (Join-Path $destination 'LICENSE-SDL.txt') -Force
Write-Host "Protótipo local: $(Join-Path $destination 'MonicaRecomp-v0.9.exe')"
Write-Host 'O código gerado e os executáveis são locais. Não os publique com a ROM.'

} finally { Pop-Location }

