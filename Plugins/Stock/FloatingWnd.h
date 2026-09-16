#pragma once

#include <afxwin.h>
#include <string>
#include <vector>
#include <atomic>
#include <StockDef.h>
#include <TransparentWnd.h>
#include "SignalAnalyzer.h"
#include "StockIndicator.h"
#include "ChartContext.h"
#include "StockListPanel.h"
#include "CallAuctionChart.h"
#include "ChipPeakPanel.h"
#include "EtfHoldingsPanel.h"
#include "BsTradePanel.h"
#include "OrderBookPanel.h"
#include "OverviewPanel.h"
#include "IndicatorChart.h"
#include "StatusBarPanel.h"
#include "KLineChart.h"
#include "TimelineChart.h"
#include "MarketCenterPanel.h"

class CManagerDialog;   // 内嵌设置子对话框（FloatingWnd.cpp 中包含完整定义）

// 定义自定义消息
#define FWND_MSG_UPDATE_STATUS (WM_USER + 100)
#define FWND_MSG_SHOW_EDIT_DLG (WM_USER + 102)
#define FWND_MSG_SHOW_ADD_DLG (WM_USER + 103)
#define FWND_MSG_SHOW_TRADE_DLG (WM_USER + 104)
#define FWND_MSG_SETTINGS_CLOSED (WM_USER + 105)  // 内嵌设置视图确定/取消/ESC 后收起设置视图

// 定义时间线可见点数常量
#define TIME_LINE_VISIBLE_COUNT_1MIN 30
#define TIME_LINE_VISIBLE_COUNT_5MIN 24
#define TIME_LINE_VISIBLE_COUNT_30MIN 16
#define TIME_LINE_VISIBLE_COUNT_1DAY 20
#define TIME_LINE_VISIBLE_COUNT_STEP 10

class CFloatingWnd : public CWnd
{
public:
	CFloatingWnd();
	virtual ~CFloatingWnd();

	BOOL Create(CFont* font, CPoint pt, std::wstring stock_id);
	const std::wstring& GetStockId() const { return m_stock_id; }
	void SetStockId(const std::wstring& stockId);
	void ToggleKLineMode(); // 切换分时/日K模式
	// 行情中心内嵌视图：右键在悬浮窗内原地切换；进入时临时放大窗口，退出还原
	void ToggleMarketCenter();   // 右键切换行情中心视图模式（悬浮窗内原地切换，不建子窗口/不改尺寸）
	void HideChartButtons(bool hide);   // 行情中心/设置视图下隐藏/恢复图表视图专属按钮
	// 内嵌“设置”视图：与行情中心一致的原地切换体验（入口=顶栏设置图标）
	void ShowSettingsView();     // 打开设置视图（悬浮窗已开启时供外部入口直达）
	void ToggleSettingsView();   // 在悬浮窗内原地切换设置视图/图表视图

	// 悬浮窗“当前状态”快照：点击桌面等隐藏时会整体销毁重建，销毁前由 Stock 暂存本快照，
	// 重新打开后调用 RestoreUiState 还原视图，避免每次回到首页K线/默认状态
	struct UiState
	{
		bool marketCenterMode{ false };   // 正处于行情中心视图
		bool settingsMode{ false };       // 正处于内嵌设置视图
		int mcPage{ 0 };                  // 行情中心侧栏页（McPage）
		int mcSectorViewMode{ 0 };        // 板块视图：0=资金树图 1=时间走向
		int mcTreemapMode{ 0 };           // 树图模式：0=红绿 1=仅流入 2=仅流出
		// K线首页（图表）状态
		int viewMode{ 3 };                // UIViewMode（0=总览 1=竞价 2=分时 3=日K 4=周K 5=月K）
		bool showChipPeak{ false };       // 右侧面板四选（互斥）：筹码峰
		bool showOrderBook{ false };      // 右侧面板四选（互斥）：盘口
		bool showEtfHoldings{ false };    // 右侧面板四选（互斥）：ETF持仓（CC）
		bool showBsTrades{ false };       // 右侧面板四选（互斥）：交易台账（BS）
		bool showMA{ false };             // 均线开关
		bool showBollBands{ true };       // 布林带开关
		int timelineIndicator{ 0 };       // 分时副图指标（TimelineIndicator）
		bool expandedMode{ false };       // 放大模式（隐藏副图）
		bool showStockList{ true };       // 左侧股票列表显隐
		int activeGroupTab{ 0 };          // 左侧列表分组（0=自选 1=持仓 >=2 自定义）
		int groupListSort{ 0 };           // 列表排序：0=默认 1=涨跌幅降序 2=涨跌幅升序
		bool showPositionSummaryPercent{ false };  // 持仓汇总栏盈亏百分比模式
		bool showJZCurve{ false };        // 基金净值曲线
	};
	UiState CaptureUiState() const;
	void RestoreUiState(const UiState& st);
	// 按快照恢复首页图表视图状态（RestoreUiState 内部步骤）
	void ApplyChartViewState(const UiState& st);

	// 鼠标移出图表区超过2秒时自动清除悬停信息卡，避免长期遮挡图表
	void CheckHoverCardAutoHide();
	// 右侧信息面板（盘口/筹码峰）当前是否可见：隐藏后宽度全部让给图表
	bool IsInfoPanelVisible(bool isIndexKLine) const;
	// 重置所有数据联动（切换至自选股、更新当前关注股票、清空图表缓存并重绘）
	void OnDataReset();
	// 动态更新背景透明度
	void UpdateOpacity(int opacityPercent);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	LRESULT OnUpdateStatus(WPARAM wParam, LPARAM lParam);
	LRESULT OnMarketCenterDataUpdated(WPARAM wParam, LPARAM lParam);   // 行情中心数据到达，重绘
	LRESULT OnMcEtfClicked(WPARAM wParam, LPARAM lParam);              // 行情中心点击 ETF，跳转首页 K 线临时查看
	LRESULT OnCloseWindow(WPARAM wParam, LPARAM lParam);
	LRESULT OnShowEditDialog(WPARAM wParam, LPARAM lParam);
	LRESULT OnShowAddDialog(WPARAM wParam, LPARAM lParam);
	LRESULT OnShowTradeDialog(WPARAM wParam, LPARAM lParam);
	LRESULT OnSettingsClosed(WPARAM wParam, LPARAM lParam);   // 内嵌设置视图关闭，收起设置视图
	afx_msg void OnBnClickedTimeLineBtn();
	afx_msg void OnBnClickedKLineBtn();
	afx_msg void OnBnClickedWeekKLineBtn();
	afx_msg void OnBnClickedMonthKLineBtn();
	afx_msg void OnBnClickedRegionStatsBtn();
	afx_msg void OnBnClickedCloseBtn();
	afx_msg void OnBnClickedMABtn();
	afx_msg void OnBnClickedBollBtn();
	afx_msg void OnBnClickedIndicatorMACDBtn();
	afx_msg void OnBnClickedIndicatorMACDSignalBtn();
	afx_msg void OnBnClickedIndicatorKDJBtn();
	afx_msg void OnBnClickedIndicatorWRBtn();
	afx_msg void OnBnClickedIndicatorRSIBtn();
	afx_msg void OnBnClickedChipPeakBtn();
	afx_msg void OnBnClickedOrderBookBtn();
	afx_msg void OnBnClickedEtfHoldingsBtn();
	afx_msg void OnBnClickedBsTradesBtn();
	afx_msg void OnBnClickedExpandBtn();
	afx_msg void OnBnClickedToggleStockListBtn();
	afx_msg void OnBnClickedSettingsBtn();
	afx_msg void OnBnClickedMcRefreshBtn();
	afx_msg void OnBnClickedCallAuctionBtn();
	afx_msg void OnBnClickedKLineSourceBtn();

private:
	void EnsureChipPeakData();
	void EnsureEtfHoldingsData();
	void EnsureKLineData(STOCK::Period period);
	void ResetHoverState();           // 重置所有悬停状态
	void SetTimelineModeDefaults();   // 设置分时模式默认参数
	void SetDayKLineModeDefaults();   // 设置日K模式默认参数
	void SetWeekKLineModeDefaults();  // 设置周K模式默认参数
	void SetMonthKLineModeDefaults(); // 设置月K模式默认参数
	static void SafeSetWindowPos(CWnd& wnd, int x, int y, int cx, int cy);
	static void SafeShowWindow(CWnd& wnd, bool show);
	// 右侧信息按钮簇（CM/PK/CC/BS）统一布局：CC 仅基金显示，BS 自动补到左侧不留空档
	void LayoutInfoButtons(int w, int obBtnTop, int obBtnW, int obBtnH, bool showObBtns);

	// TimelineDrawContext / KLineDrawData / LabelInfo 已移至 ChartContext.h，供各图表模块共享
	// MACDData/MACDCrossSignal/KDJData/WRData/RSIData/PeriodPoint 类型别名已移至各模块类
	// 走势图绘制已移至CTimelineChart
	// MACD/KDJ/WR/RSI/成交量绘制已移至CIndicatorChart
	// DrawHeader/DrawTimelinePositionInfo/DrawKLinePositionInfo/DrawKLineInfoPanel 已移至CStatusBarPanel
	// K线图绘制已移至CKLineChart
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	void UpdateModeButtons();
	void UpdateRegionStatsFromPixels();      // 根据选区像素范围换算bar区间并重算统计
	void ClearRegionSelection(bool exitMode); // 退出选区（exitMode=true 时连「区域」开关一起关）
	void UpdateIndicatorButtons();
	void UpdatePeriodComboVisibility();
	void ApplySignalColors(COLORREF bollColor, COLORREF macdColor, COLORREF kdjColor, COLORREF wrColor, COLORREF rsiColor, COLORREF maColor);
	void EnsureStockListVisible();
	// 左侧列表分组：切换 / “更多分组”下拉 / 顶栏标签悬停
	void SwitchFloatingGroup(int groupTab);
	void ShowGroupDropdownMenu(const CRect& dropdownRect);
	void UpdateGroupTabHover(const CPoint& point);

	CTransparentWnd m_CTransparentWnd;
	CMarketCenterPanel m_marketCenterPanel;            // 行情中心面板（视图模式，悬浮窗 OnPaint 里绘制）
	bool m_marketCenterMode{ false };                  // 是否处于行情中心视图
	bool m_settingsMode{ false };                      // 是否处于内嵌设置视图
	CManagerDialog* m_pSettingsDlg{ nullptr };         // 设置子对话框（WS_CHILD 内嵌铺满悬浮窗）
	void DestroySettingsDialog();                      // 安全销毁设置子对话框
	CStockListPanel m_stockListPanel;
	CCallAuctionChart m_callAuctionChart;
	CChipPeakPanel m_chipPeakPanel;
	CEtfHoldingsPanel m_etfHoldingsPanel;
	CBsTradePanel m_bsTradePanel;
	COrderBookPanel m_orderBookPanel;
	COverviewPanel m_overviewPanel;
	CIndicatorChart m_indicatorChart;
	CStatusBarPanel m_statusBarPanel;
	CKLineChart m_kLineChart;
	CTimelineChart m_timelineChart;
	CButton m_btnTimeLine;
	CButton m_btnKLine;
	CButton m_btnWeekKLine;
	CButton m_btnMonthKLine;
	CButton m_btnRegionStats;   // 区域统计开关（竞价胶囊左侧，仅K线族视图显示）
	CButton m_btnKLineSource;   // K线数据源状态与刷新按钮
	CButton m_btnMA;
	CButton m_btnBoll;
	CButton m_btnClose;
	CButton m_btnExpand;      // 放大按钮（隐藏副图，走势图占3/4）
	CButton m_btnToggleStockList;  // 股票列表显示/隐藏按钮
	CButton m_btnSettings;        // 设置按钮（收起分组左侧，点击原地切入内嵌设置视图）
	CButton m_btnMcRefresh;       // 行情中心手动刷新按钮（设置左侧，仅行情中心视图可见；无视新鲜度强制重拉当前页数据）
	CButton m_btnCallAuction;     // 集合竞价按钮
	CButton m_btnIndicatorCJL;  // CJL指标按钮
	CButton m_btnIndicatorMACD;  // MACD信号按钮
	CButton m_btnIndicatorKDJ;   // KDJ指标按钮
	CButton m_btnIndicatorWR;    // W&R指标按钮
	CButton m_btnIndicatorRSI;   // RSI指标按钮
	CButton m_btnChipPeak;       // 筹码峰按钮
	CButton m_btnOrderBook;      // 盘口按钮（与筹码峰按钮切换）
	CButton m_btnEtfHoldings;    // ETF持仓按钮（CC）
	CButton m_btnBsTrades;       // 交易台账按钮（BS）
	CFont m_chipPeakFont;        // 筹码峰按钮小字体
	std::wstring m_stock_id;
	std::wstring m_mc_return_stock_id;  // 非空 = 正在临时查看行情中心 ETF 的 K 线，值为跳转前股票 id
	UIViewMode m_viewMode{ UI_VIEW_DAY_KLINE };  // 当前界面视图模式
	bool m_klineDataLoaded{ false };
	int m_klinePeriodDays{ 250 };
	int m_scrollOffset{ 0 };
	int m_timelineScrollOffset{ -1 };  // 分时图水平滚动偏移，-1表示需要自动滚动到末尾
	int m_timelineVisibleCount{ 30 };  // 分时图可见数据点数
	int m_timelineLastTotalPoints{ 0 };  // 上次绘制的数据点数，用于判断新数据追加时是否自动跟随
	int m_vScrollOffset{ 0 };

	// 分时图指标类型
	enum class TimelineIndicator { CJL, MACD, KDJ, WR, RSI };
	TimelineIndicator m_timelineIndicator{ TimelineIndicator::CJL };
	bool m_indicatorBtnsInitialized{ false };

	// 分时图鼠标拖动滚动
	bool m_isTimelineDragging{ false };
	CPoint m_timelineDragStartPos;
	int m_timelineDragStartOffset{ 0 };
	// K线图鼠标拖动滚动
	bool m_isKLineDragging{ false };
	CPoint m_klineDragStartPos;
	int m_klineDragStartOffset{ 0 };
	// 区域统计模式（同花顺式K线区间选区，仅K线族视图；分时/竞价无此功能）
	bool m_regionStatsMode{ false };      // 周期行「区域」开关
	bool m_isRegionDragging{ false };     // 正在拖动选区
	bool m_regionAdjustingLeft{ false };  // 按住选区左边缘微调（false=新选区或右边缘）
	CRect m_regionLeftHandleRect;         // 左边界拖动手柄热区（客户区坐标，绘制时刷新）
	CRect m_regionRightHandleRect;        // 右边界拖动手柄热区
	int m_regionSelStartX{ 0 };           // 选区起点X（客户区坐标）
	int m_regionSelEndX{ 0 };             // 选区当前X
	bool m_regionHasSelection{ false };   // 当前有选区（拖动中或已定格）
	int m_regionStartBar{ -1 };           // 选区起始bar（全局下标，含）
	int m_regionEndBar{ -1 };             // 选区结束bar（全局下标，含）
	std::wstring m_regionDateTitle;       // "2026-08-21 | 20日 | 2026-09-16"
	std::wstring m_regionPctText;         // 区间涨幅（带符号）
	std::wstring m_regionHighText;        // 区间最高
	std::wstring m_regionLowText;         // 区间最低
	std::wstring m_regionAmpText;         // 区间振幅（%）
	double m_regionPctValue{ 0 };         // 区间涨幅数值（透明度/配色用）
	CRect m_regionClearRect;              // 选区✕热区（客户区坐标，绘制时刷新）
	HCURSOR m_hPrevCursor{ NULL };
	volatile BOOL m_isDestroying;
	CFont* m_pfont{};
	CString loading_state_txt;

	// 悬停信息卡自动清除：鼠标持续离开图表区的时间起点（0=鼠标在图表区内或无信息卡）
	ULONG64 m_hoverCardOutsideSince{ 0 };

	// 鼠标悬停数据
	CPoint m_mousePos;
	bool m_isHoveringVolume{ false };
	int m_hoveredBarIndex{ -1 };
	STOCK::TimelinePoint m_hoveredData;
	// 悬停点的MA值及前一点MA值（用于箭头方向）
	STOCK::Price m_hoverMa1{ 0 }, m_hoverPrevMa1{ 0 };
	std::vector<STOCK::Price> m_hoverMaValues; // 悬停点各周期均线值，顺序同 SettingData::m_ma_days
	CString m_hoverTip;
	// 分时图标题栏悬停提示
	CString m_timelinePriceTitleTip;   // 走势图标题栏：现价/均价/MA5/MA17/MA60...
	CString m_timelineVolumeTitleTip;  // 量柱图标题栏：成交量/成交额
	CString m_timelineMacdTitleTip;    // MACD标题栏：DIF/DEA/MACD
	CString m_timelineKdjTitleTip;     // KDJ标题栏：K/D/J
	CString m_timelineWrTitleTip;      // WR标题栏：WR1/WR2
	CString m_timelineRsiTitleTip;     // RSI标题栏：RSI1/RSI2
	CString m_chipPeakTip;             // 筹码峰提示

	// 双击检测
	DWORD m_lastClickTime{};
	CPoint m_lastClickPos;
	std::wstring m_pendingEditStockCode;
	CString m_pendingTradeTime;
	double m_pendingTradePrice{ 0.0 };

	// 日K线鼠标悬停数据
	bool m_isHoveringKLine{ false };
	bool m_isHoveringKLineVolume{ false };
	bool m_isHoveringKDJ{ false };
	bool m_showTrendView{ false };
	bool m_showChipPeak{ false };
	bool m_showOrderBook{ false };  // 是否显示右侧买卖盘口面板（默认不选中，给图表更多空间）
	bool m_showEtfHoldings{ false }; // 是否显示右侧 ETF 持仓面板
	int m_etfHoldingsScrollOffset{ 0 }; // ETF持仓列表垂直滚动偏移
	bool m_isEtfHoldingsDragging{ false }; // ETF持仓列表是否正在拖动
	bool m_isEtfHoldingsDragMoved{ false }; // ETF持仓列表拖动是否产生了位移
	CPoint m_etfHoldingsDragStartPos;       // ETF持仓列表拖动起点
	int m_etfHoldingsDragStartOffset{ 0 };  // ETF持仓列表拖动起始偏移
	bool m_showBsTrades{ false };           // 是否显示右侧 BS 交易台账面板
	int m_bsScrollOffset{ 0 };              // BS台账列表垂直滚动偏移
	int m_bsSelectedRow{ -1 };              // BS台账选中行（点击行高亮，-1 无）
	bool m_isBsDragging{ false };           // BS台账列表是否正在拖动
	bool m_isBsDragMoved{ false };          // BS台账拖动是否产生了位移
	CPoint m_bsDragStartPos;                // BS台账拖动起点
	int m_bsDragStartOffset{ 0 };           // BS台账拖动起始偏移
	// BS 行双击检测（窗口无 CS_DBLCLKS，双击表现为同位置两次 WM_LBUTTONDOWN）
	DWORD m_bsLastClickTick{ 0 };           // 上次 BS 行点击时刻
	CPoint m_bsLastClickPos;                // 上次 BS 行点击位置
	int m_bsLastClickRow{ -1 };             // 上次 BS 行点击行号（-1 空白区）
	bool m_expandedMode{ false };  // 放大模式：隐藏副图，走势图3/4+成交量1/4
	bool m_showStockList{ true };  // 是否显示左侧股票列表面板
	bool m_showPositionSummaryPercent{ false };  // 持仓汇总栏是否显示盈亏百分比
	int m_activeGroupTab{ 0 };     // 左侧列表当前分组：0=自选股, 1=持仓, >=2 为自定义分组
	std::vector<FloatingGroupTab> m_groupTabs;  // 顶部分组标签布局（绘制时计算，供点击命中）
	int m_hoverGroupTab{ -1 };     // 悬停的分组标签下标（m_groupTabs 下标，-1 无）
	int m_groupListSort{ 0 };      // 左侧列表排序：0=默认顺序, 1=涨跌幅降序(涨最多在上), 2=涨跌幅升序(跌最多在上)
	int m_hoverSortArrow{ -1 };    // 悬停的分组标题排序箭头：0=▲, 1=▼, -1 无
	bool m_trackingTabHover{ false };  // 是否已申请 WM_MOUSELEAVE 跟踪
	int m_stockListScrollOffset{ 0 };  // 左侧股票列表垂直滚动偏移
	bool m_isStockListDragging{ false };  // 左侧股票列表是否正在拖动
	bool m_isStockListDragMoved{ false };  // 左侧股票列表拖动是否产生了位移
	CPoint m_stockListDragStartPos;       // 左侧股票列表拖动起点
	int m_stockListDragStartOffset{ 0 };  // 左侧股票列表拖动起始偏移
	bool m_showJZCurve{ false };  // 基金净值曲线
	bool m_showMA{ false };
	bool m_showBollBands{ true };
	volatile bool m_chartDirty{ false };      // 图表数据更新标识（走势图/K线/MACD等），由PostMessage设置
	volatile bool m_orderBookDirty{ false };  // 盘口数据更新标识（五档/成交/净比等），由共享内存回调设置
	int m_klineHoveredBarIndex{ -1 };
	CString m_klineHoverTip;
	CString m_klineVolumeHoverTip;
	CString m_klineTrendHoverTip;
	CString m_kdjHoverTip;

	// 5分钟K线图整点时间标签（X轴：centerX, "h:mm"）
	std::vector<std::pair<int, CString>> m_min5HourLabels;

	// 总览表行信息（用于双击处理）
	std::vector<OverviewRowInfo> m_overviewRows;

	// 信号颜色（由ApplySignalColors设置，供OnDrawItem使用）
	COLORREF m_bollSignalColor{ CLR_INVALID };
	COLORREF m_macdSignalColor{ CLR_INVALID };
	COLORREF m_kdjSignalColor{ CLR_INVALID };
	COLORREF m_wrSignalColor{ CLR_INVALID };
	COLORREF m_rsiSignalColor{ CLR_INVALID };
	COLORREF m_maSignalColor{ CLR_INVALID };

	// K线数据源刷新与进度状态
	std::atomic<bool> m_isKLineRefreshing{ false };
	std::atomic<int> m_klineRefreshProgress{ 0 };
	std::wstring m_klineRefreshingStockId;
};
