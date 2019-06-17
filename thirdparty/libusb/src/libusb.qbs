import qbs 1.0
import "../../qbs/libTemplate.qbs" as ThirdpartyLib

ThirdpartyLib {
	name: "LibUSB"

	files: [
		"libusb/core.c",
		"libusb/descriptor.c",
		"libusb/hotplug.c",
		"libusb/io.c",
		"libusb/os/poll_windows.c",
		"libusb/strerror.c",
		"libusb/sync.c",
		"libusb/os/threads_windows.c",
		"libusb/os/windows_nt_common.c",
		"libusb/os/windows_usbdk.c",
		"libusb/os/windows_winusb.c",

		"msvc/config.h",
		"libusb/hotplug.h",
		"libusb/libusb.h",
		"libusb/libusbi.h",
		"libusb/os/poll_windows.h",
		"libusb/os/threads_windows.h",
		"libusb/version.h",
		"libusb/version_nano.h",
		"libusb/os/windows_common.h",
		"libusb/os/windows_nt_common.h",
		"libusb/os/windows_nt_shared_types.h",
		"libusb/os/windows_usbdk.h",
		"libusb/os/windows_winusb.h"
	]

	cpp.includePaths: ['msvc', 'libusb', 'libusb/os']
}
