#include "EEPROMUtils.h"

namespace cz
{

ConfigStoragePtr ConfigStorage::ptrAt(uint16_t address)
{
	return ConfigStoragePtr(*this, address);
}

} // namespace cz
