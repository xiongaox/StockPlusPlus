# tools/release.ps1
param (
    [string]$Version = ""
)

$ErrorActionPreference = "Stop"
$root = Resolve-Path "$PSScriptRoot\.."

# 1. 自动识别或解析版本号
$verHeaderPath = "$root\Plugins\Stock\Version.h"
if ([string]::IsNullOrWhiteSpace($Version)) {
    # 版本基准取「最新 Git Tag +1」：Tag 只在发包时产生，能保证 GitHub 发布号连续不断档。
    # 若以 Version.h 为基准，一旦有人提前手改版本号，发包就会跳号
    $latestTag = $null
    if (Get-Command git -ErrorAction SilentlyContinue) {
        try { $latestTag = (& git -C "$root" tag -l 'v*.*.*' --sort=-v:refname 2>$null | Select-Object -First 1) } catch { $latestTag = $null }
    }
    if ($latestTag -and $latestTag -match '^v(\d+)\.(\d+)\.(\d+)$') {
        $Version = "$([int]$Matches[1]).$([int]$Matches[2]).$([int]$Matches[3] + 1)"
        Write-Host "依据最新 Tag $latestTag 递增版本号: v$Version" -ForegroundColor Yellow
    } elseif (Test-Path $verHeaderPath) {
        $content = Get-Content $verHeaderPath -Raw
        if ($content -match 'STOCK_VERSION_MAJOR\s+(\d+)' -and $content -match 'STOCK_VERSION_MINOR\s+(\d+)' -and $content -match 'STOCK_VERSION_PATCH\s+(\d+)') {
            $curMajor = [int]$Matches[1]
            $curMinor = [int]$Matches[2]
            $curPatch = [int]$Matches[3] + 1
            $Version = "$curMajor.$curMinor.$curPatch"
        } else {
            $Version = "2.0.6"
        }
        Write-Host "未找到可用 Tag，回退依据 Version.h 递增版本号: v$Version" -ForegroundColor Yellow
    } else {
        $Version = "2.0.6"
        Write-Host "未找到可用 Tag 与 Version.h，回退默认版本: v$Version" -ForegroundColor Yellow
    }
}

$cleanVer = $Version.TrimStart('v', 'V')
$parts = $cleanVer.Split('.')
$major = if ($parts.Length -gt 0) { $parts[0] } else { "2" }
$minor = if ($parts.Length -gt 1) { $parts[1] } else { "0" }
$patch = if ($parts.Length -gt 2) { $parts[2] } else { "0" }
$displayVer = if ($parts.Length -gt 2 -and $parts[2] -ne "0") { "$major.$minor.$patch" } else { "$major.$minor" }

Write-Host ">>> [1/5] 正在更新 Version.h 为版本: v$displayVer ($major.$minor.$patch.0)..." -ForegroundColor Cyan

$versionHeader = @"
#pragma once

// 数值版本号（供 Stock.rc VERSIONINFO 使用）
#define STOCK_VERSION_MAJOR         $major
#define STOCK_VERSION_MINOR         $minor
#define STOCK_VERSION_PATCH         $patch
#define STOCK_VERSION_BUILD         0

// 辅助字符串宏转换
#define _STOCK_STR(x)               #x
#define _STOCK_TO_STR(x)            _STOCK_STR(x)
#define _STOCK_WSTR(x)              L#x
#define _STOCK_TO_WSTR(x)           _STOCK_WSTR(x)

// 宽字符版本号（供 Stock.cpp / ManagerDialog.cpp 界面与接口展示）
#define STOCK_VERSION_STR           L"$displayVer"

// 完整修订版宽字符
#define STOCK_FULL_VERSION_STR      _STOCK_TO_WSTR(STOCK_VERSION_MAJOR) L"." _STOCK_TO_WSTR(STOCK_VERSION_MINOR) L"." _STOCK_TO_WSTR(STOCK_VERSION_PATCH)

// ANSI 字符串版本号（供 Stock.rc StringFileInfo 使用）
#define STOCK_VERSION_RC_STR        _STOCK_TO_STR(STOCK_VERSION_MAJOR) "." _STOCK_TO_STR(STOCK_VERSION_MINOR) "." _STOCK_TO_STR(STOCK_VERSION_PATCH) "." _STOCK_TO_STR(STOCK_VERSION_BUILD)
"@

Set-Content -Path $verHeaderPath -Value $versionHeader -Encoding UTF8

# 1b. 更新日志闭环：给待发布的日期行补上本次版本号。
# 规范：日常提交的日期行只写日期，版本号只在发包时出现（否则 GitHub 发布号会断档）。
# kAboutLogGroups[] 按新->旧排列，顶部连续若干「无版本号」的日期分组都属于本次发布，一并补写；
# 遇到已带版本号（历史发布）的分组即停止，绝不改写历史
$mgrPath = "$root\Plugins\Stock\ManagerDialog.cpp"
if (Test-Path $mgrPath) {
    $mgrContent = Get-Content $mgrPath -Raw -Encoding UTF8
    $groupsMatch = [regex]::Match($mgrContent, 'const AboutLogGroup\s+kAboutLogGroups\[\]\s*=\s*\{(?<rows>[\s\S]*?)\};')
    if (-not $groupsMatch.Success) { throw "未能在 ManagerDialog.cpp 中定位 kAboutLogGroups[]，无法闭环更新日志！" }

    $rowsBase = $groupsMatch.Groups['rows'].Index
    $labelEdits = @()
    foreach ($row in [regex]::Matches($groupsMatch.Groups['rows'].Value, '\{\s*L"(?<label>[^"]+)"\s*,\s*(?<array>\w+)\s*,')) {
        $label = $row.Groups['label'].Value
        if ($label -notmatch '^\d{4}-\d{2}-\d{2}$') { break }
        $labelEdits += [pscustomobject]@{
            Index  = $rowsBase + $row.Groups['label'].Index
            Length = $row.Groups['label'].Length
            Text   = "$label (v$cleanVer)"
        }
    }

    if ($labelEdits.Count -eq 0) {
        Write-Host ">>> 更新日志顶部日期行已带版本号，本次未补写（请确认日志已覆盖本次改动）" -ForegroundColor Yellow
    } else {
        # 从后往前替换，避免前面的改写让后面的下标错位
        foreach ($edit in ($labelEdits | Sort-Object -Property Index -Descending)) {
            $mgrContent = $mgrContent.Remove($edit.Index, $edit.Length).Insert($edit.Index, $edit.Text)
        }
        [System.IO.File]::WriteAllText($mgrPath, $mgrContent, (New-Object System.Text.UTF8Encoding($false)))
        Write-Host ">>> 更新日志日期行已补写版本号 v$cleanVer（共 $($labelEdits.Count) 组）" -ForegroundColor Cyan
    }
}

# 2. 检查并关闭运行中的测试器
$tester = Get-Process -Name "PluginTester" -ErrorAction SilentlyContinue
if ($tester) {
    Write-Host ">>> [2/5] 检测到 PluginTester 运行中，正在关闭以释放 Stock.dll 占用..." -ForegroundColor Yellow
    Stop-Process -Name "PluginTester" -Force
}

# 3. 编译 x64 和 x86 Release
$msbuild = "D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if (-not (Test-Path $msbuild)) {
    $msbuild = "MSBuild.exe"
}

Write-Host ">>> [3/5] 正在编译 Release x64..." -ForegroundColor Cyan
& $msbuild "$root\Stock++.sln" /p:Configuration=Release /p:Platform=x64 /t:Stock /m /v:q
if ($LASTEXITCODE -ne 0) { throw "x64 编译失败！" }

Write-Host ">>> [3/5] 正在编译 Release x86..." -ForegroundColor Cyan
& $msbuild "$root\Stock++.sln" /p:Configuration=Release /p:Platform=x86 /t:Stock /m /v:q
if ($LASTEXITCODE -ne 0) { throw "x86 编译失败！" }

# 4. 打包输出至 download 目录
Write-Host ">>> [4/5] 正在打包发布产物到 download 目录..." -ForegroundColor Cyan
# 清理历史旧版本 zip 包
Get-ChildItem -Path "$root\download" -Filter "Stock_V*_x64.zip" | Remove-Item -Force
Get-ChildItem -Path "$root\download" -Filter "Stock_V*_x86.zip" | Remove-Item -Force
$x64Zip = "$root\download\Stock_V${cleanVer}_x64.zip"
$x86Zip = "$root\download\Stock_V${cleanVer}_x86.zip"
Compress-Archive -Path "$root\bin\x64\Release\Stock.dll" -DestinationPath $x64Zip -Force
Compress-Archive -Path "$root\bin\Release\Stock.dll" -DestinationPath $x86Zip -Force

# 同步更名现有的 arm64ec 包（若存在）
$armOld = Get-ChildItem -Path "$root\download" -Filter "Stock_V*_arm64ec.zip" | Select-Object -First 1
if ($armOld) {
    $armNew = "$root\download\Stock_V${cleanVer}_arm64ec.zip"
    if ($armOld.FullName -ne $armNew) {
        Move-Item -Path $armOld.FullName -Destination $armNew -Force
    }
}

# 5. 更新 download/plugin_download.md
Write-Host ">>> [5/6] 同步更新 download/plugin_download.md 下载链接..." -ForegroundColor Cyan
$dlDocPath = "$root\download\plugin_download.md"
if (Test-Path $dlDocPath) {
    (Get-Content $dlDocPath -Raw -Encoding UTF8) `
        -replace 'Stock_V[a-zA-Z0-9\.]+_x64\.zip', "Stock_V${cleanVer}_x64.zip" `
        -replace 'Stock_V[a-zA-Z0-9\.]+_x86\.zip', "Stock_V${cleanVer}_x86.zip" `
        -replace 'Stock_V[a-zA-Z0-9\.]+_arm64ec\.zip', "Stock_V${cleanVer}_arm64ec.zip" |
        Set-Content $dlDocPath -Encoding UTF8
}

# 6. 提取更新日志并生成 RELEASE_NOTES.md
Write-Host ">>> [6/6] 正在提取更新日志并生成 RELEASE_NOTES.md..." -ForegroundColor Cyan
$mgrPath = "$root\Plugins\Stock\ManagerDialog.cpp"
$bullets = @()
if (Test-Path $mgrPath) {
    $mgrContent = Get-Content $mgrPath -Raw -Encoding UTF8
    # 只取「日期行已带本次版本号」的分组：这些正是本次发布的条目（可能跨多个日期）
    $groupsMatch = [regex]::Match($mgrContent, 'const AboutLogGroup\s+kAboutLogGroups\[\]\s*=\s*\{(?<rows>[\s\S]*?)\};')
    if ($groupsMatch.Success) {
        foreach ($row in [regex]::Matches($groupsMatch.Groups['rows'].Value, '\{\s*L"(?<label>[^"]+)"\s*,\s*(?<array>\w+)\s*,')) {
            if ($row.Groups['label'].Value -notlike "*(v$cleanVer)") { continue }
            $arrName = [regex]::Escape($row.Groups['array'].Value)
            $bodyMatch = [regex]::Match($mgrContent, 'const wchar_t\*\s+' + $arrName + '\[\]\s*=\s*\{(?<items>[\s\S]*?)\};')
            if ($bodyMatch.Success) {
                $bullets += [regex]::Matches($bodyMatch.Groups['items'].Value, 'L"([^"]+)"') | ForEach-Object { "- " + $_.Groups[1].Value.Trim() }
            }
        }
    }
}

if ($bullets.Count -eq 0) { throw "未能提取到 v$cleanVer 的更新日志：请确认 ManagerDialog.cpp 的 kAboutLogGroups[] 顶部包含本次发布的日期分组" }
$bulletText = $bullets -join "`n"

$releaseNotes = @"
### 🚀 Stock Plugin v$cleanVer 更新日志

$bulletText

---
### 📦 安装包说明
- **x64 推荐版**：适用于绝大多数 64 位 Windows 系统及 64 位 TrafficMonitor
- **x86 兼容版**：适用于 32 位系统环境
- **ARM64EC 原生版**：适用于高通骁龙芯片 / Surface Pro X 等 ARM 架构设备
"@

$releaseNotesPath = "$root\RELEASE_NOTES.md"
Set-Content -Path $releaseNotesPath -Value $releaseNotes -Encoding UTF8

Write-Host ""
Write-Host "=================================================" -ForegroundColor Green
Write-Host "🎉 Stock 插件 Release v$cleanVer 打包完成！" -ForegroundColor Green
Write-Host "=================================================" -ForegroundColor Green
Write-Host "产物文件：" -ForegroundColor Green
Write-Host "  - $x64Zip"
Write-Host "  - $x86Zip"
Write-Host "  - $releaseNotesPath"
