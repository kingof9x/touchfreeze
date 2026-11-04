<#
.SYNOPSIS
    Builds TouchFreeze (x64 Release) and, if Inno Setup is installed, the installer.

.EXAMPLE
    .\build.ps1                 # build exe/dll + installer
    .\build.ps1 -NoInstaller    # build exe/dll only
    .\build.ps1 -Configuration Debug -NoInstaller

.NOTES
    Needs Visual Studio 2022+ or Build Tools with "Desktop development with C++"
    (MSVC + a Windows 10/11 SDK). The installer needs Inno Setup 6.3+:
        winget install --id JRSoftware.InnoSetup -e
#>
[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',
    [ValidateSet('x64', 'Win32')]
    [string]$Platform = 'x64',
    [switch]$NoInstaller
)

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot

# --- Locate MSBuild via vswhere -------------------------------------------
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe not found. Install Visual Studio or Build Tools with the C++ workload."
}
$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild `
    -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
if (-not $msbuild) { throw "MSBuild not found. Install the 'Desktop development with C++' workload." }

# --- Read version from the resource script ---------------------------------
$rc = Get-Content (Join-Path $root 'Sources\TestApp\KeyHookTest.rc') -Raw
if ($rc -notmatch '"ProductVersion",\s*"(\d+\.\d+\.\d+)') { throw "Could not read ProductVersion from KeyHookTest.rc" }
$version = $Matches[1]

Write-Host "Building TouchFreeze $version ($Configuration|$Platform)" -ForegroundColor Cyan
& $msbuild (Join-Path $root 'TouchFreeze.sln') /m /nologo /v:minimal `
    /p:Configuration=$Configuration /p:Platform=$Platform
if ($LASTEXITCODE -ne 0) { throw "MSBuild failed with exit code $LASTEXITCODE" }

$binDir = Join-Path $root "Executable\$Platform\$Configuration"
Write-Host "Binaries: $binDir" -ForegroundColor Green

if ($NoInstaller) { return }

if ($Platform -ne 'x64' -or $Configuration -ne 'Release') {
    Write-Warning "The installer is built from x64 Release only; skipping."
    return
}

# --- Locate Inno Setup compiler ---------------------------------------------
$iscc = @(
    (Get-Command ISCC.exe -ErrorAction SilentlyContinue | ForEach-Object Source),
    (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
    (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe')
) | Where-Object { $_ -and (Test-Path $_) } | Select-Object -First 1

if (-not $iscc) {
    Write-Warning "Inno Setup not found; installer not built. Install it with: winget install --id JRSoftware.InnoSetup -e"
    return
}

& $iscc /Q "/DAppVersion=$version" "/DBinDir=$binDir" (Join-Path $root 'installer\TouchFreeze.iss')
if ($LASTEXITCODE -ne 0) { throw "ISCC failed with exit code $LASTEXITCODE" }

Write-Host "Installer: $(Join-Path $root "dist\TouchFreeze-$version-setup.exe")" -ForegroundColor Green
