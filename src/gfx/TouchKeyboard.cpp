#include "TouchKeyboard.h"
#include "TFTeSPIWrapper.h"

namespace cz::gfx
{

#define KEY_BKG_COLOUR Colour_Yellow
#define KEY_TXT_COLOUR Colour_Black
#define KEY_OUTLINE_COLOUR Colour_DarkGrey


void TouchKeyboard::KeyButton::init(uint8_t id, const Rect& pos, const KeyInfo& info, const GFXfont* bigFont, const GFXfont* smallFont)
{
	BaseButton::initHelper(id, pos, KEY_BKG_COLOUR);
	m_info = info;
	m_bigFont = bigFont;
	m_smallFont = smallFont;
}

void TouchKeyboard::KeyButton::draw(bool forceRedraw)
{
	uint8_t r = 1; // Corner radius
	TFTeSPIWrapper::getInstance()->fillRoundRect(m_pos.x, m_pos.y, m_pos.width, m_pos.height, r, KEY_BKG_COLOUR);
	TFTeSPIWrapper::getInstance()->drawRoundRect(m_pos.x, m_pos.y, m_pos.width, m_pos.height, r, KEY_OUTLINE_COLOUR);
	TFTeSPIWrapper::getInstance()->setTextSize(1);
	TFTeSPIWrapper::getInstance()->setTextColor(KEY_TXT_COLOUR, KEY_BKG_COLOUR);

	auto printSpecial = [this](const char* txt)
	{
		TFTeSPIWrapper::getInstance()->setFont(m_smallFont);
		printAligned(m_pos, HAlign::Center, VAlign::Center, txt, false);
	};

	switch (m_info.type)
	{
		case CHR:  // Actual character
		{
			if (m_info.ch0 == ' ')
			{
				printSpecial("SPC");
			}
			else
			{
				char bigCh[2]; char smallCh[2];
				bigCh[1] = 0; smallCh[1] = 0;
				bigCh[0] = m_chIndex==0 ? m_info.ch0 : m_info.ch1;
				smallCh[0] = m_chIndex==0 ? m_info.ch1 : m_info.ch0;

			    if (m_info.ch0 < 'a' || m_info.ch0 > 'z')
			    {
					TFTeSPIWrapper::getInstance()->setFont(m_smallFont);
					printAligned(m_pos.contract(2), HAlign::Right, VAlign::Top, smallCh, false);
				}

				TFTeSPIWrapper::getInstance()->setFont(m_bigFont);
				printAligned(m_pos, HAlign::Center, VAlign::Center, bigCh, false);
			}
	    }
		break;
		case SHF:  // Shift
		{
			printSpecial("SHF");
		}
		break;
		case ENT:  // Enter
		{
			printSpecial("ENT");
		}
		break;
		case DEL:  // Delete
		{
			printSpecial("DEL");
		}
		break;
	}

}

void TouchKeyboard::init()
{
	//     ! @ # $ % ^ & * ( )
	//     1 2 3 4 5 6 7 8 9 0
	//	
	//     Q W E R T Y U I O P
	//	
	//     A S D F G H J K L ENTER
	//	
	//                     :
	// SHIFT Z X C V B N M ; DEL
	//	
	//       ~ _ + { } | " < > ?
	//  SPC  ` - = [ ] \ ' , . /

	static KeyInfo info[51] = {
		{CHR, '1', '!'}, {CHR, '2', '@'}, {CHR, '3', '#'}, {CHR, '4', '$'}, {CHR, '5', '%'}, {CHR, '6', '^'}, {CHR, '7', '&'}, {CHR, '8', '*'}, {CHR, '9', '('}, {CHR, '0', ')'},
		{CHR, 'q', 'Q'}, {CHR, 'w', 'W'}, {CHR, 'e', 'E'}, {CHR, 'r', 'R'}, {CHR, 't', 'T'}, {CHR, 'y', 'Y'}, {CHR, 'u', 'U'}, {CHR, 'i', 'I'}, {CHR, 'o', 'O'}, {CHR, 'p', 'P'},
		{CHR, 'a', 'A'}, {CHR, 's', 'S'}, {CHR, 'd', 'D'}, {CHR, 'f', 'F'}, {CHR, 'g', 'G'}, {CHR, 'h', 'H'}, {CHR, 'j', 'J'}, {CHR, 'k', 'K'}, {CHR, 'l', 'L'}, {ENT, ' ', ' '},
		{SHF, ' ', ' '}, {CHR, 'z', 'Z'}, {CHR, 'x', 'X'}, {CHR, 'c', 'C'}, {CHR, 'v', 'V'}, {CHR, 'b', 'B'}, {CHR, 'n', 'N'}, {CHR, 'm', 'M'}, {CHR, ';', ':'}, {DEL, ' ', ' '},
		{CHR, ' ', ' '}, {CHR, '`', '~'}, {CHR, '-', '_'}, {CHR, '=', '+'}, {CHR, '[', '{'}, {CHR, ']', '}'}, {CHR, '\\','|'}, {CHR, '\'','"'}, {CHR, ',', '<'}, {CHR, '.', '>'}, {CHR, '/', '?'}
	};

	constexpr int16_t height = 32;
	constexpr int16_t width1 = AW_SCREEN_WIDTH/10;
	constexpr int16_t width2 = AW_SCREEN_WIDTH/11;
	constexpr int16_t startY = AW_SCREEN_HEIGHT-32*5;
	for(int i=0; i<=51; i++)
	{
		Rect pos;
		if (i<40)
			pos = Rect((i % 10) * width1, startY + ((i / 10) * height), width1, height);
		else
			pos = Rect((i - 40) * width2, startY + (4 * height), width2, height);

		m_keys[i].init(i, pos, info[i], MEDIUM_FONT, SMALL_FONT);
	}
}

void TouchKeyboard::tick(float deltaSeconds)
{
}

void TouchKeyboard::draw(bool forceDraw = false)
{
	for(auto&& k : m_keys)
	{
		k.draw(forceDraw);
	}
}

} // namespace cz
