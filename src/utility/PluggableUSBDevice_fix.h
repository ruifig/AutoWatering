
#include <USB/PluggableUSBDevice.h>

//
// To fix "undefined reference to vtable for X" when building Arduino Core
// See https://gcc.gnu.org/wiki/VerboseDiagnostics#undefined_reference_to_vtable_for_X
namespace arduino
{
	namespace internal
	{
		const uint8_t* PluggableUSBModule::configuration_desc(uint8_t index) { return nullptr; }
		void PluggableUSBModule::callback_state_change(USBDevice::DeviceState new_state) { }
		uint32_t PluggableUSBModule::callback_request(const USBDevice::setup_packet_t *setup, USBDevice::RequestResult *result, uint8_t** data) { return 0; }
		bool PluggableUSBModule::callback_request_xfer_done(const USBDevice::setup_packet_t *setup, bool aborted) { return true; }
		bool PluggableUSBModule::callback_set_configuration(uint8_t configuration) { return true; }
		void PluggableUSBModule::callback_set_interface(uint16_t interface, uint8_t alternate) { }
		void PluggableUSBModule::init(EndpointResolver& resolver) {}
		const uint8_t* PluggableUSBModule::string_iinterface_desc() { return nullptr; }
	}
}
