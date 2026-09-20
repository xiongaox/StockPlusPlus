# TrafficMonitor 股票行情插件 (Stock Plugin)

> 本插件是专为 [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 打造的现代化、高颜值暗黑极简看盘与量化分析插件。  
> 既可在任务栏常驻简明盯盘，又支持一键唤起原地沉浸式暗黑悬浮窗与专业级量化行情中心。全面覆盖 **A股 / 港股 / 美股 / ETF基金 / 贵金属黄金 / 行业板块**。

---

## 📸 功能特性全景展示

### 一、悬浮看盘与交互（首页）

任务栏左键点击任意股票即可秒级唤起暗黑半透明悬浮窗，再次点击或点击关闭平滑收起。支持多市场混合自选、实时分时日K图、十字光标与技术指标分析。

<table width="100%">
  <tr>
    <th width="50%" align="center">首页自选盯盘</th>
    <th width="50%" align="center">首页持仓收益与ETF穿透</th>
  </tr>
  <tr>
    <td align="center"><img src="images/首页.png" width="100%"/></td>
    <td align="center"><img src="images/首页-持仓股.png" width="100%"/></td>
  </tr>
  <tr>
    <td><b>实时K线与指标</b>：支持分时、竞价、日K/周K/月K，MA均线（MA5/MA20/MA60等），VOL、MACD、KDJ、RSI、W&R与BOLL布林带</td>
    <td><b>持仓监控与ETF透视</b>：实时核算总市值、浮动盈亏与当日盈亏；ETF持仓一键穿透查看底层权重成分股明细与仓位占比</td>
  </tr>
  <tr>
    <th width="50%" align="center">实时买卖五档盘口 (PK)</th>
    <th width="50%" align="center">筹码分布峰与持仓成本 (CM)</th>
  </tr>
  <tr>
    <td align="center"><img src="images/首页-买卖盘.png" width="100%"/></td>
    <td align="center"><img src="images/首页-筹码峰.png" width="100%"/></td>
  </tr>
  <tr>
    <td><b>五档挂单与盘口明细</b>：实时买卖五档挂单量与价格分布、买卖委比、振幅与换手率统计</td>
    <td><b>筹码峰分布图</b>：筹码获利比例、全市场平均持仓成本、90%成本集中区间，直观洞察支撑与阻力位</td>
  </tr>
</table>

---

### 二、深度量化行情中心

点击悬浮窗右上角行情看板图标，原地切换至全屏暗黑量化行情中心，汇聚全市场宏观流动性与微观异动。

<table width="100%">
  <tr>
    <th width="50%" align="center">板块资金流向树图 (Treemap)</th>
    <th width="50%" align="center">主力资金时间走向 (全天折线)</th>
  </tr>
  <tr>
    <td align="center"><img src="images/板块资金流-树状图.png" width="100%"/></td>
    <td align="center"><img src="images/板块资金流-时间走向.png" width="100%"/></td>
  </tr>
  <tr>
    <td><b>资金流向热力树图</b>：按行业资金净流入/流出面积与红绿颜色直观呈现，支持右侧单板块超大单/大单/中单/小单穿透</td>
    <td><b>分时资金时间走向</b>：09:30 - 15:00 全天主力资金累计净流入走势曲线，快速复盘全天资金轮动轨迹</td>
  </tr>
  <tr>
    <th width="50%" align="center">资金净申购 (ETF申赎动向)</th>
    <th width="50%" align="center">全市场涨跌趋势分布 (大盘体检)</th>
  </tr>
  <tr>
    <td align="center"><img src="images/资金净申购.png" width="100%"/></td>
    <td align="center"><img src="images/涨跌趋势.png" width="100%"/></td>
  </tr>
  <tr>
    <td><b>ETF申赎资金榜</b>：ETF主力净流入/流出 Top10，支持侧边抽屉一键查阅板块细分ETF列表</td>
    <td><b>全市场涨跌分布直方图</b>：上涨/平盘/下跌/涨停/跌停全览，放量缩量诊断与多空阶梯分布统计</td>
  </tr>
  <tr>
    <th colspan="2" align="center">ETF 综合涨跌榜</th>
  </tr>
  <tr>
    <td colspan="2" align="center"><img src="images/ETF涨跌榜.png" width="60%"/></td>
  </tr>
  <tr>
    <td colspan="2" align="center"><b>ETF行情看板</b>：全市场ETF涨跌幅榜、成交额与主力净流入智能排序</td>
  </tr>
</table>

---

### 三、内嵌设置中心与数据管理

悬浮窗右上角齿轮图标直达内嵌设置中心，告别传统白边独立弹窗，原地沉浸式管理。

<table width="100%">
  <tr>
    <th width="50%" align="center">基础外观与常规设置</th>
    <th width="50%" align="center">分组管理与智能股票联想搜索</th>
  </tr>
  <tr>
    <td align="center"><img src="images/基础设置.png" width="100%"/></td>
    <td align="center"><img src="images/分组管理.png" width="100%"/></td>
  </tr>
  <tr>
    <td><b>外观与常规配置</b>：全天更新、任务栏显示项切换、窗口尺寸与5档停靠位置、背景透明度调节（100%~60%实时无缝调节）、SOCKS5代理配置</td>
    <td><b>多分组与拼音联想</b>：自选股、持仓股及多自定义分组；支持输入拼音/代码跨市场快速联想添加A/港/美股，支持拖拽排序</td>
  </tr>
  <tr>
    <th width="50%" align="center">WebDAV 多端云备份与恢复</th>
    <th width="50%" align="center">接口健康与心跳监测</th>
  </tr>
  <tr>
    <td align="center"><img src="images/云端备份.png" width="100%"/></td>
    <td align="center"><img src="images/接口检测.png" width="100%"/></td>
  </tr>
  <tr>
    <td><b>云端数据同步</b>：基于标准 WebDAV 协议（支持坚果云、自建 NAS 等），实现多设备自选、持仓配置的一键备份与自动同步</td>
    <td><b>全链路接口健康监测</b>：实时检测腾讯、新浪、东方财富、通达信直连等行情源连通状态、网络延迟与历史心跳色块</td>
  </tr>
</table>

---

## 🚀 快速开始

### 1. 插件安装

1. 前往 Release 或下载页面获取最新版本的 `Stock.dll`（64位系统请使用 x64 版本）。
2. 将 `Stock.dll` 复制到 TrafficMonitor 安装目录下的 `plugins` 文件夹中：
   ```text
   TrafficMonitor/
   ├── TrafficMonitor.exe
   └── plugins/
       └── Stock.dll
   ```
3. 重启 TrafficMonitor。

### 2. 启用任务栏显示

1. 在 TrafficMonitor 任务栏窗口上点击鼠标右键，选择 **“显示设置”**；
2. 在显示项目中勾选 **“股票”**，点击“确定”即可在任务栏看到实时股票行情。

### 3. 日常快捷操作

- **左键单击任务栏股票项**：快速展开/收起悬浮看盘窗口。
- **右键单击任务栏股票项**：点击 **“刷新股票信息”** 立即强制发起最新行情拉取。
- **悬浮窗右上角按钮**：
  - ⚙️ **齿轮图标**：就地切换至内嵌设置中心（基础设置、分组管理、均线指标、云端备份等）。
  - 📊 **图表图标**：切换至深度行情中心（板块资金流、资金净申购、涨跌分布、ETF榜）。
  - 📌 **展开/收起按钮**：快速切换单股极简视图与左侧股票列表栏。
  - ✕ **关闭按钮**：关闭悬浮窗。

---

## 🛠️ 编译与开发

### 环境要求

- Windows 10 / 11
- Visual Studio 2022 (带 C++ MFC 桌面开发组件)
- Windows SDK 10.0+

### 本地编译

1. 使用 Visual Studio 2022 打开根目录下的 `Stock++.sln`。
2. 将构建配置切换为 **Release | x64**。
3. 编译 `utilities` 基础库工程，然后编译 `Stock` 工程。
4. 编译输出产物位于 `bin/x64/Release/Stock.dll`。

### 编译 ARM64EC（ARM 设备原生版）

本项目**只发布 x64 / x86 两个预编译包**，不提供 ARM64EC 预编译包（原因见下方说明）。如果你在骁龙 / Surface Pro X 等 ARM 设备上想要原生速度，可以自行编译，配置现成可用。

> **先确认是否真的需要**：ARM64EC 版 TrafficMonitor **可以直接加载 x64 版插件**（Windows 会跑在 x64 模拟层上），功能完全一致，只是速度非原生。多数人直接用 Release 里的 x64 包即可。

#### 1. 安装 ARM64EC 工具链

ARM64EC 需要额外安装两样东西，**普通 VS 安装默认不含**（`platform` 选 ARM64EC 时会报 `MSB8020 无法找到 v143 的生成工具`）：

```powershell
# 需管理员权限；--installPath 换成你本机 VS 的安装路径
vs_installer.exe modify --installPath "<VS安装路径>" --add Microsoft.VisualStudio.Component.VC.Tools.ARM64EC
```

- `<VS安装路径>` 一般形如 `C:\Program Files\Microsoft Visual Studio\2022\Community` 或 `D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`。
- 也可用图形界面：打开 **Visual Studio Installer** → 修改 → **单个组件** → 勾选 **MSVC v143 - VS 2022 C++ ARM64EC 生成工具**（含 ARM64 生成工具）。
- 装完可以这样自检（两项都应在）：`<MSVC版本目录>\lib\arm64ec` 存在，且 Windows SDK 的 `Lib\<版本>\ucrt\arm64` 存在。

#### 2. 编译

**方式一：Visual Studio 界面**

1. 打开 `Stock++.sln`；
2. 顶部配置下拉选择 **Release | ARM64EC**；
3. 依次编译 `utilities`、`Stock` 两个工程；
4. 产物位于 `bin/ARM64EC/Release/Stock.dll`。

**方式二：命令行**

```powershell
# MSBuild 路径按本机实际位置调整
& "<MSBuild路径>\MSBuild.exe" "Stock++.sln" /p:Configuration=Release /p:Platform=ARM64EC /t:Stock /m
```

产物同样在 `bin\ARM64EC\Release\Stock.dll`，把它复制到 `TrafficMonitor\plugins\` 即可。

#### 3. 验证产物是不是真的 ARM64EC

这一步**必须做**，因为 ARM64EC 的最终 DLL 在 PE 头里**故意报告 `8664 machine (x64)`**，光看"是 x64 还是 ARM64"根本区分不出来。正确的判据是它含有 ARM64EC 专属的节区 `.a64xrm`（ARM64X 重定向表）与 `.hexpthk`（混合导出跳板）：

```powershell
# 用 VS 开发者命令行的 dumpbin
dumpbin /headers bin\ARM64EC\Release\Stock.dll | Select-String "machine|a64xrm|hexpthk"
```

- ✅ 正确产物：机器码显示 `8664 machine (x64) (ARM64X)`，且能看到 `.a64xrm` / `.hexpthk` 节区。
- ❌ 若显示 `8664 machine (x64)` **没有** `(ARM64X)` 后缀、也没有那两个节区，说明编出来的其实是普通 x64——此时工具链没装对，请回到第 1 步。

> **为什么不提供 ARM64EC 预编译包**：本仓库历史上确实挂过 `Stock_V*_arm64ec.zip`，但发包脚本只是给上一版**改文件名**、从未重新编译，导致那份包内容一直停留在 2025-03-08 的 Stock v1.13，且与上游 `zhongyang219/TrafficMonitorPlugins` 的 `Stock_V1.13_arm64ec.zip` 字节完全一致——下载它的用户拿到的是缺全部新功能的旧版。现已下架该包，改为文档指导自行编译，避免再次出现"包名与内容不符"。

---

## ❓ 常见问题 (Q&A)

### Q1：插件加载不出数据（一直空白、行情/行情中心转圈或显示"获取失败"）怎么办？

东方财富的行情接口分布在 `push2 / push2his / push2ex.eastmoney.com` 这几个 CDN 域名上。部分地区运营商本地 DNS 会把它们解析到已失效的 CDN 节点，表现为**插件本身正常、但数据一直拉不下来**（浏览器/其它软件也可能同时访问东财变慢）。

项目在 `tools/eastmoney-cdn/` 提供了现成脚本，把域名固定解析到一个测速可用的 CDN 节点：

1. 双击运行 **`tools/eastmoney-cdn/fix_eastmoney_cdn.bat`**（会弹 UAC 请求管理员权限，请点"是"）。脚本首次运行会把原 `hosts` 备份为 `hosts.bak_eastmoney`，随后写入东财三个域名的固定解析并刷新 DNS 缓存。
2. **重启 TrafficMonitor**（或重新加载插件），再看数据是否恢复。
3. 想回滚：运行 **`tools/eastmoney-cdn/undo_eastmoney_cdn.bat`**，用备份还原 `hosts` 并刷新 DNS（注意：是整份还原备份，备份之后对 `hosts` 的其他改动会被一并还原）。

如果过一段时间节点又失效（CDN 节点会变动），可以装一次"看门狗"让它自动巡检并切换到可用节点：

- 安装：双击 **`tools/eastmoney-cdn/install_cdn_watchdog.bat`**（把脚本复制到 `C:\ProgramData\eastmoney_cdn_watchdog\`，并创建计划任务 `EastmoneyCDNWatchdog`：每小时以 SYSTEM 身份检测当前节点，失效则从候选节点中自动挑一个可用的重写 `hosts`，安装时会立即跑一次验证）。
- 卸载：双击 **`tools/eastmoney-cdn/uninstall_cdn_watchdog.bat`**（删除该计划任务）。
- 运行日志：`C:\ProgramData\eastmoney_cdn_watchdog\watchdog.log`。

> 说明：以上脚本只改本机 `hosts` 的固定解析（首次运行自动备份），除"看门狗"外不安装任何常驻程序；不确定是否需要时可先只跑 `fix_eastmoney_cdn.bat`，恢复后随时 `undo` 回滚。

---

## 📄 开源许可证

本项目遵循 [MIT License](LICENSE) 协议。
