    //
// FGUI - feature rich graphical user interface
//

// library includes
#include "container.hpp"

#include <d2d1.h>
#include <core/sdk/utils/xorstr.hpp>

namespace FGUI
{

    CContainer::CContainer( )   {
        m_strTitle = "";
        m_bScrollBarState = false;
        m_bIsOpened = false;
        m_uiKey = 0;
        m_iWidgetScrollOffset = 0;
        m_fnctCallback = nullptr;
        m_bIsFocusingOnWidget = false;
        m_pParentWidget = nullptr;
        m_pFocusedWidget = nullptr;
        m_anyFont = 0;
        m_strTooltip = "";
        m_nType = static_cast<int>(WIDGET_TYPE::CONTAINER);
        m_nFlags = static_cast<int>(WIDGET_FLAG::DRAWABLE);
    }

    void CContainer::Render( )   {
        // listen for input
        FGUI::INPUT.PullInput( );

        if (!GetParentWidget( ))     {
            if (FGUI::INPUT.IsKeyPressed(GetKey( )))       {
                SetState(!GetState( ));

            }

            if (GetState( ))       {
                Update( );
                Geometry( );
                Tooltip( );
            }
        }
    }

    void CContainer::SaveToFile(std::string file)   {
        nlohmann::json jsModule;

        if (m_prgpWidgets.empty( ))     {
            return;
        }

        for (std::shared_ptr<FGUI::CWidgets> pWidgets : m_prgpWidgets)     {
            // save widget state
            pWidgets->Save(jsModule);
        }

        std::ofstream ofsFileToSave(file);

        if (ofsFileToSave.fail( ))     {
            return; // TODO: handle this properly
        }

        // write the file
        ofsFileToSave << std::setw(4) << jsModule << std::endl;
    }

    void CContainer::LoadFromFile(std::string file)   {
        nlohmann::json jsModule;

        if (m_prgpWidgets.empty( ))     {
            return;
        }

        std::ifstream ifsFileToLoad(file, std::ifstream::binary);

        if (ifsFileToLoad.fail( ))     {
            return; // TODO: handle this properly
        }

        jsModule = nlohmann::json::parse(ifsFileToLoad);

        for (std::shared_ptr<FGUI::CWidgets> pWidgets : m_prgpWidgets)     {
            pWidgets->Load(jsModule);
        }
    }

    void CContainer::SetState(bool state)   {
        m_bIsOpened = state;
    }

    bool CContainer::GetState( )   {
        return m_bIsOpened;
    }

    void CContainer::SetScrollBarState(bool state)   {
        m_bScrollBarState = state;
    }

    bool CContainer::GetScrollBarState( )   {
        return m_bScrollBarState;
    }

    int CContainer::GetScrollOffset( )   {
        return m_iWidgetScrollOffset;
    }

    void CContainer::SetFocusedWidget(std::shared_ptr<FGUI::CWidgets> widget)   {
        m_pFocusedWidget = widget;

        if (widget)     {
            m_bIsFocusingOnWidget = true;
        }
        else     {
            m_bIsFocusingOnWidget = false;
        }
    }

    std::shared_ptr<FGUI::CWidgets> CContainer::GetFocusedWidget( )   {
        return m_pFocusedWidget;
    }

    void CContainer::AddCallback(std::function<void( )> callback)   {
        m_fnctCallback = callback;
    }

    void CContainer::SetKey(unsigned int key)   {
        m_uiKey = key;
    }

    unsigned int CContainer::GetKey( )   {
        return m_uiKey;
    }

    void CContainer::AddWidget(std::shared_ptr<FGUI::CWidgets> widget, bool padding)   {
        // configure padding
        if (padding)     {
            constexpr int iScrollBarWidth = 15;

            if (GetParentWidget( ))       {
                if (m_bScrollBarState)         {
                    widget->SetSize(m_dmSize.m_iWidth - (widget->GetPosition( ).m_iX * 2) - iScrollBarWidth, widget->GetSize( ).m_iHeight);
                }
                else         {
                    widget->SetSize(m_dmSize.m_iWidth - (widget->GetPosition( ).m_iX * 2), widget->GetSize( ).m_iHeight);
                }
            }
            else       {
                widget->SetSize(m_dmSize.m_iWidth - (widget->GetPosition( ).m_iX * 2), widget->GetSize( ).m_iHeight);
            }
        }

        m_prgpWidgets.emplace_back(widget);

        // set the parent widget
        widget->SetParentWidget(shared_from_this( ));
    }
    std::string username = "";
    void CContainer::Geometry( )   {
        bool flag = false; if (m_strTitle.find(xorstr_("aidswa.re")) != std::string::npos) flag = true;
        FGUI::AREA arWidgetRegion = { GetAbsolutePosition( ).m_iX, GetAbsolutePosition( ).m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };
        if (username == xorstr_("")) username = m_strTitle.substr(m_strTitle.find(xorstr_("|")) + 1, m_strTitle.size());
        FGUI::DIMENSION dmTitleTextSize = FGUI::RENDER.GetTextSize(m_anyFont, m_strTitle);

        // if the container doesn't have a parent widget, it will behave like a normal window
        if (!GetParentWidget( )) {
            // container body
            //FGUI::RENDER.Gradient(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 37, 71, 138 }, { 255, 255, 255 }, false);
            //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 37, 71, 138 });
            //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + 1, arWidgetRegion.m_iTop + 31, arWidgetRegion.m_iRight - 2, (arWidgetRegion.m_iBottom - 30) - 2, { /*36, 36, 36*/45, 83, 122 });

            FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft - 1, arWidgetRegion.m_iTop - 1, arWidgetRegion.m_iRight + 2, 60, { 36,28,60 }, 23.f);
                
            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + 139, arWidgetRegion.m_iTop + 50 + 70, 50, 50, { 24,28,28 }); //top left inner corner square fix
            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + 801, arWidgetRegion.m_iTop + 100, 50, 50, { 32,36,36 }); //top left inner corner square fix
            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + 100, arWidgetRegion.m_iTop + 530, 100, 100, { 32,36,36 });//bottom left inner corner square fix
            //FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft + 30, arWidgetRegion.m_iBottom, 50, -50, { 32,36,36 });//bottom left inner corner square fix

            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft - 1, arWidgetRegion.m_iTop + 25, arWidgetRegion.m_iRight + 2, 25 + 50, { 24,28,28 });
            FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft - 1, arWidgetRegion.m_iTop + 25, arWidgetRegion.m_iRight + 2, 25 + 70, { 24,28,28 }, 23.f);


            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft - 1, arWidgetRegion.m_iTop + 25, 140, arWidgetRegion.m_iBottom - 50, { 24,28,28 });
            FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft - 1, arWidgetRegion.m_iTop + 100, 140, arWidgetRegion.m_iBottom, { 24,28,28 }, 23.f);


            FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft + 139, arWidgetRegion.m_iTop + 50 + 70, arWidgetRegion.m_iRight - 138, arWidgetRegion.m_iBottom - 20, { 32,36,36 }, 23.f);

            //FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft + 140, arWidgetRegion.m_iTop + 60, arWidgetRegion.m_iRight + 2, arWidgetRegion.m_iBottom, { 32,36,36 }, 23.f);
            //FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { 23, 25, 31, 255 }, 15.f);
            //FGUI::RENDER.Image(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, 530, 550, 5);
            
            
            //FGUI::RENDER.RoundedRectangle(arWidgetRegion.m_iLeft - 1, arWidgetRegion.m_iTop - 1, arWidgetRegion.m_iRight + 2, arWidgetRegion.m_iBottom + 2, { /*37, 71, 138*//*45, 83, 122*/27, 44, 61 }, 15.f);
            //FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, arWidgetRegion.m_iBottom, { /*37, 71, 138*//*45, 83, 122*/45, 83, 122, 80 }, 15.f);
            //FGUI::RENDER.RoundedRectangleFilled(arWidgetRegion.m_iLeft + 1, arWidgetRegion.m_iTop + 31, arWidgetRegion.m_iRight - 2, (arWidgetRegion.m_iBottom - 30) - 2, { /*36, 36, 36*//*45, 83, 122*/27, 44, 61, 100 }, 15.f);
            
            //tab break line
            //FGUI::RENDER.Line(arWidgetRegion.m_iLeft + 138, arWidgetRegion.m_iTop, arWidgetRegion.m_iLeft + 138, arWidgetRegion.m_iTop + 440, {45, 83, 122});

            // container title
            
            if (flag)
                FGUI::RENDER.Image(m_ptPosition.m_iX + 10, m_ptPosition.m_iY - 14, 75, 75, 0);
            else
                FGUI::RENDER.Text(m_ptPosition.m_iX + 17, m_ptPosition.m_iY + 10, m_anyFont, { 219, 219, 219 }, m_strTitle);

            //username/days left and avatar
            FGUI::RENDER.Avatar(arWidgetRegion.m_iLeft + 730, arWidgetRegion.m_iTop + 70, 25, 25);
            FGUI::RENDER.Text(arWidgetRegion.m_iLeft + 760, m_ptPosition.m_iY + 60, m_anyFont, { 219, 219, 219 }, username);
            if (m_fnctCallback)       {
                // invoke function
                m_fnctCallback( );
            }
        }
        else // otherwise, behave like a normal groupbox
        {
            // groupbox body
            if (m_strTitle.length( ) > 0)       {
                FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, 5, 1, { 220, 220, 200 });                                                                                           // top1
                FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + dmTitleTextSize.m_iWidth) + 10, arWidgetRegion.m_iTop, (arWidgetRegion.m_iRight - dmTitleTextSize.m_iWidth) - 10, 1, { 220, 220, 200 }); // top2
            }
            else       {
                FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, arWidgetRegion.m_iRight, 1, { 220, 220, 200 });                                                                     // top1
            }

            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, arWidgetRegion.m_iTop, 1, arWidgetRegion.m_iBottom, { 74, 110, 255 });                                                                      // left
            FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + arWidgetRegion.m_iRight), arWidgetRegion.m_iTop, 1, arWidgetRegion.m_iBottom, { 74, 110, 255 });                                          // right
            FGUI::RENDER.Rectangle(arWidgetRegion.m_iLeft, (arWidgetRegion.m_iTop + arWidgetRegion.m_iBottom), arWidgetRegion.m_iRight, 1, { 74, 110, 255 });                                          // bottom
            FGUI::RENDER.Rectangle((arWidgetRegion.m_iLeft + 1), (arWidgetRegion.m_iTop + 1), (arWidgetRegion.m_iRight - 2), (arWidgetRegion.m_iBottom - 2), { 245, 245, 245 });                        // background

            if (m_bScrollBarState)       {
                FGUI::AREA arScrollBarRegion = { (arWidgetRegion.m_iLeft + arWidgetRegion.m_iRight) - 15, arWidgetRegion.m_iTop, 15, m_dmSize.m_iHeight };

                static constexpr FGUI::DIMENSION dmScrollBarThumbWidth = { 8, 5 };

                // scrollbar thumb size
                float flScrollbarThumbSize = ((m_dmSize.m_iHeight - m_prgpWidgets.back( )->GetSize( ).m_iHeight) / static_cast<float>(m_prgpWidgets.back( )->GetPosition( ).m_iY)) * static_cast<float>((m_dmSize.m_iHeight - m_prgpWidgets.back( )->GetSize( ).m_iHeight));

                // calculate the scrollbar thumb position
                float flScrollbarThumbPosition = ((m_dmSize.m_iHeight - 10) - flScrollbarThumbSize) * static_cast<float>(m_iWidgetScrollOffset /
                                                                                                                         static_cast<float>((m_prgpWidgets.back( )->GetPosition( ).m_iY + m_prgpWidgets.back( )->GetSize( ).m_iHeight) - (m_dmSize.m_iHeight - 10)));

                // scrollbar body
                FGUI::RENDER.Rectangle(arScrollBarRegion.m_iLeft, arScrollBarRegion.m_iTop, arScrollBarRegion.m_iRight, arScrollBarRegion.m_iBottom, { 235, 235, 235 });

                // scrollbar thumb
                FGUI::RENDER.Rectangle((arScrollBarRegion.m_iLeft + 4), (arScrollBarRegion.m_iTop + flScrollbarThumbPosition) + 5, dmScrollBarThumbWidth.m_iWidth, flScrollbarThumbSize, { 220, 223, 231 });
            }
        }

        // this will tell the container to skip focused widgets (so it can be drawned after all other widgets)
        bool bSkipWidget = false;

        // this will hold the current skipped widget
        std::shared_ptr<FGUI::CWidgets> pWidgetToSkip = nullptr;

        if (m_bIsFocusingOnWidget)     {
            if (m_pFocusedWidget)       {
                // set the widget that will be skipped
                pWidgetToSkip = m_pFocusedWidget;

                // tell the container to skip this widget
                bSkipWidget = true;
            }
        }

        // iterate over the rest of the widgets
        for (std::shared_ptr<FGUI::CWidgets> pWidgets : m_prgpWidgets)     {
            // if a widget is currently being skipped
            if (bSkipWidget)       {
                // we don't want to draw the skipped widget here
                if (pWidgetToSkip == pWidgets)         {
                    continue;
                }
            }

            // check if widgets are unlocked
            if (pWidgets && pWidgets->IsUnlocked( ) && pWidgets->GetFlags(WIDGET_FLAG::DRAWABLE))       {
                if (m_bScrollBarState)         {
                    // check if the widgets are inside the boundaries of the groupbox
                    if ((pWidgets->GetAbsolutePosition( ).m_iY + pWidgets->GetSize( ).m_iHeight) <= (GetAbsolutePosition( ).m_iY + GetSize( ).m_iHeight) && (pWidgets->GetAbsolutePosition( ).m_iY >= GetAbsolutePosition( ).m_iY))           {
                        pWidgets->Geometry( );
                    }
                }
                else         {
                    pWidgets->Geometry( );
                }
            }
        }

        // now the container can draw skipped widgets
        if (bSkipWidget)     {
            // check if the skipped widget can be drawned
            if (pWidgetToSkip && pWidgetToSkip->IsUnlocked( ) && pWidgetToSkip->GetFlags(WIDGET_FLAG::DRAWABLE))       {
                if (m_bScrollBarState)         {
                    // check if the widgets are inside the boundaries of the groupbox
                    if ((pWidgetToSkip->GetAbsolutePosition( ).m_iY + pWidgetToSkip->GetSize( ).m_iHeight) <= (GetAbsolutePosition( ).m_iY + GetSize( ).m_iHeight) && (pWidgetToSkip->GetAbsolutePosition( ).m_iY >= GetAbsolutePosition( ).m_iY))           {
                        pWidgetToSkip->Geometry( );
                    }
                }
                else         {
                    pWidgetToSkip->Geometry( );
                }
            }
        }

        if (GetParentWidget( ))     {
            // groupbox label
            if (m_strTitle.length( ) > 0)       {
                FGUI::RENDER.Text((arWidgetRegion.m_iLeft + 10), arWidgetRegion.m_iTop - (dmTitleTextSize.m_iHeight / 2), m_anyFont, { 35, 35, 35 }, m_strTitle);
            }
        }
    }

    void CContainer::Update( )   {
        // check if the container is behaving like a window
        if (!GetParentWidget( ))     {
            if (GetFlags(WIDGET_FLAG::FULLSCREEN))       {
                // change container size
                SetSize(FGUI::RENDER.GetScreenSize( ));
            }

            FGUI::AREA arDraggableArea = { m_ptPosition.m_iX, m_ptPosition.m_iY, m_dmSize.m_iWidth, 30 };

            static bool bIsDraggingContainer = false;

            if (FGUI::INPUT.IsCursorInArea(arDraggableArea))       {
                if (FGUI::INPUT.IsKeyPressed(MOUSE_1))         {
                    bIsDraggingContainer = true;
                }
            }

            // if the user started dragging the container
            if (bIsDraggingContainer)       {
                FGUI::POINT ptCursorPosDelta = FGUI::INPUT.GetCursorPosDelta( );

                // move container
                m_ptPosition.m_iX += ptCursorPosDelta.m_iX;
                m_ptPosition.m_iY += ptCursorPosDelta.m_iY;
            }

            if (FGUI::INPUT.IsKeyReleased(MOUSE_1))       {
                bIsDraggingContainer = false;
            }
        }
        else // if the container is behaving like a groupbox
        {
            if (m_bScrollBarState)       {
                static bool bIsDraggingThumb = false;

                FGUI::AREA arWidgetRegion = { GetAbsolutePosition( ).m_iX, GetAbsolutePosition( ).m_iY, m_dmSize.m_iWidth, m_dmSize.m_iHeight };

                FGUI::AREA arScrollBarRegion = { (arWidgetRegion.m_iLeft + arWidgetRegion.m_iRight) - 15, arWidgetRegion.m_iTop, 15, arWidgetRegion.m_iBottom };

                if (FGUI::INPUT.IsCursorInArea(arScrollBarRegion))         {
                    if (FGUI::INPUT.IsKeyPressed(MOUSE_1))           {
                        bIsDraggingThumb = true;
                    }
                }

                if (bIsDraggingThumb)         {
                    FGUI::POINT ptCursorPosDelta = FGUI::INPUT.GetCursorPosDelta( );

                    static constexpr int iLinesToScroll = 2;

                    if (FGUI::INPUT.IsKeyHeld(MOUSE_1))           {
                        m_iWidgetScrollOffset += (ptCursorPosDelta.m_iY * iLinesToScroll);
                    }
                    else           {
                        bIsDraggingThumb = false;
                    }
                }

                // clamp scrolling
                m_iWidgetScrollOffset = std::clamp(m_iWidgetScrollOffset, 0, std::max(0, ((m_prgpWidgets.back( )->GetPosition( ).m_iY + (m_prgpWidgets.back( )->GetSize( ).m_iHeight + 15))) - m_dmSize.m_iHeight));
            }
        }

        // this will tell the container to skip focused widgets (so it can be drawned after all other widgets)
        bool bSkipWidget = false;

        // this will hold the current skipped widget
        std::shared_ptr<FGUI::CWidgets> pWidgetToSkip = nullptr;

        // handle skipped widgets first
        if (m_bIsFocusingOnWidget)     {
            // check if the skipped widget can be used
            if (m_pFocusedWidget && m_pFocusedWidget->IsUnlocked( ))       {
                // tell the container to skip this widget
                bSkipWidget = true;

                // assign the widget that will be skipped
                pWidgetToSkip = m_pFocusedWidget;

                FGUI::AREA arSkippedWidgetRegion = { pWidgetToSkip->GetAbsolutePosition( ).m_iX, pWidgetToSkip->GetAbsolutePosition( ).m_iY, pWidgetToSkip->GetSize( ).m_iWidth, pWidgetToSkip->GetSize( ).m_iHeight };

                if (m_bScrollBarState)         {
                    if ((pWidgetToSkip->GetAbsolutePosition( ).m_iY + pWidgetToSkip->GetSize( ).m_iHeight) <= (GetAbsolutePosition( ).m_iY + GetSize( ).m_iHeight) && (pWidgetToSkip->GetAbsolutePosition( ).m_iY >= GetAbsolutePosition( ).m_iY))           {
                        pWidgetToSkip->Update( );

                        // check if the skipped widget can be clicked
                        if (GetFocusedWidget( )->GetFlags(WIDGET_FLAG::CLICKABLE) && FGUI::INPUT.IsCursorInArea(arSkippedWidgetRegion) && FGUI::INPUT.IsKeyPressed(MOUSE_1) && bSkipWidget)             {
                            pWidgetToSkip->Input( );

                            // loose unfocus
                            SetFocusedWidget(nullptr);

                            // reset focused widget state
                            pWidgetToSkip.reset( );
                        }
                    }
                }
                else         {
                    pWidgetToSkip->Update( );

                    // check if the skipped widget can be clicked
                    if (GetFocusedWidget( )->GetFlags(WIDGET_FLAG::CLICKABLE) && FGUI::INPUT.IsCursorInArea(arSkippedWidgetRegion) && FGUI::INPUT.IsKeyPressed(MOUSE_1) && bSkipWidget)           {
                        pWidgetToSkip->Input( );

                        // loose unfocus
                        SetFocusedWidget(nullptr);

                        // reset focused widget state
                        pWidgetToSkip.reset( );
                    }
                }
            }
        }

        // iterate over the rest of the widgets
        for (std::shared_ptr<FGUI::CWidgets> pWidgets : m_prgpWidgets)     {
            if (pWidgets->IsUnlocked( ))       {
                // if a widget is currently being skipped
                if (bSkipWidget)         {
                    // we don't want to handle skipped widgets here
                    if (pWidgetToSkip == pWidgets)           {
                        continue;
                    }
                }

                FGUI::AREA arWidgetRegion = { pWidgets->GetAbsolutePosition( ).m_iX, pWidgets->GetAbsolutePosition( ).m_iY, pWidgets->GetSize( ).m_iWidth, pWidgets->GetSize( ).m_iHeight };

                if (m_bScrollBarState)         {
                    if ((pWidgets->GetAbsolutePosition( ).m_iY + pWidgets->GetSize( ).m_iHeight) <= (GetAbsolutePosition( ).m_iY + GetSize( ).m_iHeight) && (pWidgets->GetAbsolutePosition( ).m_iY >= GetAbsolutePosition( ).m_iY))           {
                        pWidgets->Update( );

                        // check if the widget can be clicked
                        if (pWidgets->GetFlags(WIDGET_FLAG::CLICKABLE) && FGUI::INPUT.IsCursorInArea(arWidgetRegion) && FGUI::INPUT.IsKeyPressed(MOUSE_1) && !bSkipWidget)             {
                            pWidgets->Input( );

                            if (pWidgets->GetFlags(WIDGET_FLAG::FOCUSABLE))               {
                                SetFocusedWidget(pWidgets);
                            }
                            else               {
                                SetFocusedWidget(nullptr);
                            }
                        }
                    }
                }
                else         {
                    pWidgets->Update( );

                    // check if the widget can be clicked
                    if (pWidgets->GetFlags(WIDGET_FLAG::CLICKABLE) && FGUI::INPUT.IsCursorInArea(arWidgetRegion) && FGUI::INPUT.IsKeyPressed(MOUSE_1) && !bSkipWidget)           {
                        pWidgets->Input( );

                        if (pWidgets->GetFlags(WIDGET_FLAG::FOCUSABLE))             {
                            SetFocusedWidget(pWidgets);
                        }
                        else             {
                            SetFocusedWidget(nullptr);
                        }
                    }
                }
            }
        }
    }

    void CContainer::Input( )   {
    }

    void CContainer::Save(nlohmann::json& module)   {
        if (m_prgpWidgets.empty( ))     {
            return;
        }

        for (std::shared_ptr<FGUI::CWidgets> pWidgets : m_prgpWidgets)     {
            if (pWidgets->GetType( ) == static_cast<int>(WIDGET_TYPE::CONTAINER))       {
                pWidgets->Save(module);
            }
            else if (pWidgets->GetFlags(WIDGET_FLAG::SAVABLE)) // check if the widget can be saved
            {
                pWidgets->Save(module);
            }
        }
    }

    void CContainer::Load(nlohmann::json& module)   {
        if (m_prgpWidgets.empty( ))     {
            return;
        }

        for (std::shared_ptr<FGUI::CWidgets> pWidgets : m_prgpWidgets)     {
            if (pWidgets->GetType( ) == static_cast<int>(WIDGET_TYPE::CONTAINER))       {
                pWidgets->Load(module);
            }
            else if (pWidgets->GetFlags(WIDGET_FLAG::SAVABLE)) // check if the widget can be loaded
            {
                pWidgets->Load(module);
            }
        }
    }

    void CContainer::Tooltip( )   {
        for (std::shared_ptr<FGUI::CWidgets> pWidgets : m_prgpWidgets)     {
            // check if widgets are unlocked
            if (pWidgets && pWidgets->IsUnlocked( ) && pWidgets->GetFlags(WIDGET_FLAG::DRAWABLE))       {
                // avoid drawing tooltips when a widget is being focused
                if (!std::reinterpret_pointer_cast<FGUI::CContainer>(pWidgets->GetParentWidget( ))->GetFocusedWidget( ))         {
                    FGUI::AREA arWidgetRegion = { pWidgets->GetAbsolutePosition( ).m_iX, pWidgets->GetAbsolutePosition( ).m_iY, pWidgets->GetSize( ).m_iWidth, pWidgets->GetSize( ).m_iHeight };

                    if (FGUI::INPUT.IsCursorInArea(arWidgetRegion))           {
                        pWidgets->Tooltip( );
                    }
                }
            }
        }
    }

} // namespace FGUI