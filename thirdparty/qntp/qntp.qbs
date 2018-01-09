import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

ThirdpartyLib {
	name: "qntp"

	Depends { name: "Qt"; submodules: ["network"] }

	files: [
		"src/qntp/NtpClient.h",
		"src/qntp/config.h",
		"src/qntp/NtpPacket.h",
		"src/qntp/QNtp.h",
		"src/qntp/NtpReply.h",
		"src/qntp/NtpReply_p.h",
		"src/qntp/NtpTimestamp.h",
		"src/qntp/NtpClient.cpp",
		"src/qntp/NtpReply.cpp"
	]

	cpp.defines: ['QNTP_BUILDING']

	Export {
		Depends { name: "cpp" }
		cpp.includePaths: 'include'
	}
}

