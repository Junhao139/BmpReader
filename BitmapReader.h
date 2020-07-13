#pragma once

/*

	Created by Junhao139
	Version 0.0.1_beta

*/

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

#ifndef _BITMAPREADER_H_
#define _BITMAPREADER_H_

#define BYTE unsigned char

namespace BMPreader {
	class Bitmap {
	private:
		BITMAPFILEHEADER fileHeader;
		BITMAPINFOHEADER infoHeader;
		unsigned long imgSize;
		int numColors = 3;
		bool topDown = false;

		void checkColorTable();
		long getValue(BYTE* filePtr, int bytes);
		
		bool readBitmapFileHeader();
		bool readBitmapInfoHeader();
		bool readBitmapColorInfo();
	public:
		void* bmBits = nullptr;				/* Entire Bitmap file */
		unsigned long bmWidth = 0;			/* Bitmap width */
		unsigned long bmHeight = 0;			/* Bitmap height */
		COLORREF* bmPixelColors = nullptr;	/* Bitmap colors in RGB */
		
		Bitmap(char* fileName);
		Bitmap(const char* fileName);
		bool read(char* fileName);			/* Read Bitmap by variable file name, returns true if succeeded */
		bool read(const char* fileName);	/* Read Bitmap by constant file name, returns true if succeeded */
		bool free();						/* Delete whole Bitmap data, free memory */
	};

	void sync(BITMAP* currentBitmap, BMPreader::Bitmap* bmpClass);			/* Put all bitmap informations into a specifical class */
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
		this->fileHeader.bfSize = this->imgSize;

		bmp.seekg(0, std::ios::beg);
		BYTE* file = (BYTE*)malloc(sizeof(BYTE) * this->imgSize);
		bmp.read((char*)file, this->imgSize);
		this->bmBits = file;
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

		this->imgSize = bmp.tellg();
		this->fileHeader.bfSize = this->imgSize;

		bmp.seekg(0, std::ios::beg);
		BYTE* file = (BYTE*)malloc(sizeof(BYTE) * this->imgSize);
		bmp.read((char*)file, this->imgSize);
		this->bmBits = file;
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

// private

void BMPreader::Bitmap::checkColorTable() {
	this->numColors = this->infoHeader.biBitCount / 8;
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

bool BMPreader::Bitmap::readBitmapFileHeader() {
	/* WORD = unsigned short */
	/* DWORD = unsigned long */
	/* LONG = long */
	BYTE* filePtr = (BYTE*)(this->bmBits);
	if (filePtr[0] == 'B' && filePtr[1] == 'M') {
		this->fileHeader.bfSize = getValue(&filePtr[2], 4);
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
			this->infoHeader.biWidth =			this->bmWidth =		getValue(&filePtr[4], 4);
			this->infoHeader.biHeight =			this->bmHeight =	getValue(&filePtr[8], 4);
			this->infoHeader.biPlanes =			1;
			this->infoHeader.biBitCount =		(unsigned short)getValue(&filePtr[14], 2);
			this->infoHeader.biCompression =	(unsigned long)getValue(&filePtr[16], 4);
			this->infoHeader.biSizeImage =		(this->infoHeader.biCompression == 0) ? 0 : (unsigned long)getValue(&filePtr[20], 4);
			this->infoHeader.biXPelsPerMeter =	getValue(&filePtr[24], 4);
			this->infoHeader.biYPelsPerMeter =	getValue(&filePtr[28], 4);
			this->infoHeader.biClrUsed =		(unsigned long)getValue(&filePtr[32], 4);
			this->infoHeader.biClrImportant =	(unsigned long)getValue(&filePtr[36], 4);
			return true;
			break;
		//default:
			//return false;
	}
}

bool BMPreader::Bitmap::readBitmapColorInfo() {
	bool returnValue = true;
	this->bmPixelColors = (COLORREF*)malloc(sizeof(BYTE) * (this->imgSize - 54));
	switch (this->infoHeader.biBitCount) {
		case 24:
			BYTE* filePtr = (BYTE*)(this->bmBits);
			filePtr += 54;

			for (long row = 0; row < this->bmHeight; row += this->bmWidth) {
				for (int column = 0; column < this->bmWidth; ++column) {
					this->bmPixelColors[row + column] = getValue(filePtr, 3);
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

#endif // BITMAPREADER_H_