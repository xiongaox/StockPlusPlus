#include "pch.h"
#include "Stock.h"
#include "ManagerDialog.h"
#include "resource.h"
#include "Common.h"
#include "ChartColors.h"
#include "Icons/Icons.h"

// ===================== 暗色主题自绘辅助 =====================

BEGIN_MESSAGE_MAP(CFlatHeaderCtrl, CHeaderCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CFlatHeaderCtrl::OnCustomDraw)
END_MESSAGE_MAP()

void CFlatHeaderCtrl::OnPaint()
{
	CPaintDC dc(this);
	CRect clientRect;
	GetClientRect(&clientRect);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

	// 填充全表头深色底 (#1C1F27)，彻底消除右侧白色占位
	memDC.FillSolidRect(&clientRect, RGB(28, 31, 39));
	memDC.FillSolidRect(CRect(clientRect.left, clientRect.bottom - 1, clientRect.right, clientRect.bottom), COLOR_DARK_GRAY_BORDER);

	int itemCount = GetItemCount();
	CFont* pFont = GetFont();
	if (pFont == nullptr || pFont->GetSafeHandle() == nullptr)
		pFont = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
	CFont* pOldFont = memDC.SelectObject(pFont);
	memDC.SetBkMode(TRANSPARENT);
	memDC.SetTextColor(COLOR_TEXT_MUTED);

	for (int i = 0; i < itemCount; ++i)
	{
		CRect itemRect;
		GetItemRect(i, &itemRect);

		// 右侧细分割线
		memDC.FillSolidRect(CRect(itemRect.right - 1, itemRect.top + g_data.DPI(4), itemRect.right, itemRect.bottom - g_data.DPI(4)), COLOR_DARK_GRAY_BORDER);

		wchar_t buf[128] = { 0 };
		HDITEM it = { 0 };
		it.mask = HDI_TEXT | HDI_FORMAT;
		it.pszText = buf;
		it.cchTextMax = 128;
		if (GetItem(i, &it))
		{
			CRect textRc = itemRect;
			textRc.DeflateRect(g_data.DPI(6), 0);

			UINT dtFlags = DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;
			if ((it.fmt & HDF_JUSTIFYMASK) == HDF_RIGHT)
				dtFlags |= DT_RIGHT;
			else if ((it.fmt & HDF_JUSTIFYMASK) == HDF_CENTER)
				dtFlags |= DT_CENTER;
			else
				dtFlags |= DT_LEFT;

			memDC.DrawText(buf, -1, &textRc, dtFlags);
		}
	}

	memDC.SelectObject(pOldFont);
	dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

// 平面化表头：深色底 + 细分隔线 + 灰色文字，与浮动窗表面体系一致
void CFlatHeaderCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMCUSTOMDRAW* pCD = reinterpret_cast<NMCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	if (pCD->dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pCD->dwDrawStage == CDDS_ITEMPREPAINT)
	{
		CDC dc;
		dc.Attach(pCD->hdc);
		CRect rc(pCD->rc);

		dc.FillSolidRect(rc, RGB(28, 31, 39));
		dc.FillSolidRect(CRect(rc.left, rc.bottom - 1, rc.right, rc.bottom), COLOR_DARK_GRAY_BORDER);
		dc.FillSolidRect(CRect(rc.right - 1, rc.top + 4, rc.right, rc.bottom - 4), COLOR_DARK_GRAY_BORDER);

		wchar_t buf[128] = { 0 };
		HDITEM it = { 0 };
		it.mask = HDI_TEXT | HDI_FORMAT;
		it.pszText = buf;
		it.cchTextMax = 128;
		if (GetItem(static_cast<int>(pCD->dwItemSpec), &it))
		{
			CFont* pFont = GetFont();
			if (pFont == nullptr || pFont->GetSafeHandle() == nullptr)
				pFont = CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
			CFont* pOldFont = dc.SelectObject(pFont);
			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(COLOR_TEXT_MUTED);
			CRect textRc = rc;
			textRc.DeflateRect(g_data.DPI(6), 0);

			UINT dtFlags = DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;
			if ((it.fmt & HDF_JUSTIFYMASK) == HDF_RIGHT)
				dtFlags |= DT_RIGHT;
			else if ((it.fmt & HDF_JUSTIFYMASK) == HDF_CENTER)
				dtFlags |= DT_CENTER;
			else
				dtFlags |= DT_LEFT;

			CString headerText(buf);
			dc.DrawText(headerText, textRc, dtFlags);
			dc.SelectObject(pOldFont);
		}

		dc.Detach();
		*pResult = CDRF_SKIPDEFAULT;
	}
}

// ===================== CDarkPopupMenu 暗色风格弹出菜单 =====================

BEGIN_MESSAGE_MAP(CDarkPopupMenu, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BOOL CDarkPopupMenu::CreatePopup(CWnd* pParent)
{
	CString className = AfxRegisterWndClass(CS_DROPSHADOW | CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS, ::LoadCursor(nullptr, IDC_ARROW), (HBRUSH)::GetStockObject(BLACK_BRUSH), nullptr);
	return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, className, _T("DarkPopupMenu"), WS_POPUP, CRect(0, 0, 0, 0), pParent, 0);
}

int CDarkPopupMenu::TrackMenu(const CPoint& screenPt, const std::vector<MenuItem>& items, int minWidth)
{
	m_items = items;
	m_selected_id = 0;
	m_hover_idx = -1;
	m_is_open = true;

	if (m_items.empty() || !GetSafeHwnd())
		return 0;

	CDC* pDC = GetDC();
	Gdiplus::Graphics g(pDC->GetSafeHdc());
	Gdiplus::Font font(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(10)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	int maxTextW = 0;
	for (const auto& item : m_items)
	{
		if (!item.isSeparator)
		{
			Gdiplus::RectF bounds;
			g.MeasureString(item.text.c_str(), -1, &font, Gdiplus::PointF(0, 0), &bounds);
			maxTextW = max(maxTextW, static_cast<int>(bounds.Width));
		}
	}
	ReleaseDC(pDC);

	int padX = g_data.DPI(32);
	int menuW = max(minWidth, maxTextW + padX + g_data.DPI(16));

	int itemH = g_data.DPI(28);
	int sepH = g_data.DPI(7);
	int padY = g_data.DPI(4);

	int totalH = padY * 2;
	for (const auto& item : m_items)
	{
		totalH += item.isSeparator ? sepH : itemH;
	}

	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);
	int x = screenPt.x;
	int y = screenPt.y;
	if (x + menuW > screenW) x = screenW - menuW - g_data.DPI(4);
	if (y + totalH > screenH) y = screenPt.y - totalH;
	if (x < 0) x = 0;
	if (y < 0) y = 0;

	SetWindowPos(&wndTopMost, x, y, menuW, totalH, SWP_SHOWWINDOW);
	SetCapture();

	MSG msg;
	while (m_is_open && ::GetMessage(&msg, nullptr, 0, 0))
	{
		if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_NCLBUTTONDOWN)
		{
			// 只有点击落在菜单窗口客户区内才交给 OnLButtonUp 按 hover 项处理，
			// 其余情况（点击其他窗口，或 SetCapture 捕获到的菜单外点击）视为在
			// 菜单外，关闭菜单。（不能用 msg.pt 判断：PostMessage 合成消息的 pt 不可靠）
			bool insidePopup = false;
			if (msg.hwnd == GetSafeHwnd())
			{
				CPoint pt((DWORD)msg.lParam);
				CRect rcClient;
				GetClientRect(&rcClient);
				insidePopup = rcClient.PtInRect(pt);
			}
			if (!insidePopup)
			{
				m_is_open = false;
				break;
			}
		}
		else if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
		{
			m_is_open = false;
			break;
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	if (GetCapture() == this)
		ReleaseCapture();

	ShowWindow(SW_HIDE);
	DestroyWindow();
	return m_selected_id;
}

void CDarkPopupMenu::OnPaint()
{
	CPaintDC dc(this);
	CRect clientRect;
	GetClientRect(clientRect);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

	Gdiplus::Graphics g(memDC.GetSafeHdc());
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

	Gdiplus::SolidBrush bg(Gdiplus::Color(255, 24, 27, 34)); // #181B22
	g.FillRectangle(&bg, 0, 0, clientRect.Width(), clientRect.Height());
	Gdiplus::Pen border(Gdiplus::Color(255, 42, 48, 63), 1.0f); // #2A303F
	g.DrawRectangle(&border, 0, 0, clientRect.Width() - 1, clientRect.Height() - 1);

	int padY = g_data.DPI(4);
	int curY = padY;
	int itemH = g_data.DPI(28);
	int sepH = g_data.DPI(7);

	Gdiplus::Font font(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(10)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::Font checkFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(10)), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);

	Gdiplus::StringFormat sf;
	sf.SetAlignment(Gdiplus::StringAlignmentNear);
	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	for (size_t i = 0; i < m_items.size(); ++i)
	{
		const auto& item = m_items[i];
		if (item.isSeparator)
		{
			int sepLineY = curY + sepH / 2;
			Gdiplus::Pen sepPen(Gdiplus::Color(255, 38, 42, 54), 1.0f);
			g.DrawLine(&sepPen, g_data.DPI(8), sepLineY, clientRect.Width() - g_data.DPI(8), sepLineY);
			curY += sepH;
		}
		else
		{
			CRect itemRc(g_data.DPI(3), curY, clientRect.Width() - g_data.DPI(3), curY + itemH);
			bool isHover = (static_cast<int>(i) == m_hover_idx);

			if (isHover)
			{
				Gdiplus::SolidBrush hoverBg(Gdiplus::Color(255, 37, 99, 235)); // #2563EB
				g.FillRectangle(&hoverBg, itemRc.left, itemRc.top, itemRc.Width(), itemRc.Height());
			}

			// 勾选标识 ✓
			if (item.isChecked)
			{
				Gdiplus::SolidBrush checkBrush(isHover ? Gdiplus::Color(255, 255, 255, 255) : Gdiplus::Color(255, 59, 130, 246));
				Gdiplus::RectF checkRf(static_cast<Gdiplus::REAL>(itemRc.left + g_data.DPI(4)), static_cast<Gdiplus::REAL>(itemRc.top), static_cast<Gdiplus::REAL>(g_data.DPI(16)), static_cast<Gdiplus::REAL>(itemRc.Height()));
				Gdiplus::StringFormat checkSf;
				checkSf.SetAlignment(Gdiplus::StringAlignmentCenter);
				checkSf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
				Icons::Draw(g, Icons::Id::Check, checkRf, isHover ? RGB(255, 255, 255) : RGB(59, 130, 246));
			}

			// 文本
			Gdiplus::Color textCol = isHover ? Gdiplus::Color(255, 255, 255, 255) :
				(item.isDestructive ? Gdiplus::Color(255, 239, 68, 68) : Gdiplus::Color(255, 226, 232, 240));
			Gdiplus::SolidBrush textBrush(textCol);

			int textLeft = itemRc.left + g_data.DPI(22);
			Gdiplus::RectF textRf(static_cast<Gdiplus::REAL>(textLeft), static_cast<Gdiplus::REAL>(itemRc.top), static_cast<Gdiplus::REAL>(itemRc.right - textLeft - g_data.DPI(6)), static_cast<Gdiplus::REAL>(itemRc.Height()));
			g.DrawString(item.text.c_str(), -1, &font, textRf, &sf, &textBrush);

			curY += itemH;
		}
	}

	dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

void CDarkPopupMenu::OnMouseMove(UINT nFlags, CPoint point)
{
	int padY = g_data.DPI(4);
	int curY = padY;
	int itemH = g_data.DPI(28);
	int sepH = g_data.DPI(7);

	int oldHover = m_hover_idx;
	m_hover_idx = -1;

	for (size_t i = 0; i < m_items.size(); ++i)
	{
		if (m_items[i].isSeparator)
		{
			curY += sepH;
		}
		else
		{
			CRect itemRc(0, curY, 10000, curY + itemH);
			if (point.y >= itemRc.top && point.y < itemRc.bottom)
			{
				m_hover_idx = static_cast<int>(i);
				break;
			}
			curY += itemH;
		}
	}

	if (oldHover != m_hover_idx)
		Invalidate();

	CWnd::OnMouseMove(nFlags, point);
}

void CDarkPopupMenu::OnMouseLeave()
{
	m_hover_idx = -1;
	Invalidate();
	CWnd::OnMouseLeave();
}

void CDarkPopupMenu::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_hover_idx >= 0 && m_hover_idx < static_cast<int>(m_items.size()))
	{
		if (!m_items[m_hover_idx].isSeparator)
		{
			m_selected_id = m_items[m_hover_idx].id;
			m_is_open = false;
			return;
		}
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void CDarkPopupMenu::OnKillFocus(CWnd* pNewWnd)
{
	m_is_open = false;
	CWnd::OnKillFocus(pNewWnd);
}

// ===================== CSearchResultDropdown 搜索结果与分组添加一体化暗色浮窗 =====================

BEGIN_MESSAGE_MAP(CSearchResultDropdown, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

BOOL CSearchResultDropdown::CreatePopup(CWnd* pParent)
{
	CString className = AfxRegisterWndClass(CS_DROPSHADOW | CS_HREDRAW | CS_VREDRAW, ::LoadCursor(nullptr, IDC_ARROW), (HBRUSH)::GetStockObject(BLACK_BRUSH), nullptr);
	return CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, className, L"", WS_POPUP, CRect(0, 0, 0, 0), pParent, 0);
}

int CSearchResultDropdown::GetResultsWidth() const
{
	return g_data.DPI(225);
}

int CSearchResultDropdown::GetMenuWidth() const
{
	return g_data.DPI(155);
}

void CSearchResultDropdown::ShowResults(const std::vector<StockSearchResult>& results, const CRect& editScreenRc, const std::vector<GroupMenuItem>& groupItems)
{
	m_results = results;
	m_edit_screen_rc = editScreenRc;
	m_group_items = groupItems;
	m_hover_item = -1;
	m_hover_btn = -1;
	m_hover_group_idx = -1;
	m_selected_stock_idx = -1;

	if (m_results.empty())
	{
		HidePopup();
		return;
	}

	UpdatePopupPosition();
}

void CSearchResultDropdown::HidePopup()
{
	m_selected_stock_idx = -1;
	m_hover_item = -1;
	m_hover_btn = -1;
	m_hover_group_idx = -1;
	if (GetSafeHwnd())
		ShowWindow(SW_HIDE);
}

void CSearchResultDropdown::UpdatePopupPosition()
{
	if (m_results.empty() || !GetSafeHwnd())
	{
		ShowWindow(SW_HIDE);
		return;
	}

	int resW = GetResultsWidth();
	int menuW = GetMenuWidth();
	int totalW = (m_selected_stock_idx >= 0) ? (resW + menuW) : resW;

	int maxItems = min(static_cast<int>(m_results.size()), 8);
	int itemH = g_data.DPI(34);
	int searchH = maxItems * itemH + 2;

	int menuH = 0;
	if (m_selected_stock_idx >= 0)
	{
		menuH = static_cast<int>(m_group_items.size()) * g_data.DPI(30) + g_data.DPI(28);
	}
	int totalH = max(searchH, menuH);

	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);

	int dropX = m_edit_screen_rc.right - resW;
	if (m_selected_stock_idx >= 0)
	{
		if (dropX + totalW > screenW - 10)
		{
			dropX = screenW - totalW - 10;
		}
	}
	if (dropX < 10) dropX = 10;

	int dropY = m_edit_screen_rc.bottom + 2;
	if (dropY + totalH > screenH - 10)
	{
		dropY = m_edit_screen_rc.top - totalH - 2;
	}
	if (dropY < 10) dropY = 10;

	SetWindowPos(&CWnd::wndTopMost, dropX, dropY, totalW, totalH, SWP_SHOWWINDOW | SWP_NOACTIVATE);
	Invalidate();
}

void CSearchResultDropdown::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);
	if (rc.Width() <= 0 || rc.Height() <= 0) return;

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

	Gdiplus::Graphics g(memDC.GetSafeHdc());
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

	// 1. 全局深色底与外边框
	Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 20, 22, 29));
	g.FillRectangle(&bgBrush, 0, 0, rc.Width(), rc.Height());

	Gdiplus::Pen borderPen(Gdiplus::Color(255, 59, 130, 246), 1.0f);
	g.DrawRectangle(&borderPen, 0.5f, 0.5f, static_cast<Gdiplus::REAL>(rc.Width() - 1), static_cast<Gdiplus::REAL>(rc.Height() - 1));

	int resW = (m_selected_stock_idx >= 0) ? GetResultsWidth() : rc.Width();
	int itemH = g_data.DPI(34);

	Gdiplus::Font nameFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(10.5)), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	Gdiplus::Font codeFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(10)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::Font badgeFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(8.5)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::Font plusFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(12)), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);

	Gdiplus::StringFormat sfNear;
	sfNear.SetAlignment(Gdiplus::StringAlignmentNear);
	sfNear.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	sfNear.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
	sfNear.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);

	Gdiplus::StringFormat sfCenter;
	sfCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
	sfCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	sfCenter.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

	// 2. 绘制左侧搜索结果项
	int maxItems = min(static_cast<int>(m_results.size()), 8);
	for (int i = 0; i < maxItems; ++i)
	{
		int itemY = i * itemH + 1;
		CRect itemRc(1, itemY, resW - 1, itemY + itemH);

		bool isSelected = (m_selected_stock_idx == i);
		bool isHover = (m_hover_item == i);

		if (isSelected)
		{
			Gdiplus::SolidBrush selBrush(Gdiplus::Color(255, 37, 99, 235));
			g.FillRectangle(&selBrush, static_cast<Gdiplus::REAL>(itemRc.left), static_cast<Gdiplus::REAL>(itemRc.top), static_cast<Gdiplus::REAL>(itemRc.Width()), static_cast<Gdiplus::REAL>(itemRc.Height()));
		}
		else if (isHover)
		{
			Gdiplus::SolidBrush hoverBrush(Gdiplus::Color(255, 30, 41, 59));
			g.FillRectangle(&hoverBrush, static_cast<Gdiplus::REAL>(itemRc.left), static_cast<Gdiplus::REAL>(itemRc.top), static_cast<Gdiplus::REAL>(itemRc.Width()), static_cast<Gdiplus::REAL>(itemRc.Height()));
		}

		// 分隔线
		if (i > 0)
		{
			Gdiplus::Pen divPen(Gdiplus::Color(255, 38, 42, 54), 1.0f);
			g.DrawLine(&divPen, static_cast<Gdiplus::REAL>(itemRc.left + g_data.DPI(6)), static_cast<Gdiplus::REAL>(itemRc.top), static_cast<Gdiplus::REAL>(itemRc.right - g_data.DPI(6)), static_cast<Gdiplus::REAL>(itemRc.top));
		}

		const auto& stock = m_results[i];

		// 交易所徽标
		std::wstring exch = stock.exchange;
		if (exch.empty()) exch = CCommon::GetExchangeName(stock.fullCode);
		std::wstring pureCode = stock.code;
		if (pureCode.empty()) pureCode = CCommon::GetPureCode(stock.fullCode);

		int badgeW = g_data.DPI(38);
		int badgeH = g_data.DPI(16);
		int badgeX = itemRc.left + g_data.DPI(6);
		int badgeY = itemRc.top + (itemRc.Height() - badgeH) / 2;
		Gdiplus::RectF badgeRf(static_cast<Gdiplus::REAL>(badgeX), static_cast<Gdiplus::REAL>(badgeY), static_cast<Gdiplus::REAL>(badgeW), static_cast<Gdiplus::REAL>(badgeH));

		Gdiplus::SolidBrush badgeBg(isSelected ? Gdiplus::Color(255, 30, 41, 59) : Gdiplus::Color(255, 37, 99, 235));
		g.FillRectangle(&badgeBg, badgeRf);

		Gdiplus::SolidBrush badgeTxt(Gdiplus::Color(255, 255, 255, 255));
		// GDI+ 行框居中含雅黑 descent 空白区，汉字视觉偏上，文字矩形下移补偿
		Gdiplus::RectF badgeTxtRf = badgeRf;
		badgeTxtRf.Y += static_cast<Gdiplus::REAL>(g_data.DPI(1));
		g.DrawString(exch.c_str(), -1, &badgeFont, badgeTxtRf, &sfCenter, &badgeTxt);

		// 代码
		int codeX = badgeX + badgeW + g_data.DPI(5);
		int codeW = g_data.DPI(46);
		Gdiplus::RectF codeRf(static_cast<Gdiplus::REAL>(codeX), static_cast<Gdiplus::REAL>(itemRc.top), static_cast<Gdiplus::REAL>(codeW), static_cast<Gdiplus::REAL>(itemRc.Height()));
		Gdiplus::SolidBrush codeTxt(isSelected ? Gdiplus::Color(255, 226, 232, 240) : Gdiplus::Color(255, 148, 163, 184));
		g.DrawString(pureCode.c_str(), -1, &codeFont, codeRf, &sfNear, &codeTxt);

		// 加号按钮 [+]
		int btnSize = g_data.DPI(20);
		int btnX = itemRc.right - btnSize - g_data.DPI(6);
		int btnY = itemRc.top + (itemRc.Height() - btnSize) / 2;
		Gdiplus::RectF btnRf(static_cast<Gdiplus::REAL>(btnX), static_cast<Gdiplus::REAL>(btnY), static_cast<Gdiplus::REAL>(btnSize), static_cast<Gdiplus::REAL>(btnSize));

		bool isBtnHover = (m_hover_btn == i) || isSelected;
		Gdiplus::SolidBrush plusBg(isBtnHover ? (isSelected ? Gdiplus::Color(255, 255, 255, 255) : Gdiplus::Color(255, 37, 99, 235)) : Gdiplus::Color(255, 30, 41, 59));
		g.FillRectangle(&plusBg, btnRf);

		Gdiplus::Pen plusBorder(isBtnHover ? Gdiplus::Color(255, 96, 165, 250) : Gdiplus::Color(255, 71, 85, 105), 1.0f);
		g.DrawRectangle(&plusBorder, btnRf);

		Gdiplus::SolidBrush plusTxt(isBtnHover ? (isSelected ? Gdiplus::Color(255, 37, 99, 235) : Gdiplus::Color(255, 255, 255, 255)) : Gdiplus::Color(255, 148, 163, 184));
		Icons::Draw(g, isSelected ? Icons::Id::Check : Icons::Id::Plus, btnRf,
			isBtnHover ? (isSelected ? RGB(37, 99, 235) : RGB(255, 255, 255)) : RGB(148, 163, 184));

		// 名称
		int nameX = codeX + codeW + g_data.DPI(5);
		int nameW = max(10, btnX - nameX - g_data.DPI(4));
		Gdiplus::RectF nameRf(static_cast<Gdiplus::REAL>(nameX), static_cast<Gdiplus::REAL>(itemRc.top), static_cast<Gdiplus::REAL>(nameW), static_cast<Gdiplus::REAL>(itemRc.Height()));
		Gdiplus::SolidBrush nameTxt(Gdiplus::Color(255, 255, 255, 255));
		g.DrawString(stock.name.c_str(), -1, &nameFont, nameRf, &sfNear, &nameTxt);
	}

	// 3. 绘制右侧分组选择面板
	if (m_selected_stock_idx >= 0)
	{
		// 垂直分割线
		Gdiplus::Pen vSepPen(Gdiplus::Color(255, 38, 42, 54), 1.0f);
		g.DrawLine(&vSepPen, static_cast<Gdiplus::REAL>(resW), 0.0f, static_cast<Gdiplus::REAL>(resW), static_cast<Gdiplus::REAL>(rc.Height()));

		// 右侧背景
		Gdiplus::SolidBrush menuBg(Gdiplus::Color(255, 16, 18, 24));
		g.FillRectangle(&menuBg, static_cast<Gdiplus::REAL>(resW + 1), 1.0f, static_cast<Gdiplus::REAL>(rc.Width() - resW - 2), static_cast<Gdiplus::REAL>(rc.Height() - 2));

		int curY = g_data.DPI(6);
		// 标题
		Gdiplus::Font titleFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(8.5)), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
		Gdiplus::SolidBrush titleBrush(Gdiplus::Color(255, 148, 163, 184));
		g.DrawString(L"添加到目标分组：", -1, &titleFont, Gdiplus::PointF(static_cast<Gdiplus::REAL>(resW + g_data.DPI(10)), static_cast<Gdiplus::REAL>(curY)), &titleBrush);

		curY += g_data.DPI(18);

		int grpH = g_data.DPI(28);
		int grpSepH = g_data.DPI(7);

		Gdiplus::Font grpFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(9.5)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::Font actFont(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(9.5)), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);

		for (size_t j = 0; j < m_group_items.size(); ++j)
		{
			const auto& it = m_group_items[j];
			if (it.isSeparator)
			{
				Gdiplus::Pen sepPen(Gdiplus::Color(255, 38, 42, 54), 1.0f);
				int lineY = curY + grpSepH / 2;
				g.DrawLine(&sepPen, static_cast<Gdiplus::REAL>(resW + g_data.DPI(8)), static_cast<Gdiplus::REAL>(lineY),
					static_cast<Gdiplus::REAL>(rc.right - g_data.DPI(8)), static_cast<Gdiplus::REAL>(lineY));
				curY += grpSepH;
			}
			else
			{
				CRect itemRc(resW + 1, curY, rc.right - 1, curY + grpH);
				bool isHover = (m_hover_group_idx == static_cast<int>(j));

				if (isHover)
				{
					Gdiplus::SolidBrush hoverBrush(Gdiplus::Color(255, 30, 41, 59));
					g.FillRectangle(&hoverBrush, static_cast<Gdiplus::REAL>(itemRc.left), static_cast<Gdiplus::REAL>(itemRc.top),
						static_cast<Gdiplus::REAL>(itemRc.Width()), static_cast<Gdiplus::REAL>(itemRc.Height()));
				}

				if (it.isAction)
				{
					Gdiplus::RectF txtRf(static_cast<Gdiplus::REAL>(itemRc.left + g_data.DPI(10)), static_cast<Gdiplus::REAL>(itemRc.top),
						static_cast<Gdiplus::REAL>(itemRc.Width() - g_data.DPI(14)), static_cast<Gdiplus::REAL>(itemRc.Height()));
					Gdiplus::SolidBrush actionTxt(isHover ? Gdiplus::Color(255, 147, 197, 253) : Gdiplus::Color(255, 96, 165, 250));
					g.DrawString(it.text.c_str(), -1, &actFont, txtRf, &sfNear, &actionTxt);
				}
				else
				{
					int dotSize = g_data.DPI(5);
					int dotX = itemRc.left + g_data.DPI(10);
					int dotY = itemRc.top + (itemRc.Height() - dotSize) / 2;
					Gdiplus::SolidBrush dotBrush(isHover ? Gdiplus::Color(255, 96, 165, 250) : Gdiplus::Color(255, 71, 85, 105));
					g.FillEllipse(&dotBrush, dotX, dotY, dotSize, dotSize);

					int txtLeft = dotX + dotSize + g_data.DPI(6);
					Gdiplus::RectF txtRf(static_cast<Gdiplus::REAL>(txtLeft), static_cast<Gdiplus::REAL>(itemRc.top),
						static_cast<Gdiplus::REAL>(itemRc.right - txtLeft - g_data.DPI(4)), static_cast<Gdiplus::REAL>(itemRc.Height()));
					Gdiplus::SolidBrush txtBrush(isHover ? Gdiplus::Color(255, 255, 255, 255) : Gdiplus::Color(255, 226, 232, 240));
					g.DrawString(it.text.c_str(), -1, &grpFont, txtRf, &sfNear, &txtBrush);
				}

				curY += grpH;
			}
		}
	}

	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

void CSearchResultDropdown::OnMouseMove(UINT nFlags, CPoint point)
{
	int resW = (m_selected_stock_idx >= 0) ? GetResultsWidth() : 99999;
	int oldHover = m_hover_item;
	int oldBtnHover = m_hover_btn;
	int oldGroupHover = m_hover_group_idx;

	if (point.x < resW)
	{
		int itemH = g_data.DPI(34);
		m_hover_item = point.y / itemH;
		if (m_hover_item < 0 || m_hover_item >= static_cast<int>(m_results.size()) || m_hover_item >= 8)
			m_hover_item = -1;

		int btnSize = g_data.DPI(20);
		int btnX = resW - btnSize - g_data.DPI(6);
		if (m_hover_item >= 0 && point.x >= btnX - g_data.DPI(4) && point.x <= resW)
			m_hover_btn = m_hover_item;
		else
			m_hover_btn = -1;

		m_hover_group_idx = -1;
	}
	else
	{
		m_hover_item = -1;
		m_hover_btn = -1;
		m_hover_group_idx = -1;

		int grpH = g_data.DPI(28);
		int grpSepH = g_data.DPI(7);
		int curY = g_data.DPI(24);

		for (size_t j = 0; j < m_group_items.size(); ++j)
		{
			if (m_group_items[j].isSeparator)
			{
				curY += grpSepH;
			}
			else
			{
				if (point.y >= curY && point.y < curY + grpH)
				{
					m_hover_group_idx = static_cast<int>(j);
					break;
				}
				curY += grpH;
			}
		}
	}

	if (m_hover_item != oldHover || m_hover_btn != oldBtnHover || m_hover_group_idx != oldGroupHover)
	{
		Invalidate();
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, GetSafeHwnd(), 0 };
		TrackMouseEvent(&tme);
	}
	CWnd::OnMouseMove(nFlags, point);
}

void CSearchResultDropdown::OnMouseLeave()
{
	m_hover_item = -1;
	m_hover_btn = -1;
	m_hover_group_idx = -1;
	Invalidate();
	CWnd::OnMouseLeave();
}

void CSearchResultDropdown::OnLButtonDown(UINT nFlags, CPoint point)
{
	int resW = (m_selected_stock_idx >= 0) ? GetResultsWidth() : 99999;

	if (point.x < resW)
	{
		int itemH = g_data.DPI(34);
		int clickedItem = point.y / itemH;
		if (clickedItem >= 0 && clickedItem < static_cast<int>(m_results.size()) && clickedItem < 8)
		{
			if (m_selected_stock_idx == clickedItem)
			{
				m_selected_stock_idx = -1; // 再次点击折叠
			}
			else
			{
				m_selected_stock_idx = clickedItem; // 展开分组选择面板
			}
			UpdatePopupPosition();
			return;
		}
	}
	else if (m_selected_stock_idx >= 0 && m_selected_stock_idx < static_cast<int>(m_results.size()))
	{
		int grpH = g_data.DPI(28);
		int grpSepH = g_data.DPI(7);
		int curY = g_data.DPI(24);

		for (size_t j = 0; j < m_group_items.size(); ++j)
		{
			if (m_group_items[j].isSeparator)
			{
				curY += grpSepH;
			}
			else
			{
				if (point.y >= curY && point.y < curY + grpH)
				{
					int selId = m_group_items[j].id;
					auto stock = m_results[m_selected_stock_idx];
					if (m_on_add_to_group)
					{
						m_on_add_to_group(stock, selId);
					}
					return;
				}
				curY += grpH;
			}
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CSearchResultDropdown::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp(nFlags, point);
}

// ===================== CDarkComboBox 暗色自绘下拉框 =====================

BEGIN_MESSAGE_MAP(CDarkComboBox, CComboBox)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

void CDarkComboBox::PreSubclassWindow()
{
	CComboBox::PreSubclassWindow();
	ModifyStyle(WS_BORDER, 0);
	ModifyStyleEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, 0);
	::SetWindowTheme(GetSafeHwnd(), L"", L"");
}

void CDarkComboBox::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	// 彻底消除 Windows ComboBox 原生 3D 边框预留边距，让 client 区域填满窗口
}

void CDarkComboBox::OnNcPaint()
{
}

void CDarkComboBox::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);

	// 只有闭合态需要绘制：字段区域高度 = 布局方设定的字段高（m_field_height），
	// 避免把整窗高度（含下拉列表预留区）画成一个高大的框体而下坠
	int fieldH = (m_field_height > 0) ? m_field_height : GetItemHeight(-1);
	if (fieldH > 0 && fieldH < rc.Height())
		rc.bottom = rc.top + fieldH;

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&memBmp);

	Gdiplus::Graphics g(memDC.GetSafeHdc());
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

	bool focused = (::GetFocus() == GetSafeHwnd());
	bool hovered = m_is_hovered;

	// 1. 背景底色 (#0D0F15，与单行输入框底色严格一致)
	Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 13, 15, 21));
	Gdiplus::RectF bgRect(0.0f, 0.0f, static_cast<Gdiplus::REAL>(rc.Width()), static_cast<Gdiplus::REAL>(rc.Height()));
	g.FillRectangle(&bgBrush, bgRect);

	// 2. 边框 (聚焦品牌蓝高亮，悬停亮灰，失焦暗灰)
	Gdiplus::Color borderColor = focused ? Gdiplus::Color(255, 37, 99, 235) : (hovered ? Gdiplus::Color(255, 75, 85, 105) : Gdiplus::Color(255, 52, 58, 72));
	Gdiplus::Pen borderPen(borderColor, 1.0f);
	g.DrawRectangle(&borderPen, 0.5f, 0.5f, static_cast<Gdiplus::REAL>(rc.Width() - 1), static_cast<Gdiplus::REAL>(rc.Height() - 1));

	// 3. 绘制选中项文字
	CString text;
	int curSel = GetCurSel();
	if (curSel != CB_ERR)
		GetLBText(curSel, text);
	else
		GetWindowText(text);

	Gdiplus::Font font(L"微软雅黑", static_cast<Gdiplus::REAL>(g_data.DPI(11.5)), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 241, 245, 249));
	Gdiplus::StringFormat sf;
	sf.SetAlignment(Gdiplus::StringAlignmentNear);
	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

	int textMargin = g_data.DPI(8);
	int arrowAreaWidth = g_data.DPI(22);
	Gdiplus::RectF textRect(static_cast<Gdiplus::REAL>(textMargin), 0.0f,
		static_cast<Gdiplus::REAL>(rc.Width() - textMargin - arrowAreaWidth), static_cast<Gdiplus::REAL>(rc.Height()));
	g.DrawString(text.GetString(), -1, &font, textRect, &sf, &textBrush);

	// 4. 绘制右侧下拉箭头 (精致折线 V 形)
	Gdiplus::Pen arrowPen(hovered || focused ? Gdiplus::Color(255, 241, 245, 249) : Gdiplus::Color(255, 148, 163, 184), 1.6f);
	arrowPen.SetStartCap(Gdiplus::LineCapRound);
	arrowPen.SetEndCap(Gdiplus::LineCapRound);
	arrowPen.SetLineJoin(Gdiplus::LineJoinRound);

	float arrowCenterX = static_cast<float>(rc.right - g_data.DPI(11));
	float arrowCenterY = static_cast<float>(rc.Height() / 2.0f);
	float arrowHalfW = static_cast<float>(g_data.DPI(3.5));
	float arrowHalfH = static_cast<float>(g_data.DPI(2.0));

	Gdiplus::PointF arrowPoints[3] = {
		Gdiplus::PointF(arrowCenterX - arrowHalfW, arrowCenterY - arrowHalfH),
		Gdiplus::PointF(arrowCenterX, arrowCenterY + arrowHalfH),
		Gdiplus::PointF(arrowCenterX + arrowHalfW, arrowCenterY - arrowHalfH)
	};
	g.DrawLines(&arrowPen, arrowPoints, 3);

	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

void CDarkComboBox::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_is_hovered)
	{
		m_is_hovered = true;
		Invalidate();
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, GetSafeHwnd(), 0 };
		TrackMouseEvent(&tme);
	}
	CComboBox::OnMouseMove(nFlags, point);
}

void CDarkComboBox::OnMouseLeave()
{
	m_is_hovered = false;
	Invalidate();
	CComboBox::OnMouseLeave();
}

void CDarkComboBox::OnSetFocus(CWnd* pOldWnd)
{
	CComboBox::OnSetFocus(pOldWnd);
	Invalidate();
}

void CDarkComboBox::OnKillFocus(CWnd* pNewWnd)
{
	CComboBox::OnKillFocus(pNewWnd);
	Invalidate();
}

void CDarkComboBox::DrawItem(LPDRAWITEMSTRUCT lp)
{
	CDC dc;
	dc.Attach(lp->hDC);
	CRect r = lp->rcItem;

	bool selected = (lp->itemState & ODS_SELECTED) != 0;
	bool isComboEdit = (lp->itemState & ODS_COMBOBOXEDIT) != 0;

	// 背景色：闭合状态与输入框底色 #0D0F15 一致；展开下拉项：选中深蓝 #1C2D4B，未选暗灰底 #14161D
	COLORREF bgClr;
	COLORREF textClr;

	if (isComboEdit || (int)lp->itemID < 0)
	{
		bgClr = RGB(13, 15, 21);
		textClr = RGB(241, 245, 249);
	}
	else
	{
		bgClr = selected ? RGB(28, 45, 75) : RGB(20, 22, 29);
		textClr = selected ? RGB(255, 255, 255) : RGB(226, 232, 240);
	}

	dc.FillSolidRect(&r, bgClr);

	CString text;
	if ((int)lp->itemID >= 0)
	{
		GetLBText(lp->itemID, text);
	}
	else
	{
		int curSel = GetCurSel();
		if (curSel != CB_ERR)
			GetLBText(curSel, text);
		else
			GetWindowText(text);
	}

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(textClr);

	CFont font;
	font.CreatePointFont(90, _T("微软雅黑"));
	CFont* pOldFont = dc.SelectObject(&font);

	CRect textRect = r;
	textRect.left += g_data.DPI(8);
	dc.DrawText(text, &textRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

	if (pOldFont)
		dc.SelectObject(pOldFont);

	dc.Detach();
}

void CDarkComboBox::MeasureItem(LPMEASUREITEMSTRUCT lp)
{
	// 与单行输入框字段框高度一致(26)，否则关闭态框体会比输入框矮
	lp->itemHeight = g_data.DPI(26);
}

