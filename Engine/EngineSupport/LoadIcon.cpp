/**
* @file LoadIcon.cpp
*/
#include "LoadIcon.h"

#include "../DebugLog.h"
#include <Windows.h>
#include <GLFW/glfw3.h>

/**
* .ico�t�@�C����ǂݍ���ŁAGLFWimage�\���̂��쐬����֐�
*
* @param filename �ǂݍ���.ico�t�@�C���̃p�X
*
* @return �쐬���ꂽGLFWimage�\����
*/
GLFWimage LoadIcon_as_GLFWimage(const char* filename)
{
	GLFWimage image;

	// .ico�t�@�C����ǂݍ���
	HICON icon = static_cast<HICON>(
		LoadImage(nullptr, filename, IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
	if (!icon) {
		// �G���[����
		image.width = 0;
		image.height = 0;
		image.pixels = nullptr;

		LOG_ERROR("%s��ǂݍ��߂܂���", filename);
		return image;
	}

	// �A�C�R���̏����擾����
	ICONINFO info;
	GetIconInfo(icon, &info);

	// �A�C�R���̕��ƍ������擾����
	BITMAP bmp;
	GetObject(info.hbmColor, sizeof(BITMAP), &bmp);
	image.width = bmp.bmWidth;
	image.height = bmp.bmHeight;

	// �A�C�R���̃s�N�Z���f�[�^���擾����
	HDC dc = GetDC(nullptr);
	HDC memDC = CreateCompatibleDC(dc);
	HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(memDC, info.hbmColor));

	// �s�N�Z���f�[�^���i�[���郁�������m�ۂ���
	image.pixels = new unsigned char[image.width * image.height * 4]; // RGBA�`���Ŋi�[����

	// �s�N�Z���f�[�^���擾����
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = image.width;
	bi.biHeight = -image.height; // �㉺���]������
	bi.biPlanes = 1;
	bi.biBitCount = 32; // 32bit�iRGBA�j
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	GetDIBits(dc, info.hbmColor, 0, image.height, image.pixels,
		reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

	// RGBA�`���ɕϊ�����
	for (int i = 0; i < image.width * image.height; ++i) {
		unsigned char temp = image.pixels[i * 4]; // Red
		image.pixels[i * 4] = image.pixels[i * 4 + 2]; // Blue
		image.pixels[i * 4 + 2] = temp; // Red
	}

	// �㏈��
	SelectObject(memDC, oldBmp);
	DeleteDC(memDC);
	ReleaseDC(nullptr, dc);
	DeleteObject(info.hbmColor);
	DeleteObject(info.hbmMask);

	return image;
}