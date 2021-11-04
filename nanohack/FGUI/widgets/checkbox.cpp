//
// FGUI - feature rich graphical user interface
//

// library includes
#include "checkbox.hpp"

namespace FGUI
{

  CCheckBox::CCheckBox()
  {
    m_strTitle = "";
    m_dmSize = { 10,10 };
    m_anyFont = 0;
    m_strTooltip = "";
    m_bIsChecked = false;
    m_fnctCallback = nullptr;
    m_nType = static_cast<int>(WIDGET_TYPE::CHECKBOX);
    m_nFlags = static_cast<int>(WIDGET_FLAG::DRAWABLE) | static_cast<int>(WIDGET_FLAG::CLICKABLE) | static_cast<int>(WIDGET_FLAG::SAVABLE);
  }

  void CCheckBox::SetState(bool onoff)
  {
    m_bIsChecked = onoff;
  }

  bool CCheckBox::GetState()
  {
    return m_bIsChecked;
  }

  void CCheckBox::AddCallback(std::function<void()> callback)
  {
    m_fnctCallback = callback;
  }

  void CCheckBox::DrawRoundedRectangle(int x, int y, int w, int h, FGUI::COLOR color, int smooth)
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

  void CCheckBox::Geometry()
  {
    FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, 20, 20 };

    // checkbox body
    //FGUI::RENDER.Outline((arWidgetRegion.m_iLeft - 1), (arWidgetRegion.m_iTop - 1), arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 74, 110, 255 });
    //DrawRoundedRectangle((arWidgetRegion.m_iLeft - 2), (arWidgetRegion.m_iTop - 2), arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 74, 110, 255 }, 5.f);
    //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 53, 78, 115 });
    //DrawRoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, (arWidgetRegion.m_iRight - 4), (arWidgetRegion.m_iBottom - 4), { 34, 60, 115 }, 5.f);

    FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, 20, 20, { 34, 34, 46 }, 2.f);

    //FGUI::RENDER.Circle(arWidgetRegion.m_iLeft + 5, arWidgetRegion.m_iTop + 5, { 34, 60, 115 }, 1.f, 10.f);
    if (m_bIsChecked)
    {
        if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
        {
            FGUI::RENDER.RoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, 20, 20, { 27, 73, 131 }, 2.f);
            //FGUI::RENDER.Outline((arWidgetRegion.m_iLeft - 1), (arWidgetRegion.m_iTop - 1), arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { /*33, 255, 94*/ 34, 60, 115 });
            //DrawRoundedRectangle((arWidgetRegion.m_iLeft), (arWidgetRegion.m_iTop), arWidgetRegion.m_iRight - 4, arWidgetRegion.m_iBottom - 4, { /*33, 255, 94*/ 10, 79, 120 }, 5.f);
        }

        FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, 20, 20, { 38, 148, 206 }, 2.f);

        //tick
        FGUI::RENDER.Line(arWidgetRegion.m_iLeft + 4, arWidgetRegion.m_iTop + 12, arWidgetRegion.m_iLeft + 7, arWidgetRegion.m_iTop + 15, { 255, 255, 255 }, 4.);
        FGUI::RENDER.Line(arWidgetRegion.m_iLeft + 7, arWidgetRegion.m_iTop + 15, arWidgetRegion.m_iLeft + 16, arWidgetRegion.m_iTop + 6, { 255, 255, 255 }, 4.f);

        FGUI::RENDER.Text(arWidgetRegion.m_iLeft + (arWidgetRegion.m_iRight) + 13, arWidgetRegion.m_iTop + 3, m_anyFont, { 219, 219, 219 }, m_strTitle);
        //FGUI::RENDER.Circle((arWidgetRegion.m_iLeft + 5), (arWidgetRegion.m_iTop + 5), { 13, 104, 158 }, 1.f, 5.f);
        //DrawRoundedRectangle((arWidgetRegion.m_iLeft), (arWidgetRegion.m_iTop), (arWidgetRegion.m_iRight - 4), (arWidgetRegion.m_iBottom - 4), { /*41, 217, 89*/ 13, 104, 158 }, 5.f);
    }
    else
    {
        if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
        {
            //DrawRoundedRectangle((arWidgetRegion.m_iLeft), (arWidgetRegion.m_iTop), arWidgetRegion.m_iRight - 4, arWidgetRegion.m_iBottom - 4, { 53, 78, 115 }, 5.f);
            FGUI::RENDER.RoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, 20, 20, { 27, 73, 131 }, 2.f);
        }
        FGUI::RENDER.Text(arWidgetRegion.m_iLeft + (arWidgetRegion.m_iRight) + 13, arWidgetRegion.m_iTop + 3, m_anyFont, { 150, 150, 167 }, m_strTitle);
    }

    // checkbox label
  }

  void CCheckBox::Update()
  {
    m_dmSize = { 13, 13 }; // this is required to keep the widget from being padded on groupboxes

    if (m_bIsChecked)
    {
      if (m_fnctCallback)
      {
        // call function
        m_fnctCallback();
      }
    }
  }

  void CCheckBox::Input()
  {
      FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX - 10, GetAbsolutePosition().m_iY - 10, 40, 40 };

    if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
    {
      m_bIsChecked = !m_bIsChecked;
    }
  }

  void CCheckBox::Save(nlohmann::json& module)
  {
    // remove spaces from widget name
    std::string strFormatedWidgetName = GetTitle();
    std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

    module[strFormatedWidgetName] = m_bIsChecked;
  }

  void CCheckBox::Load(nlohmann::json& module)
  {
    // remove spaces from widget name
    std::string strFormatedWidgetName = GetTitle();
    std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

    // change widget state to the one stored on file
    if (module.contains(strFormatedWidgetName))
    {
      m_bIsChecked = module[strFormatedWidgetName];
    }
  }

  void CCheckBox::Tooltip()
  {
    if (m_strTooltip.length() > 1 && !m_bIsChecked)
    {
      FGUI::DIMENSION dmTooltipTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strTooltip);

      FGUI::AREA arTooltipRegion = { (FGUI::INPUT.GetCursorPos().m_iX + 10), (FGUI::INPUT.GetCursorPos().m_iY + 10), (dmTooltipTextSize.m_iWidth + 10), (dmTooltipTextSize.m_iHeight + 10) };

      FGUI::RENDER.Outline(arTooltipRegion.m_iLeft, arTooltipRegion.m_iTop, arTooltipRegion.m_iRight, arTooltipRegion.m_iBottom, { 180, 95, 95 });
      FGUI::RENDER.Rectangle((arTooltipRegion.m_iLeft + 1), (arTooltipRegion.m_iTop + 1), (arTooltipRegion.m_iRight - 2), (arTooltipRegion.m_iBottom - 2), { 225, 90, 75 });
      FGUI::RENDER.Text(arTooltipRegion.m_iLeft + (arTooltipRegion.m_iRight / 2) - (dmTooltipTextSize.m_iWidth / 2),
        arTooltipRegion.m_iTop + (arTooltipRegion.m_iBottom / 2) - (dmTooltipTextSize.m_iHeight / 2), m_anyFont, { 245, 245, 245 }, m_strTooltip);
    }
  }

} // namespace FGUI