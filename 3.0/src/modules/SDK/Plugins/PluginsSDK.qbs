import qbs 1.0
import "../../../qbs/libTemplate.qbs" as TCLib

TCLib {
	name: "PluginsSDK"
	Depends { name: "qt5port" }
	cpp.minimumWindowsVersion: "5.2"
}
