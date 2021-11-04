//
// FGUI - feature rich graphical user interface
//

// library includes
#include "combobox.hpp"
#include "container.hpp"
#include <filesystem>
#include <ShlObj.h>

namespace FGUI
{
  CComboBox::CComboBox()
  {
    m_strTitle = "";
    m_anyFont = 0;
    m_dmSize = { 150, 20 };
    m_dmBackupSize = { m_dmSize };
    m_iEntrySpacing = 20;
    m_ullSelectedEntry = 0;
    m_prgpEntries = {};
    m_fnctCallback = nullptr;
    m_bIsOpened = false;
    m_strTooltip = "";
    m_nType = static_cast<int>(WIDGET_TYPE::COMBOBOX);
    m_nFlags = static_cast<int>(WIDGET_FLAG::DRAWABLE) | static_cast<int>(WIDGET_FLAG::CLICKABLE) | static_cast<int>(WIDGET_FLAG::FOCUSABLE) | static_cast<int>(WIDGET_FLAG::SAVABLE);
  }

  void CComboBox::SetState(bool onoff)
  {
    m_bIsOpened = onoff;
  }

  bool CComboBox::GetState()
  {
    return m_bIsOpened;
  }

  void CComboBox::SetIndex(std::size_t index)
  {
    m_ullSelectedEntry = index;
  }

  std::size_t CComboBox::GetIndex()
  {
    return m_ullSelectedEntry;
  }

  void CComboBox::SetValue(std::size_t index, unsigned int value)
  {
    m_prgpEntries.second[index] = value;
  }

  std::size_t CComboBox::GetValue()
  {
    return m_prgpEntries.second[m_ullSelectedEntry];
  }

  void CComboBox::AddEntry(std::string name, unsigned int value)
  {
    m_prgpEntries.first.emplace_back(name);
    m_prgpEntries.second.emplace_back(value);
  }

  void CComboBox::AddCallback(std::function<void()> callback)
  {
    m_fnctCallback = callback;
  }

  void CComboBox::Geometry()
  {
    FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmBackupSize.m_iHeight };

    FGUI::DIMENSION dmTitleTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strTitle);


    if (m_bIsOpened)
    {
        FGUI::RENDER.RoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom + 20, { 44, 44, 46 }, 10.f);
    }

    // combobox body
    if (FGUI::INPUT.IsCursorInArea(arWidgetRegion) || m_bIsOpened)
    {
      FGUI::RENDER.RoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 38, 148, 206 }, 10.f);
      //FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 1), (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 44, 44, 46 });
      FGUI::RENDER.RoundedRectangleFilled((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 1), (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 44, 44, 46 }, 10.f);
    }
    else
    {
      FGUI::RENDER.RoundedRectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 44, 44, 46 }, 10.f);
      FGUI::RENDER.RoundedRectangleFilled((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 1), (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 44, 44, 46 }, 10.f);
    }

    // combobox label
    FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 10), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 219, 219, 219 }, m_strTitle + ":");

    // draw current selected entry
    FGUI::RENDER.Text(arWidgetRegion.m_iLeft + (dmTitleTextSize.m_iWidth + 20), arWidgetRegion.m_iTop + (arWidgetRegion.m_iBottom / 2) - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[m_ullSelectedEntry]);

    if (m_bIsOpened)
    {
        // dropdown list body
        FGUI::RENDER.RoundedRectangle(arWidgetRegion.m_iLeft, (arWidgetRegion.m_iTop + 21), arWidgetRegion.m_iRight, (m_prgpEntries.first.size() * m_iEntrySpacing), { /*33, 255, 94*/ 44, 44, 46 }, 10.f);
        FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 21) + 1, (arWidgetRegion.m_iRight - 2), (m_prgpEntries.first.size() * m_iEntrySpacing) - 2, { 44, 44, 46 });

        for (std::size_t i = 0; i < m_prgpEntries.first.size(); i++)
        {
            FGUI::AREA arEntryRegion = { arWidgetRegion.m_iLeft, (arWidgetRegion.m_iTop + 21) + (static_cast<int>(i) * m_iEntrySpacing), arWidgetRegion.m_iRight, m_iEntrySpacing };


            if (m_ullSelectedEntry == i && i == m_prgpEntries.first.size() - 1)
            {
                if (FGUI::INPUT.IsCursorInArea(arEntryRegion))
                {
                    FGUI::RENDER.RoundedRectangleFilled(arEntryRegion.m_iLeft + 1, arEntryRegion.m_iTop, arEntryRegion.m_iRight - 2, arEntryRegion.m_iBottom, { /*48, 209, 92*/ 85, 85, 98 }, 10.f);
                    FGUI::RENDER.Text(arEntryRegion.m_iLeft + 5, arEntryRegion.m_iTop + 2, m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[i]);
                }
                else
                {
                    FGUI::RENDER.RoundedRectangleFilled(arEntryRegion.m_iLeft + 1, (arEntryRegion.m_iTop + arEntryRegion.m_iBottom), arEntryRegion.m_iRight - 1, 1, { 44, 44, 46 }, 10.f);
                    FGUI::RENDER.Text(arEntryRegion.m_iLeft + 5, arEntryRegion.m_iTop + 2, m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[i]);
                }

                continue;
            }
            // check if the user is hovering/have selected an entry
            if (FGUI::INPUT.IsCursorInArea(arEntryRegion) || m_ullSelectedEntry == i)
            {
                FGUI::RENDER.Rectangle(arEntryRegion.m_iLeft + 1, arEntryRegion.m_iTop, arEntryRegion.m_iRight - 2, arEntryRegion.m_iBottom, { /*48, 209, 92*/ 85, 85, 98 });
                FGUI::RENDER.Text(arEntryRegion.m_iLeft + 5, arEntryRegion.m_iTop + 2, m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[i]);
            }
            else
            {
                FGUI::RENDER.Rectangle(arEntryRegion.m_iLeft + 1, (arEntryRegion.m_iTop + arEntryRegion.m_iBottom), arEntryRegion.m_iRight - 1, 1, { 44, 44, 46 });
                FGUI::RENDER.Text(arEntryRegion.m_iLeft + 5, arEntryRegion.m_iTop + 2, m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[i]);
            }
        }
    }

    // combobox dropdown arrow body
    FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + arWidgetRegion.m_iRight - 10) - 8, arWidgetRegion.m_iTop + ((arWidgetRegion.m_iBottom / 2) - 3) + 1, 8, 1, { 44, 44, 46 });
    FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + arWidgetRegion.m_iRight - 10) - 7, arWidgetRegion.m_iTop + ((arWidgetRegion.m_iBottom / 2) - 3) + 2, 6, 1, { 44, 44, 46 });
    FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + arWidgetRegion.m_iRight - 10) - 6, arWidgetRegion.m_iTop + ((arWidgetRegion.m_iBottom / 2) - 3) + 3, 4, 1, { 44, 44, 46 });
    FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + arWidgetRegion.m_iRight - 10) - 5, arWidgetRegion.m_iTop + ((arWidgetRegion.m_iBottom / 2) - 3) + 4, 2, 1, { 44, 44, 46 });
  }

  void CComboBox::Update()
  {
      /*
      FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmBackupSize.m_iHeight };
      if (m_bIsOpened && m_strTitle == "config")
      {
          FGUI::RENDER.RoundedRectangle(arWidgetRegion.m_iLeft, (arWidgetRegion.m_iTop + 21), arWidgetRegion.m_iRight, (m_prgpEntries.first.size() * m_iEntrySpacing), { 44, 44, 46 }, 10.f);
          FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 21) + 1, (arWidgetRegion.m_iRight - 2), (m_prgpEntries.first.size() * m_iEntrySpacing) - 2, { 44, 44, 46 });

          PWSTR szPath = NULL;
          std::string s = "";
          if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &szPath)))
          {
              std::filesystem::create_directories(std::wstring(szPath) + L"\\aidswa.re");
              std::wstring ws = std::wstring(szPath) + L"\\aidswa.re";
              s = std::string(ws.begin(), ws.end());

              std::vector<std::string> configs = {};
              configs.clear();
              for (auto& f : std::filesystem::recursive_directory_iterator(s))
                  if (f.path().extension() == ".cfg")
                      configs.push_back(f.path().filename().string().substr(0, f.path().filename().string().find(".")));

              bool vad = false;

              for (auto a : configs)
              {
                  vad = true;
                  for (std::size_t i = 0; i < m_prgpEntries.first.size(); i++)
                      if (m_prgpEntries.first[i] == a)
                          vad = false;
                  if (vad)
                      AddEntry(a);
              }
          }
          for (std::size_t i = 0; i < m_prgpEntries.first.size(); i++)
          {
              FGUI::AREA arEntryRegion = { arWidgetRegion.m_iLeft, (arWidgetRegion.m_iTop + 21) + (static_cast<int>(i) * m_iEntrySpacing), arWidgetRegion.m_iRight, m_iEntrySpacing };


              if (m_ullSelectedEntry == i && i == m_prgpEntries.first.size() - 1)
              {
                  if (FGUI::INPUT.IsCursorInArea(arEntryRegion))
                  {
                      FGUI::RENDER.RoundedRectangleFilled(arEntryRegion.m_iLeft + 1, arEntryRegion.m_iTop, arEntryRegion.m_iRight - 2, arEntryRegion.m_iBottom, {  85, 85, 98 }, 10.f);
                      FGUI::RENDER.Text(arEntryRegion.m_iLeft + 5, arEntryRegion.m_iTop + 2, m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[i]);
                  }
                  else
                  {
                      FGUI::RENDER.RoundedRectangleFilled(arEntryRegion.m_iLeft + 1, (arEntryRegion.m_iTop + arEntryRegion.m_iBottom), arEntryRegion.m_iRight - 1, 1, { 44, 44, 46 }, 10.f);
                      FGUI::RENDER.Text(arEntryRegion.m_iLeft + 5, arEntryRegion.m_iTop + 2, m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[i]);
                  }

                  continue;
              }
              // check if the user is hovering/have selected an entry
              if (FGUI::INPUT.IsCursorInArea(arEntryRegion) || m_ullSelectedEntry == i)
              {
                  FGUI::RENDER.Rectangle(arEntryRegion.m_iLeft + 1, arEntryRegion.m_iTop, arEntryRegion.m_iRight - 2, arEntryRegion.m_iBottom, {  85, 85, 98 });
                  FGUI::RENDER.Text(arEntryRegion.m_iLeft + 5, arEntryRegion.m_iTop + 2, m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[i]);
              }
              else
              {
                  FGUI::RENDER.Rectangle(arEntryRegion.m_iLeft + 1, (arEntryRegion.m_iTop + arEntryRegion.m_iBottom), arEntryRegion.m_iRight - 1, 1, { 44, 44, 46 });
                  FGUI::RENDER.Text(arEntryRegion.m_iLeft + 5, arEntryRegion.m_iTop + 2, m_anyFont, { 219, 219, 219 }, m_prgpEntries.first[i]);
              }
          }
      }
      */
      if (m_bIsOpened)
      {
          // keep widget focused
          std::reinterpret_pointer_cast<FGUI::CContainer>(GetParentWidget())->SetFocusedWidget(shared_from_this());

          m_dmSize.m_iHeight = m_iEntrySpacing + (m_prgpEntries.first.size() * m_iEntrySpacing) + 2;
      }
    else
    {
      // restore widget size
      m_dmSize.m_iHeight = m_dmBackupSize.m_iHeight;
    }
  }

  void CComboBox::Input()
  {
    FGUI::AREA arWidgetRegion = { GetAbsolutePosition().m_iX, GetAbsolutePosition().m_iY, m_dmSize.m_iWidth, m_dmBackupSize.m_iHeight };

    // toggle dropdown list on and off
    if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))
    {
      m_bIsOpened = !m_bIsOpened;
    }

    if (m_bIsOpened)
    {
      if (!FGUI::INPUT.IsCursorInArea(arWidgetRegion))
      {
        for (std::size_t i = 0; i < m_prgpEntries.first.size(); i++)
        {
          FGUI::AREA arEntryRegion = { arWidgetRegion.m_iLeft, (arWidgetRegion.m_iTop + 21) + (static_cast<int>(i) * m_iEntrySpacing), arWidgetRegion.m_iRight, m_iEntrySpacing };

          if (FGUI::INPUT.IsCursorInArea(arEntryRegion))
          {
            // select an entry
            m_ullSelectedEntry = i;

            if (m_ullSelectedEntry == i)
            {
              if (m_fnctCallback)
              {
                // call function
                m_fnctCallback();
              }

              // close dropdown list after selecting something
              m_bIsOpened = false;
            }
          }
        }
      }
    }
  }

  void CComboBox::Save(nlohmann::json& module)
  {
    // remove spaces from widget name
    std::string strFormatedWidgetName = GetTitle();
    std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

    module[strFormatedWidgetName] = m_ullSelectedEntry;
  }

  void CComboBox::Load(nlohmann::json& module)
  {
    // remove spaces from widget name
    std::string strFormatedWidgetName = GetTitle();
    std::replace(strFormatedWidgetName.begin(), strFormatedWidgetName.end(), ' ', '_');

    // change widget selected entry to the one stored on file
    if (module.contains(strFormatedWidgetName))
    {
      m_ullSelectedEntry = module[strFormatedWidgetName];
    }
  }

  void CComboBox::Tooltip()
  {
    if (m_strTooltip.length() > 1 && !m_bIsOpened)
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