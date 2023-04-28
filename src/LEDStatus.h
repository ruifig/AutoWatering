#pragma once

#include "Component.h"

namespace cz
{

class LEDStatus : public Component
{
  public:
	LEDStatus() = default;
	virtual ~LEDStatus() = default; 

	enum class PatternMode : uint8_t
	{
		Repeat,
		Reset
	};

  private:

	// Component interface
	virtual const char* getName() const override { return "LEDStatus"; }
	virtual bool initImpl() override;
	virtual float tick(float deltaSeconds) override;
	virtual void onEvent(const Event& evt) override;
	virtual bool processCommand(const Command& cmd) override;

	void set(bool on);
	void setPattern(PatternMode mode, bool a, bool b, bool c, bool d, bool e, bool f);
	void setDefaultPattern();
	void setSuccessPattern();
	uint8_t m_pattern[6];
	int m_patternIndex = 0;
	PatternMode m_mode = PatternMode::Repeat;
};


} // namespace cz
