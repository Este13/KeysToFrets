param(
    [ValidateSet('Debug', 'Release')][string]$Config = 'Release',
    [switch]$Clean,
    [switch]$SkipTests,
    [string]$AsioSdk = ''
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

function Find-CMake {
    $cmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $install = & $vswhere -latest -products * -property installationPath
        if ($install) {
            $candidate = Join-Path $install 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
            if (Test-Path $candidate) { return $candidate }
        }
    }
    throw 'CMake not found. Install CMake or the Visual Studio "C++ CMake tools" component.'
}

function Find-Generator($cmake) {
    $help = & $cmake --help
    foreach ($name in 'Visual Studio 18 2026', 'Visual Studio 17 2022') {
        if ($help -match [regex]::Escape($name)) { return $name }
    }
    throw 'No supported Visual Studio generator found (need Visual Studio 2022 or 2026 with the C++ workload).'
}

function Ensure-Juce {
    if (Test-Path 'third_party/JUCE/CMakeLists.txt') { return }
    Write-Host 'Cloning JUCE 9.0.3...'
    git clone --depth 1 --branch 9.0.3 https://github.com/juce-framework/JUCE.git third_party/JUCE
    if ($LASTEXITCODE -ne 0) { throw 'JUCE clone failed' }
}

function Ensure-WebView2 {
    $version = '1.0.4258.31'
    $target = "third_party/webview2/Microsoft.Web.WebView2.$version"
    if (Test-Path "$target/build/native/include/WebView2.h") { return }
    Write-Host 'Downloading the WebView2 SDK package from NuGet...'
    New-Item -ItemType Directory -Force $target | Out-Null
    $zip = Join-Path $env:TEMP "webview2-$version.zip"
    Invoke-WebRequest -UseBasicParsing "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/$version/microsoft.web.webview2.$version.nupkg" -OutFile $zip
    Expand-Archive -Force $zip $target
}

function Ensure-UiVendor {
    if (Test-Path 'ui/vendor/tonal.js') { return }
    Write-Host 'Bundling Tonal.js locally...'
    npm install --no-audit --no-fund
    npm run bundle
}

$cmake = Find-CMake
$generator = Find-Generator $cmake
Ensure-Juce
Ensure-WebView2
Ensure-UiVendor

if ($Clean -and (Test-Path build)) { Remove-Item -Recurse -Force build }

$configureArgs = @('-S', '.', '-B', 'build', '-G', $generator, '-A', 'x64')
if ($AsioSdk) { $configureArgs += "-DKEYSTOFRETS_ASIO_SDK_DIR=$AsioSdk" }
& $cmake @configureArgs
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed' }

& $cmake --build build --config $Config --target KeysToFrets KeysToFretsTests --parallel
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }

if (-not $SkipTests) {
    $testExe = Get-ChildItem -Recurse build -Filter 'KeysToFretsTests.exe' | Where-Object { $_.FullName -match $Config } | Select-Object -First 1
    if ($testExe) {
        & $testExe.FullName
        if ($LASTEXITCODE -ne 0) { throw 'C++ tests failed' }
    }
    if (Get-Command node -ErrorAction SilentlyContinue) {
        npm test
        if ($LASTEXITCODE -ne 0) { throw 'JS tests failed' }
    }
}

$app = Get-ChildItem -Recurse build -Filter 'KeysToFrets.exe' | Where-Object { $_.FullName -match $Config } | Select-Object -First 1
Write-Host ''
Write-Host "Built: $($app.FullName)"
