$ErrorActionPreference = 'Stop'
$solDir = (Resolve-Path "$PSScriptRoot").Path + '\'
$msbuild = 'D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe'
if (-not (Test-Path $msbuild)) {
    $msbuild = 'MSBuild.exe'
}
# MSBuild 属性值以反斜杠结尾时，若路径含空格（如 "Program Files (x86)"），末尾反斜杠会转义掉
# 收尾引号，把 /m /v:m 并进属性值并报 MSB4184；闭合引号前写两个反斜杠即可正确收尾。
$solDirArg = "/p:SolutionDir=`"$($solDir.TrimEnd('\'))\\`""
& $msbuild "${solDir}Plugins\Stock\Stock.vcxproj" /p:Configuration=Release /p:Platform=x64 $solDirArg /m /v:m
if ($LASTEXITCODE -ne 0) { Write-Error 'Build Stock failed.'; exit 1 }
Write-Host '[+] Stock.dll build OK'
