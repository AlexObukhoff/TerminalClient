// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
* MeG JavaScript extension lib
* '2006
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// run-time parameters
// $mj_g_isDebug = true;
$mj_g_isDebug = false;

// ==========================================================================
// MJ global 'all-need' functions
// ==========================================================================
// - - - - - - -
// add blind HTML code while body TAG is not closes
// !!! dont working in runtime, while document DOM is finalized
function MjAddBlindHTML($__s) {
	document.writeln($__s);
}

// - - - - - - -
// get current time (in milliseconds)
function MjGetMeter() {
	$d = new Date();
	return $d.valueOf();
}

// - - - - - - -
function MjInclude($__fname) {
	try {
	  if ( eval('$g_included_mj_numcapacity') ) return;
	}
	catch(e) {
	}
	// по-хорошему, надо бы переписать на AJAX
	MjAddBlindHTML('\<script src="'+$__fname+'.js'+'" language="javascript" type="text/javascript"\>\</script\>');


}



// ==========================================================================
// MJ debug sybsystem
// ==========================================================================

$mj_gCrioCnt = 0;
function MjCRIO($__m, $__c) {
	if (!$mj_g_isDebug) return;
	if ($mj_gCrioCnt === 0) {
		MjAddBlindHTML('<link href="mj.css" rel="stylesheet" type="text/css">');
	}
	$si = '';
	if ('undefined' !== $__c)  $si = ' style="background-color: '+$__c+';"';
	MjAddBlindHTML('<span class="mjm"'+$si+'>|'+ $mj_gCrioCnt +'| '+ $__m +'</span><br>');
	$mj_gCrioCnt++;
}

// - - - - - - - DUMP objext
function MjCRIOD($__o) {
	MjCRIO('CRIOD('+$__o+')', '#993355');
	// $s = '';
	$cnt = 0;
	for ($v in $__o) {
		if ('innerText' === $v) continue;
		if ('outerText' === $v) continue;
		if ('innerHTML' === $v) continue;
		if ('outerHTML' === $v) continue;
		if ('all' === $v) continue;
		// $s = '<b>'+$v+':</b> '; //+$__o[$v].length;
		$s = '<b>'+$v+':</b>' +$__o[$v];
		// MjCRIO($__o.name +': '+ $s);
		MjCRIO($s, '#003355');
		$cnt++;
		// if ($cnt > 110) break;
	}

}

// - - - - - - -
// ==========================================================================
// MJ_G - global run-time settings object
// defined, as 'mj_G' in code.
//
// ==========================================================================
function MJ_G($__uname) {

	// ----- runtime part
	this.id = 'MeG';		// !!!
	this.uname = $__uname;	// !!!

	this.time_st	= MjGetMeter();
	this.mjver		= '0.9.0004';
	this.mjvertxt	= '20061222 (Imperative Reaction: Cleaned)';
	this.initcnt	= 1;

	// !!!
	// and you, BRuT! ;) - browser type: ie4, ie5, moz, opr
	// this.brt		=kklfjdfjlkj

	// ----- dom part
	this._dom = false;		// is DOM ?
	this._domo = null;		// DOM root object
	this._domb = null;		// DOM BODY root object

	try {
		if ('HTML' === document.documentElement.tagName) {
			this._dom = true;
			this._domo = document.documentElement;
			this._domb = document.body;
		}
	}
	catch (e) {
		// nothing todo
	}

	// ----- test for main viewport initialization (body) object
	if (null == this._domb) {
		this.inited = null;
	}
	else {
		this.inited = true;
	}


	MjCRIO('MJ_G constructed. ver: '+this.mjver + '[' + this.mjvertxt+']');
};


// - - - - - - -
MJ_G.prototype.init = function($__i) {
	// MjCRIO('DUFF: '+this.uname);
	if (this.inited) return;
	this.initcnt++;
};


// - - - - - - -
mj_G = new MJ_G('MJ glob');

MjCRIO('body: ' + mj_G._domb);
MjCRIOD('body: ' + mj_G._domb);
// MjCRIOD(document.documentElement.body);

/*
mj_G1 = new MJ_G('test 2');

mj_G.MJ_G();
mj_G1.MJ_G();
mj_G.MJ_G();

MjCRIO('dom: '+mj_G._dom);

MjCRIOD(mj_G._domo);

MjCRIO('[mj.js] include done.');

MjCRIOD(mj_G);

MjCRIO(document.documentElement.tagName);
*/

MjCRIO('[mj.js] include done.');


// ==========================================================================
// [EOF]
// ==========================================================================
