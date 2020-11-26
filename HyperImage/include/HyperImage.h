#pragma once

#include <iostream>
#include <string>
#include <vector>

struct Pixel
{
	uint8_t B = 0;
	uint8_t G = 0;
	uint8_t R = 0;
	uint8_t A = 255;
};

class Image
{
private:
	std::string m_FileName;
	size_t m_Width;
	size_t m_Height;

	std::vector<uint32_t> m_Pixels;

	static constexpr uint32_t ChannelCount = 4;

	friend class ImageWriter;

public:
	Image(const std::string& fileName, size_t width, size_t height)
		: m_FileName(fileName), m_Width(width), m_Height(height)
	{
		for (size_t i = 0; i < m_Width; i++)
			for (size_t j = 0; j < m_Height; j++)
				m_Pixels.emplace_back(ConvertPixel(Pixel({0, 0, 0, 255})));
	}

	void SetPixel(size_t x, size_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
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
		pixel.B = color & 0xFFFF;
		pixel.G = color >> 8 & 0xFFFF;
		pixel.R = color >> 16 & 0xFFFF;
		pixel.A = color >> 24 & 0xFFFF;
		return pixel;
	}

	uint32_t ConvertPixel(const Pixel& pixel) const
	{
		return pixel.B | (pixel.G << 8) | (pixel.R << 16) | (pixel.A << 24);
	}
};

class ImageWriter
{
private:
	static constexpr uint8_t FileHeaderSize = 14;
	static constexpr uint8_t InfoHeaderSize = 40;
	static constexpr uint8_t ChannelCount = 4;
	static constexpr uint8_t BytesPerChannel = 8;

public:
	static void GenerateImage(const Image& image)
	{
		GenerateBitmap(image);
	}

private:
	static void GenerateBitmap(const Image& image)
	{
		size_t widthBytes = image.m_Width * ChannelCount;

		FILE* imageFile = fopen(image.m_FileName.c_str(), "wb");

		unsigned char* fileHeader = CreateBitmapFileHeader(image, widthBytes);
		fwrite(fileHeader, 1, FileHeaderSize, imageFile);

		unsigned char* infoHeader = CreateBitmapInfoHeader(image);
		fwrite(infoHeader, 1, InfoHeaderSize, imageFile);

		for (int i = 0; i < image.m_Height; i++)
		{
			for (int j = 0; j < image.m_Width; j++)
			{
				Pixel pixel = image.GetPixel(j, i);
				fwrite(&pixel, sizeof(Pixel), 1, imageFile);
			}
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
		infoHeader[14] = static_cast<unsigned char>(ChannelCount * BytesPerChannel);

		return infoHeader;
	}
};