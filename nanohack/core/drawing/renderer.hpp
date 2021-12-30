#pragma once
#include <memory>
#include <D3D11.h>
#include <d3d9.h>
#include <D2D1.h>
#include <d2d1_1.h>
#include <unordered_map>
#include <dwrite_1.h>
#include <string_view>

#include <wincodec.h>

#pragma comment( lib, "dxgi" )
#pragma comment( lib, "d2d1" )
#pragma comment( lib, "dcomp" )
#pragma comment( lib, "dwrite" )
#pragma comment( lib, "WindowsCodecs.lib" )

#define M_PI 3.14159265358979323846f
#define D3DXToRadian(degree) ((degree) * (M_PI / 180.0f))

#define RET_CHK(x) if ( x != S_OK ) return

namespace Renderer {
	ID2D1Factory* m_pInterface;
	ID2D1RenderTarget* m_pCanvas;
	IDWriteFactory1* m_pTextEngine;
	IDWriteTextFormat* m_pTextFormat;
	IDWriteTextFormat* m_pIconFormat;
	ID2D1SolidColorBrush* m_pSolidBrush;
	bool initialized = false;

	ID2D1Bitmap* bitmaps[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	ID2D1Bitmap* custom_box_bitmap = NULL;

	ID2D1Bitmap* boxBitmap = nullptr;

	HRESULT LoadBitmapFromFile(const wchar_t* filename, ID2D1Bitmap** pBitmap, bool custombox = false)
	{
		HRESULT hr = S_FALSE;
		IWICImagingFactory* wic_factory = NULL;
		IWICBitmapDecoder* decoder = NULL;
		IWICBitmapFrameDecode* frame = NULL;
		IWICFormatConverter* converter = NULL;

		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void**>(&wic_factory));
		if FAILED(hr) goto clenaup;

		hr = wic_factory->CreateDecoderFromFilename(filename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
		if FAILED(hr) goto clenaup;

		hr = decoder->GetFrame(0, &frame);
		if FAILED(hr) goto clenaup;

		hr = wic_factory->CreateFormatConverter(&converter);
		if FAILED(hr) goto clenaup;

		hr = converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		if FAILED(hr) goto clenaup;

		if(custombox)
			hr = m_pCanvas->CreateBitmapFromWicBitmap(converter, 0, &boxBitmap);
		else
			hr = m_pCanvas->CreateBitmapFromWicBitmap(converter, 0, pBitmap);
		if FAILED(hr) goto clenaup;

	clenaup:
		decoder->Release();
		converter->Release();
		frame->Release();
		wic_factory->Release();
		return hr;
	}


	bool Init(IDXGISwapChain* SwapChain) {
		if (!initialized) {
			initialized = true;
			D2D1_FACTORY_OPTIONS CreateOpt = { D2D1_DEBUG_LEVEL_NONE };
			DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pTextEngine), (IUnknown**)&m_pTextEngine);
			D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &CreateOpt, (void**)&m_pInterface);

			IDWriteFontCollection* pFontCollection = NULL;
			HRESULT hr = m_pTextEngine->GetSystemFontCollection(&pFontCollection);


			m_pTextEngine->CreateTextFormat(wxorstr_(L"MinecraftCHMC"), NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.f, L"", &m_pTextFormat);
			
			if (!m_pInterface || !m_pTextEngine || !m_pTextFormat || !m_pIconFormat) return false;
		}

		ID3D11Device* d3d_device;
		if (SwapChain->GetDevice(IID_PPV_ARGS(&d3d_device))) 
			return false;

		WORD flagsOffset = *(WORD*)((*(uintptr_t**)d3d_device)[ 38 ] + 2);
		int& flags = *(INT*)((uintptr_t)d3d_device + flagsOffset);
		d3d_device->Release( );

		IDXGISurface* d3d_bbuf;
		if (SwapChain->GetBuffer(0, IID_PPV_ARGS(&d3d_bbuf)))
			return false;

		D2D1_RENDER_TARGET_PROPERTIES d2d_prop = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

		// set flags just for target creating,
		flags |= 0x20;
		HRESULT canvas_state = m_pInterface->CreateDxgiSurfaceRenderTarget(d3d_bbuf, d2d_prop, &m_pCanvas); 
		flags &= ~0x20;

		d3d_bbuf->Release( );

		if (canvas_state)
			return false;

		if (!m_pSolidBrush)
			m_pCanvas->CreateSolidColorBrush({}, &m_pSolidBrush);

		//create images
		std::wstring data_dir(settings::data_dir.begin(), settings::data_dir.end());

		//logo image is always first in index
		std::wstring logo_dir = data_dir + wxorstr_(L"\\images\\awlogo.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[0])))
			return false;
		logo_dir = data_dir + wxorstr_(L"\\images\\weapon.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[1])))
			return false;
		logo_dir = data_dir + wxorstr_(L"\\images\\visuals.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[2])))
			return false;
		logo_dir = data_dir + wxorstr_(L"\\images\\misc.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[3])))
			return false;
		logo_dir = data_dir + wxorstr_(L"\\images\\color.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[4])))
			return false;
		logo_dir = data_dir + wxorstr_(L"\\images\\menu.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[5])))
			return false;
		logo_dir = data_dir + wxorstr_(L"\\images\\avatar.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[6])))
			return false;
		logo_dir = data_dir + wxorstr_(L"\\images\\checked.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[7])))
			return false;
		logo_dir = data_dir + wxorstr_(L"\\images\\unchecked.png");
		if (!SUCCEEDED(LoadBitmapFromFile(logo_dir.c_str(), &bitmaps[8])))
			return false;

		return true;
	}

	bool new_frame(IDXGISwapChain* SwapChain) {
		if (!m_pCanvas && !Init(SwapChain))
			return false;

		m_pCanvas->BeginDraw( );
		return true;
	}

	void reset_canvas( ) {
		if (m_pCanvas) {
			m_pCanvas->Release( );
			m_pCanvas = nullptr;
		}
	}
	
	/*
	void draw_custom_box(float x, float y, float w, float h)
	{
		ID2D1Bitmap* image = (ID2D1Bitmap*)settings::custom_image_box;
		D2D1_SIZE_F size = image->GetSize();
		m_pCanvas->DrawBitmap(image, D2D1::RectF(x, y, x + w, y + h), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, size.width, size.height));
	}
	*/
	void draw_avatar(float x, float y, float w, float h)
	{
		ID2D1Layer* pLayer = NULL;
		HRESULT hr = m_pCanvas->CreateLayer(NULL, &pLayer);
		if (SUCCEEDED(hr))
		{
			//m_pCanvas->SetTransform(D2D1::Matrix3x2F::Translation(350, 50));

			ID2D1EllipseGeometry* geo;
			D2D1_ELLIPSE base{ { x, y }, w, h };
			m_pInterface->CreateEllipseGeometry(&base, &geo);

			// Push the layer with the geometric mask.

			m_pCanvas->PushLayer(
				D2D1::LayerParameters(D2D1::InfiniteRect(), geo),
				pLayer
			);

			D2D1_RECT_F sz { x - w, y - h, x + w, y + h};
			m_pCanvas->DrawBitmap(bitmaps[6], &sz);
			m_pCanvas->DrawEllipse({ { x, y }, w, h }, m_pSolidBrush);
			m_pCanvas->PopLayer();
		}
	}

	void draw_image(float x, float y, float w, float h, int file)
	{
		D2D1_SIZE_F size = bitmaps[file]->GetSize();

		m_pCanvas->DrawBitmap(bitmaps[file], D2D1::RectF(x, y, x + w, y + h), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, size.width, size.height));
	}
	
	void set_custom_box(std::wstring image_path)
	{
		std::wstring data_dir(settings::data_dir.begin(), settings::data_dir.end());

		//logo image is always first in index
		std::wstring logo_dir = data_dir + wxorstr_(L"\\images\\") + image_path;
		if (!SUCCEEDED(LoadBitmapFromFile(image_path.c_str(), &boxBitmap, true)))
			return;
	}

	void custom_box(float x, float y, float w, float h)
	{
		if (!boxBitmap)
			return;
		D2D1_SIZE_F size = boxBitmap->GetSize();
		m_pCanvas->DrawBitmap(boxBitmap, D2D1::RectF(x, y, x + w, y + h), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, size.width, size.height));
	}
	
	void rectangle_filled(Vector2 pos, Vector2 size, const Color3 color) {
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		m_pCanvas->FillRectangle(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), m_pSolidBrush);
	}

	void rectangle(Vector2 pos, Vector2 size, const Color3 color, float thickness = 1.2f) {
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		m_pCanvas->DrawRectangle(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), m_pSolidBrush, thickness, nullptr);
	}

	void rectangle_filled(Square2 rec, const Color3 color) {
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		m_pCanvas->FillRectangle(D2D1::RectF(rec.pos.x, rec.pos.y, rec.pos.x + rec.size.x, rec.pos.y + rec.size.y), m_pSolidBrush);
	}

	void rectangle(Square2 rec, const Color3 color, float thickness = 1.2f) {
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		m_pCanvas->DrawRectangle(D2D1::RectF(rec.pos.x, rec.pos.y, rec.pos.x + rec.size.x, rec.pos.y + rec.size.y), m_pSolidBrush, thickness, nullptr);
	}

	void line(const Vector2 start, const Vector2 end, Color3 color, bool outl = false, float thickness = 1.f) {
		if (outl) {
			m_pSolidBrush->SetColor(D2D1::ColorF(D3DCOLOR_RGBA(0, 0, 0, 255), 1.f));
			m_pCanvas->DrawLine({ start.x, start.y }, { end.x, end.y }, m_pSolidBrush, thickness * 2);
		}
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		m_pCanvas->DrawLine({ start.x, start.y }, { end.x, end.y }, m_pSolidBrush, thickness);
	}

	void circle(const Vector2 start, Color3 color, float radius, float thickness) {
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		m_pCanvas->DrawEllipse({ { start.x, start.y }, radius, radius }, m_pSolidBrush, thickness);
	}

	void filled_circle(const Vector2 start, Color3 color, float radius) {
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		m_pCanvas->FillEllipse({ { start.x, start.y}, radius,radius }, m_pSolidBrush);
	}

	void rounded_box(int x, int y, int w, int h, Color3 color, float rounding = 10.f) {
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		D2D1_RECT_F dre = { x, y, x + w, y + h };
		m_pCanvas->DrawRoundedRectangle({ dre, rounding, rounding }, m_pSolidBrush);
	}

	void rounded_rectangle_filled(int x, int y, int w, int h, Color3 color, float rounding) {
		m_pSolidBrush->SetColor(D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
		//m_pCanvas->FillRoundedRectangle({ { x, y, x + w, y + h }, rounding, rounding }, m_pSolidBrush);

		D2D1_RECT_F dre = {x, y, x + w, y + h};
		//m_pCanvas->DrawRoundedRectangle({ dre, rounding, rounding }, m_pSolidBrush);
		m_pCanvas->FillRoundedRectangle({ dre, rounding, rounding }, m_pSolidBrush);
	}

	Vector2 get_text_size(std::wstring text, float sz)	{
		const auto str = text;
		const auto str_len = static_cast<std::uint32_t>(str.length( ));

		IDWriteTextLayout* dwrite_layout = nullptr;
		RET_CHK(m_pTextEngine->CreateTextLayout(str.c_str(), str_len, m_pTextFormat, screen_size.x, screen_size.y, &dwrite_layout)) Vector2(0,0);

		const DWRITE_TEXT_RANGE range
		{
			0,
			str_len
		};

		dwrite_layout->SetFontSize(sz, range);

		DWRITE_TEXT_METRICS metrics;
		dwrite_layout->GetMetrics(&metrics);

		return Vector2(metrics.width, metrics.height);
	}


	void gradient_rect(Vector2 pos, Vector2 size, const Color3 color, const Color3 color_2, bool horizontal) {
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES linearGradientBrushProperties = {};
		ID2D1GradientStopCollection* pGradientStops = NULL;
		ID2D1LinearGradientBrush* m_pLinearGradientBrush;

		D2D1_GRADIENT_STOP gradientStops[ 2 ];
		gradientStops[ 0 ].color = D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
		gradientStops[ 0 ].position = 0.0f;
		gradientStops[ 1 ].color = D2D1::ColorF(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
		gradientStops[ 1 ].position = 1.0f;

		RET_CHK(m_pCanvas->CreateGradientStopCollection(
			gradientStops,
			2,
			D2D1_GAMMA_2_2,
			D2D1_EXTEND_MODE_CLAMP,
			&pGradientStops
		));

		RET_CHK(m_pCanvas->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(pos.x, pos.y),
				D2D1::Point2F(pos.x + size.x, horizontal ? pos.y : pos.y + size.y)
			),
			pGradientStops,
			&m_pLinearGradientBrush
		));
		m_pCanvas->FillRectangle(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), m_pLinearGradientBrush);
	}


	template <typename ...Args>
	void text(const Vector2 pos, const Color3 clr, const float sz, bool center, bool outline, const std::wstring_view text, Args&&... args) {
		const auto size = static_cast<std::size_t>(std::swprintf(nullptr, 0, text.data(), std::forward<Args>(args)...) + 1);

		const std::unique_ptr<wchar_t[]> buffer(new wchar_t[size]);
		std::swprintf(buffer.get(), size, text.data(), std::forward<Args>(args)...);

		const auto str = std::wstring(buffer.get(), buffer.get() + size - 1);
		const auto str_len = static_cast<std::uint32_t>(str.size());

		IDWriteTextLayout* dwrite_layout = nullptr;
		RET_CHK(m_pTextEngine->CreateTextLayout(str.c_str(), str_len, m_pTextFormat, screen_size.x, screen_size.y, &dwrite_layout));

		const DWRITE_TEXT_RANGE range
		{
			0,
			str_len
		};

		dwrite_layout->SetFontSize(sz, range);

		if (center) {
			m_pSolidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
			DWRITE_TEXT_METRICS TextInfo;
			dwrite_layout->GetMetrics(&TextInfo);
			Vector2 TextSize = { TextInfo.width / 2.f, TextInfo.height / 2.f };
			const auto x = pos.x - TextSize.x;
			const auto y = pos.y - TextSize.y;
			if (outline) {

				m_pCanvas->DrawTextLayout(D2D1::Point2F(x - 1, y), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x + 1, y), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y - 1), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y + 1), dwrite_layout, m_pSolidBrush);
			}

			m_pSolidBrush->SetColor(D2D1::ColorF(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f, clr.a / 255.f));

			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y), dwrite_layout, m_pSolidBrush);
			dwrite_layout->Release();
			return;
		}


		m_pSolidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

		if (outline) {
			const auto x = pos.x;
			const auto y = pos.y;

			m_pCanvas->DrawTextLayout(D2D1::Point2F(x - 1, y), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x + 1, y), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y - 1), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y + 1), dwrite_layout, m_pSolidBrush);
		}

		m_pSolidBrush->SetColor(D2D1::ColorF(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f, clr.a / 255.f));
		m_pCanvas->DrawTextLayout(D2D1::Point2F(pos.x, pos.y), dwrite_layout, m_pSolidBrush);
		dwrite_layout->Release();
	}

	void ProgressBar(const Vector2 start, 
		Vector2 end,
		Color3 fgcolor,
		Color3 bgcolor, 
		float value,
		float z,
		float actual = 0.0f)
	{
		rectangle_filled(start, { end.x - start.x, 6 }, bgcolor);
		float f = end.x;
		end.x = (start.x + (z * value));
		if (end.x > f) end.x = f - 2;
		rectangle_filled({ start.x + 1, start.y + 1 }, { (end.x - start.x) * (z / value), 4}, fgcolor);
		if(actual == 0.0f)
			text({ (start.x) + ((end.x - start.x) * (z / value)), start.y + 2 }, Color3(219, 219, 219), 14.f, true, true, wxorstr_(L"%.2f"), (float)(z / value));
		else 
			text({ (start.x) + ((end.x - start.x) * (z / value)), start.y + 2 }, Color3(219, 219, 219), 14.f, true, true, wxorstr_(L"%.2f"), (float)(z / value));
	}

	template <typename ...Args>
		void icon(const Vector2 pos, const Color3 clr, const float sz, bool center, bool outline, const std::wstring_view text, Args&&... args) {
		const auto size = static_cast<std::size_t>(std::swprintf(nullptr, 0, text.data(), std::forward<Args>(args)...) + 1);

		const std::unique_ptr<wchar_t[]> buffer(new wchar_t[size]);
		std::swprintf(buffer.get(), size, text.data(), std::forward<Args>(args)...);

		const auto str = std::wstring(buffer.get(), buffer.get() + size - 1);
		const auto str_len = static_cast<std::uint32_t>(str.size());

		IDWriteTextLayout* dwrite_layout = nullptr;
		RET_CHK(m_pTextEngine->CreateTextLayout(str.c_str(), str_len, m_pIconFormat, screen_size.x, screen_size.y, &dwrite_layout));

		const DWRITE_TEXT_RANGE range
		{
			0,
			str_len
		};

		dwrite_layout->SetFontSize(sz, range);

		if (center) {
			m_pSolidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
			DWRITE_TEXT_METRICS TextInfo;
			dwrite_layout->GetMetrics(&TextInfo);
			Vector2 TextSize = { TextInfo.width / 2.f, TextInfo.height / 2.f };
			const auto x = pos.x - TextSize.x;
			const auto y = pos.y - TextSize.y;
			if (outline) {

				m_pCanvas->DrawTextLayout(D2D1::Point2F(x - 1, y), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x + 1, y), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y - 1), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y + 1), dwrite_layout, m_pSolidBrush);
			}

			m_pSolidBrush->SetColor(D2D1::ColorF(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f, clr.a / 255.f));

			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y), dwrite_layout, m_pSolidBrush);
			dwrite_layout->Release();
			return;
		}


		m_pSolidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

		if (outline) {
			const auto x = pos.x;
			const auto y = pos.y;

			m_pCanvas->DrawTextLayout(D2D1::Point2F(x - 1, y), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x + 1, y), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y - 1), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y + 1), dwrite_layout, m_pSolidBrush);
		}

		m_pSolidBrush->SetColor(D2D1::ColorF(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f, clr.a / 255.f));
		m_pCanvas->DrawTextLayout(D2D1::Point2F(pos.x, pos.y), dwrite_layout, m_pSolidBrush);
		dwrite_layout->Release();
	}
	template <typename ...Args>
	void boldtext(const Vector2 pos, const Color3 clr, const float sz, bool center, bool outline, const std::wstring_view text, Args&&... args) {
		const auto size = static_cast<std::size_t>(std::swprintf(nullptr, 0, text.data( ), std::forward<Args>(args)...) + 1);

		const std::unique_ptr<wchar_t [ ]> buffer(new wchar_t[ size ]);
		std::swprintf(buffer.get( ), size, text.data( ), std::forward<Args>(args)...);

		const auto str = std::wstring(buffer.get( ), buffer.get( ) + size - 1);
		const auto str_len = static_cast<std::uint32_t>(str.size( ));

		IDWriteTextLayout* dwrite_layout = nullptr;
		RET_CHK(m_pTextEngine->CreateTextLayout(str.c_str( ), str_len, m_pTextFormat, screen_size.x, screen_size.y, &dwrite_layout));

		const DWRITE_TEXT_RANGE range
		{
			0,
			str_len
		};

		dwrite_layout->SetFontSize(sz, range);
		dwrite_layout->SetFontWeight(DWRITE_FONT_WEIGHT_EXTRA_BOLD, range);

		if (center) {
			m_pSolidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
			DWRITE_TEXT_METRICS TextInfo;
			dwrite_layout->GetMetrics(&TextInfo);
			Vector2 TextSize = { TextInfo.width / 2.f, TextInfo.height / 2.f };
			const auto x = pos.x - TextSize.x;
			const auto y = pos.y - TextSize.y;
			if (outline) {

				m_pCanvas->DrawTextLayout(D2D1::Point2F(x - 1, y), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x + 1, y), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y - 1), dwrite_layout, m_pSolidBrush);
				m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y + 1), dwrite_layout, m_pSolidBrush);
			}

			m_pSolidBrush->SetColor(D2D1::ColorF(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f, clr.a / 255.f));

			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y), dwrite_layout, m_pSolidBrush);
			dwrite_layout->Release( );
			return;
		}


		m_pSolidBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

		if (outline) {
			const auto x = pos.x;
			const auto y = pos.y;

			m_pCanvas->DrawTextLayout(D2D1::Point2F(x - 1, y), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x + 1, y), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y - 1), dwrite_layout, m_pSolidBrush);
			m_pCanvas->DrawTextLayout(D2D1::Point2F(x, y + 1), dwrite_layout, m_pSolidBrush);
		}

		m_pSolidBrush->SetColor(D2D1::ColorF(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f, clr.a / 255.f));
		m_pCanvas->DrawTextLayout(D2D1::Point2F(pos.x, pos.y), dwrite_layout, m_pSolidBrush);
		dwrite_layout->Release( );
	}
	void end_frame( ) {
		HRESULT state = m_pCanvas->EndDraw( );
		if (state == D2DERR_RECREATE_TARGET)
			reset_canvas( );
	}
}