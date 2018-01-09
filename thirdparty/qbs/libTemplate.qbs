import qbs

StaticLibrary {
	targetName: (qbs.enableDebugCode && qbs.targetOS.contains("windows")) ? (name + 'd') : name

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core"] }

	destinationDirectory: project.libDirName
	
    Group {
        fileTagsFilter: product.type.concat("dynamiclibrary_symlink")
        qbs.install: true
        qbs.installDir: project.libInstallDir
    }	
	
	Group {
		name: "install"
		fileTagsFilter: [ "application", "dynamicLibrary", "pdb"]
		qbs.installDir: project.libInstallDir
		qbs.install: true
	}	
	
    Export {
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["core"] }
        cpp.rpaths: project.libRPaths
        cpp.includePaths: "."
    }	
	
   FileTagger {
           patterns: ["*.pdb"]
           fileTags: ["pdb"]
   }
   
	// Windows XP Compability for VS 2013
	Group {
		condition: qbs.targetOS == "windows"
		
		cpp.defines: outer.concat("_USING_V110_SDK71_")
		cpp.minimumWindowsVersion: "5.01"
	}
}
