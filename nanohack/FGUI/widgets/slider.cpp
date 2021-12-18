//
// FGUI - feature rich graphical user interface
//

// library includes
#include "slider.hpp"
#include "../../core/sdk/utils/xorstr.hpp"

namespace FGUI
{

    CSlider::CSlider()
    {
        m_strTitle = "";
        m_strPrefix = "";
        m_dmSize = { 100, 11 };
        m_dmSliderThumbSize = { 1, 10 };
        m_flValue = 0.f;
        m_bIsDragging = false;
        m_rngBoundaries = { 0.f, 0.f };
        m_anyFont = 0;
        m_strTooltip = "";
        m_nType = static_cast<int>(WIDGET_TYPE::SLIDER);
        m_nFlags = static_cast<int>(WIDGET_FLAG::DRAWABLE) | static_cast<int>(WIDGET_FLAG::CLICKABLE) | static_cast<int>(WIDGET_FLAG::SAVABLE);
    }

    void CSlider::SetValue(float value)
    {
        m_flValue = value;
    }

    float CSlider::GetValue()
    {
        return m_flValue;
    }

    void CSlider::SetRange(float min, float max)
    {
        m_rngBoundaries.m_flMin = min;
        m_rngBoundaries.m_flMax = max;
    }

    void CSlider::SetPrefix(std::string prefix)
    {
        m_strPrefix = prefix;
    }


    void CSlider::DrawRoundedRectangle(int x, int y, int w, int h, FGUI::COLOR color, int smooth)
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

    void CSlider::Geometry()
    {
        FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight + 4 };

        FGUI::DIMENSION dmTitleTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strTitle);

        char tBuffer[256] = {};
        sprintf_s(tBuffer, xorstr_("%.2f"), m_flValue);
        //if (m_strTitle == "counter" || m_strTitle == "bullets")
            //sprintf_s(tBuffer, xorstr_("%i"), static_cast<int>(m_flValue));

        FGUI::DIMENSION dmValueTextSize = FGUI::RENDER.GetTextSize(m_anyFont, std::string(tBuffer) + " " + m_strPrefix);

        // slider position ratio
        float flRatio = (m_flValue - m_rngBoundaries.m_flMin) / (m_rngBoundaries.m_flMax - m_rngBoundaries.m_flMin);
        float flLocation = (flRatio * m_dmSize.m_iWidth);
        //if (m_strTitle == "counter" || m_strTitle == "bullets")
            //flLocation = static_cast<int>(flRatio * m_dmSize.m_iWidth);
        // slider body
        //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop + 3, m_dmSize.m_iWidth, 2, { 14, 36, 69 });
       // DrawRoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, m_dmSize.m_iWidth, m_dmSize.m_iHeight, { 14, 36, 69 }, 1.f);
         //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, m_dmSize.m_iWidth, m_dmSliderThumbSize.m_iHeight - 2, { 53, 78, 115 });
        //DrawRoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, m_dmSize.m_iWidth, m_dmSliderThumbSize.m_iHeight - 2, { 53, 78, 115 }, 1.f);
        //FGUI::RENDER.Line(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, m_dmSize.m_iWidth, m_dmSize.m_iHeight, { 14, 36, 69 });
        FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop + 2, m_dmSize.m_iWidth, 2, { 85, 85, 98 }, 5.f);

        // slider thumb
        FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft - 1, arWidgetRegion.m_iTop + 2, flLocation, 2, { 24,28,28 }, 5.f);
        //FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft), (arWidgetRegion.m_iTop + 3), m_dmSliderThumbSize.m_iWidth + flLocation, 2, { /*41, 217, 89*/ 5, 70, 107 });
        //FGUI::RENDER.Line(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, m_dmSize.m_iWidth, m_dmSize.m_iHeight, { 5, 70, 107 });
        //DrawRoundedRectangle((arWidgetRegion.m_iLeft), (arWidgetRegion.m_iTop), m_dmSliderThumbSize.m_iWidth + flLocation, m_dmSliderThumbSize.m_iHeight, { /*41, 217, 89*/ 5, 70, 107 }, 1.f);
         //FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft), (arWidgetRegion.m_iTop), m_dmSliderThumbSize.m_iWidth + flLocation, m_dmSliderThumbSize.m_iHeight - 2, { /*41, 217, 89*/ 13, 104, 158 });
        //DrawRoundedRectangle((arWidgetRegion.m_iLeft), (arWidgetRegion.m_iTop), m_dmSliderThumbSize.m_iWidth + flLocation, m_dmSliderThumbSize.m_iHeight - 2, { /*41, 217, 89*/ 13, 104, 158 }, 1.f);
        FGUI::RENDER.Circle((arWidgetRegion.m_iLeft + m_dmSliderThumbSize.m_iWidth) + flLocation, (arWidgetRegion.m_iTop + 2) + 1, { 32,36,36 }, 5.f, 5.f);
        FGUI::RENDER.Circle((arWidgetRegion.m_iLeft + m_dmSliderThumbSize.m_iWidth) + flLocation, (arWidgetRegion.m_iTop + 2) + 1, { 24,28,28 }, 3.f, 5.f);

        FGUI::AREA ar = { (GetAbsolutePosition().m_iX + m_dmSliderThumbSize.m_iWidth) + flLocation - 5, GetAbsolutePosition().m_iY - 5, 10, m_dmSize.m_iHeight };
        if (FGUI::INPUT.IsCursorInArea(ar))
        {
            FGUI::RENDER.Circle((arWidgetRegion.m_iLeft + m_dmSliderThumbSize.m_iWidth) + flLocation, (arWidgetRegion.m_iTop + 2) + 1, { 32,36,36 }, 3.f, 5.f);
        }

        // slider label & value
        FGUI::RENDER.Text(arWidgetRegion.m_iLeft, (arWidgetRegion.m_iTop - dmTitleTextSize.m_iHeight) - 2, m_anyFont, { 219, 219, 219 }, m_strTitle);
        FGUI::RENDER.Text((arWidgetRegion.m_iLeft + arWidgetRegion.m_iRight) - dmValueTextSize.m_iWidth, (arWidgetRegion.m_iTop - dmTitleTextSize.m_iHeight) - 2, m_anyFont, { 250,250,250 }, std::string(tBuffer) + " " + m_strPrefix);
    }

    void CSlider::Update()
    {
        FGUI::POINT ptCursorPos = FGUI::INPUT.GetCursorPos();

        // if the user is dragging the slider
        if (m_bIsDragging)
        {
            if (FGUI::INPUT.IsKeyHeld(MOUSE_1))
            {
                float flXPosDelta = 0.f;
                float flRatio = 0.f;

                // change slider value based on mouse movement
                flXPosDelta = (ptCursorPos.m_iX - GetAbsolutePosition().m_iX);

                // clamp thumb position
                if (flXPosDelta < 0.f)
                {
                    flXPosDelta = 0.f;
                }
                else if (flXPosDelta >= m_dmSize.m_iWidth)
                {
                    flXPosDelta = m_dmSize.m_iWidth;
                }

                // calculate slider ratio
                flRatio = flXPosDelta / static_cast<float>(m_dmSize.m_iWidth);

                // change slider value
                m_flValue = m_rngBoundaries.m_flMin + (m_rngBoundaries.m_flMax - m_rngBoundaries.m_flMin) * flRatio;
            }
            else
            {
                m_bIsDragging = false;
            }
        }
    }

    void CSlider::Input()
    {
        // custom height in pixels from the "clickable" area
        static constexpr int iCustomHeight = 19;

        FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, iCustomHeight };

        if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
        {
            m_bIsDragging = true;
        }
    }

    void CSlider::Save(nlohmann::json& module)
    {
        // remove spaces from widget name
        std::string strFormatedWidgetName = GetTitle();
        std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

        module[strFormatedWidgetName] = m_flValue;
    }

    void CSlider::Load(nlohmann::json& module)
    {
        // remove spaces from widget name
        std::string strFormatedWidgetName = GetTitle();
        std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

        // change widget value to the one stored on file
        if (module.contains(strFormatedWidgetName))
        {
            m_flValue = module[strFormatedWidgetName];
        }
    }

    void CSlider::Tooltip()
    {
        if (m_strTooltip.length() > 1 && !m_bIsDragging)
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