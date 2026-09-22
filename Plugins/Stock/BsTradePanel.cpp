#include "pch.h"
#include "BsTradePanel.h"
#include "ChartColors.h"
#include "DataManager.h"
#include "StockFont.h"
#include <windowsx.h>
#include <algorithm>

// 弹窗内自建控件 ID（避开 resource.h 已用区间，仅本窗口作用域有效）
static const UINT IDC_TRADE_BTN_DELETE = 1205;

int CBsTradePanel::GetRowHeight()
{
	return g_data.RDPI(22);
}

int CBsTradePanel::GetTableHeaderHeight()
{
	return g_data.RDPI(18);
}

int CBsTradePanel::GetSummaryHeight()
{
	// 四行紧凑布局（首 3 + 4×16 + 底 3）：① 买入 ② 卖出 ③ 净持+摊薄 ④ 手续费
	return g_data.RDPI(70);
}

int CBsTradePanel::HitTest(CPoint pt, int left, int right, int height, int scrollOffset, int itemCount)
{
	const int headerHeight = g_data.RDPI(26);
	const int obTitleH = g_data.RDPI(16);
	const int tableHeaderH = GetTableHeaderHeight();
	const int summaryH = GetSummaryHeight();
	const int topOffset = headerHeight + obTitleH;
	const int listTop = topOffset + tableHeaderH;
	const int listBottom = headerHeight + height - summaryH;
	const int rowH = GetRowHeight();

	if (pt.x < left || pt.x >= right || pt.y < listTop || pt.y >= listBottom)
		return -1;

	int contentY = (pt.y - listTop) + scrollOffset;
	if (contentY < 0) return -1;
	int idx = contentY / rowH;
	if (idx >= 0 && idx < itemCount)
		return idx;
	return -1;
}

void CBsTradePanel::Draw(CDC& memDC, int left, int right, int height,
	const std::vector<StockTradeRecord>& trades, int scrollOffset, int selectedRow,
	double filledHoldCount, double filledCostPrice)
{
	const int headerHeight = g_data.RDPI(26);
	const int obTitleH = g_data.RDPI(16);
	const int topOffset = headerHeight + obTitleH;
	const int panelW = right - left;
	const int contentH = height - obTitleH;
	if (panelW <= 0 || contentH <= 0) return;

	// 1. 标题栏底色与面板底色（标题栏上放 BS 按钮，与 CM/PK/CC 同排）
	memDC.FillSolidRect(left, headerHeight, panelW, obTitleH, COLOR_BG_HEADER);
	memDC.FillSolidRect(left, topOffset, panelW, contentH, COLOR_BG_PANEL);
	memDC.SetBkMode(TRANSPARENT);

	// 2. 表头子标题行
	const int tableHeaderH = GetTableHeaderHeight();
	CRect tableHeaderRect(left, topOffset, right, topOffset + tableHeaderH);
	memDC.FillSolidRect(tableHeaderRect, RGB(22, 26, 35));
	memDC.FillSolidRect(left, topOffset + tableHeaderH - 1, panelW, 1, COLOR_DARK_GRAY_BORDER);

	CFont headerFont, textFont;
	CreateStockFont(headerFont, memDC, g_data.RDPI(10), FW_NORMAL);
	CreateStockFont(textFont, memDC, g_data.RDPI(11), FW_NORMAL);

	// 列宽度：序号(14) + 时间(62, "MM-dd HH:mm") + 类型(26) + 数量(30)，价格占余宽
	const int colSeqW = g_data.RDPI(14);
	const int colTimeW = g_data.RDPI(62);
	const int colKindW = g_data.RDPI(26);
	const int colAmtW = g_data.RDPI(30);
	const int colPriceW = max(g_data.RDPI(34), panelW - (colSeqW + colTimeW + colKindW + colAmtW));

	const int colSeqX = left;
	const int colTimeX = colSeqX + colSeqW;
	const int colKindX = colTimeX + colTimeW;
	const int colAmtX = colKindX + colKindW;
	const int colPriceX = colAmtX + colAmtW;

	CFont* pOldFont = memDC.SelectObject(&headerFont);
	memDC.SetTextColor(COLOR_TEXT_MUTED);

	CRect rcSeqH(colSeqX, topOffset, colSeqX + colSeqW, topOffset + tableHeaderH);
	CRect rcTimeH(colTimeX, topOffset, colTimeX + colTimeW - g_data.RDPI(2), topOffset + tableHeaderH);
	CRect rcKindH(colKindX, topOffset, colKindX + colKindW, topOffset + tableHeaderH);
	CRect rcAmtH(colAmtX, topOffset, colAmtX + colAmtW - g_data.RDPI(2), topOffset + tableHeaderH);
	CRect rcPriceH(colPriceX, topOffset, right - g_data.RDPI(3), topOffset + tableHeaderH);

	memDC.DrawText(_T("序"), rcSeqH, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	memDC.DrawText(_T("时间"), rcTimeH, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	memDC.DrawText(_T("类型"), rcKindH, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	memDC.DrawText(_T("数量"), rcAmtH, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	memDC.DrawText(_T("价格"), rcPriceH, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	// 3. 列表区域（底部留出固定汇总行）
	const int summaryH = GetSummaryHeight();
	const int listTop = topOffset + tableHeaderH;
	const int listH = contentH - tableHeaderH - summaryH;
	if (listH <= 0)
	{
		memDC.SelectObject(pOldFont);
		return;
	}

	CRect listClipRect(left, listTop, right, listTop + listH);
	CRgn clipRgn;
	clipRgn.CreateRectRgn(listClipRect.left, listClipRect.top, listClipRect.right, listClipRect.bottom);
	memDC.SelectClipRgn(&clipRgn);

	if (trades.empty())
	{
		memDC.SelectObject(&textFont);
		memDC.SetTextColor(COLOR_TEXT_DIM);
		CRect rcMsg = listClipRect;
		memDC.DrawText(_T("暂无台账\n双击空白处新增"), rcMsg, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);
		memDC.SelectClipRgn(nullptr);
		memDC.SelectObject(pOldFont);
		return;
	}

	const int rowH = GetRowHeight();
	memDC.SelectObject(&textFont);

	for (size_t i = 0; i < trades.size(); ++i)
	{
		int itemY = listTop - scrollOffset + static_cast<int>(i) * rowH;
		if (itemY + rowH <= listTop || itemY >= listTop + listH)
			continue; // 视口外跳过绘制

		const auto& record = trades[i];
		CRect rowRect(left, itemY, right, itemY + rowH);

		// 选中行高亮，其次斑马条纹浅底
		if (static_cast<int>(i) == selectedRow)
		{
			memDC.FillSolidRect(rowRect, COLOR_CARD_SELECTED);
		}
		else if (i % 2 == 1)
		{
			memDC.FillSolidRect(rowRect, RGB(22, 24, 32));
		}

		// 1) 序号
		CRect rcSeq(colSeqX, itemY, colSeqX + colSeqW, itemY + rowH);
		memDC.SetTextColor(COLOR_TEXT_DIM);
		CString seqStr;
		seqStr.Format(_T("%d"), static_cast<int>(i + 1));
		memDC.DrawText(seqStr, rcSeq, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

		// 2) 时间（yyyy-MM-dd HH:mm 取 MM-dd HH:mm 短格式，当年成交不丢信息）
		CRect rcTime(colTimeX, itemY, colTimeX + colTimeW - g_data.RDPI(2), itemY + rowH);
		memDC.SetTextColor(COLOR_TEXT_MUTED);
		std::wstring t = record.time;
		std::wstring shortTime = (t.size() >= 16 && t.compare(4, 1, L"-") == 0) ? t.substr(5, 11) : t;
		memDC.DrawText(shortTime.c_str(), rcTime, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

		// 3) 类型（按历史推导 建仓/加仓=红（买入），减仓/清仓=绿（卖出））
		CRect rcKind(colKindX, itemY, colKindX + colKindW, itemY + rowH);
		std::wstring kind = CDataManager::GetTradeKindLabel(trades, i);
		memDC.SetTextColor(record.isSell ? COLOR_GREEN_DOWN : COLOR_RED_UP);
		memDC.DrawText(kind.c_str(), rcKind, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

		// 4) 数量（股）
		CRect rcAmt(colAmtX, itemY, colAmtX + colAmtW - g_data.RDPI(2), itemY + rowH);
		memDC.SetTextColor(COLOR_WHITE);
		CString amtStr;
		amtStr.Format(_T("%.0f"), record.amount);
		memDC.DrawText(amtStr, rcAmt, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

		// 5) 价格
		CRect rcPrice(colPriceX, itemY, right - g_data.RDPI(3), itemY + rowH);
		memDC.SetTextColor(COLOR_WHITE);
		CString priceStr;
		priceStr.Format(_T("%.3f"), record.price);
		memDC.DrawText(priceStr, rcPrice, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	}

	memDC.SelectClipRgn(nullptr);

	// 4. 底部汇总行（固定，不随滚动）：买入/卖出总笔数与股数 + 台账重放净持仓
	int summaryY = listTop + listH;
	memDC.FillSolidRect(left, summaryY, panelW, 1, COLOR_DARK_GRAY_BORDER);

	double buyAmt = 0.0, sellAmt = 0.0, fee = 0.0, hold = 0.0, netCost = 0.0;
	int buyCount = 0, sellCount = 0;
	for (const auto& record : trades)
	{
		if (record.isSell)
		{
			++sellCount;
			sellAmt += record.amount;
			hold -= record.amount;
			netCost -= record.price * record.amount;
		}
		else
		{
			++buyCount;
			buyAmt += record.amount;
			hold += record.amount;
			netCost += record.price * record.amount;
		}
		if (hold < 0.0)
		{
			hold = 0.0;       // 与 GetTradeKindLabel 重放口径一致：下限截 0
			netCost = 0.0;    // 净持截 0 时成本余额同步清零，避免残值拉高后续摊薄均价
		}
		fee += record.fee;
	}
	const double ledgerAvgCost = hold > 0.0 ? netCost / hold : 0.0;

	// 重放净持/摊薄成本与填写值任一不一致时整行橙色提醒
	//（filledHoldCount<0 表示持股数未知不比对；filledCostPrice<=0 表示成本价未知不比对）
	const bool holdMismatch = filledHoldCount >= 0.0 &&
		(hold > filledHoldCount + 0.5 || hold < filledHoldCount - 0.5);
	const bool costMismatch = filledCostPrice > 0.0 && ledgerAvgCost > 0.0 &&
		(ledgerAvgCost > filledCostPrice + 0.00005 || ledgerAvgCost < filledCostPrice - 0.00005);
	const bool rowMismatch = holdMismatch || costMismatch;

	memDC.SelectObject(&headerFont);
	// 四行紧凑布局：总高 RDPI(70) = 首行上 3 + 4×行高 16 + 底部余 3。
	// ① 买入 ② 卖出 ③ 净持+摊薄（两段同行，实测窄侧栏放得下） ④ 手续费
	const int padX = g_data.RDPI(10);
	const int lineH = g_data.RDPI(16);
	const int line1Y = summaryY + g_data.RDPI(3);
	const int line2Y = line1Y + lineH;
	const int line3Y = line2Y + lineH;
	const int line4Y = line3Y + lineH;

	auto drawLine = [&](const CString& text, int y, COLORREF color) {
		if (text.IsEmpty())
			return;
		CRect rc(left + padX, y, right - padX, y + lineH);
		memDC.SetTextColor(color);
		memDC.DrawText(text, rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	};

	// 第三行拆两段：净持左对齐，摊薄右对齐（两端分布，数字列上下齐整）
	auto drawLineRight = [&](const CString& text, int y, COLORREF color) {
		if (text.IsEmpty())
			return;
		CRect rc(left + padX, y, right - padX, y + lineH);
		memDC.SetTextColor(color);
		memDC.DrawText(text, rc, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	};

	// 行色规则：与填写值相关的行不一致时橙色提醒，其余常规灰阶
	const COLORREF cLedger = rowMismatch ? COLOR_DARK_ORANGE : COLOR_TEXT_MUTED;
	CString sBuy, sSell, sHold, sAvg, sFee;
	sBuy.Format(_T("买入 %d 笔 · %.0f 股"), buyCount, buyAmt);
	sSell.Format(_T("卖出 %d 笔 · %.0f 股"), sellCount, sellAmt);
	sHold.Format(_T("净持 %.0f 股"), hold);
	drawLine(sBuy, line1Y, cLedger);
	drawLine(sSell, line2Y, cLedger);
	drawLine(sHold, line3Y, cLedger);
	if (ledgerAvgCost > 0.0)
	{
		sAvg.Format(_T("摊薄 %.4f"), ledgerAvgCost);
		drawLineRight(sAvg, line3Y, cLedger);
	}
	if (fee > 0.005)
	{
		sFee.Format(_T("手续费 %.2f 元"), fee);
		drawLine(sFee, line4Y, COLOR_TEXT_DIM);
	}

	memDC.SelectObject(pOldFont);
}

// ── 暗色成交录入/编辑弹窗 ────────────────────────────────────────────

namespace
{
	const COLORREF kTradeDlgBg = RGB(18, 20, 26);        // 弹窗底色
	const COLORREF kTradeDlgEditBg = RGB(13, 15, 21);    // 输入框底色

	// 方角描边：用四条实心边条拼，笔直且无抗锯齿溢出（本弹窗一律方角，不用圆角）
	void FillFlatOutline(Gdiplus::Graphics& g, const CRect& rect, const Gdiplus::Color& color)
	{
		Gdiplus::SolidBrush brush(color);
		g.FillRectangle(&brush, rect.left, rect.top, rect.Width(), 1);
		g.FillRectangle(&brush, rect.left, rect.bottom - 1, rect.Width(), 1);
		g.FillRectangle(&brush, rect.left, rect.top, 1, rect.Height());
		g.FillRectangle(&brush, rect.right - 1, rect.top, 1, rect.Height());
	}

	// 标签左对齐绘制，与「设置持仓信息」弹窗同一约定：
	// 左内边距永远等于 marginX，不会因标签长短而变（右对齐会让短标签的左留白看起来更大）
	void DrawLeftAlignedLabel(Gdiplus::Graphics& g, const wchar_t* text, Gdiplus::Font& font,
		int left, const CRect& anchorRow, const Gdiplus::SolidBrush& brush)
	{
		Gdiplus::StringFormat fmt;
		fmt.SetAlignment(Gdiplus::StringAlignmentNear);
		fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		fmt.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
		Gdiplus::RectF rf(static_cast<Gdiplus::REAL>(left), static_cast<Gdiplus::REAL>(anchorRow.top),
			static_cast<Gdiplus::REAL>(g_data.DPI(200)), static_cast<Gdiplus::REAL>(anchorRow.Height()));
		g.DrawString(text, -1, &font, rf, &fmt, &brush);
	}
}

BOOL CDarkTradeEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// 标题带上代码与名称，便于在同名标的间区分（代码前缀用大写，与行情软件一致）
	if (m_name.empty())
		SetWindowText(_T("成交录入"));
	else
	{
		CString title;
		title.Format(_T("%s %s · 成交录入"),
			CString(CCommon::GetDisplayCode(m_code).c_str()), CString(m_name.c_str()));
		SetWindowText(title);
	}

	BOOL darkCaption = TRUE;
	::DwmSetWindowAttribute(GetSafeHwnd(), 20 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/, &darkCaption, sizeof(darkCaption));

	m_font.CreatePointFont(100, _T("微软雅黑"));
	m_font_bold.CreatePointFont(100, _T("微软雅黑"));
	SetFont(&m_font);

	m_bg_brush.CreateSolidBrush(kTradeDlgBg);
	m_edit_brush.CreateSolidBrush(kTradeDlgEditBg);

	// 列布局：标签列固定宽（单位写在标签里，与「设置持仓信息」弹窗同一约定），输入框统一右边界
	const int marginX = g_data.DPI(16);       // 左右留白一致：左=marginX，右=marginX
	const int labelW = g_data.DPI(80);
	const int rowH = g_data.DPI(26);          // 视觉外框高（描边所在的框）
	const int editBoxH = g_data.DPI(18);      // 输入框控件实际高度：单行编辑框文字靠上，
	const int editPadX = g_data.DPI(6);       // 控件须比外框小一圈并居中嵌入，文字才垂直居中、
	                                          // 且左右描边不会被控件自身盖住（沿用「设置持仓信息」做法）
	const int editPadY = (rowH - editBoxH) / 2;
	const int rowPitch = g_data.DPI(38);
	const int btnH = g_data.DPI(28);
	const int btnW = g_data.DPI(64);
	const int contentW = g_data.DPI(200);     // 输入区固定宽度，日期/时间同排也够用

	// 行序：方向 / 日期+时间 / 数量 / 价格 / 手续费
	const int topPad = g_data.DPI(16);
	const int dirY = topPad;
	const int dateTimeY = dirY + rowPitch;
	const int amountY = dateTimeY + rowPitch;
	const int priceY = amountY + rowPitch;
	const int feeY = priceY + rowPitch;
	const int btnY = feeY + rowPitch + g_data.DPI(4);

	// 先把窗口调到内容所需尺寸（模板按两行设计，这里收紧/补齐），再据此计算控件坐标，
	// 否则先取的客户区尺寸会在调整后过期，导致右侧控件错位。
	// 模板带 DS_CENTER，缩放时仅调整尺寸、不自行挪位置（挪了会与居中叠加而偏移）
	{
		const int neededClientW = marginX + labelW + contentW + marginX;
		const int neededClientH = btnY + btnH + g_data.DPI(16);
		CRect cr0, wr;
		GetClientRect(&cr0);
		GetWindowRect(&wr);
		const int deltaW = neededClientW - cr0.Width();
		const int deltaH = neededClientH - cr0.Height();
		if (deltaW != 0 || deltaH != 0)
			SetWindowPos(nullptr, 0, 0, wr.Width() + deltaW, wr.Height() + deltaH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	// 客户区尺寸在窗口调整后重新读取，保证左右留白严格相等
	CRect crClient;
	GetClientRect(&crClient);
	const int contentRight = crClient.right - marginX;
	const int contentLeft = contentRight - contentW;

	// 方向：两枚按钮等宽铺满内容列，形似分段开关（比两个小按钮浮在左侧更整齐）
	const int dirGap = g_data.DPI(8);
	const int dirBtnW = (contentRight - contentLeft - dirGap) / 2;
	m_buy_btn_rect = CRect(contentLeft, dirY, contentLeft + dirBtnW, dirY + rowH);
	m_sell_btn_rect = CRect(contentLeft + dirBtnW + dirGap, dirY, contentRight, dirY + rowH);

	// 输入框按「外框内缩」放置：控件只占内圈，四周留出的环带用于画圆角描边，
	// 同时天然形成文字左内边距
	auto createEdit = [&](CEdit& edit, int rowY, int left, int right, UINT id, const wchar_t* cue) {
		edit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
			CRect(left + editPadX, rowY + editPadY, right - editPadX, rowY + editPadY + editBoxH), this, id);
		edit.ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
		::SetWindowTheme(edit.GetSafeHwnd(), L"", L"");
		edit.SetFont(&m_font);
		edit.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)cue);
	};
	// 日期与时间同排：日期拿主体宽度，时间只留 "HH:mm" 所需
	const int dateGap = g_data.DPI(8);
	const int dateW = (contentRight - contentLeft) * 58 / 100;
	createEdit(m_date_edit, dateTimeY, contentLeft, contentLeft + dateW, 1101, L"yyyy-MM-dd");
	createEdit(m_time_edit, dateTimeY, contentLeft + dateW + dateGap, contentRight, 1102, L"HH:mm");
	createEdit(m_amount_edit, amountY, contentLeft, contentRight, 1103, L"输入数量");
	createEdit(m_price_edit, priceY, contentLeft, contentRight, 1104, L"输入成交价");
	createEdit(m_fee_edit, feeY, contentLeft, contentRight, 1105, L"手续费，可留空");

	// 底部按钮：确定/取消靠右成组（右边界与输入框对齐）；
	// 删除按钮单独放到最左边（与标签列左边界对齐，远离确定/取消，避免误按）
	const int okLeft = contentRight - btnW;
	const int cancelLeft = okLeft - g_data.DPI(8) - btnW;

	m_btn_ok.Create(_T("确定"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
		CRect(okLeft, btnY, contentRight, btnY + btnH), this, IDOK);
	m_btn_ok.SetFont(&m_font_bold);

	m_btn_cancel.Create(_T("取消"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_OWNERDRAW,
		CRect(cancelLeft, btnY, cancelLeft + btnW, btnY + btnH), this, IDCANCEL);
	m_btn_cancel.SetFont(&m_font);

	if (!m_is_new)
	{
		m_btn_delete.Create(_T("删除"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_OWNERDRAW,
			CRect(marginX, btnY, marginX + btnW, btnY + btnH), this, IDC_TRADE_BTN_DELETE);
		m_btn_delete.SetFont(&m_font);
	}

	// 预填：编辑模式回显记录值；新增模式默认今天当前时间
	if (m_date_text.empty())
	{
		CTime now = CTime::GetCurrentTime();
		m_date_text = now.Format(L"%Y-%m-%d").GetString();
		m_time_text = now.Format(L"%H:%M").GetString();
	}
	m_date_edit.SetWindowText(CString(m_date_text.c_str()));
	if (!m_time_text.empty())
		m_time_edit.SetWindowText(CString(m_time_text.c_str()));
	if (m_amount > 0)
	{
		CString s;
		s.Format(_T("%.0f"), m_amount);
		m_amount_edit.SetWindowText(s);
	}
	if (m_price > 0)
	{
		CString s;
		s.Format(_T("%.3f"), m_price);
		m_price_edit.SetWindowText(s);
	}
	if (m_fee > 0)
	{
		// 编辑模式：回显台账已存的手续费（含手动修正过的 0.35 这类特例）
		CString s;
		s.Format(_T("%.2f"), m_fee);
		m_fee_edit.SetWindowText(s);
	}

	m_date_edit.SetFocus();
	m_date_edit.SetSel(0, -1);
	return FALSE;
}

LRESULT CDarkTradeEditDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_ERASEBKGND)
	{
		return TRUE;
	}
	else if (message == WM_CTLCOLOREDIT)
	{
		HDC hdc = (HDC)wParam;
		::SetTextColor(hdc, RGB(255, 255, 255));
		::SetBkColor(hdc, RGB(13, 15, 21));
		return (LRESULT)(HBRUSH)m_edit_brush;
	}
	else if (message == WM_CTLCOLORSTATIC)
	{
		HDC hdc = (HDC)wParam;
		::SetTextColor(hdc, RGB(226, 232, 240));
		::SetBkColor(hdc, RGB(18, 20, 26));
		return (LRESULT)(HBRUSH)m_bg_brush;
	}
	else if (message == WM_LBUTTONDOWN)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		if (HitTestDirectionButtons(pt))
		{
			// 两个按钮整体一起失效（含各自外扩 1px 的余量），确保切换后无旧色残留
			CRect dirty = m_buy_btn_rect;
			dirty.UnionRect(dirty, m_sell_btn_rect);
			dirty.InflateRect(1, 1);
			InvalidateRect(dirty);
			return 0;
		}
	}
	else if (message == WM_COMMAND)
	{
		const WORD notify = HIWORD(wParam);
		if (notify == BN_CLICKED && LOWORD(wParam) == IDC_TRADE_BTN_DELETE)
		{
			// 删除按钮：无消息映射，直接在 WindowProc 处理
			m_result = RES_DELETE;
			EndDialog(RES_DELETE);
			return TRUE;
		}
		// 数量/价格/手续费均为纯手填，无自动计算逻辑
		(void)notify; (void)wParam;
		// 输入框焦点变化时重绘，让蓝色焦点描边跟随光标所在框
		if (notify == EN_SETFOCUS || notify == EN_KILLFOCUS)
			InvalidateRect(nullptr, FALSE);
	}
	else if (message == WM_DRAWITEM)
	{
		LPDRAWITEMSTRUCT pDI = (LPDRAWITEMSTRUCT)lParam;
		if (pDI->CtlType == ODT_BUTTON)
		{
			CDC dc;
			dc.Attach(pDI->hDC);
			CRect rect = pDI->rcItem;
			UINT state = pDI->itemState;
			CString text;
			if (pDI->CtlID == IDOK) text = _T("确定");
			else if (pDI->CtlID == IDC_TRADE_BTN_DELETE) text = _T("删除");
			else text = _T("取消");

			COLORREF bgColor;
			COLORREF textColor;
			if (pDI->CtlID == IDOK)
			{
				bgColor = (state & ODS_SELECTED) ? RGB(29, 78, 216) : RGB(37, 99, 235);
				textColor = RGB(255, 255, 255);
			}
			else if (pDI->CtlID == IDC_TRADE_BTN_DELETE)
			{
				bgColor = (state & ODS_SELECTED) ? RGB(60, 15, 22) : RGB(40, 20, 26);
				textColor = RGB(248, 113, 113);
			}
			else
			{
				bgColor = (state & ODS_SELECTED) ? RGB(20, 25, 35) : RGB(30, 35, 46);
				textColor = RGB(255, 255, 255);
			}

			// 方角实底填充（按钮一律方角，与「设置持仓信息」弹窗一致）
			dc.FillSolidRect(rect, bgColor);

			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(textColor);
			CFont* pFont = (pDI->CtlID == IDOK) ? &m_font_bold : &m_font;
			CFont* pOldFont = dc.SelectObject(pFont);
			dc.DrawText(text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			dc.SelectObject(pOldFont);
			dc.Detach();
			return TRUE;
		}
	}
	else if (message == WM_PAINT)
	{
		CPaintDC dc(this);
		CRect rc;
		GetClientRect(rc);

		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		CBitmap memBmp;
		memBmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
		CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

		Gdiplus::Graphics g(memDC.GetSafeHdc());
		g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

		// 1. 底色
		Gdiplus::SolidBrush bg(Gdiplus::Color(255, 18, 20, 26));
		g.FillRectangle(&bg, 0, 0, rc.Width(), rc.Height());

		const int marginX = g_data.DPI(16);

		// 2. 标签：左对齐、单位写进标签（与「设置持仓信息」弹窗同一约定），
		//    左留白恒为 marginX，不会随标签长短变化
		Gdiplus::Font labelFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(12)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush labelBrush(Gdiplus::Color(255, 203, 213, 225));

		auto editRectInClient = [&](CWnd& edit) -> CRect {
			CRect er(0, 0, 0, 0);
			if (!edit.GetSafeHwnd()) return er;
			edit.GetWindowRect(er);
			ScreenToClient(er);
			return er;
		};

		// 行高与视觉外框对齐：输入框控件比外框小一圈，这里把控件矩形还原成外框矩形
		auto frameRectForEdit = [&](const CRect& editRc) {
			if (editRc.IsRectEmpty()) return editRc;
			CRect r = editRc;
			const int padX = g_data.DPI(6);
			const int padY = (g_data.DPI(26) - g_data.DPI(18)) / 2;
			r.InflateRect(padX, padY);
			return r;
		};
		auto drawRowLabel = [&](const wchar_t* text, const CRect& anchorRow) {
			DrawLeftAlignedLabel(g, text, labelFont, marginX, anchorRow, labelBrush);
		};

		// 日期与时间同排，故只用一行标签统辖两个框
		drawRowLabel(L"方向", m_buy_btn_rect);
		drawRowLabel(L"日期/时间", frameRectForEdit(editRectInClient(m_date_edit)));
		drawRowLabel(L"数量 (股)", frameRectForEdit(editRectInClient(m_amount_edit)));
		drawRowLabel(L"价格 (元)", frameRectForEdit(editRectInClient(m_price_edit)));
		drawRowLabel(L"手续费 (元)", frameRectForEdit(editRectInClient(m_fee_edit)));

		// 3. 输入框：按行高绘制方角底与描边，控件本体居中嵌在其中（控件矮一圈、窄一圈，
		//    文字才垂直居中且左右留出内边距，描边也不会被控件盖住）
		auto drawEdit = [&](CWnd& edit) {
			CRect frame = frameRectForEdit(editRectInClient(edit));
			if (frame.IsRectEmpty()) return;

			Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 13, 15, 21));
			g.FillRectangle(&bgBrush, frame.left, frame.top, frame.Width(), frame.Height());

			CWnd* pFocus = GetFocus();
			bool focused = (pFocus && pFocus->GetSafeHwnd() == edit.GetSafeHwnd());
			FillFlatOutline(g, frame, focused ? Gdiplus::Color(255, 37, 99, 235) : Gdiplus::Color(255, 52, 58, 72));
		};
		drawEdit(m_date_edit);
		drawEdit(m_time_edit);
		drawEdit(m_amount_edit);
		drawEdit(m_price_edit);
		drawEdit(m_fee_edit);

		DrawDirectionButtons(g);

		dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(pOldBmp);
		return 0;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CDarkTradeEditDlg::DrawDirectionButtons(Gdiplus::Graphics& g)
{
	// 分段开关样式：选中侧语义色实底，未选中侧深底 + 灰描边（一律方角）
	auto drawDir = [&](const CRect& rect, bool isBuy) {
		bool isSel = (m_is_sell != isBuy);
		Gdiplus::Color bgCol = isSel
			? (isBuy ? Gdiplus::Color(255, 246, 70, 93) : Gdiplus::Color(255, 14, 203, 129))
			: Gdiplus::Color(255, 24, 27, 34);
		Gdiplus::SolidBrush bgBrush(bgCol);
		g.FillRectangle(&bgBrush, rect.left, rect.top, rect.Width(), rect.Height());

		if (!isSel)
			FillFlatOutline(g, rect, Gdiplus::Color(255, 52, 58, 72));

		Gdiplus::Font dirFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(12)),
			isSel ? Gdiplus::FontStyleBold : Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush txtBrush(isSel ? Gdiplus::Color(255, 255, 255) : Gdiplus::Color(255, 148, 163, 184));
		Gdiplus::StringFormat sf;
		sf.SetAlignment(Gdiplus::StringAlignmentCenter);
		sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		sf.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
		const wchar_t* label = isBuy ? L"买入" : L"卖出";
		Gdiplus::RectF rf(static_cast<Gdiplus::REAL>(rect.left), static_cast<Gdiplus::REAL>(rect.top),
			static_cast<Gdiplus::REAL>(rect.Width()), static_cast<Gdiplus::REAL>(rect.Height()));
		g.DrawString(label, -1, &dirFont, rf, &sf, &txtBrush);
	};
	drawDir(m_buy_btn_rect, true);
	drawDir(m_sell_btn_rect, false);
}

bool CDarkTradeEditDlg::HitTestDirectionButtons(CPoint pt)
{
	if (m_buy_btn_rect.PtInRect(pt))
	{
		m_is_sell = false;
		return true;
	}
	if (m_sell_btn_rect.PtInRect(pt))
	{
		m_is_sell = true;
		return true;
	}
	return false;
}

bool CDarkTradeEditDlg::ValidateAndFill()
{
	CString strDate, strTime, strAmount, strPrice;
	m_date_edit.GetWindowText(strDate);
	m_time_edit.GetWindowText(strTime);
	m_amount_edit.GetWindowText(strAmount);
	m_price_edit.GetWindowText(strPrice);
	strDate.Trim();
	strTime.Trim();
	strAmount.Trim();
	strPrice.Trim();

	// 日期 yyyy-MM-dd：逐段解析并校验（2月30日这类会被 GetDaysInMonth 拒掉）
	if (strDate.GetLength() != 10 || strDate[4] != L'-' || strDate[7] != L'-')
	{
		AfxMessageBox(_T("日期格式不对，请按 yyyy-MM-dd 填写，如 2026-09-16"));
		return false;
	}
	wchar_t* pEnd = nullptr;
	const long y = wcstol(strDate.GetString(), &pEnd, 10);
	if (pEnd != strDate.GetString() + 4 || y < 1990 || y > 2100)
	{
		AfxMessageBox(_T("日期年份不对，请按 yyyy-MM-dd 填写，如 2026-09-16"));
		return false;
	}
	const long mo = wcstol(strDate.GetString() + 5, &pEnd, 10);
	if (pEnd != strDate.GetString() + 7 || mo < 1 || mo > 12)
	{
		AfxMessageBox(_T("日期月份不对，请按 yyyy-MM-dd 填写，如 2026-09-16"));
		return false;
	}
	const long dy = wcstol(strDate.GetString() + 8, &pEnd, 10);
	static const int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int maxDay = daysInMonth[mo - 1];
	if (mo == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0))
		maxDay = 29;
	if (pEnd != strDate.GetString() + 10 || dy < 1 || dy > maxDay)
	{
		AfxMessageBox(_T("日期日子不对，请检查该月是否有这一天"));
		return false;
	}

	// 时间 HH:mm：必填（新增时已预填当前时间）
	if (strTime.GetLength() != 5 || strTime[2] != L':')
	{
		AfxMessageBox(_T("时间格式不对，请按 HH:mm 填写，如 14:30"));
		return false;
	}
	const long hh = wcstol(strTime.GetString(), &pEnd, 10);
	if (pEnd != strTime.GetString() + 2 || hh < 0 || hh > 23)
	{
		AfxMessageBox(_T("时间小时不对，请按 HH:mm 填写，如 14:30"));
		return false;
	}
	const long mi = wcstol(strTime.GetString() + 3, &pEnd, 10);
	if (pEnd != strTime.GetString() + 5 || mi < 0 || mi > 59)
	{
		AfxMessageBox(_T("时间分钟不对，请按 HH:mm 填写，如 14:30"));
		return false;
	}

	// 数量与价格：正数
	const double amt = wcstod(strAmount.GetString(), &pEnd);
	if (strAmount.IsEmpty() || *pEnd != L'\0' || amt <= 0)
	{
		AfxMessageBox(_T("数量要为大于 0 的数字（单位：股）"));
		return false;
	}
	const double price = wcstod(strPrice.GetString(), &pEnd);
	if (strPrice.IsEmpty() || *pEnd != L'\0' || price <= 0)
	{
		AfxMessageBox(_T("成交价要为大于 0 的数字（单位：元）"));
		return false;
	}

	// 手续费：可留空（记 0），填了必须是不小于 0 的数字
	CString strFee;
	m_fee_edit.GetWindowText(strFee);
	strFee.Trim();
	double fee = 0.0;
	if (!strFee.IsEmpty())
	{
		fee = wcstod(strFee.GetString(), &pEnd);
		if (*pEnd != L'\0' || fee < 0)
		{
			AfxMessageBox(_T("手续费要为不小于 0 的数字（单位：元），留空表示不计费"));
			return false;
		}
	}

	m_date_text = strDate.GetString();
	m_time_text = strTime.GetString();
	m_amount = amt;
	m_price = price;
	m_fee = fee;
	return true;
}

void CDarkTradeEditDlg::OnOK()
{
	if (!ValidateAndFill())
		return; // 校验失败留在弹窗
	m_result = RES_OK;
	CDialog::OnOK();
}
