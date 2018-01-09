// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
* MeG JavaScript extension lib
* '2006
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// run-time parameters
$mj_g_isDebug = true;

$mj_g_mjver		= '0.9.0009';
$mj_g_mjvertxt	= '20061226 (Age Of Computers v3)';


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
	this.mjver		= $mj_g_mjver;
	this.mjvertxt	= $mj_g_mjvertxt;
	this.initatt	= 0;	// count of initialization tryes

	// !!!
	var $nav = navigator.userAgent.toLowerCase();
	this.is_ie = !!($nav.indexOf("msie") >= 0 && document.all);

	// and you, BRuT! ;) - browser type: ie4, ie5, moz, opr
	// this.brt		=kklfjdfjlkj

	// - - - - - - - dom part
	this._dom = false;		// is DOM ?
	this._domo = null;		// DOM root object - а нужен вообще?
	this._domb = null;		// DOM BODY root object

	this._a = new Array();	// global ID and NODE array

	try {
		if ('HTML' === document.documentElement.tagName) {
			this._dom = true;
			this._domo = document.documentElement;
			// this._domb = document.body;
		}
	}
	catch (e) {
		// nothing todo
		// !!! а можно и выругаться, что DOM мы не поддерживаем
	}

	// - - - - - - - test for main viewport initialization (body) object
	this.inited = false;
	this.init();

	MjCRIO('MJ_G constructed. ver: '+this.mjver + '[' + this.mjvertxt+']');
};


// - - - - - - -
MJ_G.prototype.init = function($__i) {
	// MjCRIO('DUFF: '+this.uname);
	this.initatt++;

	if (this.inited) return;

	if (null != document.body) {
		this._domb = document.body;
		this.inited = true;
		MjCRIO('MJ_G.init - done. from attempt: '+this.initatt);

		// creating global viewport
		mj_gVP = new MJ_VD(-1, 'vd_base');
		mj_gVP.MoveTo(0, 0, 0);
		mj_gVP.SizeTo(0, this._domb.clientWidth, this._domb.clientHeight);
		mj_gVP.Border(0, '1px dotted red');
		mj_gVP.Redraw();
		mj_gVP.Setp('position', 'absolute');
		mj_gVP.Setp('visibility', 'visible');
		// mj_gVP.Setp('background', 'maroon');
		/*
		mj_gVP.Setp('background-attachment', 'fixed');
		mj_gVP.Setp('background-image', 'url(geor.gif)');
		mj_gVP.Setp('background-repeat', 'repeat');
		*/
		// mj_gVP.Setp('backgroundImage', 'url(bg.jpg)');



		MjCRIOD(mj_G._a[mj_gVP.i]['i'].style);
		// 0, 0, this._domb.clientWidth, this._domb.clientHeight
		// mozilla
		// mj_gVP = new MJ_VP(0, 0, this._domb.offsetWidth, this._domb.offsetHeight);
		// MjCRIOD(this._domb);

	}

};

// - - - - - - -
// add object to DOM, update MJ_G._a array
/*
'l' - label: id/name
't' - DIC/IMG
'n' - node object in DOM
'i' - ID element in Browser model

'p' - index to prev element in MJ_G._a array
*/
MJ_G.prototype.AddDOM = function($__pa, $__typ, $__id, $__attr) {
	var $i = this._a.length;
	this._a[$i] = new Array();
	var $a = this._a[$i];

	$a['p'] = $__pa;			// parent element index

	$a['l'] = $__id;			// label
	$a['t'] = $__typ;			// type

	var $ni = document.createElement($__typ);
	$ni.setAttribute('id', $__id);
	$a['n'] = $ni;				// node object

	// attributes with pre-addition to DOM
	if ('undefined' != typeof($__attr)) {
		for ($kv in $__attr) {
			$ni.setAttribute($kv, $__attr[$kv]);
		}
	}
	else {
		// alert('gretchka: '+$__id);
	}
	// not done yet!


	// node parent element
	var $nip = '';
	if ($__pa < 0) {
		$nip = document.body;
	}
	else {
		$nip = mj_G._a[$__pa];
	}
	$nip.appendChild($ni);

	$a['i'] = document.getElementById($__id);	// id in browser collection
	// alert($a['i']);
	if (!$a['i']) {
		alert('failed to create: '+$__id);
	}
	return $i;
}

// - - - - - - -
mj_G = new MJ_G('MJ glob');


// ==========================================================================
// MJ_V - class prototype for 'view' element - image or div
// ==========================================================================
function MJ_V($__pa, $__typ, $__lab, $__attr) {
	this.i = null;	// index in my doc cashe (array: mj_G._a)

	this.x = null;		// x coord
	this._x = false;	// need x update

	this.y = null;		// y coord
	this._y = false;	// need y update

	this.w = null;		// w coord
	this._w = false;	// need w update

	this.h = null;		// h coord
	this._h = false;	// need h update

	this.op = 1;		// opacity
	this._op = false;	// opacity

	this.bo = null;		// border
	this._bo = false;	// border

	/*
	var $i = mj_G.AddDOM('DIV', 'vp_glob');
	this.i = $i;
	var $id = mj_G._a[$i]['i'];

	$id.style['position'] = 'absolute';
	$id.style['visibility'] = 'visible';

	$id.style['border'] = '1px dotted maroon';

	$id.style['background'] = 'maroon';

	MjCRIOD($id.style);
	*/
   if ( arguments.length > 0 )
        this._init($__pa, $__lab);
}


MJ_V.prototype._init = function($__pa, $__typ, $__lab, $__attr) {
	var $i = mj_G.AddDOM($__pa, $__typ, $__lab, $__attr);
	this.i = $i;
	var $id = mj_G._a[$i]['i'];
}



// - - - - - - - Move to position
// parms:
//	$__u - immediate CSS update
//	$__x - abs X coord	(or NULL if not need)
//	$__y - abs Y coord  (or NULL if not need)
MJ_V.prototype.MoveTo = function($__u, $__x, $__y) {

	if (null !== $__x) {
		this._x = true;
		this.x = $__x;
	}
	if (null !== $__y) {
		this._y = true;
		this.y = $__y;
	}

	if ($__u) this._MoveTo();

}

MJ_V.prototype._MoveTo = function() {
	var $id = mj_G._a[this.i]['i'];

	if (this._x) {
		$id.style['left'] = parseInt(this.x) + 'px';
		this._x = false;
	}
	if (this._y) {
		$id.style['top'] = parseInt(this.y) + 'px';
		this._y = false;
	}
}


// - - - - - - - Size to position
// parms:
//	$__u - immediate CSS update
//	$__x - abs X coord
//	$__y - abs Y coord
MJ_V.prototype.SizeTo = function($__u, $__w, $__h) {

	if (null !== $__w) {
		this._w = true;
		this.w = $__w;
	}
	if (null !== $__h) {
		this._h = true;
		this.h = $__h;
	}

	if ($__u) this._SizeTo();

}

MJ_V.prototype._SizeTo = function() {
	var $id = mj_G._a[this.i]['i'];

	if (this._w) {
		$id.style['width'] = parseInt(this.w) + 'px';
		this._w = false;
	}
	if (this._h) {
		$id.style['height'] = parseInt(this.h) + 'px';
		this._h = false;
	}
}

// - - - - - - - Opacity
// parms:
//	$__u - immediate CSS update
//	$__n - opacity in (0-1) interval
MJ_V.prototype.Opacity = function($__u, $__op) {

	this._op = true;
	this.op = $__op;

	if ($__u) this._Opacity();
}

MJ_V.prototype._Opacity = function() {
	var $id = mj_G._a[this.i]['i'];

	if (this._op) {
		if (mj_G.is_ie) {
			$id.style.filter = "Alpha(opacity="+parseInt(100*parseFloat(this.op))+")";
		}
		else {
			$id.style['opacity'] = parseFloat(this.op);
		}
		this._op = false;
	}

}

// - - - - - - - BORDER
// parms:
//	$__u - immediate CSS update
//	$__op - border in '1px dotted red'
MJ_V.prototype.Border = function($__u, $__bo) {

	this._bo = true;
	this.bo = $__bo;

	if ($__u) this._Border();
}

MJ_V.prototype._Border = function() {
	var $id = mj_G._a[this.i]['i'];

	if (this._bo) {
		$id.style['border'] = this.bo;
	}

}


// - - - - - - - Update CSS from curr. params
MJ_V.prototype.Redraw = function() {
	this._MoveTo();
	this._SizeTo();
	this._Opacity();
	this._Border();
}
// MJ_VP.prototyp

// - - - - - - - Set da CSS attribute
MJ_V.prototype.Setp = function($__an, $__av) {
	mj_G._a[this.i]['i'].style[$__an] = $__av;
}



// ==========================================================================
// MJ_VI - class prototype for 'view image' element
// --- основан на MJ_V
// ==========================================================================
MJ_VI.prototype = new MJ_V();
// MJ_B.prototype.constructor = MJ_B;
MJ_VI.supaca = MJ_V.prototype;

function MJ_VI($__pa, $__lab, $__src) {
   if ( arguments.length > 0 )
        this._init($__pa, $__lab, $__src);
}


MJ_VI.prototype._init = function($__pa, $__lab, $__src) {


	MJ_VI.supaca._init.apply(this, [$__pa, 'IMG', $__lab, {'src':$__src}]);
	// alert(this.i);
}


// ==========================================================================
// MJ_VD - class prototype for 'view div' element
// ==========================================================================
MJ_VD.prototype = new MJ_V();
// MJ_B.prototype.constructor = MJ_B;
MJ_VD.supaca = MJ_V.prototype;

function MJ_VD($__pa, $__lab) {
   if ( arguments.length > 0 )
        this._init($__pa, $__lab);
}

MJ_VD.prototype._init = function($__pa, $__lab) {

	MJ_VD.supaca._init.apply(this, [$__pa, 'DIV', $__lab]);
	// alert(this.i);
}




/*
// ==========================================================================
// MJ_VP - class prototype for viewport
// ==========================================================================
OLD!!!!!!!!
function MJ_VP($__x, $__y, $__w, $__h) {
	this.i = null;	// index in my doc cashe (array: mj_G._a)

	this.x = $__x;		// x coord
	this._x = false;	// need x update

	this.y = $__y;		// y coord
	this._y = false;	// need y update

	this.w = $__w;		// w coord
	this._w = false;	// need w update

	this.h = $__h;		// h coord
	this._h = false;	// need h update

	this.op = 1;		// opacity
	this._op = false;	// opacity

	var $i = mj_G.AddDOM(-1, 'DIV', 'vp_glob');
	this.i = $i;
	var $id = mj_G._a[$i]['i'];

	$id.style['position'] = 'absolute';
	$id.style['visibility'] = 'visible';

	$id.style['border'] = '1px dotted maroon';

	$id.style['background'] = 'maroon';

	this.MoveTo(1, $__x, $__y);
	this.SizeTo(1, $__w, $__h);
	this.Opacity(1, 0.5);

	MjCRIOD($id.style);
}

// - - - - - - - Move to position
// parms:
//	$__u - immediate CSS update
//	$__x - abs X coord	(or NULL if not need)
//	$__y - abs Y coord  (or NULL if not need)
MJ_VP.prototype.MoveTo = function($__u, $__x, $__y) {

	if (null !== $__x) {
		this._x = true;
		this.x = $__x;
	}
	if (null !== $__y) {
		this._y = true;
		this.y = $__y;
	}

	if ($__u) this._MoveTo();

}

MJ_VP.prototype._MoveTo = function() {
	var $id = mj_G._a[this.i]['i'];

	if (this._x) {
		$id.style['left'] = parseInt(this.x) + 'px';
		this._x = false;
	}
	if (this._y) {
		$id.style['top'] = parseInt(this.y) + 'px';
		this._y = false;
	}
}


// - - - - - - - Size to position
// parms:
//	$__u - immediate CSS update
//	$__x - abs X coord
//	$__y - abs Y coord
MJ_VP.prototype.SizeTo = function($__u, $__w, $__h) {

	if (null !== $__w) {
		this._w = true;
		this.w = $__w;
	}
	if (null !== $__h) {
		this._h = true;
		this.h = $__h;
	}

	if ($__u) this._SizeTo();

}

MJ_VP.prototype._SizeTo = function() {
	var $id = mj_G._a[this.i]['i'];

	if (this._w) {
		$id.style['width'] = parseInt(this.w) + 'px';
		this._w = false;
	}
	if (this._h) {
		$id.style['height'] = parseInt(this.h) + 'px';
		this._h = false;
	}
}

// - - - - - - - Opacity
// parms:
//	$__u - immediate CSS update
//	$__n - opacity in (0-1) interval
MJ_VP.prototype.Opacity = function($__u, $__op) {

	this._op = true;
	this.op = $__op;

	if ($__u) this._Opacity();
}

MJ_VP.prototype._Opacity = function() {
	var $id = mj_G._a[this.i]['i'];

	if (this._op) {
		if (mj_G.is_ie) {
			$id.style.filter = "Alpha(opacity="+parseInt(100*parseFloat(this.op))+")";
		}
		else {
			$id.style['opacity'] = parseFloat(this.op);
		}
		this._op = false;
	}

}


// - - - - - - - Update CSS from curr. params
MJ_VP.prototype.Redraw = function() {
	this._MoveTo();
}
// MJ_VP.prototyp
*/

// ==========================================================================
// MJ_RunTimer() - class for threading emulation
// ==========================================================================

function mj_RunTimer_Looper() {
	mj_RT.runs++;

	for ($j=0; $j<mj_RT.a.length; $j++) {
		if (mj_RT.a[$j]['f']) continue;	// if frozen

		var $a = mj_RT.a[$j];
		$a['t'] += mj_RT.to;

		if ($a['t'] >= $a['to']) {

			eval($a['fn'])();
			// eval($a['fn']);

			$a['t'] = 0;
			$a['c']++;
			if  ( ($a['cm'] > 0) && ($a['c'] >= $a['cm']) ) {
				$a['f'] = true;	// froze, if run-limit exceded
			}
		}	// if time is come!
	} // for


	if (0 < mj_RT.to) {
		setTimeout('mj_RunTimer_Looper()', mj_RT.to);
	}
}


function MJ_RunTimer($__to) {
	this.a = new Array();		// array with run-timers
	this.runs = 0;
	this.to = $__to;
}

MJ_RunTimer.prototype.Start = function($__to) {
	this.to = $__to;
	mj_RunTimer_Looper();
}

MJ_RunTimer.prototype.Stop = function() {
	clearTimeout('mj_RunTimer_Looper()');
	this.to = 0;
}


// add a run-timer
// params: function_name(), timeout, count of run till freese
// returns: id of a run-timer
MJ_RunTimer.prototype.Add = function($__f, $__to, $__c) {
	var $i = this.a.length;
	this.a[$i] = new Array();
	$a = this.a[$i];
	$a['fn'] = $__f;		// function_name()
	$a['to'] = $__to;	// timeout
	$a['cm'] = (undefined === $__c) ? -1 : $__c;
						// repeats max count criteria (-1 - infinitive)
	$a['f'] = false;	// frozen ?
	// run-time
	$a['c'] = 0;		// repeats count
	$a['t'] = 0;		// milliseconds of run-to-run interval counter ;)
	// ok, init done.
	// alert(this.a.length);
	return $i;
}

MJ_RunTimer.prototype.Freeze = function($__id) {
	this.a[$__id]['f'] = true;
}

MJ_RunTimer.prototype.Restart = function($__id) {
	this.a[$__id]['c'] = 0;
	this.a[$__id]['f'] = false;
}


mj_RT = new MJ_RunTimer();
mj_RT.Start(1);


// ==========================================================================

MjCRIO('[mj.js] include done.');

/*
$rrr = '';
function TTT() {
	$a1 = (('undefined' === typeof($meg_g1)) ? '0' : '1');
	$a2 = (('undefined' === typeof($meg_g2)) ? '0' : '1');
	$a3 = (('undefined' === typeof($meg_g3)) ? '0' : '1');

	$i = document.getElementById('meg_o');
	if ($i) $i.innerHTML = $rrr;

	$rrr += '<br>'+$a1 +', '+$a2 +', '+$a3;
}

MjInclude('mj_test');
TTT();
mj_RT.Add('TTT()', 1, 15);
*/

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
		// alert('all loaded FAILED!!!');
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





$vv = 0;
$icnt = 33;
function Rot() {
	if (!$vv) {
		// mj_gVP.i
		mj_i1 = new MJ_VI(-1, 'timg_bg', 'bg.bmp');
		mj_i1.Setp('zIndex', 60);
		mj_i1.Setp('position', 'absolute');
		mj_i1.MoveTo(1, 0, 0);

		mj_i = new Array();
		for (var $i=0; $i<$icnt; $i++)  {
			mj_i[$i] = new MJ_VI(-1, 'timg'+$i, 'logo.gif');
			mj_i[$i].Setp('zIndex', 70);
			mj_i[$i].Setp('position', 'absolute');
		}
	}

	$vv++;
	// alert('wow');
	for (var $i=0; $i<$icnt; $i++)  {
		mj_i[$i].MoveTo(0, Math.cos(($vv+$i*3)/20)*(250-$i*5)+250, Math.sin(($vv+$i*3)/20)*(250-$i*5)+250);
		//mj_i[$i].Opacity(0, (Math.sin(($vv+3)/10)/4)+0.5);
		mj_i[$i].Redraw();
	}

	/*
	if (0 == ($vv % 10)) {
	mj_gVP.Redraw();
	}
	*/
}

mj_RT.Add('Rot', 3);





// ==========================================================================
// [EOF]
// ==========================================================================

