//
// FGUI - feature rich graphical user interface
//

// library includes
#include "keybinder.hpp"

namespace FGUI
{

  CKeyBinder::CKeyBinder()
  {
    m_strTitle = "";
    m_anyFont = 0;
    m_dmSize = { 150, 20 };
    m_uiKey = 0;
    m_strStatus = "none";
    m_bIsGettingKey = false;
    m_strTooltip = "";
    m_nType = static_cast<int>(WIDGET_TYPE::KEYBINDER);
    m_nFlags = static_cast<int>(WIDGET_FLAG::DRAWABLE) | static_cast<int>(WIDGET_FLAG::CLICKABLE) | static_cast<int>(WIDGET_FLAG::SAVABLE);
  }

  void CKeyBinder::SetKey(unsigned int key_code)
  {
    m_uiKey = key_code;
  }

  unsigned int CKeyBinder::GetKey()
  {
    return m_uiKey;
  }

  void CKeyBinder::Geometry()
  {
    FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };

    FGUI::DIMENSION dmTitleTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strTitle);

    // keybinder body
    if (FGUI::INPUT.IsCursorInArea(arWidgetRegion) || m_bIsGettingKey)
    {
      //FGUI::RENDER.Outline(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { /*174, 255, 156*/ 34, 60, 115 });
      //FGUI::RENDER.Rectangle();                                                                                                                                       
        FGUI::RENDER.RoundedRectangleFilled((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 1), (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 40,36,36 }, 5.f);
    }
    else
    {
      //FGUI::RENDER.Outline(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 74, 110, 255 });
      //FGUI::RENDER.Rectangle();
      FGUI::RENDER.RoundedRectangleFilled((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 1), (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 24,28,28 }, 5.f);
    }

    // keybinder label
    FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 10), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 219, 219, 219 }, m_strTitle + ":");

    // change status 
    // TODO: improve this (clean it up)
    switch (FGUI::INPUT.GetInputType())
    {
      case static_cast<int>(INPUT_TYPE::WIN_32) :
      {
        m_strStatus = m_kcCodes.m_strVirtualKeyCodes[m_uiKey].data();
        break;
      }
      case static_cast<int>(INPUT_TYPE::INPUT_SYSTEM) :
      {
        m_strStatus = m_kcCodes.m_strInputSystem[m_uiKey].data();
        break;
      }
      default:
      {
        std::throw_with_nested(std::runtime_error(""));
        break;
      }
    }

    // keybinder current key
    FGUI::RENDER.Text(arWidgetRegion.m_iLeft + (dmTitleTextSize.m_iWidth + 20), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 191, 191, 191 }, m_strStatus);
  }

  void CKeyBinder::Update()
  {
    if (m_bIsGettingKey)
    {
      for (std::size_t key = 0; key < 256; key++)
      {
        // if the user has pressed a valid key
        if (FGUI::INPUT.IsKeyPressed(key))
        {
          // if the user press ESCAPE
          if (key == KEY_ESCAPE)
          {
            // change the key to an invalid key
            m_uiKey = 0;

            // reset status
            m_strStatus = "None";

            // block keybinder
            m_bIsGettingKey = false;
          }
          else // iterate the rest of the keys
          {
            // change status to currently pressed key
            switch (FGUI::INPUT.GetInputType())
            {
              case static_cast<int>(INPUT_TYPE::WIN_32) :
              {
                m_strStatus = m_kcCodes.m_strVirtualKeyCodes[key].data();
                break;
              }
              case static_cast<int>(INPUT_TYPE::INPUT_SYSTEM) :
              {
                m_strStatus = m_kcCodes.m_strInputSystem[key].data();
                break;
              }
              case static_cast<int>(INPUT_TYPE::CUSTOM) :
              {
                m_strStatus = "NOT IMPLEMENTED";
                break;
              }
              default:
              {
                std::throw_with_nested(std::runtime_error("make sure to set an input type. Take a look at the wiki for more info."));
                break;
              }
            }

            // set key
            m_uiKey = key;

            // block keybinder from receiving input
            m_bIsGettingKey = false;
          }
        }
      }
    }
  }

  void CKeyBinder::Input()
  {
    FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };

    if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
    {
      m_bIsGettingKey = !m_bIsGettingKey;
    }
  }

  void CKeyBinder::Save(nlohmann::json& module)
  {
    // remove spaces from widget name
    std::string strFormatedWidgetName = GetTitle();
    std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

    module[strFormatedWidgetName] = m_uiKey;
  }

  void CKeyBinder::Load(nlohmann::json& module)
  {
    // remove spaces from widget name
    std::string strFormatedWidgetName = GetTitle();
    std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

    // change widget default key to the one stored on file
    if (module.contains(strFormatedWidgetName))
    {
      m_uiKey = module[strFormatedWidgetName];
      
      // update widget status
      m_strStatus = m_kcCodes.m_strVirtualKeyCodes[m_uiKey].data();
    }
  }

  void CKeyBinder::Tooltip()
  {
    if (m_strTooltip.length() > 1 && !m_bIsGettingKey)
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