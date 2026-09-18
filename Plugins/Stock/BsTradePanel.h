#pragma once

#include <StockDef.h>
#include <Common.h>
#include <afxwin.h>
#include "DataManager.h"
#include <vector>
#include <string>

// BS 交易台账面板（只读展示，数据源为 DataManager 的 trades 表流水）
// 职责：在右侧信息面板区域绘制当前股票的成交流水（序号、时间、方向、类型、数量、价格）
//       + 底部汇总行；支持垂直滚动（隐藏滚动条）与行命中测试
// 录入/修改通过 CDarkTradeEditDlg 弹窗完成，面板自身不直接写库
class CBsTradePanel
{
public:
	// 单行高度
	static int GetRowHeight();
	// 表头高度
	static int GetTableHeaderHeight();
	// 底部汇总行高度（固定在列表区域底部，不随内容滚动）
	static int GetSummaryHeight();

	// 命中测试：返回点击的流水行下标（0 ~ itemCount-1），未命中返回 -1
	// 与 EtfHoldingsPanel 的坐标约定一致：height 为从 headerHeight 起算的面板总高
	static int HitTest(CPoint pt, int left, int right, int height, int scrollOffset, int itemCount);

	// 绘制台账面板
	// left, right: 面板左右边界
	// height: 面板总高度（从 headerHeight 起算，含盘口标题栏）
	// trades: 该股票全部流水（时间升序），类型标签由 CDataManager::GetTradeKindLabel 推导
	// scrollOffset: 列表垂直滚动偏移量
	// selectedRow: 当前选中行（高亮显示，-1 表示无选中）
	static void Draw(CDC& memDC, int left, int right, int height,
		const std::vector<StockTradeRecord>& trades, int scrollOffset = 0, int selectedRow = -1);
};

// 暗色主题成交录入/编辑弹窗（新增与编辑共用，编辑模式带删除按钮）
// 字段：日期(yyyy-MM-dd) + 时间(HH:mm) + 方向(买/卖) + 数量(股) + 价格(元)
// 结果通过 GetResult() 读取：RES_OK=确认保存，RES_DELETE=删除该笔，RES_CANCEL=取消
class CDarkTradeEditDlg : public CDialog
{
public:
	enum EditResult
	{
		RES_CANCEL = 0,
		RES_OK = 1,
		RES_DELETE = 2,
	};

	std::wstring m_code;          // 股票代码（仅标题展示用）
	std::wstring m_name;          // 股票名称（仅标题展示用）
	bool m_is_sell{ false };      // 方向：true=卖出，false=买入
	std::wstring m_date_text;     // 日期 yyyy-MM-dd（确定后回填）
	std::wstring m_time_text;     // 时间 HH:mm（确定后回填）
	double m_price{ 0.0 };        // 成交价（确定后回填）
	double m_amount{ 0.0 };       // 数量·股（确定后回填）
	bool m_is_new{ true };        // true=新增模式（无删除按钮）

	EditResult GetResult() const { return m_result; }

	CDarkTradeEditDlg(const std::wstring& code, const std::wstring& name = L"", CWnd* pParent = nullptr)
		: CDialog(), m_code(code), m_name(name)
	{
		if (m_name.empty())
		{
			auto stockData = g_data.GetStockData(code);
			if (stockData && !stockData->info.displayName.empty())
				m_name = stockData->info.displayName;
			else
				m_name = code;
		}
	}

	INT_PTR DoModal(CWnd* pParent = nullptr)
	{
		BYTE buffer[512] = { 0 };
		DLGTEMPLATE* pDlg = (DLGTEMPLATE*)buffer;
		pDlg->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CENTER;
		pDlg->dwExtendedStyle = 0;
		pDlg->cdit = 0;
		pDlg->x = 0;
		pDlg->y = 0;
		pDlg->cx = 220;
		pDlg->cy = 140;

		InitModalIndirect(pDlg, pParent);
		return CDialog::DoModal();
	}

	virtual BOOL OnInitDialog() override;
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
	virtual void OnOK() override;

private:
	// 买/卖方向切换按钮重绘
	void DrawDirectionButtons(Gdiplus::Graphics& g);
	// 方向按钮点击命中（返回 true 表示点中了方向按钮并已切换）
	bool HitTestDirectionButtons(CPoint pt);
	// 校验并回填 m_date_text/m_time_text/m_amount/m_price，非法输入返回 false 并提示
	bool ValidateAndFill();

	CEdit m_date_edit;
	CEdit m_time_edit;
	CEdit m_amount_edit;
	CEdit m_price_edit;
	CButton m_btn_ok;
	CButton m_btn_delete;
	CButton m_btn_cancel;

	CRect m_buy_btn_rect;     // 方向按钮矩形（客户区，绘制与命中共用）
	CRect m_sell_btn_rect;

	CFont m_font;
	CFont m_font_bold;
	CBrush m_bg_brush;
	CBrush m_edit_brush;
	EditResult m_result{ RES_CANCEL };
};
