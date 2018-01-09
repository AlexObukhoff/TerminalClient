// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
* MeG JavaScript extension lib
* '2006-2007
*
* Cyberplat interface objects
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// объекты дл€ генерации интерфейсов автоматов  иберѕлат-а

// get true X coordinate
function GTX($__x) {
	if (0 >= $__x) return $__x;
	else return parseInt(mj_G.Wpx*$__x/1280);
}

// get true Y coordinate
function GTY($__y) {
	if (0 >= $__y) return $__y;
	else return parseInt(mj_G.Hpx*$__y/1024);
}



// ==========================================================================
// MJ_VP - class prototype for 'viewport' element - with image or div
// ==========================================================================
/*
соглашение о координатах:
-1  - центр
-2 - прижать к левому/верхнему краю
-3 - прижать к правому краю

//!!! пока не реализовано!!!! соглашение о размерах:
//!!! пока не реализовано!!!! -xx или -yy - раст€гивать до макс. размера
*/

/*
($__typ) если изображение, то в $__para = src; div - bgcolor
*/

// массив с MJ_VP
mj_g_avp = new Array();

/*
.l -  label (name/id of object)
.o -  inner object link
.a - array of objects
.t - type of viewport object
.p - viewport - хоз€ин (ссылка на объект)
.s - show true/false - активен и отобр

.is_active - is viewport active
.is_show - is viewport shown
.is_auto_active_passive
*/

function MJ_VP($__p, $__n, $__x, $__y, $__w, $__h, $__typ, $__para) {
	// position in VP array
	// this.ip =

	this.a = new Array();		// array of sub-elements

	this.p = $__p;				// хоз€ин
	this.is_active = false;
	this.is_show = false;
	this.is_auto_active_passive = false;
	// this.x =


	var $avp_l = mj_g_avp.length;
	mj_g_avp[$avp_l] = this;

	// if id/name is not spcifyed
	if (0 >= $__n) {
		this.l = 'vp_'+$avp_l;
	}
	else {
		this.l = $__n;
	}


	// пересчет координат
	$__x = GTX($__x);
	$__y = GTY($__y);

	// с шириной и высотой - непон€тно ???
	$__w = GTX($__w);
	$__h = GTY($__h);


	// var $s_s = $__n + '\n' + $__x +', '+ $__y +' : '+ $__w +', '+ $__h;
	// alert($s_s);



	// основной объект viewport-а: картинка, или DIV
	this.t = $__typ;
	if ('IMG' === $__typ) {
		this.o = new MJ_VI(-1, this.l, $mj_g_img_pth+$__para);
		this.o.Visi(0, 0);
		this.o.AbsPos(1, 1);
		// alert($mj_g_img_pth+$__para);
		this.o.w = $__w;
		this.o.h = $__h;
		this.o.SizeTo(1, $__w, $__h);
	}
	else {
		this.o = new MJ_VD(-1, this.l);
		this.o.Visi(0, 0);
		this.o.AbsPos(1, 1);
		if ('undefined' !== $__para) {
			this.o.SetBG(0, $__para);
		}
		// alert($__w + ', ' + $__h);
		this.o.SizeTo(0, $__w, $__h);
	}

	// отладка viewport-ов
	if ($mg_g_vp_debug) this.o.Border(0, '1px dotted red');


	// смотрим координаты и центрируем отностительно тек. viewport-а-хоз€ина
	if (-1 !== this.p) {
		if (-1 == $__x) {
			$__x = parseInt( (this.p.o.w-this.o.w)/2 );
		}
		else if (-2 == $__x) {
			$__x = 0;
		}
		else if (-3 == $__x) {
			$__x = this.p.o.w-this.o.w;

		}

		if (-1 == $__y) {
			$__y = parseInt( (this.p.o.h-this.o.h)/2 );
		}
		else if (-2 == $__y) {
			$__y = 0;
		}
		else if (-3 == $__y) {
			$__y = this.p.o.h-this.o.h;

		}


	}

	// moving to position, относительно пред. viewport-а
	if (-1 !== this.p) {
		$__x += this.p.o.x;
		$__y += this.p.o.y;
	}


	this.o.MoveTo(0, $__x, $__y);
	// return this; // ????

}


// добавить DIV-элемент к вьюпорту
MJ_VP.prototype.AddD = function($__x, $__y, $__hideonvp) {
	var $l = this.a.length;

	this.a[$l] = new MJ_VD(-1, this.l+'_sub_'+$l);
	if ('undefined' !== $__hideonvp) this.a[$l].hidden = $__hideonvp;

	this.a[$l].AbsPos(1, 1);
	this.a[$l].Visi(0, 0);

	this.a[$l].MoveTo(0, GTX($__x)+this.o.x, GTY($__y)+this.o.y);

	return this.a[$l];
}


// добавить изображение к вьюпорту
MJ_VP.prototype.AddI = function($__s, $__x, $__y, $__hideonvp) {
	var $l = this.a.length;

	this.a[$l] = new MJ_VI(-1, this.l+'_sub_'+$l, $mj_g_img_pth+$__s);
	if ('undefined' !== $__hideonvp) this.a[$l].hidden = $__hideonvp;

	this.a[$l].AbsPos(1, 1);
	this.a[$l].Visi(0, 0);
	this.a[$l].MoveTo(0, GTX($__x)+this.o.x, GTY($__y)+this.o.y);

	return this.a[$l];

}

// отобрахить


// завершить добавление элементов к вьюпорту и отобразить/сделать активным его?
MJ_VP.prototype.Finalize = function($__is_show, $__auto_active_passive) {

	this.is_auto_active_passive =  $__auto_active_passive;
	this.Show($__is_show);
	this.Redraw();
}


// завершить добавление элементов к вьюпорту и отобразить/сделать активным его?
MJ_VP.prototype.Show = function($__is_show, $_imm_redraw) {
	if (this.is_show == $__is_show) return;

	if (this.is_auto_active_passive) {
		this.Activate($__is_show);
	}

	for (var $i=0; $i<this.a.length; $i++) {
		if (this.a[$i].hidden) this.a[$i].Visi(0, 0);
		else this.a[$i].Visi(0, $__is_show);
	}
	this.o.Visi(0, $__is_show);
	this.is_show = $__is_show;

	if ($_imm_redraw) this.Redraw();
}


MJ_VP.prototype.Activate = function($__is_active) {
	if (this.is_active == $__is_active) return;

	for (var $i=0; $i<this.a.length; $i++) {
		// this.a[$i].Redraw();
		var $id = this.a[$i].i;
		// alert($id);
		mj_G._a[$id]['a'] = $__is_active;
	}
	this.is_active = $__is_active;
}


// перерисовать )
MJ_VP.prototype.Redraw = function() {
		this.o.Redraw();
		for (var $i=0; $i<this.a.length; $i++) this.a[$i].Redraw();
}



//  - - - - - - - переменные настройки
// $mj_g_img_pth = 'i/';


// $mg_g_vp_bg = '#cccccc';

// $mg_g_vp_debug = true;



// - - - - - - - - управление скинами
function SKP($__s) {
	if ('undefined' === typeof($ga_skp[$__s])) {
		alert('SKP element: ' + $__s + ' not found');
	}
	return $ga_skp[$__s];
}

// - - - - - - - - управление локал€ми
function LCP($__s) {
	if ('undefined' === typeof($ga_lcp[$__s])) {
		alert('LCP element: ' + $__s + ' not found');
	}
	return $ga_lcp[$__s];
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// dynamically interface elements generation
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=







