// TODO use c++20 module
// import vulkan_hpp;

#include <iostream>

// TODO use c++20 designated initializers
// #define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include "editor/app.hpp"

#include "common/logging.h"
#include "platform/platform.h"
#include "plugins/plugins.h"

#include <core/platform/entrypoint.hpp>

#if defined(PLATFORM__ANDROID)
#	include "platform/android/android_platform.h"
#elif defined(PLATFORM__WINDOWS)
#	include "platform/windows/windows_platform.h"
#elif defined(PLATFORM__LINUX_D2D)
#	include "platform/unix/unix_d2d_platform.h"
#elif defined(PLATFORM__LINUX) || defined(PLATFORM__MACOS)
#	include "platform/unix/unix_platform.h"
#else
#	error "Platform not supported"
#endif

#include <filesystem>

#include "plugins/start_sample/start_sample.h"
using PeakStartSampleTags = vkb::PluginBase<vkb::tags::Entrypoint>;
class PeakSample : public PeakStartSampleTags
{
	using Super = PeakStartSampleTags;
public:
	PeakSample();

	virtual ~PeakSample() = default;

	virtual bool is_active(const vkb::CommandParser& parser) override;

	virtual void init(const vkb::CommandParser& parser) override;
};

PeakSample::PeakSample() : Super("Peak", "Peak Engine Plugin")
{
}

bool PeakSample::is_active(const vkb::CommandParser& parser)
{
	return true;
}

void PeakSample::init(const vkb::CommandParser& parser)
{
	{
		// Launch
		static apps::AppInfo appInfo = {"Peak",  create_peak_app};
		{
			vkb::Window::OptionalProperties properties;
			std::string                     title = "Peak";
			properties.title = title;
			platform->set_window_properties(properties);
			platform->request_application(&appInfo);
		}
	}
}

CUSTOM_MAIN(context)
{
#if defined(PLATFORM__ANDROID)
	vkb::AndroidPlatform platform{ context };
#elif defined(PLATFORM__WINDOWS)
	vkb::WindowsPlatform platform{ context };
#elif defined(PLATFORM__LINUX_D2D)
	vkb::UnixD2DPlatform platform{ context };
#elif defined(PLATFORM__LINUX)
	vkb::UnixPlatform platform{ context, vkb::UnixType::Linux };
#elif defined(PLATFORM__MACOS)
	vkb::UnixPlatform platform{ context, vkb::UnixType::Mac };
#else
#	error "Platform not supported"
#endif

	std::filesystem::current_path(std::filesystem::path(ROOT_SOURCE_DIR));

	PeakSample a;

	auto&& ptrs = plugins::get_all();
	ptrs.push_back(&a);

	auto code = platform.initialize(ptrs);

	if (code == vkb::ExitCode::Success)
	{
		code = platform.main_loop();
	}

	platform.terminate(code);

	return 0;
}


