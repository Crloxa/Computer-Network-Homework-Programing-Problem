param(
    [string]$file
)

chcp 65001 | Out-Null
if (-not $file) {
    Write-Error "No file specified"
    exit 1
}

$full = Resolve-Path $file
$dir = Split-Path $full
Set-Location $dir

$env:PATH = 'E:/Opencv/opencv/build/x64/vc16/bin;' + $env:PATH

$filename = Split-Path $full -Leaf
$out = [System.IO.Path]::ChangeExtension($filename, ".exe")

Write-Host "Compiling $filename -> $out ..."

g++ -g -I E:/Opencv/opencv/build/include $filename -L E:/Opencv/opencv/build/x64/vc16/lib -lopencv_world4120 -o $out

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build succeeded, running .\$out"
    & .\$out
} else {
    Write-Error "Build failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}
