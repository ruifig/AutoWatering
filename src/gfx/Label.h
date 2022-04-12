#pragma once

#include "Widget.h"

namespace cz::gfx
{

/**
 * Data for a label that the text can change, but not the other parameters
 */
struct LabelData
{
	Rect pos;
	HAlign halign;
	VAlign valign;
	const GFXfont* font;
	Colour textColour;
	Colour bkgColour;
	WidgetFlag flags;
};

/**
 * Data for a label that doesn't change at all, and all data can be in PROGMEM
 */
struct StaticLabelData
{
	LabelData fixed;
	const __FlashStringHelper* value;
};


class BaseLabel : public Widget
{

protected:
	void drawImplHelper(const LabelData& data);
	void drawImpl(const LabelData& data, const char* value);
	void drawImpl(const StaticLabelData& data);
};

/**
 * Label where nothing can change.
 * The entire setup can be put in PROGMEM
 */
class StaticLabel : public BaseLabel
{
  public:
	StaticLabel(const StaticLabelData* data_P);
	virtual void draw(bool forceDraw = false) override;
  private:
	const StaticLabelData* m_data_P;
};

/**
 * Label that the text can change, but not the rest of the setup.
 */
template<int BUFSIZE=10>
class FixedLabel : public BaseLabel
{
  public:
	static int constexpr m_bufSize = BUFSIZE;

	FixedLabel(const LabelData* data_P, const __FlashStringHelper* value = nullptr)
		: m_data_P(data_P)
	{
		if (value)
		{
			strncpy_P(m_value, (const char*)value, m_bufSize);
			m_value[m_bufSize-1] = 0;
		}
		else
		{
			strncpy_P(m_value, (const char*)F("99%"), m_bufSize);
			m_value[m_bufSize-1] = 0;
		}
	}

	void setText(const char* text)
	{
		// If the text is the same, nothing to do
		if (strncmp(text, m_value, m_bufSize)==0)
		{
			return;
		}

		strncpy(m_value, text, m_bufSize);
		m_value[m_bufSize-1] = 0;
		m_needsRedraw = true;
	}

	/**
	 * Forcibly marks the label as needing redrawing or not on the next call to draw()
	 */
	void setDirty(bool value)
	{
		m_needsRedraw = value;
	}

	virtual void draw(bool forceDraw = false) override
	{
		if (forceDraw || m_needsRedraw)
		{
			drawImpl(getFixedData(), m_value);
			m_needsRedraw = false;
		}
	}

	Rect getRect() const
	{
		return getFixedData().pos;
	}

  private:

	LabelData getFixedData() const
	{
		LabelData data;
		memcpy_P(&data, m_data_P, sizeof(data));
		return data;
	}
	const LabelData* m_data_P;
	char m_value[m_bufSize];
	bool m_needsRedraw;
};


template<bool FlashStorage>
struct LabelDataStorage{ };

//
// Flash storage specialization
//
template<>
struct LabelDataStorage<true>
{
protected:
	LabelDataStorage(const LabelData& data_P)
		: m_data_P(&data_P)
	{
	}

	const LabelData& getData() const
	{
		return getDataImpl();
	}

	LabelData& getData()
	{
		return const_cast<LabelData&>(getDataImpl());
	}

private:

	const LabelData& getDataImpl() const
	{
		// We use a global so we don't need to copy out
		static LabelData tmp;
		memcpy_P(&tmp, m_data_P, sizeof(tmp));
		return tmp;
	}
	
	const LabelData* m_data_P = nullptr;
};

//
// Ram storage specialization
//
template<>
struct LabelDataStorage<false>
{
protected:
	LabelDataStorage(const LabelData& data)
		: m_data(data)
	{
	}
	
	LabelData& getData()
	{
		return m_data;
	}

private:
	LabelData m_data;
};


/**
 * Label that displays a number
 */
template<bool StorageLocation>
class NumLabel : public BaseLabel, public LabelDataStorage<StorageLocation>
{
  public:

	/*
	* \para data
	*	If FlashStorage is true, then this should be a PROGMEM pointer
	*/
	NumLabel(const LabelData& data, int value = 0)
		: LabelDataStorage<StorageLocation>(data) 
		, m_value(value)
		, m_needsRedraw(true)
		, m_hasValue(false)
	{
	}

	virtual void draw(bool forceDraw = false) override
	{
		if (!m_needsRedraw && !forceDraw)
		{
			return;
		}

		LabelData& data = LabelDataStorage<StorageLocation>::getData();
		if (m_hasValue)
		{
			const char *str;
			if (enumHasAnyFlags(data.flags, WidgetFlag::NumAsPercentage))
			{
				str = formatString(F("%3u%%"), m_value);
			}
			else
			{
				str = itoa(m_value, getTemporaryString(), 10);
			}

			drawImpl(data, str);
		}
		else
		{
			fillRect(data.pos, data.bkgColour);
		}

		m_needsRedraw = false;
	}

	void setValue(int value)
	{
		if (!m_hasValue  || (value != m_value))
		{
			m_hasValue = true;
			m_value = value;
			m_needsRedraw = true;
		}
	}

	void clearValue()
	{
		if (m_hasValue)
		{
			m_hasValue = false;
			m_needsRedraw = true;
		}
	}

	void setValueAndDraw(int value, bool forceDraw)
	{
		setValue(value);
		draw(forceDraw);
	}

	void clearValueAndDraw(bool forceDraw = false)
	{
		clearValue();
		draw(forceDraw);
	}
	
	// If using flash storage, we return an lvalue
	template<
		typename Dummy = void,
		typename = std::enable_if_t<StorageLocation, Dummy>
		>
	cz::Rect getRect() const
	{
		return LabelDataStorage<StorageLocation>::getData().pos;
	}

	// If using ram, we return an a const reference
	template<
		typename Dummy = void,
		typename = std::enable_if_t<!StorageLocation, Dummy>
		>
	const cz::Rect& getRect() const
	{
		return LabelDataStorage<StorageLocation>::getData().pos;
	}

  private:
	int m_value;
	bool m_needsRedraw : 1;
	bool m_hasValue : 1;
};

	
} // namespace cz::gfx

