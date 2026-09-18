#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <tuple>
#include <mutex>
#include "StockDef.h"

struct sqlite3;

// 均幅统计数据
struct AvgDiffStats
{
	double minVal;
	double maxVal;
	double currentVal;
};

// 交易台账一笔（trades 表的一行，人工录入的建仓/加仓/减仓）
struct StockTradeRecord
{
	long long id{ 0 };
	bool isSell{ false };       // true=卖出，false=买入
	std::wstring time;          // 成交时间 yyyy-MM-dd HH:mm
	double price{ 0.0 };        // 成交价
	double amount{ 0.0 };       // 数量（股）
	double fee{ 0.0 };          // 手续费（默认 0；口径上不影响当日盈亏，仅台账留存）
};

// 台账整表备份用一条（含股票归属，供 WebDAV 云端备份携带）
// id 不参与备份：恢复时按插入顺序重建，避免与本地既有主键冲突
struct StockTradeBackupRecord
{
	std::wstring stockCode;
	std::wstring stockName;
	int tradeType{ 0 };         // 1=卖出，0=买入（trades.trade_type 口径）
	std::wstring time;          // 成交时间 yyyy-MM-dd HH:mm
	double price{ 0.0 };
	double amount{ 0.0 };
	double fee{ 0.0 };
};

// 股票数据库管理类
// 负责所有 SQLite 数据库的 CRUD 操作，与业务逻辑、UI 解耦。
// CDataManager 持有其一个实例，并将原数据库方法转发到此。
class CStockDbManager
{
public:
	CStockDbManager();
	~CStockDbManager();

	CStockDbManager(const CStockDbManager&) = delete;
	CStockDbManager& operator=(const CStockDbManager&) = delete;

	// 数据库生命周期管理
	// 根据配置文件路径初始化数据库连接并创建所有表
	bool Init(const std::wstring& config_path);
	void Close();
	bool IsOpen() const { return m_db != nullptr; }
	// 清空并重置所有数据库数据，重新生成空表
	bool ResetAllData();
	// 清理超过7天的快照、K线和筹码峰缓存
	void CleanExpiredData();

	// 交易记录
	bool SaveTradeRecord(const std::wstring& stockCode, const std::wstring& stockName,
		int tradeType, const std::wstring& time,
		double price, double amount, double totalAmount,
		double fee, double total);
	// 交易台账（人工录入的建仓/加仓/减仓，用于当日盈亏修正与日K B/S 标记）
	// 插入一笔并返回新记录 id（0 表示失败）；total 按 SaveTradeRecord 的符号约定重算
	long long InsertTradeRecord(const std::wstring& stockCode, const std::wstring& stockName,
		int tradeType, const std::wstring& time, double price, double amount, double fee);
	// 按股票查询全部记录，时间升序（同一时刻按 id 升序）
	std::vector<StockTradeRecord> LoadTradeRecords(const std::wstring& stockCode);
	// 按 id 更新一笔（金额与合计按 tradeType/价格/数量/费用重算）
	bool UpdateTradeRecord(long long id, int tradeType, const std::wstring& time,
		double price, double amount, double fee);
	// 按 id 删除一笔
	bool DeleteTradeRecord(long long id);

	// 台账整表导出（WebDAV 云端备份携带用）：读取全部股票的成交记录，
	// 按 stock_code、trade_time、id 升序返回；表为空时返回空 vector 且返回 true
	bool ExportAllTradeRecords(std::vector<StockTradeBackupRecord>& recordsOut);
	// 台账整表恢复：单事务内清空 trades 表后按传入顺序重建（id 重新分配，total 按约定重算）。
	// 仅在备份文件确实带有台账段时调用；records 为空视为「恢复为空台账」
	bool ReplaceAllTradeRecords(const std::vector<StockTradeBackupRecord>& records);

	// 交易明细（一档行情逐笔成交）
	// 批量插入，写前会先按 (code, trade_date) 删除同日旧数据，保证幂等覆盖
	bool SaveTransactions(const std::wstring& stockCode, const std::string& tradeDate,
		const std::vector<STOCK::Transaction>& data);
	// 按 (code, trade_date) 删除
	bool DeleteTransactions(const std::wstring& stockCode, const std::string& tradeDate);
	// 按 code + trade_date 批量查询（按插入顺序，即时间先后返回）
	std::vector<STOCK::Transaction> LoadTransactions(const std::wstring& stockCode,
		const std::string& tradeDate);

	// 内外盘快照
	bool SaveInnerOuterSnapshot(const std::wstring& stockCode, time_t timestamp,
		STOCK::Volume innerVolume, STOCK::Volume outerVolume);
	// 加载指定股票从 startTime 之后的全部快照（按时间升序）
	std::vector<std::tuple<time_t, STOCK::Volume, STOCK::Volume>> LoadInnerOuterSnapshots(
		const std::wstring& stockCode, time_t startTime);

	// 分时缓存
	bool SaveTimelineCache(const std::wstring& stockCode,
		const std::vector<STOCK::TimelinePoint>& data);
	// 加载指定交易日的分时数据
	std::vector<STOCK::TimelinePoint> LoadTimelineCache(const std::wstring& stockCode,
		const std::string& tradeDate);
	// 加载最近一个交易日的分时数据（今天无缓存时回退）
	std::vector<STOCK::TimelinePoint> LoadLatestTimelineCache(const std::wstring& stockCode);

	// K线缓存
	bool SaveKLineCache(const std::wstring& stockCode, STOCK::Period period,
		const std::vector<STOCK::KLinePoint>& data);
	bool HasKLineCache(const std::wstring& stockCode, STOCK::Period period);
	std::vector<STOCK::KLinePoint> LoadKLineCache(const std::wstring& stockCode,
		STOCK::Period period);
	// 自愈：删除日K缓存中存在异常跳变（不复权口径断崖）的股票数据，返回清理的股票数
	// 用于修复历史版本混入的不复权缓存；被清理的股票等下次网络获取成功后重建
	int HealAbnormalDayKLineCache();

	// 股票基础数据
	bool SaveStockBasicData(const std::wstring& stockCode, STOCK::Volume circulatingAShares);
	bool LoadStockBasicData(const std::wstring& stockCode, STOCK::Volume& outCirculatingAShares);

	// 筹码分布
	bool SaveChipDistribution(const std::wstring& stockCode,
		const STOCK::ChipDistribution& chipData);
	bool LoadLatestChipDistribution(const std::wstring& stockCode,
		STOCK::ChipDistribution& chipData);

	// 关联股票均幅统计
	bool SaveAvgDiffStats(const std::wstring& stockCode, double minVal, double maxVal, double currentVal);
	AvgDiffStats LoadAvgDiffStats(const std::wstring& stockCode);

	// 基金净值按分钟缓存
	bool SaveFundNavCache(const std::wstring& stockCode,
		const std::vector<STOCK::TimelinePoint>& data);
	std::vector<STOCK::TimelinePoint> LoadFundNavCache(const std::wstring& stockCode,
		const std::string& tradeDate);
	std::vector<STOCK::TimelinePoint> LoadLatestFundNavCache(const std::wstring& stockCode);

	// 行情中心跨进程快照（一数据集一行 UTF-8 JSON，独立原子覆盖）
	bool SaveMarketCenterCache(int dataSet, const std::string& payload, time_t fetchedAt, const std::string& tradeDate, int schemaVersion = 1);
	bool LoadMarketCenterCache(int dataSet, std::string& payload, time_t& fetchedAt, std::string& tradeDate, int& schemaVersion);
	bool DeleteMarketCenterCache(int dataSet);

private:
	sqlite3* m_db{ nullptr };
	std::wstring m_db_path;
	std::wstring m_config_path;
	// SQLite 以 SQLITE_THREADSAFE=0 编译（内部完全无锁），而该连接同时被 UI 线程（图表/缓存读取）
	// 与后台抓取线程（预加载、写入）访问：用递归锁把所有成员方法串行化，
	// 杜绝并发 prepare/step 破坏 sqlite 内部结构（表现为随机在解析器处读空指针崩溃）。
	mutable std::recursive_mutex m_db_mutex;
};
