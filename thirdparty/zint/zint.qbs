import qbs 1.0
import "../qbs/libTemplate.qbs" as ThirdpartyLib

/*
   Zint librarry 
   http://www.sourceforge.net/projects/zint

   A barcode encoding library supporting over 50 symbologies including Code 128, Data Matrix, USPS OneCode, EAN-128, UPC/EAN, ITF, 
   QR Code, Code 16k, PDF417, MicroPDF417, LOGMARS, Maxicode, GS1 DataBar, Aztec, Composite Symbols and more.
   After some time of inactivity this project is now back to life! This is the one and original ZINT barcode generator, the reference in open source barcodes.
   ZINT is licensed under the terms of the GPL v3. Only the contained backend (aka ZINT shared library) is licensed under BSD 3.
   
    Thanks to Robin. 
*/

Project
{

	ThirdpartyLib {
		name: "QZint"

		Depends { name: "Qt"; submodules: ["gui"] }
		
		cpp.defines: ["NO_PNG", 'ZINT_VERSION="2.4.2"']
		
		// Common files
		files: [
			"backend/*.h",
			"backend/common.c",
			"backend/library.c",
			"backend/render.c",
			"backend/ps.c",
			"backend/large.c",
			"backend/reedsol.c",
			"backend/gs1.c",
			"backend/svg.c",
			"backend/png.c",

			// Qt wrapper files
			"backend_qt4/qzint.h",
			"backend_qt4/qzint.cpp",

			// 1D barcode files
			"backend/code.c",
			"backend/code128.c",
			"backend/2of5.c",
			"backend/upcean.c",
			"backend/telepen.c",
			"backend/medical.c",
			"backend/plessey.c",
			"backend/rss.c",

			// 2D barcode files
			"backend/code16k.c",
			"backend/dmatrix.c",
			"backend/pdf417.c",
			"backend/qr.c",
			"backend/maxicode.c",
			"backend/composite.c",
			"backend/aztec.c",
			"backend/code49.c",
			"backend/code1.c",
			"backend/gridmtx.c", 
			
			// Postal codes
			"backend/postal.c",
			"backend/auspost.c",
			"backend/imail.c"
		]

		cpp.includePaths: [ ".", "backend"]
		
		Export {
			Depends { name: "cpp" }
			Depends { name: "Qt"; submodules: ["core"] }

			cpp.includePaths: [
				product.sourceDirectory + "/backend_qt4"
			]
		}	
	}
	
/*
	CppApplication {
		name: "QZintTest"
		consoleApplication: false

		targetName: (qbs.enableDebugCode && qbs.targetOS.contains("windows")) ? (name + 'd') : name
		
		Depends { name: "Qt"; submodules: ["core", "widgets", "uitools"] }
		Depends { name: "QZint" }
		
		cpp.includePaths: [ ".", "backend", "backend_qt4", "frontend_qt4"]
		
		files: [
			"frontend_qt4/mainwindow.h", "frontend_qt4/datawindow.h", "frontend_qt4/sequencewindow.h", "frontend_qt4/exportwindow.h", 
			"frontend_qt4/barcodeitem.cpp", "frontend_qt4/main.cpp", "frontend_qt4/mainwindow.cpp", "frontend_qt4/datawindow.cpp", "frontend_qt4/sequencewindow.cpp", "frontend_qt4/exportwindow.cpp",
			"frontend_qt4/mainWindow.ui", "frontend_qt4/extData.ui", "frontend_qt4/extSequence.ui", "frontend_qt4/extExport.ui",
			"frontend_qt4/resources.qrc",
		]
		
		Group {
			name: "install"
			fileTagsFilter: [ "application", "dynamicLibrary", "pdb"]
			qbs.installDir: project.libInstallDir
			qbs.install: true
		}	
	}
*/
}

