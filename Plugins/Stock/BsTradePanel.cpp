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
	return g_data.RDPI(22);
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
	const std::vector<StockTradeRecord>& trades, int scrollOffset, int selectedRow)
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

	// 4. 底部汇总行（固定，不随滚动）：买入/卖出总股数与重放净持仓
	int summaryY = listTop + listH;
	memDC.FillSolidRect(left, summaryY, panelW, 1, COLOR_DARK_GRAY_BORDER);

	double buyAmt = 0.0, sellAmt = 0.0, fee = 0.0;
	int buyCount = 0, sellCount = 0;
	for (const auto& record : trades)
	{
		if (record.isSell)
		{
			++sellCount;
			sellAmt += record.amount;
		}
		else
		{
			++buyCount;
			buyAmt += record.amount;
		}
		fee += record.fee;
	}

	memDC.SelectObject(&headerFont);
	CString sumStr;
	sumStr.Format(_T("买%d笔%.0f 卖%d笔%.0f"), buyCount, buyAmt, sellCount, sellAmt);
	CRect rcSummary(left + g_data.RDPI(4), summaryY, right - g_data.RDPI(4), summaryY + summaryH);
	memDC.SetTextColor(COLOR_TEXT_MUTED);
	memDC.DrawText(sumStr, rcSummary, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	CString feeStr;
	if (fee > 0.005)
	{
		feeStr.Format(_T("费%.2f"), fee);
		CRect rcFee(right - g_data.RDPI(60), summaryY, right - g_data.RDPI(3), summaryY + summaryH);
		memDC.SetTextColor(COLOR_TEXT_DIM);
		memDC.DrawText(feeStr, rcFee, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	}

	memDC.SelectObject(pOldFont);
}

// ── 暗色成交录入/编辑弹窗 ────────────────────────────────────────────

BOOL CDarkTradeEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString title;
	title.Format(_T("%s %s · 成交录入"), CString(m_code.c_str()), CString(m_name.c_str()));
	SetWindowText(title);

	BOOL darkCaption = TRUE;
	::DwmSetWindowAttribute(GetSafeHwnd(), 20 /*DWMWA_USE_IMMERSIVE_DARK_MODE*/, &darkCaption, sizeof(darkCaption));

	m_font.CreatePointFont(100, _T("微软雅黑"));
	m_font_bold.CreatePointFont(105, _T("微软雅黑"));
	SetFont(&m_font);

	m_bg_brush.CreateSolidBrush(RGB(18, 20, 26));      // #12141A
	m_edit_brush.CreateSolidBrush(RGB(13, 15, 21));

	CRect cr;
	GetClientRect(&cr);
	const int marginX = g_data.DPI(16);
	const int editH = g_data.DPI(26);
	const int editBoxH = g_data.DPI(18);
	const int editOffset = g_data.DPI(4);
	const int labelColW = g_data.DPI(58);
	const int editBorderLeft = marginX + labelColW;
	const int editInnerLeft = editBorderLeft + g_data.RDPI(6);
	const int editInnerRight = cr.right - marginX - g_data.DPI(6);

	// 行布局：方向 / 日期 + 时间 / 数量 / 价格
	const int dirY = g_data.DPI(52);
	const int dateY = dirY + editH + g_data.DPI(10);
	const int amountY = dateY + editH + g_data.DPI(10);
	const int priceY = amountY + editH + g_data.DPI(10);

	// 模板高度（140 对话框单位）在 96DPI 下不足以放下四行内容，按实际内容补高；底部按钮坐标由客户区底边推导会自动跟随
	const int neededClientH = priceY + editH + g_data.DPI(14) + editH + g_data.DPI(12);
	if (cr.Height() < neededClientH)
	{
		CRect wr;
		GetWindowRect(&wr);
		SetWindowPos(nullptr, 0, 0, wr.Width(), wr.Height() + (neededClientH - cr.Height()),
			SWP_NOMOVE | SWP_NOZORDER);
		GetClientRect(&cr);
	}

	// 方向切换按钮（买红 / 卖绿），绘制与命中共用矩形
	const int dirBtnW = g_data.DPI(70);
	const int dirBtnH = g_data.DPI(24);
	m_buy_btn_rect = CRect(editBorderLeft, dirY, editBorderLeft + dirBtnW, dirY + dirBtnH);
	m_sell_btn_rect = CRect(editBorderLeft + dirBtnW + g_data.DPI(10), dirY,
		editBorderLeft + dirBtnW * 2 + g_data.DPI(10), dirY + dirBtnH);

	// 日期 + 时间：两个并排小框
	const int dateW = (editInnerRight - editInnerLeft - g_data.DPI(10)) / 2;
	m_date_edit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
		CRect(editInnerLeft, dateY + editOffset, editInnerLeft + dateW, dateY + editOffset + editBoxH), this, 1101);
	m_date_edit.ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	::SetWindowTheme(m_date_edit.GetSafeHwnd(), L"", L"");
	m_date_edit.SetFont(&m_font);
	m_date_edit.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)L"yyyy-MM-dd");

	m_time_edit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
		CRect(editInnerLeft + dateW + g_data.DPI(10), dateY + editOffset, editInnerRight, dateY + editOffset + editBoxH), this, 1102);
	m_time_edit.ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	::SetWindowTheme(m_time_edit.GetSafeHwnd(), L"", L"");
	m_time_edit.SetFont(&m_font);
	m_time_edit.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)L"HH:mm");

	m_amount_edit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
		CRect(editInnerLeft, amountY + editOffset, editInnerRight, amountY + editOffset + editBoxH), this, 1103);
	m_amount_edit.ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	::SetWindowTheme(m_amount_edit.GetSafeHwnd(), L"", L"");
	m_amount_edit.SetFont(&m_font);
	m_amount_edit.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)L"输入数量(股)");

	m_price_edit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
		CRect(editInnerLeft, priceY + editOffset, editInnerRight, priceY + editOffset + editBoxH), this, 1104);
	m_price_edit.ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	::SetWindowTheme(m_price_edit.GetSafeHwnd(), L"", L"");
	m_price_edit.SetFont(&m_font);
	m_price_edit.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)L"输入成交价(元)");

	// 底部按钮：编辑模式插入删除按钮（红描边文案），三钮等宽
	const int btnW = g_data.DPI(70);
	const int btnH = g_data.DPI(26);
	const int btnY = cr.bottom - btnH - g_data.DPI(12);

	m_btn_ok.Create(_T("确定"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON | BS_OWNERDRAW,
		CRect(cr.right - btnW - marginX, btnY, cr.right - marginX, btnY + btnH), this, IDOK);
	m_btn_ok.SetFont(&m_font_bold);

	m_btn_cancel.Create(_T("取消"), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_OWNERDRAW,
		CRect(cr.right - btnW * 2 - marginX - g_data.DPI(10), btnY, cr.right - btnW - marginX - g_data.DPI(10), btnY + btnH), this, IDCANCEL);
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
			InvalidateRect(m_buy_btn_rect);
			InvalidateRect(m_sell_btn_rect);
			return 0;
		}
	}
	else if (message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		// 删除按钮：无消息映射，直接在 WindowProc 处理
		if (LOWORD(wParam) == IDC_TRADE_BTN_DELETE)
		{
			m_result = RES_DELETE;
			EndDialog(RES_DELETE);
			return TRUE;
		}
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

		// 2. 标签文字（行 y 取各输入框实际位置，标签基线略上移）
		Gdiplus::Font labelFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(12)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush labelBrush(Gdiplus::Color(255, 203, 213, 225));

		auto editTopY = [&](CWnd& edit) -> int {
			CRect er;
			if (!edit.GetSafeHwnd()) return 0;
			edit.GetWindowRect(er);
			ScreenToClient(er);
			return er.top;
		};
		const int dirY = m_buy_btn_rect.top;
		CRect dateRc, amountRc, priceRc;
		int dateY = 0, amountY = 0, priceY = 0;
		if (m_date_edit.GetSafeHwnd())
		{
			m_date_edit.GetWindowRect(dateRc);
			ScreenToClient(dateRc);
			dateY = dateRc.top;
		}
		if (m_amount_edit.GetSafeHwnd())
		{
			m_amount_edit.GetWindowRect(amountRc);
			ScreenToClient(amountRc);
			amountY = amountRc.top;
		}
		if (m_price_edit.GetSafeHwnd())
		{
			m_price_edit.GetWindowRect(priceRc);
			ScreenToClient(priceRc);
			priceY = priceRc.top;
		}

		g.DrawString(L"方向:", -1, &labelFont, Gdiplus::PointF(static_cast<Gdiplus::REAL>(marginX), static_cast<Gdiplus::REAL>(dirY + g_data.DPI(3))), &labelBrush);
		g.DrawString(L"时间:", -1, &labelFont, Gdiplus::PointF(static_cast<Gdiplus::REAL>(marginX), static_cast<Gdiplus::REAL>(dateY + g_data.DPI(1))), &labelBrush);
		g.DrawString(L"数量(股):", -1, &labelFont, Gdiplus::PointF(static_cast<Gdiplus::REAL>(marginX), static_cast<Gdiplus::REAL>(amountY + g_data.DPI(1))), &labelBrush);
		g.DrawString(L"价格(元):", -1, &labelFont, Gdiplus::PointF(static_cast<Gdiplus::REAL>(marginX), static_cast<Gdiplus::REAL>(priceY + g_data.DPI(1))), &labelBrush);

		// 3. 输入框底色与边框（含焦点高亮），按各控件实际矩形绘制
		auto drawEdit = [&](CWnd& edit) {
			if (!edit.GetSafeHwnd()) return;
			CRect er;
			edit.GetWindowRect(er);
			ScreenToClient(er);

			Gdiplus::SolidBrush editBg(Gdiplus::Color(255, 13, 15, 21));
			g.FillRectangle(&editBg, er.left, er.top, er.Width(), er.Height());

			CWnd* pFocus = GetFocus();
			bool focused = (pFocus && pFocus->GetSafeHwnd() == edit.GetSafeHwnd());
			Gdiplus::Pen pen(focused ? Gdiplus::Color(255, 37, 99, 235) : Gdiplus::Color(255, 52, 58, 72), 1.0f);
			g.DrawRectangle(&pen, er.left, er.top, er.Width() - 1, er.Height() - 1);
		};
		drawEdit(m_date_edit);
		drawEdit(m_time_edit);
		drawEdit(m_amount_edit);
		drawEdit(m_price_edit);

		DrawDirectionButtons(g);

		dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(pOldBmp);
		return 0;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

void CDarkTradeEditDlg::DrawDirectionButtons(Gdiplus::Graphics& g)
{
	auto drawDir = [&](const CRect& rect, bool isBuy) {
		bool isSel = (m_is_sell != isBuy);
		Gdiplus::Color bgCol = isSel ? (isBuy ? Gdiplus::Color(255, 246, 70, 93) : Gdiplus::Color(255, 14, 203, 129))
			: Gdiplus::Color(255, 24, 27, 34);
		Gdiplus::SolidBrush bgBrush(bgCol);
		g.FillRectangle(&bgBrush, rect.left, rect.top, rect.Width(), rect.Height());

		Gdiplus::Color borderCol = isSel ? bgCol : Gdiplus::Color(255, 52, 58, 72);
		Gdiplus::Pen borderPen(borderCol, 1.0f);
		g.DrawRectangle(&borderPen, rect.left, rect.top, rect.Width() - 1, rect.Height() - 1);

		Gdiplus::Font dirFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(11)),
			isSel ? Gdiplus::FontStyleBold : Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush txtBrush(isSel ? Gdiplus::Color(255, 255, 255, 255) : Gdiplus::Color(255, 148, 163, 184));
		Gdiplus::StringFormat sf;
		sf.SetAlignment(Gdiplus::StringAlignmentCenter);
		sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
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

	m_date_text = strDate.GetString();
	m_time_text = strTime.GetString();
	m_amount = amt;
	m_price = price;
	return true;
}

void CDarkTradeEditDlg::OnOK()
{
	if (!ValidateAndFill())
		return; // 校验失败留在弹窗
	m_result = RES_OK;
	CDialog::OnOK();
}
