import qbs 1.0

StaticLibrary {
	name: "Packer"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }

	cpp.includePaths: [ Qt.core.incPath + "/../src/3rdparty/zlib" ]
	
	files: [ "src/*.cpp", "src/*.h" ]
}

