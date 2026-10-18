param([Parameter(Mandatory=$true)][string]$RomPath,[string]$WorkRoot=(Join-Path $PSScriptRoot 'work'))
$ErrorActionPreference = 'Stop'
$rom = (Resolve-Path -LiteralPath $RomPath).Path
$verify = Join-Path $PSScriptRoot 'local-bin/v0.8/monica-verify.exe'
if (-not (Test-Path -LiteralPath $verify)) { throw 'Compile o projeto antes de executar os testes.' }
$testRoot = [IO.Path]::GetFullPath((Join-Path $WorkRoot "validation-$([Guid]::NewGuid().ToString('N'))"))
New-Item -ItemType Directory -Force $testRoot | Out-Null
Push-Location $testRoot
try {
    foreach ($case in @(@{End=800;Save=650},@{End=6000;Save=3500})) {
        $prefix = "case-$($case.Save)"
        & $verify $rom $case.End $case.Save "$prefix.msave" *> "$prefix-continuous.log"
        if ($LASTEXITCODE) { throw "Falha na execução nativa $prefix." }
        & $verify $rom $case.End 0 unused.msave "$prefix.msave" *> "$prefix-restored.log"
        if ($LASTEXITCODE) { throw "Falha ao restaurar $prefix." }
        $continuous = (Select-String -Path "$prefix-continuous.log" -Pattern '^RESULT ').Line
        $restored = (Select-String -Path "$prefix-restored.log" -Pattern '^RESULT ').Line
        if (-not $continuous -or $continuous -ne $restored) { throw "Estado, imagem ou áudio divergente em $prefix." }
        Write-Host "PASS: reconstrução $($case.Save) -> $($case.End), estado/imagem/áudio iguais e sem fallback."
    }
    $oldAction=$env:MONICA_VERIFY_ACTION
    try {
        foreach($mask in @(1,2,4,8,16,32,17,33,5,9,18,40)) {
            $env:MONICA_VERIFY_ACTION="$mask"
            & $verify $rom 2400 0 unused.msave *> "buttons-$mask.log"
            if($LASTEXITCODE){throw "Falha no roteiro de botão $mask."}
            $result=(Select-String -Path "buttons-$mask.log" -Pattern '^RESULT ').Line
            if(-not $result -or $result -notmatch 'misses=0$'){throw "Tradução incompleta no roteiro $mask."}
        }
        Write-Host 'PASS: 12 roteiros de botões com 2400 quadros, sem trecho ausente.'
    } finally { $env:MONICA_VERIFY_ACTION=$oldAction }
    $oldPause=$env:MONICA_VERIFY_PAUSE
    try {
        $env:MONICA_VERIFY_PAUSE='1'
        & $verify $rom 1800 850 pause.msave *> pause-continuous.log
        if($LASTEXITCODE){throw 'Falha nos dois acionamentos do Pause do console.'}
        & $verify $rom 1800 0 unused.msave pause.msave *> pause-restored.log
        if($LASTEXITCODE){throw 'Falha ao carregar com o jogo pausado.'}
        $a=(Select-String -Path pause-continuous.log -Pattern '^RESULT ').Line
        $b=(Select-String -Path pause-restored.log -Pattern '^RESULT ').Line
        if(-not $a -or $a -ne $b){throw 'Save com Pause divergiu.'}
        Write-Host 'PASS: Pause do console em duas bordas, segurar sem repetir, save pausado/restauração.'
    } finally { $env:MONICA_VERIFY_PAUSE=$oldPause }
    $bytes = [IO.File]::ReadAllBytes((Join-Path $testRoot 'case-650.msave'))
    $bytes[70] = $bytes[70] -bxor 1
    [IO.File]::WriteAllBytes((Join-Path $testRoot 'corrupt.msave'),$bytes)
    & $verify $rom 800 0 unused.msave corrupt.msave *> corrupt.log
    if ($LASTEXITCODE -ne 3) { throw 'Arquivo corrompido não foi rejeitado corretamente.' }
    Write-Host 'PASS: save corrompido rejeitado.'
    & (Join-Path $PSScriptRoot 'local-bin/v0.8/monica-system-verify.exe') (Join-Path $testRoot 'preferences.ini')
    if ($LASTEXITCODE) { throw 'Falha nos testes de controles/configurações.' }
    $app = Join-Path $PSScriptRoot 'local-bin/v0.8/MonicaRecomp-v0.8.exe'
    $report = Join-Path $testRoot 'navigation.json'
    Start-Process -FilePath $app -ArgumentList @('--selftest-navigation', ('"'+$report+'"'), '--data-dir', ('"'+$testRoot+'"')) -Wait -WindowStyle Hidden
    $nav = Get-Content -LiteralPath $report -Raw | ConvertFrom-Json
    if ($nav.menu_isolated -ne 1 -or $nav.menu_restored -ne 1 -or $nav.embedded_pages -ne 1 -or $nav.page_switches -ne 24 -or $nav.no_menu_bar -ne 1) { throw 'Falha na navegação integrada.' }
    Write-Host 'PASS: navegação integrada, foco/redimensionamento e retorno sem sobreposição.'
    Write-Host 'Estes testes validam a reconstrução do save. Não comprovam correção da jogabilidade.'
    Write-Host "Logs: $testRoot"
} finally { Pop-Location }
