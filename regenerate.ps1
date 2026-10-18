param([Parameter(Mandatory=$true)][string]$RomPath)
& (Join-Path $PSScriptRoot 'build-local.ps1') -RomPath $RomPath
