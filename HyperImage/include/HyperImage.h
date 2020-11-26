#pragma once

#include <iostream>
#include <string>
#include <vector>

struct Pixel
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
};

class Image
{
private:
	std::string m_FileName;
	size_t m_Width;
	size_t m_Height;
	std::vector<uint32_t> m_Pixels;

	static constexpr uint32_t BytesPerPixel = 3;

	friend class ImageWriter;

public:
	Image(const std::string& fileName, size_t width, size_t height)
		: m_FileName(fileName), m_Width(width), m_Height(height)
	{
		for (size_t i = 0; i < m_Width; i++)
			for (size_t j = 0; j < m_Height; j++)
				m_Pixels.emplace_back(ConvertPixel(Pixel({255, 0, 0})));
	}

	void SetPixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b)
	{
		SetPixel(x, y, Pixel({ r, g, b }));
	}

	void SetPixel(size_t x, size_t y, Pixel pixel)
	{
		if (x < m_Width && y < m_Height)
			m_Pixels[y * m_Width + x] = ConvertPixel(pixel);
	}

	Pixel GetPixel(size_t x, size_t y) const
	{
		if (x < m_Width && y < m_Height)
			return ConvertColor(m_Pixels[y * m_Width + x]);
		return Pixel{};
	}

private:
	Pixel ConvertColor(uint32_t color) const
	{
		Pixel pixel{};
		pixel.R = color & 0xFF;
		pixel.G = color >> 8 & 0xFF;
		pixel.B = color >> 16 & 0xFF;
		return pixel;
	}

	uint32_t ConvertPixel(const Pixel& pixel) const
	{
		return pixel.R | (pixel.G << 8) | (pixel.B << 16);
	}
};

class ImageWriter
{
private:
	static constexpr uint32_t FileHeaderSize = 14;
	static constexpr uint32_t InfoHeaderSize = 40;
	static constexpr uint32_t BytesPerPixel = 3;

public:
	static void GenerateImage(const Image& image)
	{
		GenerateBitmap(image);
	}

private:
	static void GenerateBitmap(const Image& image)
	{
		size_t widthBytes = image.m_Width * BytesPerPixel;

		unsigned char padding[4] = { 0, 0, 0, 0 };
		size_t paddingSize = (4 - widthBytes % 4) % 4;

		size_t stride = widthBytes + paddingSize;

		FILE* imageFile = fopen(image.m_FileName.c_str(), "wb");

		unsigned char* fileHeader = CreateBitmapFileHeader(image, stride);
		fwrite(fileHeader, 1, FileHeaderSize, imageFile);

		unsigned char* infoHeader = CreateBitmapInfoHeader(image);
		fwrite(infoHeader, 1, InfoHeaderSize, imageFile);

		for (int i = 0; i < image.m_Height; i++)
		{
			for (int j = 0; j < image.m_Width; j++)
			{
				Pixel pixel = image.GetPixel(j, i);
				fwrite(&pixel.B, sizeof(uint8_t), 1, imageFile);
				fwrite(&pixel.G, sizeof(uint8_t), 1, imageFile);
				fwrite(&pixel.R, sizeof(uint8_t), 1, imageFile);
			}
			fwrite(padding, sizeof(unsigned char), paddingSize, imageFile);
		}

		fclose(imageFile);
	}

	static unsigned char* CreateBitmapFileHeader(const Image& image, size_t stride)
	{
		size_t fileSize = FileHeaderSize + InfoHeaderSize + (stride * image.m_Height);

		static unsigned char fileHeader[] = {
			0, 0,      
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
		};

		fileHeader[0] = static_cast<unsigned char>('B');
		fileHeader[1] = static_cast<unsigned char>('M');
		fileHeader[2] = static_cast<unsigned char>(fileSize);
		fileHeader[3] = static_cast<unsigned char>(fileSize >> 8);
		fileHeader[4] = static_cast<unsigned char>(fileSize >> 16);
		fileHeader[5] = static_cast<unsigned char>(fileSize >> 24);
		fileHeader[10] = static_cast<unsigned char>(FileHeaderSize + InfoHeaderSize);

		return fileHeader;
	}

	static unsigned char* CreateBitmapInfoHeader(const Image& image)
	{
		static unsigned char infoHeader[] = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0,      
			0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
		};

		infoHeader[0] = static_cast<unsigned char>(InfoHeaderSize);
		infoHeader[4] = static_cast<unsigned char>(image.m_Width);
		infoHeader[5] = static_cast<unsigned char>(image.m_Width >> 8);
		infoHeader[6] = static_cast<unsigned char>(image.m_Width >> 16);
		infoHeader[7] = static_cast<unsigned char>(image.m_Width >> 24);
		infoHeader[8] = static_cast<unsigned char>(image.m_Height);
		infoHeader[9] = static_cast<unsigned char>(image.m_Height >> 8);
		infoHeader[10] = static_cast<unsigned char>(image.m_Height >> 16);
		infoHeader[11] = static_cast<unsigned char>(image.m_Height >> 24);
		infoHeader[12] = static_cast<unsigned char>(1);
		infoHeader[14] = static_cast<unsigned char>(BytesPerPixel * 8);

		return infoHeader;
	}
};