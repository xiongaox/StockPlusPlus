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

namespace
{
	const COLORREF kTradeDlgBg = RGB(18, 20, 26);        // 弹窗底色
	const COLORREF kTradeDlgEditBg = RGB(13, 15, 21);    // 输入框底色
	const int kTradeDlgRadius = 5;                       // 圆角半径（逻辑像素）

	// 圆角矩形路径：GDI+ 无直接圆角 API，用四段弧拼
	void BuildRoundRectPath(Gdiplus::GraphicsPath& path, const CRect& rect, int radius)
	{
		const Gdiplus::REAL r = static_cast<Gdiplus::REAL>(max(0, radius));
		const Gdiplus::REAL l = static_cast<Gdiplus::REAL>(rect.left);
		const Gdiplus::REAL t = static_cast<Gdiplus::REAL>(rect.top);
		const Gdiplus::REAL w = static_cast<Gdiplus::REAL>(rect.Width());
		const Gdiplus::REAL h = static_cast<Gdiplus::REAL>(rect.Height());
		path.Reset();
		if (r <= 0.5f)
		{
			path.AddRectangle(Gdiplus::RectF(l, t, w, h));
			return;
		}
		const Gdiplus::REAL rr = min(r, min(w, h) / 2);
		path.AddArc(l, t, rr * 2, rr * 2, 180.0f, 90.0f);
		path.AddArc(l + w - rr * 2, t, rr * 2, rr * 2, 270.0f, 90.0f);
		path.AddArc(l + w - rr * 2, t + h - rr * 2, rr * 2, rr * 2, 0.0f, 90.0f);
		path.AddArc(l, t + h - rr * 2, rr * 2, rr * 2, 90.0f, 90.0f);
		path.CloseFigure();
	}

	// 圆角实底填充
	void FillRoundRect(Gdiplus::Graphics& g, const CRect& rect, const Gdiplus::Color& color, int radius = kTradeDlgRadius)
	{
		Gdiplus::GraphicsPath path;
		BuildRoundRectPath(path, rect, g_data.DPI(radius));
		Gdiplus::SolidBrush brush(color);
		g.FillPath(&brush, &path);
	}

	// 圆角描边。GDI+ 1px 笔会跨半像素抗锯齿把颜色溢到矩形外，故描边矩形整体内缩 1px，
	// 既避免溢出，也保证描边完全落在控件矩形内
	void DrawRoundRectOutline(Gdiplus::Graphics& g, const CRect& rect, const Gdiplus::Color& color, int radius = kTradeDlgRadius)
	{
		CRect inner = rect;
		inner.DeflateRect(1, 1);
		if (inner.IsRectEmpty()) return;
		Gdiplus::GraphicsPath path;
		BuildRoundRectPath(path, inner, max(1, g_data.DPI(radius) - 1));
		Gdiplus::Pen pen(color, 1.0f);
		g.DrawPath(&pen, &path);
	}

	// 标签右对齐绘制，使各行标签尾部成一列
	void DrawRightAlignedLabel(Gdiplus::Graphics& g, const wchar_t* text, Gdiplus::Font& font,
		const CRect& cell, const Gdiplus::SolidBrush& brush)
	{
		Gdiplus::StringFormat fmt;
		fmt.SetAlignment(Gdiplus::StringAlignmentFar);
		fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		fmt.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
		Gdiplus::RectF rf(static_cast<Gdiplus::REAL>(cell.left), static_cast<Gdiplus::REAL>(cell.top),
			static_cast<Gdiplus::REAL>(cell.Width()), static_cast<Gdiplus::REAL>(cell.Height()));
		g.DrawString(text, -1, &font, rf, &fmt, &brush);
	}
}

BOOL CDarkTradeEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// 标题带上代码与名称，便于在同名标的间区分
	if (m_name.empty())
		SetWindowText(_T("成交录入"));
	else
	{
		CString title;
		title.Format(_T("%s %s · 成交录入"), CString(m_code.c_str()), CString(m_name.c_str()));
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
	const int marginX = g_data.DPI(16);
	const int labelW = g_data.DPI(80);
	const int editH = g_data.DPI(26);
	const int rowPitch = g_data.DPI(38);
	const int btnH = g_data.DPI(28);
	const int btnW = g_data.DPI(64);
	const int contentW = g_data.DPI(200);     // 输入区固定宽度，日期/时间同排也够用

	// 行序：方向 / 日期+时间 / 数量 / 价格
	const int topPad = g_data.DPI(16);
	const int dirY = topPad;
	const int dateTimeY = dirY + rowPitch;
	const int amountY = dateTimeY + rowPitch;
	const int priceY = amountY + rowPitch;
	const int btnY = priceY + rowPitch + g_data.DPI(4);

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

	const int contentLeft = marginX + labelW;
	const int contentRight = contentLeft + contentW;

	// 方向：两枚按钮等宽铺满内容列，形似分段开关（比两个小按钮浮在左侧更整齐）
	const int dirGap = g_data.DPI(8);
	const int dirBtnW = (contentRight - contentLeft - dirGap) / 2;
	m_buy_btn_rect = CRect(contentLeft, dirY, contentLeft + dirBtnW, dirY + editH);
	m_sell_btn_rect = CRect(contentLeft + dirBtnW + dirGap, dirY, contentRight, dirY + editH);

	auto createEdit = [&](CEdit& edit, int rowY, int left, int right, UINT id, const wchar_t* cue) {
		edit.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
			CRect(left, rowY, right, rowY + editH), this, id);
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

	// 底部按钮：确定/取消靠右成组（右边界与输入框对齐），编辑模式在左侧插入删除按钮
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
			CRect(contentLeft, btnY, contentLeft + btnW, btnY + btnH), this, IDC_TRADE_BTN_DELETE);
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
			// 两个按钮整体一起失效（含各自外扩 1px 的余量），确保切换后无旧色残留
			CRect dirty = m_buy_btn_rect;
			dirty.UnionRect(dirty, m_sell_btn_rect);
			dirty.InflateRect(1, 1);
			InvalidateRect(dirty);
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

			// 圆角底：先铺弹窗底色填掉直角，再落圆角；否则四角会留下方形色块
			dc.FillSolidRect(rect, kTradeDlgBg);
			{
				Gdiplus::Graphics g(dc.GetSafeHdc());
				g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
				FillRoundRect(g, rect, Gdiplus::Color(255, GetRValue(bgColor), GetGValue(bgColor), GetBValue(bgColor)));
			}

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
		const int labelW = g_data.DPI(80);

		// 2. 标签：单位直接写进标签（与「设置持仓信息」弹窗同一约定），令输入框右边界整齐
		Gdiplus::Font labelFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(12)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush labelBrush(Gdiplus::Color(255, 203, 213, 225));

		auto editRectInClient = [&](CWnd& edit) -> CRect {
			CRect er(0, 0, 0, 0);
			if (!edit.GetSafeHwnd()) return er;
			edit.GetWindowRect(er);
			ScreenToClient(er);
			return er;
		};

		// 标签在各自行内垂直居中：以对应输入框（或方向按钮）的垂直中线为基准
		auto drawRowLabel = [&](const wchar_t* text, const CRect& anchorRow) {
			CRect cell(marginX, anchorRow.top, marginX + labelW - g_data.DPI(10), anchorRow.bottom);
			DrawRightAlignedLabel(g, text, labelFont, cell, labelBrush);
		};

		CRect dateRc = editRectInClient(m_date_edit);
		CRect amountRc = editRectInClient(m_amount_edit);
		CRect priceRc = editRectInClient(m_price_edit);

		// 日期与时间同排，故只用一行标签统辖两个框
		drawRowLabel(L"方向", m_buy_btn_rect);
		drawRowLabel(L"日期/时间", dateRc);
		drawRowLabel(L"数量 (股)", amountRc);
		drawRowLabel(L"价格 (元)", priceRc);

		// 3. 输入框：圆角实底 + 描边（含焦点高亮）。注意输入框是子窗口，圆角外的底色
		//    会露出弹窗底色，故先铺满弹窗底色再落圆角底
		auto drawEdit = [&](CWnd& edit) {
			CRect er = editRectInClient(edit);
			if (er.IsRectEmpty()) return;

			CWnd* pFocus = GetFocus();
			bool focused = (pFocus && pFocus->GetSafeHwnd() == edit.GetSafeHwnd());
			FillRoundRect(g, er, Gdiplus::Color(255, 13, 15, 21));
			DrawRoundRectOutline(g, er, focused ? Gdiplus::Color(255, 37, 99, 235) : Gdiplus::Color(255, 52, 58, 72));
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
	// 分段开关样式：选中侧语义色实底，未选中侧深底 + 灰描边
	auto drawDir = [&](const CRect& rect, bool isBuy) {
		bool isSel = (m_is_sell != isBuy);
		Gdiplus::Color bgCol = isSel
			? (isBuy ? Gdiplus::Color(255, 246, 70, 93) : Gdiplus::Color(255, 14, 203, 129))
			: Gdiplus::Color(255, 24, 27, 34);
		FillRoundRect(g, rect, bgCol);

		if (!isSel)
			DrawRoundRectOutline(g, rect, Gdiplus::Color(255, 52, 58, 72));

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
