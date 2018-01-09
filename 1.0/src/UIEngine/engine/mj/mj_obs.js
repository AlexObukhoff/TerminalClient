// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
* MeG JavaScript extension lib
* '2006-2007
*
* obsolete code
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// - - - - - - -
function MjInclude($__fname) {
	MjAddBlindHTML('\<script src="'+$__fname+'.js'+'" language="javascript" type="text/javascript"\>\</script\>');
}



// ==========================================================================
// MJ JS dynamic loader
// ==========================================================================
function Mj_IncDyn_Looper() {
	this.ic++;
	// alert($mj_IncDyn.a.length);

	var $unl = 0;
	for (var $i=0; $i<$mj_IncDyn.a.length; $i++) {
		if (!  eval('$inc_'+$mj_IncDyn.a[$i]['o']) ) {
			$unl++;
		}
	}

	// все загрузились?
	if ($unl == 0) {
		mj_RT.Freeze($mj_IncDyn.loop_i);
		// alert('all loaded OK.'+$mj_IncDyn.r);
		// MjCRIO('IncDyn: all loaded DONE. '+$mj_IncDyn.ic, ' iterations.', '#ff3333');
	}

	$mj_IncDyn.r--;
	if ($mj_IncDyn.r <= 0) {
		mj_RT.Freeze($mj_IncDyn.loop_i);
		alert('all objects loading FAILED!!!');
		// MjCRIO('IncDyn: all loading FAILED!!! '+$mj_IncDyn.ic, ' iterations.', '#ff3333');
	}
}


function MJ_IncDyn() {
	this.a = new Array();		// array of dynamically loading scripts
	this.r_i = 20;				// 2000 msecs to load one js
	this.r = this.r_i;			// global runs set
	this.ic = 0;				// interations load count

	this.loop_i = -1;
}

MJ_IncDyn.prototype.Add = function($__objname) {
	var $i = this.a.length;
	this.a[$i] = new Array();
	this.a[$i]['o'] = $__objname;
}

$mj_IncDyn = new MJ_IncDyn();
$mj_IncDyn.loop_i = mj_RT.Add('Mj_IncDyn_Looper', 10);

// - - - - - - -
function MjIncdyn($__fname) {
	MjAddBlindHTML('\<script src="'+$__fname+'.js'+'" language="javascript" type="text/javascript"\>\</script\>');
	eval('$inc_'+$__fname+' = false;');
	$mj_IncDyn.Add($__fname);
	$mj_IncDyn.r = $mj_IncDyn.r_i;
	mj_RT.Restart($mj_IncDyn.loop_i);
}


