/**
* @file LoadIcon.cpp
*/
#include "LoadIcon.h"

#include "../DebugLog.h"
#include <Windows.h>
#include <GLFW/glfw3.h>

/**
* .icoファイルを読み込んで、GLFWimage構造体を作成する関数
*
* @param filename 読み込む.icoファイルのパス
*
* @return 作成されたGLFWimage構造体
*/
GLFWimage LoadIcon_as_GLFWimage(const char* filename)
{
	GLFWimage image;

	// .icoファイルを読み込む
	HICON icon = static_cast<HICON>(
		LoadImage(nullptr, filename, IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
	if (!icon) {
		// エラー処理
		image.width = 0;
		image.height = 0;
		image.pixels = nullptr;

		LOG_ERROR("%sを読み込めません", filename);
		return image;
	}

	// アイコンの情報を取得する
	ICONINFO info;
	GetIconInfo(icon, &info);

	// アイコンの幅と高さを取得する
	BITMAP bmp;
	GetObject(info.hbmColor, sizeof(BITMAP), &bmp);
	image.width = bmp.bmWidth;
	image.height = bmp.bmHeight;

	// アイコンのピクセルデータを取得する
	HDC dc = GetDC(nullptr);
	HDC memDC = CreateCompatibleDC(dc);
	HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(memDC, info.hbmColor));

	// ピクセルデータを格納するメモリを確保する
	image.pixels = new unsigned char[image.width * image.height * 4]; // RGBA形式で格納する

	// ピクセルデータを取得する
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = image.width;
	bi.biHeight = -image.height; // 上下反転させる
	bi.biPlanes = 1;
	bi.biBitCount = 32; // 32bit（RGBA）
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	GetDIBits(dc, info.hbmColor, 0, image.height, image.pixels,
		reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

	// RGBA形式に変換する
	for (int i = 0; i < image.width * image.height; ++i) {
		unsigned char temp = image.pixels[i * 4]; // Red
		image.pixels[i * 4] = image.pixels[i * 4 + 2]; // Blue
		image.pixels[i * 4 + 2] = temp; // Red
	}

	// 後処理
	SelectObject(memDC, oldBmp);
	DeleteDC(memDC);
	ReleaseDC(nullptr, dc);
	DeleteObject(info.hbmColor);
	DeleteObject(info.hbmMask);

	return image;
}