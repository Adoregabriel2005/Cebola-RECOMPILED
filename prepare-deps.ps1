param([string]$WorkRoot = (Join-Path $PSScriptRoot 'work'))
$ErrorActionPreference = 'Stop'
foreach ($name in @('git','cmake','clang','mingw32-make')) { Get-Command $name -ErrorAction Stop | Out-Null }
$work = [IO.Path]::GetFullPath($WorkRoot)
$deps = Join-Path $work 'deps'
New-Item -ItemType Directory -Force $deps | Out-Null
$engine = Join-Path $deps 'smsggrecomp'
$engineCommit = '224d5bb2c150a2c295033d35dec629ef9ee42940'
if (-not (Test-Path -LiteralPath $engine)) {
    & git clone --no-checkout https://github.com/mstan/smsggrecomp.git $engine
    if ($LASTEXITCODE) { throw 'Não foi possível baixar a base.' }
    & git -C $engine checkout --detach $engineCommit
    if ($LASTEXITCODE) { throw 'Não foi possível selecionar a versão da base.' }
}
$actual = & git -C $engine rev-parse HEAD
if ($LASTEXITCODE -or $actual -ne $engineCommit) { throw 'A base existente usa outra revisão. Preserve seu trabalho e escolha outro WorkRoot.' }
& git -C $engine submodule update --init --recursive
if ($LASTEXITCODE) { throw 'Falha ao preparar os submódulos.' }
foreach ($name in @('banked-aot.patch','runtime-aot.patch')) {
    $patch = Join-Path $PSScriptRoot "patches/$name"
    & git -C $engine apply --reverse --check $patch 2>$null
    if ($LASTEXITCODE) {
        & git -C $engine apply --check $patch
        if ($LASTEXITCODE) {
            $upgrade=Join-Path $PSScriptRoot 'patches/upgrade-0.9-to-0.10.patch'
            if($name -eq 'banked-aot.patch') {
                & git -C $engine apply --check $upgrade
                if(-not $LASTEXITCODE) {
                    & git -C $engine apply $upgrade
                    if($LASTEXITCODE){throw 'Falha na atualização incremental do gerador.'}
                    & git -C $engine apply --reverse --check $patch
                    if($LASTEXITCODE){throw 'A atualização não corresponde ao patch esperado.'}
                    continue
                }
            }
            throw "Patch incompatível: $name. Nenhum arquivo será redefinido automaticamente."
        }
        & git -C $engine apply $patch
        if ($LASTEXITCODE) { throw "Falha ao aplicar $name." }
    }
}
$sdl = Join-Path $deps 'SDL3'
if (-not (Test-Path -LiteralPath $sdl)) {
    & git clone --depth 1 --branch release-3.4.18 https://github.com/libsdl-org/SDL.git $sdl
    if ($LASTEXITCODE) { throw 'Falha ao baixar SDL3.' }
}
$actual = & git -C $sdl rev-parse HEAD
if ($LASTEXITCODE -or $actual -ne '829a65d769d935c4852f8159e964312c0957260a') { throw 'Revisão de SDL3 diferente da esperada.' }
$sdlBuild = Join-Path $deps 'sdl-build'
& cmake -S $sdl -B $sdlBuild -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release -DSDL_AUDIO=ON -DSDL_DIRECTX=ON -DSDL_VIDEO=OFF -DSDL_RENDER=OFF -DSDL_GPU=OFF -DSDL_CAMERA=OFF -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF -DSDL_INSTALL=OFF
if ($LASTEXITCODE) { throw 'Falha ao configurar SDL3.' }
& cmake --build $sdlBuild -j 4
if ($LASTEXITCODE) { throw 'Falha ao compilar SDL3.' }
Write-Host "Dependências prontas: $deps"
