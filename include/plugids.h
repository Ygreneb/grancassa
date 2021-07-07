#pragma once

namespace Benergy {
namespace GranCassa {

using namespace Steinberg;

// HERE are defined the parameter Ids which are exported to the host
enum HelloWorldParams : Vst::ParamID
{
	kBypassId = 100,

	kParamVolId = 102,
	kParamOnId = 1000
};


// HERE you have to define new unique class ids: for processor and for controller
// you can use GUID creator tools like https://www.guidgenerator.com/
static const FUID MyProcessorUID (0x51A7E9BA, 0x82DE4F6E, 0xB64B6384, 0xEE075ED2);
static const FUID MyControllerUID (0x143C25B4, 0xD9C14CDD, 0x9D534026, 0xE73EF071);


//------------------------------------------------------------------------
} // namespace GranCassa
} // namespace Benergy
