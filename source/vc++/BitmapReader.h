#pragma once

/*

	Created by Junhao139
	Version 0.0.1_200715
	Tested under Visual C++

*/

#ifndef _BITMAPREADER_H_
#define _BITMAPREADER_H_

// #define RGB(r, g, b) Color {r, g, b}

#include <iostream>
#include <Windows.h>
#include <wingdi.h>
#include <fstream>

/*

typedef struct BITMAP {
  LONG   bmType;		// BMP 类型, 此处为 0
  LONG   bmWidth;		// WIDTH, 单位像素
  LONG   bmHeight;		// HEIGHT, 单位像素
  LONG   bmWidthBytes;	// 每行扫描的 BYTE 数量
  WORD   bmPlanes;		// 
  WORD   bmBitsPixel;	// 规定一个像素颜色的 bit 数量
  LPVOID bmBits;		// 指向图片所有以 char 存储的信息的指针
};

typedef struct BITMAPFILEHEADER {
  WORD  bfType;			// 文件类型, 应为 BM
  DWORD bfSize;			// BMP 图片的大小
  WORD  bfReserved1;	// 保留, 应为 0
  WORD  bfReserved2;	// 保留, 应为 0
  DWORD bfOffBits;		// 文件相对于栅格数据的偏移
};

typedef struct BITMAPINFOHEADER {
  DWORD biSize;			// InfoHeader 的长度(Byte), 应为 40
  LONG  biWidth;		// BMP 图片的 width(像素)
  LONG  biHeight;		// BMP 图片的 height(像素)
  WORD  biPlanes;		// BMP 图片的 Plane 数量, 应为 1
  WORD  biBitCount;		// 
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
};

*/

#define BYTE unsigned char

namespace BMPreader {

	struct Color {
		BYTE R = 0;
		BYTE G = 0;
		BYTE B = 0;
	};

	class Bitmap {
	private:
		BITMAPFILEHEADER* fileHeader = new BITMAPFILEHEADER;
		BITMAPINFOHEADER* infoHeader = new BITMAPINFOHEADER;
		unsigned long imgSize;
		int numColors = 3;
		bool topDown = false;

		void checkColorTable();
		long getValue(BYTE* filePtr, int bytes);
		long getValueInOrder(BYTE* filePtr, int bytes);
		void outValue(BYTE* filePtr, int bytes, long value);
		
		bool readBitmapFileHeader();
		bool readBitmapInfoHeader();
		bool readBitmapColorInfo();
	public:
		
		void* bmBits = nullptr;				/* Entire Bitmap file */
		unsigned long bfSize = 0;			/* Bitmap size */
		unsigned long bmWidth = 0;			/* Bitmap width */
		long bmHeight = 0;					/* Bitmap height */
		Color* bmPixelColors = nullptr;		/* Bitmap colors in RGB */
		
		Bitmap(char* fileName);
		Bitmap(const char* fileName);
		bool read(char* fileName);			/* Read Bitmap by variable file name, returns true if succeeded */
		bool read(const char* fileName);	/* Read Bitmap by constant file name, returns true if succeeded */
		bool free();						/* Delete whole Bitmap data, free memory */
		bool free(int OPTION);				/* Delete an expecific part of data by OPTION to free a part of memory */

		bool save(const char* fileName);	/* Save image as .bmp called fileName */
	};

	COLORREF RGBtoCOLORREF(Color color);
	void sync(BITMAP* currentBitmap, BMPreader::Bitmap* bmpClass);	/* Put all bitmap informations into a specifical class */
}

// public

BMPreader::Bitmap::Bitmap(char* fileName) {
	read(fileName);
}

BMPreader::Bitmap::Bitmap(const char* fileName) {
	read(fileName);
}

bool BMPreader::Bitmap::read(char* fileName) {
	std::ifstream bmp;
	bmp.open(fileName, std::ios::binary | std::ios::ate);

	if (bmp.is_open()) {

		this->imgSize = bmp.tellg();
		this->fileHeader->bfSize = this->bfSize = this->imgSize;

		bmp.seekg(0, std::ios::beg);
		BYTE* file = (BYTE*)malloc(sizeof(BYTE) * this->imgSize);
		bmp.read((char*)file, this->imgSize);
		this->bmBits = (void*)file;
		bmp.close();

		if (!readBitmapFileHeader())		{ std::cout << "readFileHeaderFailed;"; return false; }
		else if (!readBitmapInfoHeader())	{ std::cout << "readInfoHeaderFailed;"; return false; }
		else if (!readBitmapColorInfo())	{ std::cout << "readColorInfoFailed;"; return false; }
		else {
			return true;
		}
	}
	else return false;
}

bool BMPreader::Bitmap::read(const char* fileName) {
	std::ifstream bmp;
	bmp.open(fileName, std::ios::binary | std::ios::ate);

	if (bmp.is_open()) {

		this->imgSize = 374 /*bmp.tellg()*/;
		this->fileHeader->bfSize = this->bfSize = this->imgSize;

		bmp.seekg(0, std::ios::beg);
		BYTE* file = (BYTE*)malloc(sizeof(BYTE) * this->bfSize);
		bmp.read((char*)file, this->bfSize);
		this->bmBits = (void*)file;
		bmp.close();

		if		(!readBitmapFileHeader())	{ std::cout << "readFileHeaderFailed;"; return false; }
		else if (!readBitmapInfoHeader())	{ std::cout << "readInfoHeaderFailed;"; return false; }
		else if (!readBitmapColorInfo())	{ std::cout << "readColorFailed;"; return false; }
		else {
			return true;
		}
	}
	else return false;
}

bool BMPreader::Bitmap::free() {
	delete[] (void*)this->bmBits;
	delete[] (void*)this->bmPixelColors;
	
	delete[] (void*)this->fileHeader;
	delete[] (void*)this->infoHeader;
	return true;
}

// private

void BMPreader::Bitmap::checkColorTable() {
	this->numColors = this->infoHeader->biBitCount / 8;
}

long BMPreader::Bitmap::getValue(BYTE* filePtr, int bytes) {
	long valueGotten = 0;
	for (int i = 0; i < bytes; ++i) {
		if (filePtr[i] != 0) {
			valueGotten *= 0x100;
			valueGotten += filePtr[i];
		} else {
			break;
		}
	}
	return valueGotten;
}

long BMPreader::Bitmap::getValueInOrder(BYTE* filePtr, int bytes) {
	long valueGotten = 0;
	for (int i = 0; i < bytes; ++i) {
		valueGotten *= 0x100;
		valueGotten += filePtr[i];
	}
	return valueGotten;
}

void BMPreader::Bitmap::outValue(BYTE* filePtr, int bytes, long value) {
	for (int i = 0; i < bytes; ++i) {
		if (value != 0) {
			filePtr[i] = (long)(value / 0x100);
		}
	}
}

bool BMPreader::Bitmap::readBitmapFileHeader() {
	/* WORD = unsigned short */
	/* DWORD = unsigned long */
	/* LONG = long */
	BYTE* filePtr = (BYTE*)(this->bmBits);
	if (filePtr[0] == 'B' && filePtr[1] == 'M') {
		this->fileHeader->bfSize = getValue(&filePtr[2], 4);
		// this->fileHeader.bfOffBits = getValue(&filePtr[10], 4);
		return true;
	} else return false;
}

bool BMPreader::Bitmap::readBitmapInfoHeader() {
	unsigned int bfSize = 14;
	BYTE* filePtr = (BYTE*)(this->bmBits);
	filePtr += 14;

	switch (getValue(&filePtr[0], 4)) {
		case 40:
			this->infoHeader->biWidth =			this->bmWidth =		getValue(&filePtr[4], 4);
			this->infoHeader->biHeight =		this->bmHeight =	getValue(&filePtr[8], 4);
			this->infoHeader->biPlanes =		1;
			this->infoHeader->biBitCount =		(unsigned short)getValue(&filePtr[14], 2);
			this->infoHeader->biCompression =	(unsigned long)getValue(&filePtr[16], 4);
			this->infoHeader->biSizeImage =		(this->infoHeader->biCompression == 0) ? 0 : (unsigned long)getValue(&filePtr[20], 4);
			this->infoHeader->biXPelsPerMeter =	getValue(&filePtr[24], 4);
			this->infoHeader->biYPelsPerMeter =	getValue(&filePtr[28], 4);
			this->infoHeader->biClrUsed =		(unsigned long)getValue(&filePtr[32], 4);
			this->infoHeader->biClrImportant =	(unsigned long)getValue(&filePtr[36], 4);
			return true;
			break;
		//default:
			//return false;
	}
}

bool BMPreader::Bitmap::readBitmapColorInfo() {
	bool returnValue = true;
	this->bmPixelColors = (Color*)malloc(sizeof(BYTE) * (this->imgSize - 54));
	BYTE* filePtr = (BYTE*)(this->bmBits);

	switch (this->infoHeader->biBitCount) {
		case 1:
			break;
		case 4:
			break;
		case 8:
			break;
		case 24:
			
			filePtr += 54;

			for (unsigned long row = 0; row < this->bmHeight; ++row) {
				for (unsigned int column = 0; column < this->bmWidth; ++column) {
					this->bmPixelColors[row * this->bmWidth + column] = { *filePtr, *(filePtr + 1), *(filePtr + 2) };
					filePtr += 3;
				}
				if (*filePtr != 0) {
					returnValue = false;
					break;
				}
				filePtr += 1;
			}
			break;
	}
	return returnValue;
}

bool BMPreader::Bitmap::save(const char* fileName) {
	std::ofstream file;
	file.open(fileName, std::ios::binary);
	if (file.is_open()) {
		BYTE* data = (BYTE*)malloc(sizeof(BYTE) * (this->bfSize));

		// BITMAPFILEHEADER
		data[0];
	}
	return false;
}

COLORREF BMPreader::RGBtoCOLORREF(BMPreader::Color color) {
	return DWORD((color.R * 0x10000) + (color.G * 0x100) + color.B);
}

#endif // BITMAPREADER_H_
