//
// FGUI - feature rich graphical user interface
//

// library includes
#include "tabpanel.hpp"
#include <locale>
#include <codecvt>
#include <comdef.h>

namespace FGUI
{

  CTabPanel::CTabPanel()
  {
    m_strTitle = "";
    m_anyFont = 0;
    m_ullSelectedEntry = 0;
    m_dmSize = { 110, 25 };
    m_prgpTabButtons = {};
    m_strTooltip = "";
    m_nStyle = static_cast<int>(TAB_STYLE::VERTICAL);
    m_nType = static_cast<int>(WIDGET_TYPE::TABPANEL);
    m_nFlags = static_cast<int>(WIDGET_FLAG::DRAWABLE) | static_cast<int>(WIDGET_FLAG::CLICKABLE);
  }

  void CTabPanel::AddTab(std::string title)
  {
    m_prgpTabButtons.emplace_back(title);
  }

  void CTabPanel::SetIndex(std::size_t index)
  {
    m_ullSelectedEntry = index;
  }

  std::size_t CTabPanel::GetIndex()
  {
    return m_ullSelectedEntry;
  }

  void CTabPanel::SetStyle(FGUI::TAB_STYLE style)
  {
    m_nStyle = static_cast<int>(style);
  }

  int CTabPanel::GetStyle()
  {
    return m_nStyle;
  }

  void CTabPanel::DrawRoundedRectangle(int x, int y, int w, int h, FGUI::COLOR color, int smooth)
  {
      POINT pt[4];
      pt[0].m_iX = x + (w - smooth);
      pt[0].m_iY = y + (h - smooth);

      pt[1].m_iX = x + smooth;
      pt[1].m_iY = y + (h - smooth);

      pt[2].m_iX = x + smooth;
      pt[2].m_iY = y + smooth;

      pt[3].m_iX = x + w - smooth;
      pt[3].m_iY = y + smooth;

      FGUI::RENDER.Rectangle(x, y + smooth, w, h - smooth * 2, color);
      FGUI::RENDER.Rectangle(x + smooth, y, w - smooth * 2, h, color);

      //rectangle_filled({ { x, y + smooth }, { w, h - smooth * 2 } }, color);
      //rectangle_filled({ { x + smooth, y }, { w - smooth, h * 2 } }, color);

      float fDegree = 0.f;

      for (size_t i = 0; i < 4; i++)
      {
          for (float k = 0; k < fDegree + (M_PI * M_PI) / 4; k += D3DXToRadian(1))
          {
              FGUI::RENDER.Line(pt[i].m_iX,
                  pt[i].m_iY,
                  pt[i].m_iX + (cosf(k) * smooth),
                  pt[i].m_iY + (sinf(k) * smooth),
                  color, 1.f);
          }
          fDegree += (M_PI * M_PI) / 4;
      }
  }

  void CTabPanel::Geometry()
  {
      // don't proceed if the tab container is empty
      if (m_prgpTabButtons.empty())
      {
          return;
      }

      FGUI::AREA arWidgetRegion = { 0, 100, 0, 0 };

      for (std::size_t i = 0; i < m_prgpTabButtons.size(); i++)
      {
          if (m_nStyle == static_cast<int>(TAB_STYLE::HORIZONTAL))
          {
              m_iEntrySpacing = 115;

              arWidgetRegion = { GetAbsolutePosition().m_iX + (static_cast<int>(i) * m_iEntrySpacing), GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };

              if (m_ullSelectedEntry == i)
              {
                  //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 45, 83, 122 });
                  //FGUI::RENDER.Gradient(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop + arWidgetRegion.m_iBottom, arWidgetRegion.m_iRight, 2, { 105, 127, 255 }, { 1, 26, 51 }, false);
                  //DrawRoundedRectangle();
                  //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, (arWidgetRegion.m_iTop - 5), arWidgetRegion.m_iRight, (arWidgetRegion.m_iBottom + 5), { 45, 83, 122 });
                  FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 45, 83, 122 }, 5.f);
                  //FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 20), (arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - 5), m_anyFont, { 219, 219, 219 }, m_prgpTabButtons[i]);
                  FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 20), (arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - 5), m_anyFont, { 38, 148, 206 }, m_prgpTabButtons[i]);
              }
              else
              {
                  //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 45, 83, 122 });
                  //DrawRoundedRectangle();
                  FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 45, 83, 122 }, 5.f);
                  FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 20), (arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - 5), m_anyFont, { 27, 73, 131 }, m_prgpTabButtons[i]);
              }
          }
          else if (m_nStyle == static_cast<int>(TAB_STYLE::VERTICAL))
          {
              m_iEntrySpacing = 100;
              int index = 0;
              if (m_prgpTabButtons[i] == "Combat") index = 1;
              if (m_prgpTabButtons[i] == "Visuals") index = 2;
              if (m_prgpTabButtons[i] == "Misc") index = 3;
              if (m_prgpTabButtons[i] == "Colors") index = 4;

              arWidgetRegion = { GetAbsolutePosition().m_iX - 8, GetAbsolutePosition().m_iY + (static_cast<int>(i) * m_iEntrySpacing) + 30, m_dmSize.m_iWidth, m_dmSize.m_iHeight + 30 };

              if (m_ullSelectedEntry == i)
              {
                  //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop - 2, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 57, 117, 179 });
                  //DrawRoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 28, 77, 128 }, 10.f);
                  //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + 105, arWidgetRegion.m_iTop - 2, arWidgetRegion.m_iRight - 105, arWidgetRegion.m_iBottom, { 53, 109, 166 });
                  //DrawRoundedRectangle(arWidgetRegion.m_iLeft + 100, arWidgetRegion.m_iTop + 2, arWidgetRegion.m_iRight - 110, arWidgetRegion.m_iBottom - 4, { 25, 117, 212 }, 10.f);

                  FGUI::RENDER.Line(arWidgetRegion.m_iLeft - 2, arWidgetRegion.m_iTop, arWidgetRegion.m_iLeft + 98, arWidgetRegion.m_iTop, { 100, 100, 117 }, 1.f);
                  FGUI::RENDER.Line(arWidgetRegion.m_iLeft - 2, arWidgetRegion.m_iTop + arWidgetRegion.m_iBottom, arWidgetRegion.m_iLeft + 98, arWidgetRegion.m_iTop + arWidgetRegion.m_iBottom, { 100, 100, 117 }, 1.f);

                  FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft - 1, arWidgetRegion.m_iTop + 1, 99, arWidgetRegion.m_iBottom - 2, { 23, 25, 31, 190 }, 0.f);

                  //FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 15), (arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - 8), m_anyFont, { 38, 148, 206 }, m_prgpTabButtons[i]);
                  FGUI::RENDER.Image((arWidgetRegion.m_iLeft + 20), arWidgetRegion.m_iTop + 2, 50, 50, index);
              }
              else
              {
                 // FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 45, 83, 122 });
                  //DrawRoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 31, 87, 145 }, 10.f);
                  //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + 105, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight - 105, arWidgetRegion.m_iBottom, { 31, 87, 145 });
                  //DrawRoundedRectangle(arWidgetRegion.m_iLeft + 100, arWidgetRegion.m_iTop + 2, arWidgetRegion.m_iRight - 110, arWidgetRegion.m_iBottom - 4, { 29, 99, 171 }, 10.f);

                  FGUI::RENDER.Line(arWidgetRegion.m_iLeft - 2, arWidgetRegion.m_iTop, arWidgetRegion.m_iLeft + 96, arWidgetRegion.m_iTop, { 121,121,140 }, 1.f);
                  FGUI::RENDER.Line(arWidgetRegion.m_iLeft - 2, arWidgetRegion.m_iTop + arWidgetRegion.m_iBottom, arWidgetRegion.m_iLeft + 96, arWidgetRegion.m_iTop + arWidgetRegion.m_iBottom, { 121,121,140 }, 1.f);
                  //FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 15), (arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - 8), m_anyFont, { 38, 148, 206 }, m_prgpTabButtons[i]);
                  FGUI::RENDER.Image((arWidgetRegion.m_iLeft + 20), arWidgetRegion.m_iTop + 2, 50, 50, index);
              }
          }
      }
  }

  void CTabPanel::Update()
  {
    FGUI::AREA arWidgetRegion = { 0, 0, 0, 0 };

    for (std::size_t i = 0; i < m_prgpTabButtons.size(); i++)
    {
      if (m_nStyle == static_cast<int>(TAB_STYLE::HORIZONTAL))
      {
        m_iEntrySpacing = 113;

        arWidgetRegion = { GetAbsolutePosition().m_iX + (static_cast<int>(i) * m_iEntrySpacing), GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };
      }
      else if (m_nStyle == static_cast<int>(TAB_STYLE::VERTICAL))
      {
        m_iEntrySpacing = 100;

        arWidgetRegion = { GetAbsolutePosition().m_iX - 8, GetAbsolutePosition().m_iY + (static_cast<int>(i) * m_iEntrySpacing) + 10, m_dmSize.m_iWidth, m_dmSize.m_iHeight + 57 };
      }

      if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
      {
        if (FGUI::INPUT.IsKeyPressed(MOUSE_1))
        {
          m_ullSelectedEntry = i;
        }
      }
    }
  }

  void CTabPanel::Input()
  {
  }

  void CTabPanel::Save(nlohmann::json& module)
  {
    IGNORE_ARG(module);
  }

  void CTabPanel::Load(nlohmann::json& module)
  {
    IGNORE_ARG(module);
  }

  void CTabPanel::Tooltip()
  {
  }

} // namespace FGUI