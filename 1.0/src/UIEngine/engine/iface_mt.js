var $g_is_mt_scenario = true;

var $g_htmlo_mt_debug_id = -1;
//activate_field(1);

// eleminate ????
var $g_vpa_main = null;
var $g_vpa_fields = null;

var $ga_mt_fields = Array();
var $ga_mt_fields_count = 0;

$g_button_rect = [400, 120, 500, 80];
$g_button_rect_recepient = [400, 320, 500, 80];

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
var MT_MSG_MSG = '333333';
var MT_MSG_STA = '003399';
var MT_MSG_ACT = '009900';
var MT_MSG_EVT = '666600';
var MT_MSG_ERR = 'ff0000';

function MT_MSG($__s, $__t) {
	if (!$is_debug) {
		return;
	}

	if ($g_htmlo_mt_debug_id === -1) {
		alert('MT_MSG: '+$__s);
	}
	else {
		$__o = mj_G._a[$a_srt[$g_htmlo_mt_debug_id]['m'].i]['i'].innerHTML;
		$__s = '<span style="background: #'+$__t+'">'+$__s+'</span><br />';
        mj_G._a[$a_srt[$g_htmlo_mt_debug_id]['m'].i]['i'].innerHTML += $__s;
        mj_G._a[$a_srt[$g_htmlo_mt_debug_id]['b'].i]['i'].innerHTML += $__s;
	}
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function main_MT() {
	add_term_info();
	mj_RT.Add('goto_Main()', 180000, 0);
	create_vpa('VIEW_OP_MARK1');
    if ($is_debug) {
       var $s = SKP('DEBUG_MINI_TEXT');
       $s.b = '#cccccc';
       $s.f = ['Verdana', '11', 'bold', '#ffffff', 'left'],
       $s.s = [0, '#000000']

       $g_htmlo_mt_debug_id = set_rect_text(cvp_G, [20, 800, 800, 10], $s, 'CYBERPLAT Money Transfers DEBUG<br /><br />');
       $a_srt[$g_htmlo_mt_debug_id]['m'].MoveTo(1, 300, 50);
       $a_srt[$g_htmlo_mt_debug_id]['b'].MoveTo(1, 300, 50);

       mj_G._a[$a_srt[$g_htmlo_mt_debug_id]['m'].i]['i']['style']['filter'] = 'alpha(opacity=75)';
       mj_G._a[$a_srt[$g_htmlo_mt_debug_id]['b'].i]['i']['style']['filter'] = 'alpha(opacity=65)';

       mj_G._a[$a_srt[$g_htmlo_mt_debug_id]['m'].i]['i']['style']['zIndex'] = '99';
       mj_G._a[$a_srt[$g_htmlo_mt_debug_id]['b'].i]['i']['style']['zIndex'] = '99';

       //MT_MSG('Money Transfers debug info inited: ' + $g_sss_debug_navi_info.replace("\n", '<br /><br />'), MT_MSG_MSG);
       MT_MSG('Skin used: ' +  SkinName, MT_MSG_MSG);
    }
    $g_vpa_main = create_vpa('VIEW_MAINMENU', 'show_mt_vpa', '-1', '-1');
};

function nextTerm(id) {
       Parameters['term_id']    = id;
       $s = getParametersAllAsUrl();
       $s = 'file://'+document.location.pathname +'?'+$s
       document.location.href=$s;
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// create all fields due to current operator
function create_account_fields1($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];
        //set_rect_text($v, [-300, 15, $s.r[2], 3], $s, LCP('title_requizites') );

        var opId = getParameter('recepient', 'undefined');

        var $op = jcfg_findOperatorById(opId)
        var opNme = $op['name'];
        var opImg = $op['image'];


        var $xofs = GTX(45);
        var $yofs = 100;

        var $sk = SKP('OPERATOR_ICON');
        $v['yofs'] = 55;

        // create and detect fields
        $fields_count = 0;
        for ($f_id in $ga_mt_fields) {
	        if ($ga_mt_fields[$f_id]['type'] == 't') {
	        	create_field($__v, $ga_mt_fields[$f_id]['desc'], $f_id, 't', null, '', $ga_mt_fields[$f_id]['val']);
	        }
	        else if ($ga_mt_fields[$f_id]['type'] == 'p') {
	        	create_field($__v, $ga_mt_fields[$f_id]['desc'], $f_id, 'p', {min: 4, max: 12}, '', $ga_mt_fields[$f_id]['val']);
	        }
	        else if ($ga_mt_fields[$f_id]['type'] == 'e') {
	        	create_field($__v, $ga_mt_fields[$f_id]['desc'], $f_id, 'e', {en: $ga_mt_fields[$f_id]['enm']}, '', $ga_mt_fields[$f_id]['enm'][$ga_mt_fields[$f_id]['val']]);
	        }
	        else if ($ga_mt_fields[$f_id]['type'] == 'm') {
	        	create_field($__v, $ga_mt_fields[$f_id]['desc'], $f_id, 'm', {mask: $ga_mt_fields[$f_id]['mask']}, '', $ga_mt_fields[$f_id]['val']);
	        }
	        else {
	        	create_field($__v, $ga_mt_fields[$f_id]['desc'], $f_id, 'i', {min: 3, max: 5}, '', $ga_mt_fields[$f_id]['val']);
	        }
	        $fields_count++;
        }
        $goto_check_overload = 'goto_CheckMT';
        $goto_back_overload = "_processEvt($ga_mt_scen[$gmt_curr_state]['c_lfb']['_evt'], 'fin fields')";

        var cnt=getParameter('cnt_enter', '');
				//$fields_count = 2
				var active = 0;
        if(''!=cnt)
					if(parseInt(cnt)!=0)
							active = 1;
        activate_field(active);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// create all fields due to current operator
function create_pay_fields($__v) {
    var $v = $g_vpa[$__v]['v'];
    var $s = $g_vpa[$__v]['s'];

    var $xofs = GTX(45);
    var $yofs = 100;

    $v['yofs'] = 55;

    var recepientExist=false;
    var buf=$g_button_rect_recepient[1];
    for (var i=0;i<5;i++){
	    var last_rec = $ga_mt['last_recipients'];
        var param;
        if(last_rec){
    	    param=$ga_mt['last_recipients']['recipient'+i];
        }
        if(param){
            recepientExist=true;
		    var text=param['last_name']+" "+param['first_name']+" "+param['middle_name'];
	        var buf = $__s1.f[3];
	        $__s1.f[3]='#ebffff';
	        $__s1.f[4]='center';
	        var $id = set_rect_text($v, $g_button_rect_recepient, $__s1, text);
	        $__s1.f[3]=buf;
        	    
    	    $a_srt[$id]['m'].SetBG(1, '#2e92cf');
		    $a_srt[$id]['m'].AddHandler('goto_Loc("mt.html?recepient='+Parameters['recepient']+'&recepient_mt='+i+'&state=info_page");');
    	    $g_button_rect_recepient[1] += 100;
        }
        else if(true==recepientExist){
	        var buf = $__s1.f[3];
	        $__s1.f[3]='#ebffff';
	        $__s1.f[4]='center';
	        var $id = set_rect_text($v, $g_button_rect_recepient, $__s1, '');
	        $__s1.f[3]=buf;
    	    $a_srt[$id]['m'].SetBG(1, '#2e92cf');
    	    $g_button_rect_recepient[1] += 100;
        }
    }
    $g_button_rect[1]=buf;
    if(false==recepientExist){
	    set_rect_text($v, [350, 400, 600, 500], $__s1, LCP("not_exist_recepient_mt"));
    }
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function goto_CheckMT() {
	_processEvt($ga_mt_scen[$gmt_curr_state]['c_rfb']['_evt'], 'fin fields')
}



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// money transfers global variables
var $gmt_curr_state = 'main';
var $gmt_needs_reload = false;
var $gmt_dest_page = 'mt';


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_mt_vpa($__v) {
		if ('undefined' === typeof($ga_mt_scen)) {
			goto_Main();
		}
    var $v = $g_vpa[$__v]['v'];
    var $__s = $g_vpa[$__v]['s'];

    create_bigbut(-2);
    create_bigbut(-3);

    $__s1 = SKP('VIEW_TEXT1');
    $__s1.f[3] = '#ff3333';
	 	$gmt_curr_state = getParameter('state', 'main');
   	var $c = 5;
    while (processState($__v)) {
	 		 	if ($gmt_needs_reload) doPage();
	   		$c--;
      	if ($c <= 0) {
						MT_MSG('SCENARIO: !!!ERROR: '+$gmt_curr_state+' more than 5 states changed in one page - seems like deadloop, aborted!', MT_MSG_ERR);
						break;
      	}
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function processState($__v) {

	// alert($__v);
	var $vp = $g_vpa[$__v]['v'];
	var $__s = $g_vpa[$__v]['s'];

	$__s1.b = false;
	$__s1 = SKP('VIEW_TEXT1');
    $__s1.f[3]='black';
    $__s1.f[4]='left';


	// - - - - - - - - -
	MT_MSG('SCENARIO: in state: '+$gmt_curr_state, MT_MSG_STA);

	if ('undefined' === typeof($ga_mt_scen[$gmt_curr_state])) {
		MT_MSG('invalid state in scenario: "'+$gmt_curr_state+"'", MT_MSG_ERR);
	}

	var $l_s = $ga_mt_scen[$gmt_curr_state];

	for ($v in $l_s) {
	    if('undefined' !== typeof($l_s[$v]['last_init'])){
	        if('true'==$l_s[$v]['last_init']){
	            continue;
	        }
	    }
		switch ($v.substring(0, 5)) {

			// - - - - action on state
			case '_act':
			case '_act_':
		        if (_processAct($l_s[$v])) {
			        return true;
		        }
			break;

			// - - - - 'check' construction
			case '_chk':
			    var $pname = $l_s[$v][0];
			    var $p_val = getParameter($pname, '0');
			    if (($p_val != 0) && ($p_val !== '')) {
				    if ($l_s[$v][1] !== '') {
					    if (_processAct($l_s[$v][1])) {
						    return true;
					    }
				    }
			    }
			    else {
				    if ($l_s[$v][2] !== '') {
					    if (_processAct($l_s[$v][2])) {
						    return true;
					    }
				    }
			    }
			break;
			// - - - -
			case 'c_txt':
			    var $r = [15, 210, 600, 70];
			    if ('undefined' !== typeof($l_s[$v]['r'])) {
				    $r = $l_s[$v]['r'];
			    }

			    var $id = set_rect_text($vp, $r, $__s1, '');
			    /*
			    var $o = $vp.AddI('button_up.gif', 100, 100);
			    $o.MoveTo(1, 1150, 300);
			    $o.AddHandler('scrollIdBy("'+$id+'", -100)');


			    var $o = $vp.AddI('button_down.gif', 100, 100);
			    $o.MoveTo(1, 1150, 780);
			    $o.AddHandler('scrollIdBy("'+$id+'", 100)');
                */
			    $a_srt[$id]['m'].Setp('overflow', 'hidden');
			    $a_srt[$id]['b'].Setp('overflow', 'hidden');

			    if ('undefined' !== typeof($l_s[$v]['_act'])) {
				    _processAct($l_s[$v]['_act'], $id);
			    }

			    break;
			// - - - -
			case '_txt_':
			    var $r = [15, 210, 600, 70];
                $__s1.f[3]='black';
                $__s1.f[4]='left';
			    if ('undefined' !== typeof($l_s[$v]['r'])) {
				    $r = $l_s[$v]['r'];
			    }
			    if ('undefined' !== typeof($l_s[$v]['align'])) {
				    $__s1.f[4] = $l_s[$v]['align'];
			    }

			    var $id = set_rect_text($vp, $r, $__s1, '');
			    $a_srt[$id]['m'].Setp('overflow', 'hidden');
			    $a_srt[$id]['b'].Setp('overflow', 'hidden');

			    if ('undefined' !== typeof($l_s[$v]['_act'])) {
				    _processAct($l_s[$v]['_act'], $id);
			    }

			    break;
			// - - - -
			case 'c_lfb':
		        $ga_buttbig[0]['v'].Show(1, 1);
		        //change_srt_text($ga_buttbig[0]['txi'], $l_s[$v]['desc']);
				changeButtonImage($ga_buttbig[0]['o'],'back');
		        $ga_buttbig[0]['o'].AddHandler('_processEvt("'+$l_s[$v]['_evt']+'","c_lfb")');
			break;

			// - - - -
			case 'c_rfb':
		        $ga_buttbig[1]['v'].Show(1, 1);
		        //change_srt_text($ga_buttbig[1]['txi'], $l_s[$v]['desc']);
				changeButtonImage($ga_buttbig[1]['o'],'next');
		        $ga_buttbig[1]['o'].AddHandler('_processEvt("'+$l_s[$v]['_evt']+'","c_rfb")');
			break;

			// - - - -
			case 'c_btn':
			    //
			    var buf = $__s1.f[3];
			    $__s1.f[3]='#ebffff';
			    $__s1.f[4]='center';
			    var $id = set_rect_text($vp, $g_button_rect, $__s1, $l_s[$v]['desc']);
			    $__s1.f[3]=buf;
			    $g_button_rect[1] += 120;
            	    
      	        $a_srt[$id]['m'].SetBG(1, '#2e92cf');
                $a_srt[$id]['m'].AddHandler('_processEvt("'+$l_s[$v]['_evt']+'","'+$v+'")');
			    break;
			case 'r_btn':
			    var $r = $g_button_rect;
	            if ('undefined' !== typeof($l_s[$v]['r'])) {
		            $r = $l_s[$v]['r'];
	            }
		        var buf = $__s1.f[3];
		        $__s1.f[3]='#ebffff';
		        $__s1.f[4]='center';
	            var $id = set_rect_text($vp, $r, $__s1, $l_s[$v]['desc']);
		        $__s1.f[3]=buf;
                $a_srt[$id]['m'].SetBG(1, '#2e92cf');
	            $a_srt[$id]['m'].AddHandler('_processEvt("'+$l_s[$v]['_evt']+'","'+$v+'")');
	            break;

			// - - - -
			case 'c_str':
			    // --- field creation
			    var $id = $l_s[$v]['id'];
			    $ga_mt_fields[$id] = Array();
			    $ga_mt_fields[$id]['id'] = $id;

			    // --- field description/comment
			    if ('undefined' !== typeof($l_s[$v]['type'])) {
				    $ga_mt_fields[$id]['type'] = $l_s[$v]['type'];
			    }
			    else {
				    $ga_mt_fields[$id]['type'] = ['i'];
			    }

			    // --- field description/comment
			    if ('undefined' !== typeof($l_s[$v]['desc'])) {
				    $ga_mt_fields[$id]['desc'] = $l_s[$v]['desc'];
			    }
			    else {
				    $ga_mt_fields[$id]['desc'] = $l_s[$v]['id'];
			    }

			    // --- field value
			    if ('undefined' !== typeof($l_s[$v]['_def'])) {
				    $ga_mt_fields[$id]['val'] = $l_s[$v]['_def'];
			    }
			    else {
				    $ga_mt_fields[$id]['val'] = '';
			    }
			    // --- enum value
			    if ('undefined' !== typeof($l_s[$v]['enm'])) {
				    $ga_mt_fields[$id]['enm'] = $l_s[$v]['enm'];
			    }
			    // --- mask
			    if ('undefined' !== typeof($l_s[$v]['mask'])) {
				    $ga_mt_fields[$id]['mask'] = $l_s[$v]['mask'];
			    }

			    var $rval = getParameter($l_s[$v]['id'], -1);
			    if ($rval != -1) {
				    $ga_mt_fields[$id]['val'] = $rval;
			    }
                	    if($id){
		            	if("old_passwd_md5"==$id){
		                	var old_psw=getParameter('old_password', '');
			                if(""!=old_psw)
			                    $ga_mt_fields[$id]['val']=old_psw;
			        }
		        	if("login"==$id){
		                	var cnt=getParameter('cnt_enter', '');
			                if(''!=cnt){
			                    var login=getParameter('login', '');
			                    if(parseInt(cnt)!=0){
		        	                $ga_mt_fields[$id]['val']=login;
			                    }
			                    else{
			                        $ga_mt_fields[$id]['val']='';
		        	            }
		                	}
		            	}
			    }
			    $ga_mt_fields_count++;
			break;

			case 'c_inf':
			    //create_pay_fields();
			    $g_vpa_fields = create_vpa('VIEW_ACCOUNT_NONE', 'create_pay_fields', '-1', '-1');
			    break;
			// - - - -
			case 'stub':
			break;

			// - - - -
			default:
				MT_MSG('SCENARIO: !!!ERROR: invalid command "'+$v+'" in state '+$gmt_curr_state, MT_MSG_ERR);
			break;
		} // switch
	} // for


	// MjCRIODA($ga_mt_fields);
	if ($ga_mt_fields_count > 0) {
		$g_vpa_fields = create_vpa('VIEW_ACCOUNT_DETAILS', 'create_account_fields1', '-1', '-1');
	}
	
	for ($v in $l_s) {
	    if('undefined' !== typeof($l_s[$v]['last_init'])){
	        if('true'!=$l_s[$v]['last_init']){
	            continue;
	        }
	    }
	    else{
	        continue;
	    }
		switch ($v.substring(0, 5)) {
			case '_txt_':
			    var $r = [15, 210, 600, 70];
			    if ('undefined' !== typeof($l_s[$v]['r'])) {
				    $r = $l_s[$v]['r'];
			    }

                //$__s1.f[5]=100;
			    var $id = set_rect_text($vp, $r, $__s1, '');

			    $a_srt[$id]['m'].Setp('overflow', 'hidden');
			    $a_srt[$id]['b'].Setp('overflow', 'hidden');

			    if ('undefined' !== typeof($l_s[$v]['_act'])) {
				    _processAct($l_s[$v]['_act'], $id);
			    }

			    break;
			case 'c_lfb':
		        $ga_buttbig[0]['v'].Show(1, 1);
		        change_srt_text($ga_buttbig[0]['txi'], $l_s[$v]['desc']);
		        $ga_buttbig[0]['o'].AddHandler('_processEvt("'+$l_s[$v]['_evt']+'","c_lfb")');
			    break;
			case 'c_rfb':
		        $ga_buttbig[1]['v'].Show(1, 1);
		        change_srt_text($ga_buttbig[1]['txi'], $l_s[$v]['desc']);
		        $ga_buttbig[1]['o'].AddHandler('_processEvt("'+$l_s[$v]['_evt']+'","c_rfb")');
			    break;
		}
	}
	MT_MSG('Action processing done.', MT_MSG_ACT);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function scrollIdBy($id, $__ofs) {
			mj_G._a[$a_srt[$id]['b'].i]['i'].scrollTop += $__ofs;
			mj_G._a[$a_srt[$id]['m'].i]['i'].scrollTop += $__ofs;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// returns "true" if state was changed
//
function _processAct($__s, $__p1, $__p2) {
	MT_MSG('action: '+$__s, MT_MSG_ACT);
	var $a = $__s.split('|');

	switch ($a[0]) {
		case 'toState':
			$gmt_curr_state = $a[1];
			if (('undefined' !== typeof($a[2])) && ($a[2] == 'reload')) {
				$gmt_needs_reload = true;
			}
			for (var $i=2; $i<10; $i++) {
				if ('undefined' !== typeof($a[$i])) {
					$a1 = $a[$i].split(':');
					if ($a1[0] === '_set') {
						Parameters[$a1[1]] = $a1[2];
					}
				}
			}
			return true;
		    break;
        case 'toCheck':
            //document.location.href="checking.html";
            //document.location.href="checking.html?recepient="+getParameter("recepient","")+"&recepient_mt="+getParameter("recepient_mt","");
            return true;
            break;
		case 'toPage':
			$gmt_needs_reload = true;
			$gmt_dest_page = $a[1];
			return true;
		    break;
		case 'leavParm':
		    // alert('here!');
		    var $Parms = Array();
		    var $LeaveIds = {'recepient':1, 'state':1,'login':1,'cnt_enter':1};
		    for (var $ii=1; $ii<$a.length; $ii++) {
			    if ('undefined' !== typeof($a[$ii])) {
				    $LeaveIds[$a[$ii]] = 1;
			    }
		    }

		    for (var $v in $LeaveIds) {
			    if ('undefined' !== typeof(Parameters[$v])) {
				    $Parms[$v] = Parameters[$v];
			    }
		    }
		    Parameters = $Parms;
		    break;
		case 'fillParm':
			var $s = '[undefined text]';
			switch ($a[1]) {
				case 'terms':
				    $term_id = getParameter("term_id","0");
				    change_srt_text($__p1, $gs_mt_terms[$term_id]); 
				    break;
			    case "error_text":
				    var $s = LCP("can_not_connect_to_server_mt");
				    var error=getParameter("error", "");
				    if(""!=error){
					    var existError=$ga_mt_errors_list["error"+error];
					    if(existError) $s=existError["user_message"];
				    }
				    change_srt_text($__p1, $s);
				    mj_RT.Add('goto_Main', 10000, 0);
				    break;
			    case 'check_use_last':
				    var $use_last = parseInt(getParameter('use_last', 0));
				    var $c = 0;
				    for (var $v in $ga_mt['last_recipients']) {
					    $c++;
					    if ($c === $use_last) {
						    var $lp = $ga_mt['last_recipients'][$v];
						    Parameters['last_name'] = $lp['last_name'];
						    Parameters['first_name'] = $lp['first_name'];
						    Parameters['middle_name'] = $lp['middle_name'];
						    Parameters['mt_system'] = $lp['mt_system'];
						    Parameters['bank_address'] = $lp['bank_address'];
						    Parameters['r'] = $lp['id'];
						    break;
					    }
				    }
			        break;
			    case 'use_last_id':
				    alert('here!');
				    var $use_last = parseInt(getParameter('use_last', 0));
				    var $c = 0;
				    for (var $v in $ga_mt['last_recipients']) {
					    $c++;
					    if ($c === $use_last) {
						    var $lp = $ga_mt['last_recipients'][$v];
						    Parameters['last_name'] = $lp['last_name'];
						    Parameters['first_name'] = $lp['first_name'];
						    Parameters['middle_name'] = $lp['middle_name'];
						    Parameters['mt_system'] = $lp['mt_system'];
						    Parameters['bank_address'] = $lp['bank_address'];
						    Parameters['r'] = $lp['id'];
						    break;
					    }
				    }
			        break;
			    case 'hello':
			    	    var text = LCP("hello_user_mt") + login_info['first_name'] + " " + login_info['middle_name'] + LCP("rule_mt");
				    change_srt_text($__p1, text);
				    break;
			    case 'pwd_change_good':
				    change_srt_text($__p1, LCP("password_change_success_mt"));
				    break;
				case 'pwd_confirm_bad':
				    change_srt_text($__p1, LCP("password_change_bad_mt"));
				    break;
			    case 'comment_to_passw':
				    change_srt_text($__p1, LCP("comment_to_password_mt"));
				    break;
			    case 'comment_for_place':
			    	    if("undefined" != typeof($ga_jcfg['config']['BusinessDealerAddress'])){
				    	change_srt_text($__p1, LCP("registration_place_mt")+$ga_jcfg['config']['BusinessDealerAddress']);
				    }
				    else{
				    	change_srt_text($__p1, "");
				    }
				    break;

			    case 'calculate':
			        var text;
        		        text  = LCP("accept_mt") + getParameter('amount_all', LCP("not_calculate_mt")) + LCP("sum_transfer_mt");
        		        text += getParameter('amount', LCP("not_calculate_mt")) + LCP("commission_mt") + getParameter('system_commission', LCP("not_calculate_mt"));
        		        text += LCP("rent_mt") + getParameter('rent_commission', LCP("not_calculate_mt"));
			        change_srt_text($__p1, text);
			        return true;
				    break;
			    case 'info_text':
			        var recMT=parseInt(getParameter("recepient_mt",""));
	                	var param = $ga_mt['last_recipients']['recipient'+recMT];
			        var text="<span style='font-size:24px'>"+LCP("info_transfer_system_mt")+"</span><span style='font-size:36px'>"+param['mt_system']+"</span><br>";
			        text+="<span style='font-size:24px'>"+LCP("info_sender_mt")+"</span><span style='font-size:36px'>"+login_info['last_name']+" "+login_info['first_name']+" "+login_info['middle_name']+"</span><br>";
			        text+="<span style='font-size:24px'>"+LCP("info_reciver_mt")+"</span><span style='font-size:36px'>"+param['last_name']+" "+param['first_name']+" "+param['middle_name']+"</span><br>";
			        text+="<span style='font-size:24px'>"+LCP("info_destination_mt")+"</span><span style='font-size:36px'>"+param['bank_address']+"</span>";
			        
			        change_srt_text($__p1, text);
			        break;
			    case 'info_text_in_create':
			        change_srt_text($__p1, LCP("choose_reciver_mt"));
			        break;
			    default:
				    alert('invalid fillParm argument: '+$a[1]);
			        break;
			} // switch [fill criteria]
		    break; // break for fillParm action
		default:
			MT_MSG('SCENARIO: !!!ERROR: invalid action "'+$a[0]+'" in state '+$gmt_curr_state, MT_MSG_ERR);
		    break;
	}

	return false;

	// MjCRIODA($a);

	// $gmt_needs_reload
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//
function _processEvt($__s, $__e) {
	MT_MSG('event: '+$__s, MT_MSG_EVT);

	if (!_processAct($__s)) {
			MT_MSG('SCENARIO: !!!ERROR: event "'+$__e+" must reload the page"+$__s+'" in state '+$gmt_curr_state, MT_MSG_ERR);
	}

	doPage();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function doPage() {
	var currentState = (typeof(Parameters['state']) == 'undefined') ? "" : Parameters['state'];
	if ($gmt_dest_page === 'main') {
		Parameters = new Array();
		Parameters['cleanup'] = 1;
	}
	else {
		Parameters['state'] = $gmt_curr_state;
	}

	// --- ---
	for (var $i in $ga_fields) {
		Parameters[$ga_fields[$i]['id']] = $ga_fields[$i]['val'];
	}

	// MjCRIODA(Parameters);

	var $dest_url;
	if (currentState == 'calculate' && $gmt_curr_state == 'info_page'){ 
		$dest_url = $gmt_dest_page+".html?recepient=" + Parameters['recepient'] + "&recepient_mt=" + Parameters['recepient_mt'] + "&state=" + $gmt_curr_state;
	}
	else{
		$dest_url = $gmt_dest_page+".html?"+goto_get_allurlparams();
	}
	MT_MSG('dest page url: ' + $dest_url, MT_MSG_MSG);
	// alert($dest_url);
	goto_Loc($dest_url);
}



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function MjImagesReindex($__pref_from, $__pref_to) {
	// get da link on images coillection
	var $l_c = document.images;
    // MjCRIODA(document.images);

    var $s = '';
    for ($n in $l_c) {
    	if ('length' === $n) continue;

    	var $s_name_old = $l_c[$n].src;
    	var $s_name_new = $s_name_old.replace($__pref_from, $__pref_to);

	   	$s += $n + ": " + $s_name_old + ', ' + $s_name_new + "\n";
    }

    alert($s);

    for ($n in $l_c) {
    	if ('length' === $n) continue;

    	var $s_name_old = $l_c[$n].src;
    	var $s_name_new = $s_name_old.replace($__pref_from, $__pref_to);

    	$l_c[$n].src = $s_name_new;
    }

    alert('done.');
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// [EOF]


