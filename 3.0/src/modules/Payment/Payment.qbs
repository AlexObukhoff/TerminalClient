import qbs 1.0

StaticLibrary {
	name: "PaymentBase"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "script"] }

	Depends { name: "Core" }
	Depends { name: "boost" }

	files: [ "src/*.cpp", "src/*.h" ]
	
	Export {
		Depends { name: "Qt"; submodules: ["script"] }
	}	
}

