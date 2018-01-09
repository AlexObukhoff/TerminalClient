import qbs
import qbs.ModUtils
import qbs.FileInfo

Module {
	id: MultiLocale

	additionalProductTypes: [ "qm" ]

	FileTagger {
		patterns: [ "*.ts" ]
		fileTags: [ "ts" ]
	}
	
	Rule {
		multiplex: true
		inputs: [ "ts" ]
		
		Artifact {
			filePath: FileInfo.joinPaths(ModUtils.moduleProperty(product, "qmFilesDir"), product.name + '.qm')
			fileTags: ["qm"]
		}
		
		prepare: {
			var parameters = ['-silent'];
			
			inputs.ts.forEach(function(tsFile) {
				parameters.push(tsFile.filePath);
			});
			
			parameters.push('-qm');
			parameters.push(product.buildDirectory + '/'+ product.name + '.qm');

			var cmd = new Command("lrelease", parameters);
			cmd.description = "LRelease " + product.name + '.qm';
            cmd.highlight = 'codegen';
			return cmd;
		}
	}

	Group {
		fileTagsFilter: "qm"
		qbs.install: true
		qbs.installDir: "locale"
	}		
}
