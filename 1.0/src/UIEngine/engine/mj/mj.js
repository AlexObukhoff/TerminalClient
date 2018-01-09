/**
/**
* |+|----------------------------------------------------------------------|
* | MeG JavaScript extension lib
* | [CYBERPLAT interface build]
* |
* | |----------------------------------------------------------------------|
* | | main MJ lib file
* | |
* | |
* | |----------------------------------------------------------------------|
* |
* | Dmitriy "MeG" Agafonov // L.i.R. (LostIn)
* | (C) 2006-2007 Cyberplat.com
* |
* |------------------------------------------------------------------------|
* | todo:
* |
* | +
* |
* |-|----------------------------------------------------------------------|
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// run-time parameters
$mj_g_isDebug = false;

$mj_g_mjver                = '1.0.0041';
$mj_g_mjvertxt        = '20070608 (Independence of Faith, ressurected damnation)';

// ==========================================================================
// keyboard constants - scancodes
// ==========================================================================

var mjsc_Esc                = 0x1b;
var mjsc_Enter                = 0x0d;

var mjsc_Delete                = 0x2e;
// var mjsc_Backspace        = 0x08;

var mjsc_Space                = 0x20;

/* HELP FOR YOU
0x30        0
0x31        1
0x32        2
0x33        3
0x34        4
0x35        5
0x36        6
0x37        7
0x38        8
0x39        9
0x41        A
0x42        B
0x43        C
0x44        D
0x45        E
0x46        F
0x47        G
0x48        H
0x49        I
0x4a        J
0x4b        K
0x4c        L
0x4d        M
0x4e        N
0x4f        O
0x50        P
0x51        Q
0x52        R
0x53        S
0x54        T
0x55        U
0x56        V
0x57        W
0x58        X
0x59        Y
0x5a        Z
0x60        0 на цифровой клавиатуре
0x61        1 на цифровой клавиатуре
0x62        2 на цифровой клавиатуре
0x63        3 на цифровой клавиатуре
0x64        4 на цифровой клавиатуре
0x65        5 на цифровой клавиатуре
0x66        6 на цифровой клавиатуре
0x67        7 на цифровой клавиатуре
0x68        8 на цифровой клавиатуре
0x69        9 на цифровой клавиатуре
0x70        F1
0x71        F2
0x72        F3
0x73        F4
0x74        F5
0x75        F6
0x76        F7
0x77        F8
0x78        F9
0x79        F10
0x7a        F11
0x7b        F12
0x7c        F13
0x7d        F14
0x7e        F15
0x7f        F16
*/

/* !!! You personal codes for LEFT-RIGHT buttons !!! */
/* !!! You personal codes for LEFT-RIGHT buttons !!! */
/* !!! You personal codes for LEFT-RIGHT buttons !!! */

var mjsc_l1                       = 0x41; // a
var mjsc_l2                       = 0x42; // b
var mjsc_l3                       = 0x43; // c
var mjsc_l4                       = 0x44; // d
var mjsc_r1                       = 0x45; // e
var mjsc_r2                       = 0x46; // f
var mjsc_r3                       = 0x47; // g
var mjsc_r4                       = 0x48; // h


/*
var mjsc_l1                       = 0x47; // g
var mjsc_l2                       = 0x45; // e
var mjsc_l3                       = 0x43; // c
var mjsc_l4                       = 0x41; // a
var mjsc_r1                       = 0x48; // h
var mjsc_r2                       = 0x46; // f
var mjsc_r3                       = 0x44; // d
var mjsc_r4                       = 0x42; // b
*/

/*
var mjsc_l1                       = 0x41; // a
var mjsc_l2                       = 0x42; // b
var mjsc_l3                       = 0x43; // c
var mjsc_l4                       = 0x44; // d
var mjsc_r1                       = 0x45; // e
var mjsc_r2                       = 0x46; // f
var mjsc_r3                       = 0x47; // g
var mjsc_r4                       = 0x48; // h
*/

/*
var mjsc_l1                       = 0x70; // F1
var mjsc_l2                       = 0x71; // F2
var mjsc_l3                       = 0x72; // F3
var mjsc_l4                       = 0x73; // F4
var mjsc_r1                       = 0x74; // F5
var mjsc_r2                       = 0x75; // F6
var mjsc_r3                       = 0x76; // F7
var mjsc_r4                       = 0x77; // F8
*/



/* DO NOT CHANGE!!! */
var mjsc_0                        = 0x30;
var mjsc_1                        = 0x31;
var mjsc_2                        = 0x32;
var mjsc_3                        = 0x33;
var mjsc_4                        = 0x34;
var mjsc_5                        = 0x35;
var mjsc_6                        = 0x36;
var mjsc_7                        = 0x37;
var mjsc_8                        = 0x38;
var mjsc_9                        = 0x39;

var mjsc_0p                        = 0x60;
var mjsc_1p                        = 0x61;
var mjsc_2p                        = 0x62;
var mjsc_3p                        = 0x63;
var mjsc_4p                        = 0x64;
var mjsc_5p                        = 0x65;
var mjsc_6p                        = 0x66;
var mjsc_7p                        = 0x67;
var mjsc_8p                        = 0x68;
var mjsc_9p                        = 0x69;


// ==========================================================================
function mjScanCodeById($__i) {
        var $kode = mjsc_0;
        switch ($__i) {
                case 0:
                $kode =  mjsc_l1;
                break;
                case 2:
                $kode = mjsc_l2;
                break;
                case 4:
                $kode = mjsc_l3;
                break;
                /*
                case 6:
                $kode = mjsc_l4;
                break;
                */
                case 1:
                $kode = mjsc_r1;
                break;
                case 3:
                $kode = mjsc_r2;
                break;
                case 5:
                $kode = mjsc_r3;
                break;
                /*
                case 7:
                $kode = mjsc_r4;
                break;
                */
                default:
                $kode = mjsc_l1;
                break;
        }

        //alert('$__i='+$__i+', $kode='+$kode );
        return $kode;

}



// - - - - - - -
// returns true, if variablel is defined
function MjIVE($__s) {
        if ('undefined' === eval('typeof('+$__s+')') ) {
                return false;
        }
        else {
                return true;
        }
}

// - - - - - - -
// In-Var data
// returns true, if variablel is defined
function MjIVS($__s, $__d) {
        var $s = '('+$__s+'.'+$__d+')';
        alert($s);

        if ('undefined' === eval('typeof('+$__s+'.'+$__d+')') ) {
                return false;
        }
        else {
                return true;
        }
}



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


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// xp, yp - point, (x1, y1 - x2,y2) - rect
function MjIsPointInRect($xp, $yp, $x1, $y1, $x2, $y2) {
        if (($xp >= $x1) && ($xp <= $x2) && ($yp >= $y1) && ($yp <= $y2)) return true;
        return false;
}


// ==========================================================================
// MJ debug sybsystem
// ==========================================================================

$mj_gCrioCnt = 0;
function MjCRIO($__m, $__c) {
        if (!$mj_g_isDebug) return;
        if ($mj_gCrioCnt === 0) {
                MjAddBlindHTML('<link href="j/mj.css" rel="stylesheet" type="text/css">');
        }
        $si = '';
        if ('undefined' !== $__c)  $si = ' style="background-color: '+$__c+';"';
        MjAddBlindHTML('<span class="mjm"'+$si+'>|'+ $mj_gCrioCnt +'| '+ $__m +'</span><br>');
        $mj_gCrioCnt++;
}

// - - - - - - - DUMP objeсt
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


function MjCRIODA($__o) {
        var $cnt = 0;
        var $s1 = '';
        // .srcElement
        for ($v in $__o) {        // . target .srcElement  .srcElement.offsetY
                $s1 += $v + ' = ' + $__o[$v] + '\n';
                // $s1 += $v + '\n';
                $cnt++;
                if ($cnt > 10) {
                        alert($s1 + '\n>>>');
                        $s1 = '';
                        $cnt = 0;
                }
        }
        alert($s1);
}

// - - - - - - -
// ==========================================================================
// MJ_G - global run-time settings object
// defined, as 'mj_G' in code.
//
// ==========================================================================
function MJ_G($__uname) {

        // ----- runtime part
        this.id = 'MeG';                // !!!
        this.uname = $__uname;        // !!!

        this.time_st        = MjGetMeter();
        this.mjver                = $mj_g_mjver;
        this.mjvertxt        = $mj_g_mjvertxt;
        this.initatt        = 0;        // count of initialization tryes

        // !!!
        var $nav = navigator.userAgent.toLowerCase();
        this.is_ie = !!($nav.indexOf("msie") >= 0 && document.all);

        // and you, BRuT! ;) - browser type: ie4, ie5, moz, opr
        // this.brt                =kklfjdfjlkj

        // - - - - - - - dom part
        this._dom = false;                // is DOM ?
        this._domo = null;                // DOM root object - а нужен вообще?
        this._domb = null;                // DOM BODY root object

        this.Wpx = 0;                        // body width
        this.Hpx = 0;                        // body height

        this._a = new Array();        // global ID and NODE array

        this._kh = new Array(); // keyboard handlers

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

                document.onclick = mj_gf_onclick;
                // document.onkeypress = mj_gf_keypress;

                document.onkeydown = mj_gf_keypress;

                MjCRIO('MJ_G.init - done. from attempt: '+this.initatt);

                // creating global viewport
                this.Wpx = this._domb.clientWidth;
                this.Hpx = this._domb.clientHeight;

        }

};



// - - - - - - -
// keyboard events router
function mj_gf_keypress(evt) {

        var $evt = (evt)?evt : ((window.event)?window.event:"");

        var $e_type = 0;
        var $e_type = window.event.type;
        var $e_code = window.event.keyCode;


        if ('undefined' === typeof(mj_G._kh[$e_code])) {
                return false;
        }

        for ($v in mj_G._kh[$e_code]) {
                if (mj_G._kh[$e_code][$v] == 1) {
                        if (mj_gf_global_click($v)) {
                                break;
                        }
                }
        }

        // alert('defined!');


        // MjCRIODA(window.event);
        // alert(window.event.keyCode);
        // alert('pressed!');
        // MjCDRIODA($evt);
        // MjCRIODA();
        // alert('pressed ok!');
        // alert('e_type: ' + $e_type + '\ne_code: ' + $e_code);
        /*
                var $e_type = window.event.type;
                var $e_code = window.event.keyCode;

                alert('e_type: ' + $e_type + '\ne_code: ' + $_code);
                */
        document.focus();
}

// - - - - - - -
// mouse events router
function mj_gf_onclick(evt) {
        if ('undefined' === mj_G) {
                // alert('suxx');
                return false;
        }

        evt = (evt)?evt:(window.event)?window.event:"";
        var evtp = null;
        // different 4 msie and mozilla
        if (mj_G.is_ie) { evtp = evt.srcElement; }
        else evtp = evt.target;

        // alert( evt.srcElement.tagName);

        if (0) {
                var ojd = evtp;
                var $cnt = 0;
                var $s1 = '';
                // .srcElement
                for ($v in ojd) {        // . target .srcElement  .srcElement.offsetY
                        $s1 += $v + ' = ' + ojd[$v] + '\n';
                        // $s1 += $v + '\n';
                        $cnt++;
                        if ($cnt > 30) {
                                alert($s1 + '\n>>>');
                                $s1 = '';
                                $cnt = 0;
                        }
                }
                alert($s1);
        }

        var ofsX = 0;
        var ofsY = 0;
        if (mj_G.is_ie) {
                ofsX = evt.offsetX + evtp.offsetLeft;
                ofsY = evt.offsetY + evtp.offsetTop;
        }
        else {
                ofsX = evt.pageX; //  + evtp.offsetLeft;
                ofsY = evt.pageY; // + evtp.offsetTop;
        }



        // - - - - - - -
        /*
        var $aa = new Array();
        for (var $i=1; $i<mj_G._a.length; $i++) {
                $aa[$i] = mj_G._a[$i]['l'] + ':' + mj_G._a[$i]['a'];
                if ('vp_4_sub_8' === mj_G._a[$i]['l']) {
                        mj_G._a[$i]['o'].SetBG(1, '#ff0000');
                        mj_G._a[$i]['o'].Border(1, '7px dashed #880000');
                }
        }
        MjCRIODA($aa);
        */


        // - - - - - - -
        for (var $i=1; $i<mj_G._a.length; $i++) {

                $o = mj_G._a[$i]['o'];

                // alert(evt.offsetY +'\n'+ evt.clientY);

                // if object found!!!
                // if (MjIsPointInRect(evt.clientX, evt.clientY, $o.x, $o.y, $o.x+$o.w, $o.y+$o.h)) {

                // overloaded width ???
                var $trw = $o.w;
                if ($o.ow !== null) {
                        // alert('$trw: ' + $trw);
                        $trw = $o.ow;
                }
                // overloaded height ???
                var $trh = $o.h;
                if ($o.oh != null) {
                        // alert('$trh: ' + $trh);
                        $trh = $o.oh;
                }

                // alert($trw);


                if (MjIsPointInRect(ofsX, ofsY, $o.x, $o.y, $o.x+$trw, $o.y+$trh)) {
                        // break if clicked
                        if (mj_gf_global_click($i)) {
                                break;
                        }
                }
        }

        return false;
}

// - - - - - - -
// test for global click into object $i
// false if none
// true if clicked
function mj_gf_global_click($i) {

        // alert('found!: '+mj_G._a[$i]['l']);
        if ( ('undefined' !== typeof(mj_G._a[$i]['hl'])) && ((mj_G._a[$i]['hl']) !== '') && (mj_G._a[$i]['a']) ) {
                // alert(mj_G._a[$i]['l']);
                /*
                if ('function' === typeof(gfxa_aniobj_use) && ('cvp_PP_PinPress' !== mj_G._a[$i]['hl'].substring(0, 15))) {
                        //gfxa_aniobj_use($i);
                        //setTimeout("eval(mj_G._a["+$i+"]['hl']);", 30);
                }
                else {
                        
                }
                */
                eval(mj_G._a[$i]['hl']);
                return true;
        }
        return false;
}




// - - - - - - -
// add object to dom, update MJ_G._a array
/*
'l' - label: id/name
't' - DIV/IMG
'n' - node object in DOM
'i' - ID element in Browser model

'p' - index to prev element in MJ_G._a array
'o' - link to object, based on MJ_V class

'hl' - left mouse click event handler
'a' - object active/passive (passive by default)

*/

MJ_G.prototype.AddDOM = function($__pa, $__typ, $__id, $__attr) {
        var $i = this._a.length;
        this._a[$i] = new Array();
        var $a = this._a[$i];

        $a['a'] = false;                        // object is passive
        $a['p'] = $__pa;                        // parent element index

        $a['l'] = $__id;                        // label

        $a['t'] = $__typ;                        // type

        var $ni = document.createElement($__typ);
        $ni.setAttribute('id', $__id);
        // $ni.setAttribute('onclick', sorro );
        // $ni.setAttribute('name', $__typ);
        $a['n'] = $ni;                                // node object

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
                $nip = mj_G._a[$__pa][$i];
        }
        $nip.appendChild($ni);

        $a['i'] = document.getElementById($__id);        // id in browser collection
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
        this.i = null;        // index in my doc cashe (array: mj_G._a)

        this.v = null;                // visibility
        this._v = null;                // need visibility update

        this.a = null;                // absolute position
        this._a = null;                // need absolute position update

        this.x = null;                // x coord
        this._x = false;        // need x update

        this.y = null;                // y coord
        this._y = false;        // need y update

        this.w = null;                // w coord
        this._w = false;        // need w update

        this.h = null;                // h coord
        this._h = false;        // need h update

        this.bg = null;                // background
        this._bg = false;        // need background update
        
        this.bgimage = null;                // background image
        this._bgimage = false;        // need background image update

        this.op = 1;                // opacity
        this._op = false;        // opacity

        this.bo = null;                // border
        this._bo = false;        // border

        this.ow = null;                // overloaded width
        this.oh = null;                // overloaded height
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

//
// returns index in mj_G._a array
//
MJ_V.prototype._init = function($__pa, $__typ, $__lab, $__attr) {
        var $i = mj_G.AddDOM($__pa, $__typ, $__lab, $__attr);
        this.i = $i;
        var $id = mj_G._a[$i]['i'];

        this.w = $id.width;
        this.h = $id.height;
        this.x = $id.left;
        this.y = $id.top;


        mj_G._a[$i]['o'] = this;
        return $i;
}


// - - - - - - - VISIBILITY
// parms:
//        $__u - immediate CSS update
//        $__v - border in '1px dotted red'
MJ_V.prototype.Visi = function($__u, $__v) {

        this._v = true;
        this.v = $__v;

        if ($__u) this._Visi();
}

MJ_V.prototype._Visi = function() {
        var $id = mj_G._a[this.i]['i'];

        if (this._v) {
                if (this.v) {
                        $id.style['visibility'] = 'visible';
                }
                else {
                        $id.style['visibility'] = 'hidden';
                }
        }

}



// - - - - - - - ABSOLUTE POSITION?
// parms:
//        $__u - immediate CSS update
//        $__v - is absolute?
MJ_V.prototype.AbsPos = function($__u, $__v) {

        this._a = true;
        this.a = $__v;

        if ($__u) this._AbsPos();
}

MJ_V.prototype._AbsPos = function() {
        var $id = mj_G._a[this.i]['i'];

        if (this.a) {
                $id.style['position'] = 'absolute';
        }
        else {
                $id.style['position'] = 'relative';
        }

}


// - - - - - - - Move to position
// parms:
//        $__u - immediate CSS update
//        $__x - abs X coord        (or NULL if not need)
//        $__y - abs Y coord  (or NULL if not need)
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

                // alert(parseInt(this.x) + ' : ' +parseInt(this.y));

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
//        $__u - immediate CSS update
//        $__w - width
//        $__h - height
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

// - - - - - - - NEED BACKGROUND?
// parms:
//        $__u - immediate CSS update
//        $__v - background coloring
MJ_V.prototype.SetBG = function($__u, $__v) {

        this._bg = true;
        this.bg = $__v;

        if ($__u) this._SetBG();
}

MJ_V.prototype._SetBG = function() {
        var $id = mj_G._a[this.i]['i'];
        if (this.bg) $id.style['background'] = this.bg;
}

MJ_V.prototype.SetBGImage = function($__u, $__v) {
        this._bgimage = true;
        this.bgimage = $__v;
        if ($__u) this._SetBGImage();
}

MJ_V.prototype._SetBGImage = function() {
        var $id = mj_G._a[this.i]['i'];
        if (this.bgimage) $id.style['backgroundImage'] = 'url('+this.bgimage+')';
}

// - - - - - - - Opacity
// parms:
//        $__u - immediate CSS update
//        $__n - opacity in (0-1) interval
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
//        $__u - immediate CSS update
//        $__op - border in '1px dotted red'
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


MJ_V.prototype.AddHandler = function($_fname, $__ow, $__oh) {
        mj_G._a[this.i]['hl'] = $_fname;
        if ('undefined' !== typeof($__ow)) {
                this.ow = GTX($__ow);
        }
        if ('undefined' !== typeof($__oh)) {
                this.oh = GTY($__oh);
        }
}

MJ_V.prototype.AddKHMask = function($__mask) {
        if ('undefined' === typeof(mj_G._kh[$__mask])) {
                mj_G._kh[$__mask] = new Array();
        }
        mj_G._kh[$__mask][this.i] = 1;
        /*
        mj_G._a[this.i]['hl'] = $_fname;
        if ('undefined' !== typeof($__ow)) {
                this.ow = $__ow;
        }
        if ('undefined' !== typeof($__oh)) {
                this.oh = $__oh;
        }
        */
}


// - - - - - - - Update CSS from curr. params
MJ_V.prototype.Redraw = function() {
        this._AbsPos();
        this._MoveTo();
        this._SizeTo();
        this._SetBG();
        this._SetBGImage();
        this._Opacity();
        this._Border();
        this._Visi();
}
// MJ_VP.prototyp

// - - - - - - - Set da CSS attribute
MJ_V.prototype.Setp = function($__an, $__av) {
        mj_G._a[this.i]['i'].style[$__an] = $__av;
}


// - - - - - - - Set da CSS attribute
MJ_V.prototype.OverloadDime = function($__w, $__h) {
        this.ow = $__w;
        this.oh = $__h;
}



// ==========================================================================
// MJ_VI - class prototype for 'view image' element
// --- основан на MJ_V
// ==========================================================================
MJ_VI.prototype = new MJ_V();
// MJ_B.prototype.constructor = MJ_B;
MJ_VI.supaca = MJ_V.prototype;

function MJ_VI($__pa, $__lab, $__src) {
        if ( arguments.length > 0 ) {
                return this._init($__pa, $__lab, $__src);
        }
        return -1;
}


MJ_VI.prototype._init = function($__pa, $__lab, $__src) {
        var $i = MJ_VI.supaca._init.apply(this, [$__pa, 'IMG', $__lab, {'src':$__src}]);
        mj_G._a[$i]['o'] = this;
        return $i;
}


// ==========================================================================
// MJ_VD - class prototype for 'view div' element
// ==========================================================================
MJ_VD.prototype = new MJ_V();
// MJ_B.prototype.constructor = MJ_B;
MJ_VD.supaca = MJ_V.prototype;

function MJ_VD($__pa, $__lab) {
   if ( arguments.length > 0 ) {
        return this._init($__pa, $__lab);
   }
   return -1;
}

MJ_VD.prototype._init = function($__pa, $__lab) {
        var $i = MJ_VD.supaca._init.apply(this, [$__pa, 'DIV', $__lab]);
        mj_G._a[$i]['o'] = this;
        return $i;
}




// ==========================================================================
// MJ_RunTimer() - class for threading emulation
// ==========================================================================

function mj_RunTimer_Looper() {
        mj_RT.runs++;

        for ($j=0; $j<mj_RT.a.length; $j++) {
                if (mj_RT.a[$j]['f']) continue;        // if frozen

                var $a = mj_RT.a[$j];
                $a['t'] += mj_RT.to;

                if ($a['t'] >= $a['to']) {
                        if ( -1 !== $a['fn'].search(/\(/) ) eval($a['fn']);
                        else eval($a['fn'])();
                        $a['t'] = 0;
                        $a['c']++;
                        if  ( ($a['cm'] > 0) && ($a['c'] >= $a['cm']) ) {
                                $a['f'] = true;        // froze, if run-limit exceded
                        }
                }        
        } 
        if (0 < mj_RT.to) {
                setTimeout('mj_RunTimer_Looper()', mj_RT.to);
        }
}


function MJ_RunTimer($__to) {
        this.a = new Array();                // array with run-timers
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
// params: function_name(), timeout, count of run till freeze
// returns: id of a run-timer
MJ_RunTimer.prototype.Add = function($__f, $__to, $__c) {
        var $i = this.a.length;
        this.a[$i] = new Array();
        $a = this.a[$i];
        $a['fn'] = $__f;                // function_name()
        $a['to'] = $__to;        // timeout
        //$a['cm'] = (undefined === $__c) ? -1 : $__c; // repeats max count criteria (-1 - infinitive)
                                                
        $a['f'] = false;        // frozen ?
        // run-time
        //$a['c'] = 0;                // repeats count
        $a['t'] = 0;                // milliseconds of run-to-run interval counter ;)
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
mj_RT.Start(10);


// ==========================================================================

MjCRIO('[mj.js] include done.');


// ==========================================================================
// [EOF]
// ==========================================================================

