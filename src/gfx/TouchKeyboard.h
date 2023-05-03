#pragma once

#include "Button.h"

namespace cz::gfx
{

class TouchKeyboard : public Widget
{
  public:

	void init();
	virtual void tick(float deltaSeconds);
	virtual void draw(bool forceDraw) override;

  protected:

	enum Type
	{
		CHR,  // Actual character
		SHF, // Shift
		ENT, // Enter
		DEL // Delete
	};

	struct KeyInfo
	{
		Type type;
		char ch0;
		char ch1;
	};

	struct KeyButton : public BaseButton
	{
	  public:

		void init(uint8_t id, const Rect& pos, const KeyInfo& info, const GFXfont* bigFont, const GFXfont* smallFont);
		virtual void draw(bool forceDraw) override;

	  protected:
	  	KeyInfo m_info;
		// A key can be used for 2 characters
		char m_chIndex = 0; // The character currenty active
		const GFXfont* m_bigFont;
		const GFXfont* m_smallFont;
	};

	//
	// Following this https://upload.wikimedia.org/wikipedia/commons/5/51/KB_United_States-NoAltGr.svg
	// https://cio-wiki.org/wiki/File:ASCII.png
	//
	// I'm using 5 rows of keys, 10 keys per row, except the last row, which has 11 keys
	//
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
	// 
	// Note that the 
	KeyButton m_keys[51];

};

}