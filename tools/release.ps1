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

Write-Host ">>> [1/6] 正在更新 Version.h 为版本: v$displayVer ($major.$minor.$patch.0)..." -ForegroundColor Cyan

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

# 1c. 发布前置校验：日志里必须已存在本次版本号，否则说明本次改动还没写进日志。
# 校验放在编译之前：否则要白等一轮 x64+x86 构建才会在生成发布说明时失败
if (Test-Path $mgrPath) {
    $mgrCheck = Get-Content $mgrPath -Raw -Encoding UTF8
    $verToken = "(v$cleanVer)"
    if ($mgrCheck -notmatch [regex]::Escape($verToken)) {
        throw "更新日志中没有任何分组标记为 $verToken：请先在 ManagerDialog.cpp 的 kAboutLogGroups[] 顶部补充本次改动的日期分组（日期行只写日期），再执行发包"
    }
}

# 2. 检查并关闭运行中的测试器
$tester = Get-Process -Name "PluginTester" -ErrorAction SilentlyContinue
if ($tester) {
    Write-Host ">>> [2/6] 检测到 PluginTester 运行中，正在关闭以释放 Stock.dll 占用..." -ForegroundColor Yellow
    Stop-Process -Name "PluginTester" -Force
}

# 3. 编译 x64 和 x86 Release
$msbuild = "D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if (-not (Test-Path $msbuild)) {
    $msbuild = "MSBuild.exe"
}

Write-Host ">>> [3/6] 正在编译 Release x64..." -ForegroundColor Cyan
& $msbuild "$root\Stock++.sln" /p:Configuration=Release /p:Platform=x64 /t:Stock /m /v:q
if ($LASTEXITCODE -ne 0) { throw "x64 编译失败！" }

Write-Host ">>> [3/6] 正在编译 Release x86..." -ForegroundColor Cyan
& $msbuild "$root\Stock++.sln" /p:Configuration=Release /p:Platform=x86 /t:Stock /m /v:q
if ($LASTEXITCODE -ne 0) { throw "x86 编译失败！" }

# 4. 产物闸门 + 打包（本仓库只发布 x64 / x86 两个预编译包）。
# ARM64EC 不再提供预编译包：需要原生版的用户请照 README「编译 ARM64EC」一节自行编译。
$candidates = @(
    [pscustomobject]@{ Dll = "$root\bin\x64\Release\Stock.dll"; Zip = "$root\download\Stock_V${cleanVer}_x64.zip"; Arch = 'x64' },
    [pscustomobject]@{ Dll = "$root\bin\Release\Stock.dll";     Zip = "$root\download\Stock_V${cleanVer}_x86.zip"; Arch = 'x86' }
)

# 4a. 产物闸门：必须在 Compress-Archive 之前校验并中止。
# 否则即便中止，download 里也已经躺着一只贴着新版本号的坏包，
# 而 CI 正是拿 download\Stock_V*.zip 直接上传到 Release 的——校验就等于没做。
$expectVer = "$major.$minor.$patch.0"
foreach ($item in $candidates) {
    if (-not (Test-Path $item.Dll)) { throw "缺少 $($item.Arch) 产物：$($item.Dll)（编译未成功？）" }
    $actualVer = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($item.Dll).FileVersion
    if ($actualVer -ne $expectVer) {
        throw "产物版本不符：$($item.Dll) 报告 $actualVer，期望 $expectVer。疑似残留的旧文件被拿来冒充本次发布，已中止发包（未写出任何 zip）"
    }
}

# 4b. 打包输出至 download 目录（校验通过后才写文件）
Write-Host ">>> [4/6] 正在打包发布产物到 download 目录..." -ForegroundColor Cyan
# 先清掉所有历史 Stock_V*.zip：其中也包括任何残留的 arm64ec 包。
# 那些包历史上只是被改名冒充新版本（内容实为 2025-03-08 的 Stock v1.13 旧版），
# 留在 download 里会被 CI 的通配符一并上传到 Release，必须就地清掉。
Get-ChildItem -Path "$root\download" -Filter 'Stock_V*.zip' -ErrorAction SilentlyContinue | Remove-Item -Force
foreach ($item in $candidates) {
    Compress-Archive -Path $item.Dll -DestinationPath $item.Zip -Force
    Write-Host "    已打包 $($item.Arch): $($item.Zip)" -ForegroundColor Green
}

# 5. 更新 download/plugin_download.md
Write-Host ">>> [5/6] 同步更新 download/plugin_download.md 下载链接..." -ForegroundColor Cyan
$dlDocPath = "$root\download\plugin_download.md"
if (Test-Path $dlDocPath) {
    $dlDoc = (Get-Content $dlDocPath -Raw -Encoding UTF8) `
        -replace 'Stock_V[a-zA-Z0-9\.]+_x64\.zip', "Stock_V${cleanVer}_x64.zip" `
        -replace 'Stock_V[a-zA-Z0-9\.]+_x86\.zip', "Stock_V${cleanVer}_x86.zip"
    # 本仓库不再发布 ARM64EC 预编译包：表格里若还留着该行就整行删掉，
    # 页面上绝不出现指向不存在文件的链接。
    $dlDoc = ($dlDoc -split "`r?`n" | Where-Object { $_ -notmatch '^(?i)\s*\|\s*\*\*ARM64EC' }) -join "`r`n"
    Set-Content $dlDocPath -Value $dlDoc -Encoding UTF8
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

> **ARM64 设备（骁龙 / Surface Pro X 等）**：请直接使用上面的 **x64 版**——ARM64EC 版 TrafficMonitor
> 可以正常加载 x64 插件（走系统模拟执行，功能完全一致，仅速度非原生）。
> 本项目不再提供 ARM64EC 预编译包；确需原生版本请参照 README 的「编译 ARM64EC」一节自行编译。
"@

$releaseNotesPath = "$root\RELEASE_NOTES.md"
Set-Content -Path $releaseNotesPath -Value $releaseNotes -Encoding UTF8

Write-Host ""
Write-Host "=================================================" -ForegroundColor Green
Write-Host "🎉 Stock 插件 Release v$cleanVer 打包完成！" -ForegroundColor Green
Write-Host "=================================================" -ForegroundColor Green
Write-Host "产物文件：" -ForegroundColor Green
foreach ($item in $candidates) { Write-Host "  - $($item.Zip)" }
Write-Host "  - $releaseNotesPath"
