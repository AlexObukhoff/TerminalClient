import qbs
import qbs.TextFile
import qbs.Environment
import qbs.FileInfo

Module {
	additionalProductTypes: ["VersionUpdaterType"]

	FileTagger {
		patterns: [ "*.in" ]
		fileTags: [ "VersionUpdaterIn" ]
	}
	
	Rule {
		inputs: [ "VersionUpdaterIn" ]
		Artifact {
			filePath: product.sourceDirectory + "/includes/Common/" + FileInfo.completeBaseName(input.filePath)
			fileTags: [ "VersionUpdaterType" ]
		}
		prepare: {
			
			var cmd = new JavaScriptCommand();
			cmd.description = "Processing '" + input.fileName + "' -> '" + FileInfo.completeBaseName(input.filePath);
			cmd.highlight = "codegen";
			cmd.sourceCode = function() {
			
				var version = product.TC_VERSION == undefined ? "3.0.0" : product.TC_VERSION;
				var versionComma = version.replace(".", ",");
				
				var build = product.BUILD_NUMBER == undefined ? "0" : product.BUILD_NUMBER;
				
				var file = new TextFile(input.filePath);
				var content = file.readAll();
				
				content = content.replace(/3\.0\.0/g, version);
				content = content.replace(/000000000000/g, build + ((product.TC_USE_TOKEN > 0) ? " token" : ""));
				content = content.replace(/0\.0\.0\.0/g, version + "." + build);
				content = content.replace(/0\,0\,0\,0/g, versionComma + "," + build);

				file = new TextFile(output.filePath, TextFile.WriteOnly);
				file.truncate();
				file.write(content);
				file.close();
			} 
			
			return cmd;
		}
	}
}
