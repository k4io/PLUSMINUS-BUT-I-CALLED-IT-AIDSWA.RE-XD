//
// FGUI - feature rich graphical user interface
//

// library includes
#include "button.hpp"

namespace FGUI
{

    CButton::CButton()
    {
        m_strTitle = "";
        m_dmSize = { 100, 20 };
        m_strTooltip = "";
        m_anyFont = 0;
        m_fnctCallback = nullptr;
        m_nType = static_cast<int>(WIDGET_TYPE::BUTTON);
        m_nFlags = static_cast<int>(WIDGET_FLAG::DRAWABLE) | static_cast<int>(WIDGET_FLAG::CLICKABLE);
    }

    void CButton::AddCallback(std::function<void()> callback)
    {
        m_fnctCallback = callback;
    }

    void CButton::DrawRoundedRectangle(int x, int y, int w, int h, FGUI::COLOR color, int smooth)
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

    void CButton::Geometry()
    {
        FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };

        FGUI::DIMENSION dmTitleTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strTitle);

        // button body
        if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
        {
            //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { /*64, 201, 108*/ 38, 148, 206 });

            FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { /*64, 201, 108*/ 38, 148, 206 }, 10.f);
        }
        else
        {
            //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { /*48, 156, 82*/ 44, 44, 46 });
            FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { /*48, 156, 82*/ 44, 44, 46 }, 10.f);
        }

        // button label
        FGUI::RENDER.Text(arWidgetRegion.m_iLeft + (arWidgetRegion.m_iRight / 2) - (dmTitleTextSize.m_iWidth / 2),
            arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 245, 245, 245 }, m_strTitle);
    }

    void CButton::Update()
    {
    }

    void CButton::Input()
    {
        FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };

        if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
        {
            if (m_fnctCallback)
            {
                // call function
                m_fnctCallback();
            }
        }
    }

    void CButton::Save(nlohmann::json& module)
    {
        IGNORE_ARG(module);
    }

    void CButton::Load(nlohmann::json& module)
    {
        IGNORE_ARG(module);
    }

    void CButton::Tooltip()
    {
        if (m_strTooltip.length() > 1)
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