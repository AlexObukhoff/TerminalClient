// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
* MeG JavaScript extension lib
* '2006-2007
*
* dynamic JavaScipt loader
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// - - - - - - -
/*
function MjInclude($__fname) {
	MjAddBlindHTML('\<script src="'+$__fname+'.js'+'" language="javascript" type="text/javascript"\>\</script\>');
}
*/



// ==========================================================================
// MJ JS dynamic loader
// ==========================================================================
function Mj_IncDyn_Looper() {
	if (!$mj_IncDyn._is_activated) return;

	this.ic++;
	// alert($mj_IncDyn.a.length);

	var $unl = 0;
	for (var $i=0; $i<$mj_IncDyn.a.length; $i++) {
		if (!  eval($mj_IncDyn.a[$i]['o']) ) {
			$unl++;
		}
	}

	// все загрузились?
	if ($unl == 0) {
		mj_RT.Freeze($mj_IncDyn.loop_i);
		// alert('all loaded OK.'+$mj_IncDyn.r);
		eval($mj_IncDyn._func_ok)();
		// MjCRIO('IncDyn: all loaded DONE. '+$mj_IncDyn.ic, ' iterations.', '#ff3333');
		return true;
	}

	$mj_IncDyn.r--;
	if ($mj_IncDyn.r <= 0) {
		mj_RT.Freeze($mj_IncDyn.loop_i);
		// alert('all objects loading FAILED!!!');

		$s_r = '';
		for (var $i=0; $i<$mj_IncDyn.a.length; $i++) {
			if (!  eval($mj_IncDyn.a[$i]['o']) ) {
				$s_r += 'failed loading: ('+$mj_IncDyn.a[$i]['o']+') in file: '+$mj_IncDyn.a[$i]['f']+'\n';
			}
		}

		eval($mj_IncDyn._func_fail)($s_r);

		// MjCRIO('IncDyn: all loading FAILED!!! '+$mj_IncDyn.ic, ' iterations.', '#ff3333');
		return false;
	}
}


function MJ_IncDyn() {
	this.a = new Array();		// array of dynamically loading scripts
	this.r_i = 20;				// 2000 msecs to load one js
	this.r = this.r_i;			// global runs set
	this.ic = 0;				// interations load count

	this.loop_i = -1;
}

MJ_IncDyn.prototype.Add = function($__fullpath, $__control_objname) {
	var $i = this.a.length;
	this.a[$i] = new Array();
	this.a[$i]['o'] = $__control_objname;
	this.a[$i]['f'] = $__fullpath;
}

$mj_IncDyn = new MJ_IncDyn();
$mj_IncDyn.loop_i = mj_RT.Add('Mj_IncDyn_Looper', 10);
$mj_IncDyn._is_activated = false;

// - - - - - - -
function MjIncdyn($__fullpath, $__control_objname) {
	MjAddBlindHTML('\<script src="'+$__fullpath+'" language="javascript" type="text/javascript"\>\</script\>');
	eval($__control_objname+' = false;');
	$mj_IncDyn.Add($__fullpath, $__control_objname);
	$mj_IncDyn.r = $mj_IncDyn.r_i;
	mj_RT.Restart($mj_IncDyn.loop_i);
}

// - - - - - - -
function MjIncdynGo($__funcok, $__funcfail) {
	$mj_IncDyn._func_ok = $__funcok;
	$mj_IncDyn._func_fail = $__funcfail;

	$mj_IncDyn._is_activated = true;
}

