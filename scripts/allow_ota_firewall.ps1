#Requires -RunAsAdministrator
[CmdletBinding()]
param(
    [string]$EspAddress = '192.168.0.48',
    [ValidateRange(1, 65534)]
    [int]$Port = 8267,
    [string]$PythonExe = (Join-Path $env:USERPROFILE '.platformio/python3/python.exe')
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $PythonExe -PathType Leaf)) {
    throw 'The PlatformIO Python executable was not found.'
}
$PythonExe = (Get-Item -LiteralPath $PythonExe).FullName
if (([ipaddress]$EspAddress).AddressFamily -ne 'InterNetwork') {
    throw 'The ESP address must be IPv4.'
}

$ruleName = 'DeadspaceESP8266OTA'
$changes = @()
$filters = Get-NetFirewallApplicationFilter | Where-Object { $_.Program -ieq $PythonExe }
foreach ($filter in $filters) {
    $rules = Get-NetFirewallRule -AssociatedNetFirewallApplicationFilter $filter
    foreach ($rule in $rules) {
        if ($rule.Enabled -ne 'True' -or $rule.Direction -ne 'Inbound' -or $rule.Action -ne 'Block') {
            continue
        }
        $portFilter = $rule | Get-NetFirewallPortFilter
        if ($portFilter.Protocol -eq 'TCP' -and @($portFilter.LocalPort).Count -eq 1 -and $portFilter.LocalPort -eq 'Any') {
            $changes += [PSCustomObject]@{ Name = $rule.Name; LocalPort = @($portFilter.LocalPort) }
        }
    }
}

# Explicit block rules override allow rules. Keep this Python program blocked
# on every other TCP port and allow only the ESP on the single OTA callback port.
$otherPorts = @(('0-' + ($Port - 1)), (($Port + 1).ToString() + '-65535'))
$projectDir = Split-Path -Parent $PSScriptRoot
$backupPath = Join-Path $projectDir '.pio/ota-firewall-backup.xml'
if ($changes.Count -gt 0 -and -not (Test-Path -LiteralPath $backupPath)) {
    $changes | Export-Clixml -LiteralPath $backupPath
}

try {
    foreach ($change in $changes) {
        Set-NetFirewallRule -Name $change.Name -LocalPort $otherPorts
    }

    $allowRule = @{
        Direction = 'Inbound'
        Action = 'Allow'
        Enabled = 'True'
        Profile = 'Any'
        Program = $PythonExe
        Protocol = 'TCP'
        LocalPort = $Port
        RemoteAddress = $EspAddress
    }
    if (Get-NetFirewallRule -Name $ruleName -ErrorAction SilentlyContinue) {
        Set-NetFirewallRule -Name $ruleName @allowRule
    } else {
        New-NetFirewallRule -Name $ruleName -DisplayName 'Deadspace ESP8266 OTA' @allowRule | Out-Null
    }
} catch {
    foreach ($change in $changes) {
        Set-NetFirewallRule -Name $change.Name -LocalPort $change.LocalPort
    }
    throw
}

Write-Output "Allowed OTA callbacks from $EspAddress to PlatformIO Python on TCP port $Port."
