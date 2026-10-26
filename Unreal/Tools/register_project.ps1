# Register the repository parent in the launcher/browser, without moving sources.
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$scanRoot = Split-Path $projectRoot -Parent
$launcherConfig = Join-Path $env:LOCALAPPDATA 'EpicGamesLauncher\Saved\Config'
$candidates = @('WindowsEditor', 'Windows') | ForEach-Object { Join-Path $launcherConfig "$_\GameUserSettings.ini" }
$settings = @($candidates | Where-Object { Test-Path -LiteralPath $_ })
if ($settings.Count -eq 0) { throw 'Launcher configuration not found. Open Epic Games Launcher once, then run again.' }
$editorSettings = Join-Path $env:LOCALAPPDATA 'UnrealEngine\5.7\Saved\Config\WindowsEditor\EditorSettings.ini'
if (Test-Path -LiteralPath $editorSettings) { $settings += $editorSettings }
$backupRoot = Join-Path $projectRoot 'Saved\RegistrationBackup'
New-Item -ItemType Directory -Force -Path $backupRoot | Out-Null
foreach ($path in $settings) {
    $section = if ($path -eq $editorSettings) { '[/Script/UnrealEd.EditorSettings]' } else { '[Launcher]' }
    $lines = [Collections.Generic.List[string]]::new()
    [IO.File]::ReadAllLines($path) | ForEach-Object { $lines.Add($_) }
    $start = $lines.IndexOf($section)
    if ($start -lt 0) { throw "Expected section missing: $section" }
    $end = $start + 1
    while ($end -lt $lines.Count -and !$lines[$end].StartsWith('[')) { $end++ }
    $entry = 'CreatedProjectPaths=' + $scanRoot.Replace('\', '/')
    $present = $false
    for ($index = $start + 1; $index -lt $end; $index++) {
        if ($lines[$index].Replace('\', '/').TrimEnd('/') -ieq $entry.TrimEnd('/')) { $present = $true }
    }
    if (!$present) {
        # Contains local launcher settings: backup is local and excluded from Git.
        $backupName = (Split-Path (Split-Path $path -Parent) -Leaf) + '-' + (Split-Path $path -Leaf) + '-' + [DateTime]::Now.ToString('yyyyMMddHHmmss') + '.bak'
        Copy-Item -LiteralPath $path -Destination (Join-Path $backupRoot $backupName)
        $lines.Insert($start + 1, $entry)
        [IO.File]::WriteAllLines($path, $lines, [Text.UTF8Encoding]::new($false))
    }
}
Write-Output "Project scan directory registered: $scanRoot"
Write-Output 'Open Unreal Engine > Library > My Projects. Refresh/reopen the launcher if its list is cached.'
