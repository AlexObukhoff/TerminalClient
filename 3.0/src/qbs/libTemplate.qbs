import qbs

StaticLibrary {
	targetName: (qbs.enableDebugCode && qbs.targetOS.contains("windows")) ? (name + 'd') : name
	
	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	Depends { name: "Core" }

	files: [ "src/*.cpp", "src/*.h" ]

	Group {
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: "lib"
    }
}
