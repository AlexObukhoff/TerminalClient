/**
* |+|----------------------------------------------------------------------|
* | [CYBERPLAT interface runtime filescript - ga_jcfg service functions]
* |
* | |----------------------------------------------------------------------|
* | | interface javascript configuration service functions
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


function jcfgGetParentOp(tree, id) {
        var $rr = -1;
        for (var $k in tree) {
						var $r = 'op'+id;
            if ($r == $k) {
    		     		$rr = tree['id'];
                break;
            }
            if (_jcfg_mnuIsGroupId($k) !== -1) {
                $rr = jcfgGetParentOp(tree[$k], id);
                if ($rr !== -1) break;
            }
        }
        return $rr;
}
function jcfgGetParentGr(tree, id) {
        var $rr = -1;
        for (var $k in tree) {
        		//alert($k);
            if (id == $k) {
    		     		$rr = tree['id'];
                break;
            }
            if (_jcfg_mnuIsGroupId($k) !== -1) {
            		
                $rr = jcfgGetParentGr(tree[$k], id);
                if ($rr !== -1) break;
            }
        }
        return $rr;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// returns menu group id, or -1 if not a menu item id string!
function _jcfg_mnuIsGroupId($s) {
        if (parseInt($s) > 0) {
                return $s;
        }
        else {
                return -1;
        }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// returns menu operator id, or -1 if not a menu operator id string!
function _jcfg_mnuIsMenuOperatorId($s) {
        // alert('here!');
        // $s = toString($s);
        if ($s.substring(0, 2) === 'op') {
                return (parseInt($s.substring(2)));
        }
        else {
                return -1;
        }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// returns menu item id, or -1 if not
function _jcfg_mnuIsMenuItemId($s) {
        var $r = _jcfg_mnuIsGroupId($s);
        if ($r !== -1) {
                return $r;
        }
        $r = _jcfg_mnuIsMenuOperatorId($s);
        if ($r !== -1) {
                return $r;
        }
        return -1;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// recusive find submenu
function _jcfg_traceSubMenuFor($__l, $__id, $__lev) {
        // return value
        var $rr = -1;

        // recursion level protection
        if ($__lev > 7) {
                return $rr;
        }

        // try to find
        for (var $v in $__l) {
                var $r = _jcfg_mnuIsGroupId($v);
                if ($r == $__id) {
                        $rr = $__l[$v];
                        break;
                }
                // if submenu
                if ($r !== -1) {
                        $rr = _jcfg_traceSubMenuFor($__l[$v], $__id, $__lev+1)
                        if ($rr !== -1) {
                                break;
                        }
                }
        }
        return $rr;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// find menu object by id
// returns: menu object link
function jcfg_findMenuById($__id) {
        //alert('menu id: ' + $__id);
        // var $a = new Array();        // array of leaves
        // alert($__id);
        var $l = $ga_jcfg['menu'][0];
        // var $l = 0;                // current level

        var $o = _jcfg_traceSubMenuFor($l, $__id, 0);
        return $o;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// add enumerations 'id' to menu elements
// returns menu elements count
function jcfg_enumerateMenuItems($__l) {
        var $c = 0;
        for (var $v in $__l) {
        		if (_jcfg_mnuIsMenuItemId($v) !== -1) $c++;
        }
        return $c;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// returns operator object link, or -1 if not exist
function jcfg_findOperatorById($__id) {
        if ('undefined' === typeof($ga_jcfg['operators'][$__id])) return -1;
        else return $ga_jcfg['operators'][$__id];
}


function jcfg_getCommission($__id) {
		
		var $sr = '';
        
    //Если 999 - commission_variable
    if ($__id == 999) return LCP('commission_variable');
        
    //Если нет такого оператора - то пусто        
    var $c_l = jcfg_findOperatorById($__id);
    if ($c_l == -1) return $sr;
                
    var $a = new Array();                // array with
    var $c = 0;                                 // count of commission entries

    // get array true links and count entries
    for (var $ci in $c_l['comission']) {
    		$a[$c] = $c_l['comission'][$ci];
        $c++;
		}
		//Если про коммисию нет упоминания - говорим, что ее нет
		if (!$c) return LCP('commission_none');
                        
    
    var $sc = 0;
    if ($c > 1) {
    		var $is_srted = false;
        for (var $ou=0; $ou<$c; $ou++) {
         		$is_srted = false;
             		for (var $oi=0; $oi<($c-1); $oi++) {
		                 if ( parseInt($a[$oi+1]['min_day']) < parseInt($a[$oi]['min_day']) ) {
    		                $is_srted = true;
                        var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                     } else if ( parseInt($a[$oi+1]['min_time']) < parseInt($a[$oi]['min_time']) ) {
												$is_srted = true;
                        var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                     } else
                     		if ( parseInt($a[$oi+1]['min']) < parseInt($a[$oi]['min']) ) {
	                      		$is_srted = true;
  													var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                        }
                        $sc++;
                     }
										 if (!$is_srted) break;
                }
        }

		// - - - - find commission by criteria
		var $dte = new Date();
		var $d_dow = ($dte.getDay()+1);
    var $d_min = ($dte.getHours()*60+$dte.getMinutes());
    var $lf = $a[0];
    for (var $i=1; $i<$c; $i++) {
				if ($d_dow < $a[$i]['min_day']) break;
        if ($d_min < $a[$i]['min_time']) break;
        $lf = $a[$i];
    }
    var $sa = new Array();
    // - - - - show minimal amount
    var $mina = 0;
    var $hop = false;
    for (var $i=0; $i<$c; $i++) {
    		if ( ($a[$i]['min_day'] == $lf['min_day']) &&  ($a[$i]['min_time'] == $lf['min_time']) ) {
			      if ($hop) {
      		      $mina = $a[$i]['min'];
                 break;
            }
            if ($a[$i]['min'] == 0) $hop = true;
        }
		}
		localFont = SKP('VIEW_ACCOUNT_DETAILS_FCOMMENT');
	  // - - - - mark commissions by stages
    for (var $i=0; $i<$c; $i++) {
    		$sa[$i] = new Array();
    		if ( ($a[$i]['min_day'] == $lf['min_day']) &&  ($a[$i]['min_time'] == $lf['min_time']) ) {
			  		var $valu = $a[$i]['value'];
            if ($valu.substring($valu.length-1) != '%') $valu = $valu +  ('</span>&nbsp;' + LCP('currency')[CurrencyType]+'');
						else $valu = $valu.replace(/\%/g, '</span>&nbsp;%');
            	if ($i == 0) {
            		$sa[$i]['val'] = $valu;
            		$sa[$i]['txt'] = '';
            		$sa[$i]['sum'] = 0;
            	}
            	else {
            		$sa[$i]['val'] = $valu;
            		$sa[$i]['txt'] = $a[$i]['min'] +' <span style="font-size:smaller">'+LCP('currency')[CurrencyType]+'</span>';
            		$sa[$i]['sum'] = $a[$i]['min'];
              }
				} else $a[$i]['show'] = false;
    }
	  if ($lf['value'].substring(0, 1) === '0' && $c == 0) {
	  	$sr = LCP('commission_none');
	  	return $sr;
	  } else return $sa;
    
}



// returns commission text string for operator with $__id
function jcfg_getCommissionText($__id,$__plus_br) {

            var $br = '';
            var $comission_color_prefix = '';
            var $comission_color_postfix = '';

        if('undefined' !== typeof($__plus_br)){
                $br = '<br>';
                $comission_color_prefix = '<span style="color: #ff3333;">';
                $comission_color_postfix = '</span>';

        }

        var $sr = '';
        if ($__id == 999) {
                return LCP('commission_variable');
        }


        var $c_l = jcfg_findOperatorById($__id);
        if ($c_l == -1) {
                return $sr;
        }

        //  - - - now, generate commission string
        var $a = new Array();                // array with
        var $c = 0;                                 // count of commission entries

        // get array true links and count entries
        for (var $ci in $c_l['comission']) {
                $a[$c] = $c_l['comission'][$ci];
                $c++;
        }

        if (!$c) {
                        // $sr = LCP('commission_none');
                        return LCP('commission_none');
        }

/*
        var $sc = 0;
        if ($c > 1) {
                var $is_srted = false;
                for (var $ou=0; $ou<$c; $ou++) {
                        $is_srted = false;
                        for (var $oi=0; $oi<($c-1); $oi++) {

                                if ($a[$oi+1]['min_day'] < $a[$oi]['min_day']) {
                                        $is_srted = true;
                                        var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                                }
                                else if ($a[$oi+1]['min_time'] < $a[$oi]['min_time']) {
                                        $is_srted = true;
                                        var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                                }
                                else if ($a[$oi+1]['min'] < $a[$oi]['min']) {
                                        $is_srted = true;
                                        var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                                }

                                $sc++;


                        }

                        if (!$is_srted) {
                                break;
                        }

                }

        }
        */

        var $sc = 0;
        if ($c > 1) {
                var $is_srted = false;
                for (var $ou=0; $ou<$c; $ou++) {
                        $is_srted = false;
                        for (var $oi=0; $oi<($c-1); $oi++) {

                                if ( parseInt($a[$oi+1]['min_day']) < parseInt($a[$oi]['min_day']) ) {
                                        $is_srted = true;
                                        var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                                }
                                else if ( parseInt($a[$oi+1]['min_time']) < parseInt($a[$oi]['min_time']) ) {
                                        $is_srted = true;
                                        var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                                }
                                else
                                        if ( parseInt($a[$oi+1]['min']) < parseInt($a[$oi]['min']) ) {
                                        $is_srted = true;
                                        var $tmp; $tmp = $a[$oi+1]; $a[$oi+1] = $a[$oi]; $a[$oi] = $tmp;
                                }

                                $sc++;


                        }

                        if (!$is_srted) {
                                break;
                        }

                }

        }



        /*
        alert($sc);
        for (var $ii in $a) {
                MjCRIODA($a[$ii]);
        }
        */


        // - - - - find commission by criteria
        // alert('sort count: ' +  $sc);
        var $dte = new Date();
        var $d_dow = ($dte.getDay()+1);
        var $d_min = ($dte.getHours()*60+$dte.getMinutes());


        var $lf = $a[0];
        for (var $i=1; $i<$c; $i++) {
                if ($d_dow < $a[$i]['min_day']) {
                        break;
                }
                if ($d_min < $a[$i]['min_time']) {
                        break;
                }
        /*
                if ($__sum < $a[$i]['min']) {
                        break;
                }
                */


                $lf = $a[$i];
        }


        var $sa = '';

        // - - - - show minimal amount
        var $mina = 0;
        var $hop = false;
        for (var $i=0; $i<$c; $i++) {
                if ( ($a[$i]['min_day'] == $lf['min_day']) &&  ($a[$i]['min_time'] == $lf['min_time']) ) {
                        ///$a[$i]['show'] = true;
                        if ($hop) {
                                $mina = $a[$i]['min'];
                                break;
                        }
                        if ($a[$i]['min'] == 0)  {
                                $hop = true;
                        }
                }
        }


        // alert($mina);

        // - - - - mark commissions by stages
        for (var $i=0; $i<$c; $i++) {
                if ( ($a[$i]['min_day'] == $lf['min_day']) &&  ($a[$i]['min_time'] == $lf['min_time']) ) {
                        var $valu = $a[$i]['value'];
                        if ($valu.substring($valu.length-1) != '%') {
                                $valu = $comission_color_prefix + $valu +  (' ' + LCP('currency')[CurrencyType]) + $comission_color_postfix;
                        }else{
                                    $valu = $comission_color_prefix + $valu + $comission_color_postfix;
                        }
                        //2 or more blocks of comissin
                        if ($c > 1){
                                if ($i == 0){
                                         $sa += $br + 'до ' + ($a[($i+1)]['min']-1) +' '+LCP('currency')[CurrencyType] + ' - ' + $valu;
                                }
                                else if ($i == $c-1) {
                                         $sa += ''+$br+' от ' + $a[$i]['min'] +' '+LCP('currency')[CurrencyType] + ' - ' + $valu;
                                }
                                else{
                                         $sa += ''+$br+' от ' + $a[$i]['min'];// +' ' + LCP('currency')[CurrencyType];
                                         $sa += ' до ' + ($a[($i+1)]['min']-1) +' '+LCP('currency')[CurrencyType] + ' - ' + $valu;
                                }
                        }
                        //simple comission (1 block)
                        else{
                                $sa += $valu;
                        }

/*
                        if ($a[$i]['min'] == 0)  {
                                $sa += 'до ' + $mina+' '+LCP('currency')[CurrencyType] + ' - ' + $valu;
                        }
                        else {
                                $sa += ', от ' + $a[$i]['min']+' '+LCP('currency')[CurrencyType] + ' (включительно) - ' + $valu;
                        }
*/

                }
                else {
                        $a[$i]['show'] = false;
                }

        }


        if ($lf['value'].substring(0, 1) === '0' && $c == 0) {
                $sr = LCP('commission_none');
        }
        else {
                $sr += LCP('commission_prefix') + $br + $sa + LCP('commission_postfix');

        }

        // alert($sr);

        return $sr;

}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function detOpIco($__s) {
        if ($__s.length < 2) {
                return 'no_image.gif';
        }
        else {
                return $__s;
        }
}




// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// stub for getcardsinfo pins
function GetCardsInfo() {
        alert('GetCardsInfo - routed to jcfg_getCardsInfo()');
        jcfg_getCardsInfo();
        return;


        // protection, if no pins received yet
        if ('undefined' === typeof($ga_jcfg['details'])) {
                $ga_jcfg['details'] = new Array();
                alert('ga_jcfg updated!.');
        }

        // now, scan da massive of cards
        if ('undefined' === typeof($ga_jcfg['details']['pin_info'])) {
                alert('ga_jcfg pininfo updated!.');
                $ga_jcfg['details']['pin_info']  = new Array();
        }

        /*
        $ga_jcfg['details']['parsed'] = new Array();
        var $l_ps = $ga_jcfg['details']['parsed'];
        */

        var $l_up = $ga_jcfg['details']['pin_info'];
        var $ops_length = 0;

        for (var $v in $l_up) {
                // $l_ps[$v] = new Array();

                // alert($v +' is '+ $l_up[$v]);
                if ($l_up[$v]['data'] != '') {
                        var $a_cards = $l_up[$v]['data'].split('==');
                        $l_up[$v]['count'] = $a_cards.length;
                        $l_up[$v]['cards'] = new Array();

                        for (var $vv in $a_cards) {
                                var $a_curcard = $a_cards[$vv].split('=');
                                var $card_name = $a_curcard[0];
                                var $card_id = $a_curcard[1];
                                var $card_value = $a_curcard[2];

                        }
                }
                else {
                        $l_up[$v]['count'] = 0;
                        $l_up[$v]['cards'] = new Array();
                }


                alert(  $l_up[$v]['count'] );

                /*
                if () {
                        MjCRIODA($l_up[$v]);
                }
                */



                // alert($cardsa.length);

                // $l_ps[$v][]


                $ops_length++;
        }

        alert('parsed ops: ' + $ops_length);




        // 'pin_info'

        alert('GetCardsInfo done.');


}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// parse ga_jcfg['details'] array from jcfg_details.js
//
// ga_jcfg['details']['pin_info'] - > (1) by operators ID and
// follow that structure:
// (1) -> [id][cards] -> (2)
//
// (2) ->
var $g_jcfg_getCardsInfo = false;

function jcfg_getCardsInfo() {
        // if always created
        if ($g_jcfg_getCardsInfo) {
                alert('cards recreation');
                return;
        }

        // protection, if no pins received yet
        if ('undefined' === typeof($ga_jcfg['details'])) {
                $ga_jcfg['details'] = new Array();
                // alert('ga_jcfg updated!.');
        }

        // now, scan da massive of cards
        if ('undefined' === typeof($ga_jcfg['details']['pin_info'])) {
                // alert('ga_jcfg pininfo updated!.');
                $ga_jcfg['details']['pin_info']  = new Array();
        }

        // parse da pins
        var $l_up = $ga_jcfg['details']['pin_info'];
        var $ops_length = 0;

        // follow each operator
        for (var $v in $l_up) {
                // get all cards
                var $a_cards = $l_up[$v]['data'].split(':');
                var $a_imags = $l_up[$v]['imgs'].split(':');

                //
                var $n_img_offset = 0;
                $l_up[$v]['cards'] = new Array();
                for (var $crd in $a_cards) {
                        if ($crd == '') continue;
                        var $a_flds = $a_cards[$crd].split('=');
                        if ('undefined' == typeof($a_flds[1])) continue;

                        // create new card
                        $l_up[$v]['cards'][ $a_flds[1] ] = new Array();
                        var $l_ccard = $l_up[$v]['cards'][ $a_flds[1] ];
                        $l_ccard['name'] = $a_flds[0];
                        $l_ccard['amount'] = $a_flds[2];
                        $l_ccard['comment'] = $a_flds[3];
                        $l_ccard['opname'] = $a_flds[4];
                        $l_ccard['image'] =  $a_imags [ $n_img_offset ];

                        // MjCRIODA($l_ccard);
                        $n_img_offset++;
                }
                // alert('op: '+$v+', cards: '+ MjGetAL( $l_up[$v]['cards'] ) );

                $ops_length++;
        }

        // alert('parsed operators: '+ $ops_length);
        $g_jcfg_getCardsInfo = true;
}



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// $__l_fe - link to da field entry
//
function parse_fld_name($__l_fe) {
        var $s_ret = $__l_fe['vali'];
        switch ($__l_fe['type'].toLowerCase()) {
                // - - - - - if masked field
                case 'masked':
                var $s_mask = $__l_fe['mask'];
                var $s_number = $__l_fe['vali'];
                var res = maskFieldValidation($s_number, $s_mask);
                $s_ret = res['view'];
                break;

                // - - - - - if enum field
                case 'enum':
                $s_ret = $__l_fe['enum'][ $__l_fe['vali'] ];
                break;
                
                case 'pwd':
                var tmp = '';
                for (var i=0; i< $__l_fe['vali'].length; i++) tmp+='•'
                $s_ret = tmp;
                break;
                
                case 'textpwd':
                var tmp = '';
                for (var i=0; i< $__l_fe['vali'].length; i++) tmp+='•'
                $s_ret = tmp;
                break;
        }
        return $s_ret;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function jcfg_findMetroItem($__id) {
        for (var $ii in $ga_metro_menu['Menu']) {

                if ($ga_metro_menu['Menu'][$ii][0] == $__id) {
                        return $ga_metro_menu['Menu'][$ii];

                }
        }

        return -1;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// [ file include done. ]
$iface_jcfg_js = true;



