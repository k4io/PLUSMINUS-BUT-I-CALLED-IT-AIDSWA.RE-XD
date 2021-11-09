//
// FGUI - DirectX 11 Renderer
//

#pragma once

#define FGUI_IMPLEMENTATION
#include "FGUI/FGUI.hpp"

namespace FGUI_D3D11
{
	inline void CreateFont(FGUI::FONT& _font, std::string _family, int _size, int _flags, bool _bold) // TODO: handle font flags
	{
		FGUI::FONT cFont = new FGUI::CFont();
		cFont->size = _size;

		_font = cFont;
	}

	inline FGUI::DIMENSION GetScreenSize()
	{
		return { (int)screen_size.x, (int)screen_size.y };
	}

	inline FGUI::DIMENSION GetTextSize(FGUI::FONT _font, std::string _text)
	{
		auto text_size = Renderer::get_text_size(StringConverter::ToUnicode(_text), _font->size);
		return { (int)text_size.x, (int)text_size.y };
	}

	inline void Image(int x, int y, int w, int h, const wchar_t* filename)
	{
		Renderer::draw_image(x, y, w, h, filename);
	}

	inline void Text(int _x, int _y, FGUI::FONT _font, FGUI::COLOR _color, std::string _text)
	{
		Renderer::text({ (float)_x, (float)_y }, Color3(_color.m_ucRed, _color.m_ucGreen, _color.m_ucBlue, _color.m_ucAlpha), _font->size, false, true, StringConverter::ToUnicode(_text));
	}

	inline void Icon(int _x, int _y, FGUI::FONT _font, FGUI::COLOR _color, std::string _text)
	{
		Renderer::icon({ (float)_x, (float)_y }, Color3(_color.m_ucRed, _color.m_ucGreen, _color.m_ucBlue, _color.m_ucAlpha), _font->size, false, true, StringConverter::ToUnicode(_text));
	}

	inline void Circle(float x, float y, FGUI::COLOR color, float radius, float thickness)
	{
		Renderer::circle({x, y}, Color3(color.m_ucRed, color.m_ucGreen, color.m_ucBlue, color.m_ucAlpha), radius, thickness);
	}

	inline void Rectangle(int _x, int _y, int _width, int _height, FGUI::COLOR _color)
	{
		Renderer::rectangle_filled({ (float)_x, (float)_y }, { (float)_width, (float)_height }, Color3(_color.m_ucRed, _color.m_ucGreen, _color.m_ucBlue, _color.m_ucAlpha));
	}

	inline void Outline(int _x, int _y, int _width, int _height, FGUI::COLOR _color)
	{
		// WARNING: do not repeat this at home.
		Rectangle(_x, _y, _width, 1, _color);
		Rectangle(_x, _y, 1, _height, _color);
		Rectangle(_x + _width - 1, _y, 1, _height, _color);
		Rectangle(_x, _y + _height - 1, _width, 1, _color);
	}

	inline void Rounded_Rectangle_Filled(int _x, int _y, int _w, int _h, FGUI::COLOR _color, float smoothness = 10.f)
	{
		Renderer::rounded_rectangle_filled(_x, _y, _w, _h, Color3(_color.m_ucRed, _color.m_ucGreen, _color.m_ucBlue, _color.m_ucAlpha), smoothness);
	}

	inline void Rounded_Rectangle(int _x, int _y, int _w, int _h, FGUI::COLOR _color, float smoothness = 10.f)
	{
		Renderer::rounded_box(_x, _y, _w, _h, Color3(_color.m_ucRed, _color.m_ucGreen, _color.m_ucBlue, _color.m_ucAlpha), smoothness);
	}

	inline void Line(int _from_x, int _from_y, int _to_x, int _to_y, FGUI::COLOR _color, float thickness = 0.f)
	{
		Renderer::line({ (float)_from_x, (float)_from_y }, { (float)_to_x, (float)_to_y }, Color3(_color.m_ucRed, _color.m_ucGreen, _color.m_ucBlue, _color.m_ucAlpha), thickness);
	}

	inline void Gradient(int _x, int _y, int _width, int _height, FGUI::COLOR _color1, FGUI::COLOR _color2, bool _horizontal)
	{
		Renderer::gradient_rect({ (float)_x, (float)_y }, { (float)_width, (float)_height }, Color3(_color1.m_ucRed, _color1.m_ucGreen, _color1.m_ucBlue, _color1.m_ucAlpha), Color3(_color2.m_ucRed, _color2.m_ucGreen, _color2.m_ucBlue, _color2.m_ucAlpha), _horizontal);
	}

	// NOTE: call this function only once (preferably when you initialize your application)
	inline void OnEntryPoint()
	{
		FGUI::RENDER.CreateFont = FGUI_D3D11::CreateFont;
		FGUI::RENDER.GetScreenSize = FGUI_D3D11::GetScreenSize;
		FGUI::RENDER.GetTextSize = FGUI_D3D11::GetTextSize;
		FGUI::RENDER.Rectangle = FGUI_D3D11::Rectangle;
		FGUI::RENDER.Outline = FGUI_D3D11::Outline;
		FGUI::RENDER.Line = FGUI_D3D11::Line;
		FGUI::RENDER.Text = FGUI_D3D11::Text;
		FGUI::RENDER.Gradient = FGUI_D3D11::Gradient;
		FGUI::RENDER.Icon = FGUI_D3D11::Icon;
		FGUI::RENDER.Circle = FGUI_D3D11::Circle;
		FGUI::RENDER.RoundedRectangleFilled = FGUI_D3D11::Rounded_Rectangle_Filled;
		FGUI::RENDER.RoundedRectangle = FGUI_D3D11::Rounded_Rectangle;
		FGUI::RENDER.Image = FGUI_D3D11::Image;
	}
}