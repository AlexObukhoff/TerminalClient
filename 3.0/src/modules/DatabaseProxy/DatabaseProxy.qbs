import qbs 1.0

StaticLibrary {
	name: "DatabaseProxy"

	Depends { name: 'cpp' }
	Depends { name: "Qt"; submodules: ["core", "sql"] }
	Depends { name: "Core" }

	files: [
		"src/*.h",
		"src/*.cpp"
	]
}

