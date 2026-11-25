### 🚀 StockPlusPlus v2.2.0 更新日志

- 【变更】 插件改名为 StockPlusPlus：产物由 Stock.dll 改为 StockPlusPlus.dll，显示名同步改为 StockPlusPlus，可与上游原版 Stock 插件区分并独立登记到 TrafficMonitor 官方插件下载页；任务栏显示项 ID 保持不变，勾选状态不会重置
- 【新增】 自动迁移旧版配置：首次以 StockPlusPlus.dll 启动时，若同目录存在旧 Stock.ini（或 Stock.dll.ini），会自动复制为 StockPlusPlus.ini，自选、持仓、分组、预警与 WebDAV 设置全部保留；台账 stock_trades.db 文件名不变、原地继续使用
- 【提示】 升级时请删除 plugins 目录下的旧 Stock.dll：TrafficMonitor 会加载目录下所有 dll，新旧两个插件同时存在会重复显示条目并争抢同一份台账数据库；检测到旧 dll 时插件会弹窗提醒

---
### 📦 安装包说明
- **x64 推荐版**：适用于绝大多数 64 位 Windows 系统及 64 位 TrafficMonitor
- **x86 兼容版**：适用于 32 位系统环境
- **从旧版升级（重要）**：请先删除 TrafficMonitor\plugins 下的旧 Stock.dll，再放入本包的 StockPlusPlus.dll。两个 dll 同时存在会被 TrafficMonitor 分别加载，导致重复显示并争抢同一份台账数据库；旧的 Stock.ini 配置会在首次启动时自动复制为 StockPlusPlus.ini，无需手工迁移。

> **ARM64 设备（骁龙 / Surface Pro X 等）**：请直接使用上面的 **x64 版**——本项目不提供 ARM64EC 预编译包，确需原生版本请参照 README 的「编译 ARM64EC」一节自行编译。
