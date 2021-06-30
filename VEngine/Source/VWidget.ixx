module;
#include <string>
#include <d2d1_1.h>
#include <SpriteBatch.h>
#include <unordered_map>
#include <wrl.h>
export module VWidget;

using namespace Microsoft::WRL;

//I can never get the D2D1_RECT_F things right
//left = The x-coordinate of the upper-left corner of the rectangle
//top = The y-coordinate of the upper-left corner of the rectangle.
//right = The x-coordinate of the lower-right corner of the rectangle
//bottom = The y-coordinate of the lower-right corner of the rectangle.

class ID3D11ShaderResourceView;

//Base widget class for in-game UI
export class VWidget
{
public:
	ComPtr<ID3D11ShaderResourceView> CreateTexture(const std::wstring& filename);

	void Text(const std::wstring& text, D2D1_RECT_F layout);
	bool Button(const std::wstring& text, D2D1_RECT_F layout, float lineWidth = 1.0f);
	void Image(const std::wstring& filename, float x, float y);

	DirectX::SpriteBatch* spriteBatch;

	std::unordered_map<std::wstring, ID3D11ShaderResourceView*> texturesMap;

	bool bRender = true;

	void Tick(float deltaTime)
	{
		Text(L"Hello baby", { 0,0,200,200 });
		Button(L"button", { 200, 200, 300, 300 });
		Image(L"penguin.png", 200, 200);
	}

	void Start()
	{
		spriteBatch = new DirectX::SpriteBatch(gRenderSystem.context);
	}

	ComPtr<ID3D11ShaderResourceView> CreateTexture(const std::wstring& filename)
	{
		ComPtr<ID3D11ShaderResourceView> textureView;
		std::wstring filepath = L"Textures/" + filename;
		CreateWICTextureFromFile(gRenderSystem.device, filepath.c_str(), nullptr, &textureView);
		assert(textureView && "texture filename will be wrong");

		return textureView;
	}

	void Text(const std::wstring& text, D2D1_RECT_F layout)
	{
		gUISystem.d2dRenderTarget->DrawTextA(text.c_str(), text.size(),
			gUISystem.textFormat, layout, gUISystem.brushText);
	}

	//Make sure the buttons layout isn't backwards(bottom and top less than left and right)
	//else the mouse check won't work.
	bool Button(const std::wstring& text, D2D1_RECT_F layout, float lineWidth)
	{
		gUISystem.d2dRenderTarget->DrawRectangle(layout, gUISystem.brushShapes, lineWidth);
		Text(text, layout);

		if (gUISystem.mousePos.x > layout.left && gUISystem.mousePos.x < layout.right)
		{
			if (gUISystem.mousePos.y > layout.top && gUISystem.mousePos.y < layout.bottom)
			{
				if (gInputSystem.GetMouseLeftUpState())
				{
					return true;
				}
			}
		}

		return false;
	}

	//REF:https://github.com/Microsoft/DirectXTK/wiki/Sprites-and-textures
	void Image(const std::wstring& filename, float x, float y)
	{
		//TODO: sprite rendering is fucked up. There's something going wrong with its state changes
		//and maybe its shader binding, models get messed up when this function is called.
		//REF:https://github.com/Microsoft/DirectXTK/wiki/SpriteBatch#state-management
		//REF:https://stackoverflow.com/questions/35558178/directxspritefont-spritebatch-prevents-3d-scene-from-drawing
		/*auto textureIt = texturesMap.find(filename);
		if (textureIt == texturesMap.end())
		{
			auto texture = CreateTexture(filename);
			texturesMap[filename] = texture.Get();
		}
		else
		{
			DirectX::XMFLOAT2 screenPos(x, y);
			DirectX::XMFLOAT2 origin(0.f, 0.f);

			spriteBatch->Begin();
			spriteBatch->Draw(textureIt->second, screenPos, nullptr, Colors::White, 0.f, origin);
			spriteBatch->End();
		}*/
	}

};
