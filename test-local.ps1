param([Parameter(Mandatory=$true)][string]$RomPath,[string]$WorkRoot=(Join-Path $PSScriptRoot 'work'))
$ErrorActionPreference = 'Stop'
$rom = (Resolve-Path -LiteralPath $RomPath).Path
$verify = Join-Path $PSScriptRoot 'local-bin/v0.10/cebola-verify.exe'
if (-not (Test-Path -LiteralPath $verify)) { throw 'Compile o projeto antes de executar os testes.' }
$testRoot = [IO.Path]::GetFullPath((Join-Path $WorkRoot "validation-$([Guid]::NewGuid().ToString('N'))"))
New-Item -ItemType Directory -Force $testRoot | Out-Null
Push-Location $testRoot
try {
    & (Join-Path $PSScriptRoot 'local-bin/v0.10/cebola-decoder-verify.exe')
    if($LASTEXITCODE){throw 'Falha na regressão HALT prefixado.'}
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
    $oldIdle=$env:CEBOLA_VERIFY_IDLE
    $introReference=$env:CEBOLA_VERIFY_REFERENCE
    try {
        $env:CEBOLA_VERIFY_IDLE='1';$env:CEBOLA_VERIFY_REFERENCE=$null
        & $verify $rom 5000 0 unused.msave *> intro-native.log
        if($LASTEXITCODE){throw 'Falha na introdução sem atalhos.'}
        $env:CEBOLA_VERIFY_REFERENCE='1'
        & $verify $rom 5000 0 unused.msave *> intro-reference.log
        if($LASTEXITCODE){throw 'Falha na introdução de referência.'}
        $a=(Select-String intro-native.log -Pattern '^RESULT ').Line -split ' '
        $b=(Select-String intro-reference.log -Pattern '^RESULT ').Line -split ' '
        if($a[3] -ne $b[3] -or $a[4] -ne $b[4]){throw 'Introdução divergente.'}
        Write-Host 'PASS: abertura completa sem atalhos, imagem e áudio iguais à referência.'
    } finally {$env:CEBOLA_VERIFY_IDLE=$oldIdle;$env:CEBOLA_VERIFY_REFERENCE=$introReference}
    $oldTimeline=$env:CEBOLA_VERIFY_TIMELINE
    $oldReference=$env:CEBOLA_VERIFY_REFERENCE
    try {
        foreach($case in @(@{Name='first-door';End=1300;Save=1000},@{Name='first-dialogue';End=3500;Save=1300},@{Name='first-road';End=5000;Save=3500},@{Name='first-boss';End=5500;Save=5100},@{Name='round-two';End=8500;Save=7900},@{Name='round-two-explore';End=18500;Save=10100})) {
            $env:CEBOLA_VERIFY_TIMELINE=Join-Path $PSScriptRoot "tests/routes/$($case.Name).txt"
            $env:CEBOLA_VERIFY_REFERENCE=$null
            & $verify $rom $case.End $case.Save "$($case.Name).msave" *> "$($case.Name)-native.log"
            if($LASTEXITCODE){throw "Falha nativa: $($case.Name)."}
            & $verify $rom $case.End 0 unused.msave "$($case.Name).msave" *> "$($case.Name)-restored.log"
            if($LASTEXITCODE){throw "Falha no save: $($case.Name)."}
            $a=(Select-String "$($case.Name)-native.log" -Pattern '^RESULT ').Line
            $b=(Select-String "$($case.Name)-restored.log" -Pattern '^RESULT ').Line
            if(-not $a -or $a -ne $b){throw "Save divergente: $($case.Name)."}
            $env:CEBOLA_VERIFY_REFERENCE='1'
            & $verify $rom $case.End 0 unused.msave *> "$($case.Name)-reference.log"
            if($LASTEXITCODE){throw "Falha na referência: $($case.Name)."}
            $r=(Select-String "$($case.Name)-reference.log" -Pattern '^RESULT ').Line -split ' '
            $n=$a -split ' '
            if($n[3] -ne $r[3] -or $n[4] -ne $r[4]){throw "Imagem/áudio divergente: $($case.Name)."}
            Write-Host "PASS: $($case.Name), imagem/áudio iguais à referência e save restaurado."
        }
    } finally {$env:CEBOLA_VERIFY_TIMELINE=$oldTimeline;$env:CEBOLA_VERIFY_REFERENCE=$oldReference}
    $oldAction=$env:CEBOLA_VERIFY_ACTION
    try {
        foreach($mask in @(1,2,4,8,16,32,17,33,5,9,18,40)) {
            $env:CEBOLA_VERIFY_ACTION="$mask"
            & $verify $rom 2400 0 unused.msave *> "buttons-$mask.log"
            if($LASTEXITCODE){throw "Falha no roteiro de botão $mask."}
            $result=(Select-String -Path "buttons-$mask.log" -Pattern '^RESULT ').Line
            if(-not $result -or $result -notmatch 'misses=0$'){throw "Tradução incompleta no roteiro $mask."}
        }
        Write-Host 'PASS: 12 roteiros de botões com 2400 quadros, sem trecho ausente.'
    } finally { $env:CEBOLA_VERIFY_ACTION=$oldAction }
    $oldPause=$env:CEBOLA_VERIFY_PAUSE
    try {
        $env:CEBOLA_VERIFY_PAUSE='1'
        & $verify $rom 1800 850 pause.msave *> pause-continuous.log
        if($LASTEXITCODE){throw 'Falha nos dois acionamentos do Pause do console.'}
        & $verify $rom 1800 0 unused.msave pause.msave *> pause-restored.log
        if($LASTEXITCODE){throw 'Falha ao carregar com o jogo pausado.'}
        $a=(Select-String -Path pause-continuous.log -Pattern '^RESULT ').Line
        $b=(Select-String -Path pause-restored.log -Pattern '^RESULT ').Line
        if(-not $a -or $a -ne $b){throw 'Save com Pause divergiu.'}
        Write-Host 'PASS: Pause do console em duas bordas, segurar sem repetir, save pausado/restauração.'
    } finally { $env:CEBOLA_VERIFY_PAUSE=$oldPause }
    $bytes = [IO.File]::ReadAllBytes((Join-Path $testRoot 'case-650.msave'))
    $bytes[70] = $bytes[70] -bxor 1
    [IO.File]::WriteAllBytes((Join-Path $testRoot 'corrupt.msave'),$bytes)
    & $verify $rom 800 0 unused.msave corrupt.msave *> corrupt.log
    if ($LASTEXITCODE -ne 3) { throw 'Arquivo corrompido não foi rejeitado corretamente.' }
    Write-Host 'PASS: save corrompido rejeitado.'
    & (Join-Path $PSScriptRoot 'local-bin/v0.10/cebola-system-verify.exe') (Join-Path $testRoot 'preferences.ini')
    if ($LASTEXITCODE) { throw 'Falha nos testes de controles/configurações.' }
    $app = Join-Path $PSScriptRoot 'local-bin/v0.10/CebolaRecompiled-v0.10.exe'
    $report = Join-Path $testRoot 'navigation.json'
    Start-Process -FilePath $app -ArgumentList @('--selftest-navigation', ('"'+$report+'"'), '--data-dir', ('"'+$testRoot+'"')) -Wait -WindowStyle Hidden
    $nav = Get-Content -LiteralPath $report -Raw | ConvertFrom-Json
    if ($nav.menu_isolated -ne 1 -or $nav.menu_restored -ne 1 -or $nav.embedded_pages -ne 1 -or $nav.page_switches -ne 24 -or $nav.no_menu_bar -ne 1 -or $nav.history_switches -ne 28 -or $nav.coverage_route -ne 1) { throw 'Falha na navegação integrada.' }
    Write-Host 'PASS: navegação integrada, 28 trocas de página do guia, exportação de roteiro e retorno sem sobreposição.'
    Write-Host 'Estes testes validam a reconstrução do save. Não comprovam correção da jogabilidade.'
    Write-Host "Logs: $testRoot"
} finally { Pop-Location }
