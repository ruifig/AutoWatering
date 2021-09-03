#pragma once

namespace cz
{


// Assign human-readable names to some common 16-bit color values:
#if 0
enum class Colour : uint16_t
{
	Black       = 0x0000,
	Blue        = 0x001F,
	Cyan        = 0x07FF,
	DarkGreen   = 0x03E0,
	DarkCyan    = 0x03EF,
	DarkGrey    = 0x7BEF,
	Green       = 0x07E0,
	GreenYellow = 0xB7E0,
	LightGrey   = 0xC618,
	Magenta     = 0xF81F,
	Maroon      = 0x7800,
	Navy        = 0x000F,
	Olive       = 0x7BE0,
	Orange      = 0xFDA0,
	Pink        = 0xFC9F,
	Purple      = 0x780F,
	Red         = 0xF800,
	White       = 0xFFFF,
	Yellow      = 0xFFE0,
	VeryDarkGrey = 0x2945
};
#endif

/**
 * 565 RGB colour
 */ 
class Colour
{
  private:
	uint16_t m_val;

  public:
	constexpr explicit Colour(uint16_t rgb)
		: m_val(rgb)
	{
	}

	constexpr Colour()
		: m_val(0x0000)
	{
	}

	constexpr operator uint16_t() const
	{
		return m_val;
	}

	// Based on https://stackoverflow.com/questions/58449462/rgb565-to-grayscale
	constexpr Colour toGrey() const
	{
		int16_t red = ((m_val & 0xF800)>>11);
		int16_t green = ((m_val & 0x07E0)>>5);
		int16_t blue = (m_val & 0x001F);
		int16_t greyscale = (0.2126 * red) + (0.7152 * green / 2.0) + (0.0722 * blue);
		return Colour((greyscale<<11) + (greyscale<<6) + greyscale);
	}

};
	static constexpr Colour Colour_Black(0x0000);
	static constexpr Colour Colour_Blue(0x001F);
	static constexpr Colour Colour_Cyan(0x07FF);
	static constexpr Colour Colour_DarkGreen(0x03E0);
	static constexpr Colour Colour_DarkCyan(0x03EF);
	static constexpr Colour Colour_DarkGrey(0x7BEF);
	static constexpr Colour Colour_Green(0x07E0);
	static constexpr Colour Colour_GreenYellow(0xB7E0);
	static constexpr Colour Colour_LightGrey(0xC618);
	static constexpr Colour Colour_Magenta(0xF81F);
	static constexpr Colour Colour_Maroon(0x7800);
	static constexpr Colour Colour_Navy(0x000F);
	static constexpr Colour Colour_Olive(0x7BE0);
	static constexpr Colour Colour_Orange(0xFDA0);
	static constexpr Colour Colour_Pink(0xFC9F);
	static constexpr Colour Colour_Purple(0x780F);
	static constexpr Colour Colour_Red(0xF800);
	static constexpr Colour Colour_White(0xFFFF);
	static constexpr Colour Colour_Yellow(0xFFE0);
	static constexpr Colour Colour_VeryDarkGrey(0x2945);
}