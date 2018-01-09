/**
* |+|----------------------------------------------------------------------|
* | [CYBERPLAT interface runtime filescript]
* |
* | version 2.0.0080.000 (20070921 13:52)
* | |----------------------------------------------------------------------|
* | | interface runtime processing script
* | |
* | | ! does runtime interface generation and processing
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
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function add_term_info() {
        create_vpa('VIEW_TERMINFO_CURRENT_TIME', 'create_current_time', '-1', '-1');
        mj_RT.Add('timer_refresh', 756, 0);
        create_vpa('VIEW_TERMINFO_SUPPPHONE', 'create_supphone', '-1', '-1');
        create_vpa('VIEW_TERMINFO_TERMNUMBER', 'create_termnunmber', '-1', '-1');

        create_vpa('SOUND_CONTROL', 'create_sound_a', '-1', '-1');
        if ('undefined' !== typeof($ga_skp['VIEW_LOCALE_SELECT'])) {
                create_vpa('VIEW_LOCALE_SELECT', 'create_locale_select', '-1', '-1');
        }
}

function add_backtimer() {
        create_vpa('VIEW_BACKTIMER', 'create_BACKTIMER', '-1', '-1');
        mj_RT.Add('backtimer_refresh', 756, 0);
}

function addTimeInfo() {
	timeInfoExist = true;
	setTimeout("addTimeInfoBack()", 100);
}

function addTimeInfoBack() {
	if (!is_vpa_exist('VIEW_TERMINFO_CURRENT_TIME'))
		create_vpa('VIEW_TERMINFO_CURRENT_TIME', 'create_current_time', '-1', '-1');
	timer_refresh();
	addTimeInfo();
}

//$is_debug = true;


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
var $gs_marquee_text = MainMenuMarqueeString; // 'Итыть, бегю.строка.тут.вот!';
var $gs_comission_text = '';

var $gs_localbase_included = false;        //        инициализирована ли уже база номерных емкостей
var $go_operator_icon = null;                //  граф. объект-иконка оператора

var $g_forced_mobile_selection = false;                // принудительный выбор мобильного опреатора при невозможности опеределения онного по базе номерных емкостей

var $gs_global_check_handler = 'fields_preCheck()';

var $g_vpaincreation = '';

var $g_is_metro_recepient = false;

var $fields_count = 0;
var $pb_entry = 0;  //флажок 1-го захода на pb.html?pb=dataentry
var is_no_keyboard = true;	// флажок отображения клавиатуры при заполнении филда
var $after_entry = false;	//флажок отрисовки филдов при загрузке
var PPButtonShowDelay = 300;	//время "отжатия" кнопки пинпада, в [мс]
var KPButtonShowDelay = 33;		//время "промигивания" кнопки клавиатуры, в [мс]
var pinpadActive = false;
var timeInfoExist = false;	//запустили обновление  поля времени

function mc_delay(timeout) {
	var tbefore = new Date;
	var mc_before = tbefore.getTime();
	delete tbefore;
	
	do {
		var tafter = new Date;
		var mc_after = tafter.getTime();
		delete tafter;
	} while ((mc_after - tbefore) < parseInt(timeout));
}

function create_ssaver($__v) {
        var $i=5;
        $i = ++$i + ++$i;
        alert('alive: '+$i);

        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];

        var $o = $v.AddD(400, 400);
        $o.MoveTo(1, 0, 0);
        $o.SizeTo(1, 400, 400);

        $o.Border(1, '7px solid #ff0000');
        $o.SetBG(1, '#990000');

        $o.AddHandler("alert('wow!')");

        alert('done.');

}

var menuTree = new Array();

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function main_loop()
{
        //var kk = _jcfg_traceSubMenuFor($ga_jcfg['menu'][0], '1200', '0');
        //kk = 101;
        //kk = jcfgGetParentGr($ga_jcfg['menu'][0], kk)
//   			alert(kk);
   			   			
   			//var kk=jcfgGetParentGr($ga_jcfg['menu'][0], '1200');
   			//alert(kk);
        
                
        mj_RT.Add('document.focus', 20000, 0);
		if ('undefined' === typeof($intro)) $intro = false;
        
        // - - - - - - debug info
        var $sss = 'locale test: ' + LCP('achtung')+'\n\nMAIN loop alive with:\n';
        $sss += 'fupath: ' + $mj_g_pathwc + ',\nlocale: ' + $mj_g_locale + ',\ndocnme: ' + $mj_g_docnme+'\n\n';
        $sss += 'realpath: '+document.location.href;
        CybIf_Logger(CYBIF_LOG_NFO, $sss);
        // CybIf_Logger(1, $sss);
        // alert($sss);
        if ($is_debug === true) {
                var $sk = SKP('DEBUG_TOP_BIG');
                set_rect_text(cvp_G, [0, 0, 800, 30], $sk, $sss);
        }



        // - - - - - - detecting metro recepient
        var $opObj = jcfg_findOperatorById( getParameter('recepient', -1) );
        if (($opObj !== -1) && ($opObj['processor']['type'] === 'Cyberplat_Metro')) {
                $g_is_metro_recepient = true;
        }
        // alert('passed!');

        // - - - - - - main action switch

			if ($mj_g_docnme == 'main.html') {
				if ($intro) {
					create_vpa('INTRO', 'createIntro', '-1', '-1')
				} else {
		        	add_term_info();
		            create_vpa('VIEW_OP_MARK1');
					
		            $g_current_vpa = create_vpa('VIEW_MAINMENU', 'create_menu_icons', '-1', '-1');
		            var $opId = getParameter('recepient', 'undefined');
		            if ($opId != 'undefined') {
		            	var groupId = jcfgGetParentOp($ga_jcfg['menu'][0], $opId);
		            	if (groupId !=-1) {
		            		var localGroupId = groupId;
		            		while ( (parentGroup = jcfgGetParentGr($ga_jcfg['menu'][0], localGroupId)) != -1) {
		            			if (parentGroup != -1 && parentGroup != 'undefined') {
		            				menuTree[localGroupId] = parentGroup;
		            				localGroupId = parentGroup;
		            			}
		            		}
		            		
		            		route_to_vp('VIEW_OPERATORS', 'create_submenu_icons', groupId, 'VIEW_OPERATORS_101');			
		            	}
		            }
		            PlaySoundA('choose_operators');
				}
			} else if ($mj_g_docnme == 'main_selection.html') {
			  clearTimeout(timerMainPage);
			  timerMainPage = setTimeout("goto_Main()",60000);
			  if ($intro) {
	              add_term_info();
              	  create_vpa('VIEW_OP_MARK1');
                  //create_vpa('VIEW_MARQUEE', 'create_marquee', '-1', '-1');
                  $g_current_vpa = create_vpa('VIEW_MAINMENU', 'create_menu_icons', '-1', '-1');
		            var $opId = getParameter('recepient', 'undefined');
		            if ($opId != 'undefined') {
		            	var groupId = jcfgGetParentOp($ga_jcfg['menu'][0], $opId);
		            	if (groupId !=-1) {
		            		var localGroupId = groupId;
		            		while ( (parentGroup = jcfgGetParentGr($ga_jcfg['menu'][0], localGroupId)) != -1) {
		            			if (parentGroup != -1 && parentGroup != 'undefined') {
		            				menuTree[localGroupId] = parentGroup;
		            				localGroupId = parentGroup;
		            			}
		            		}
		            		
		            		route_to_vp('VIEW_OPERATORS', 'create_submenu_icons', groupId, 'VIEW_OPERATORS_101');			
		            	}
		            }
                  PlaySoundA('choose_operators');
			  }
			} else {
        			switch ($mj_g_docnme.toLowerCase()) {
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                // interface recipient requisites input page
	                case 'data-entry.html':
	                add_term_info();
	
	                $gs_marquee_text = jcfg_getCommissionText(getParameter('recepient', -1));
	//                create_vpa('VIEW_MARQUEE', 'create_marquee', '-1', '-1');
	
	
	                create_vpa('VIEW_OP_MARK2');
	                if (!$g_is_metro_recepient) {
	                        $g_current_vpa = create_vpa('VIEW_ACCOUNT_DETAILS', 'create_account_fields', '-1', '-1');
	                }
	                else {
	                        if (getParameter('cmd', 'unknown') == 'showmenu') {
	                                $g_current_vpa = create_vpa('VIEW_METRO_MENU', 'create_metro_menu', '-1', '-1');
	                        }
	                        else {
	                                $g_current_vpa = create_vpa('VIEW_METRO_MENU', 'create_metro_message', '-1', '-1');
	                        }
	                }
	                // create_vpa('VIEW_ACCOUNT_INLET', 'create_pinpad', '-1', '-1');
	
									
									var flag = -1;
									for (i=0; i<$ga_fields.length; i++) {
										if ($ga_fields[i]['pass']!=1) {
				    					flag = i;
				     					break
			  	 					}
			  	 				}

									if (flag != -1) PlaySoundA('enter_number');
	                
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                // interface recipient requisites online checking page
	                case 'checking.html':
	                add_term_info();
	
	                create_vpa('VIEW_OP_MARK2');
	                $gs_marquee_text = jcfg_getCommissionText(getParameter('recepient', -1));
	//                create_vpa('VIEW_MARQUEE', 'create_marquee', '-1', '-1');
	                create_vpa('VIEW_MESSAGE_CHECKING', 'show_message_checking', '-1', '-1');
	                create_vpa('VIEW_MESSAGE_CHECKING_FLASH');
	
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                // interface payment, cash-in page
	                case 'payment.html':
	                add_term_info();
	                //var $s_timeout_func = 'goto_Loc("Payment.html?end=1&'+goto_get_allurlparams()+'");';
	                //mj_RT.Add($s_timeout_func, 75000, 0);
	                create_vpa('VIEW_OP_MARK2');
	
	                $gs_marquee_text = jcfg_getCommissionText(getParameter('recepient', -1));
	                $gs_comission_text = jcfg_getCommissionText(getParameter('recepient', -1),'get with BR');
	                create_vpa('VIEW_MESSAGE_COMISSION', 'show_comission_in_payment', '-1', '-1');
	                create_vpa('VIEW_MESSAGE_PAYMENT', 'show_message_payment', '-1', '-1');
	
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                // interface payment complete info page
	                case 'paymentcomplete.html':
	                add_term_info();

	                var $opId = getParameter('recepient', 'undefined');
	                var $op = jcfg_findOperatorById($opId);	
	                if ('cyberplat_mt' != $op['processor']['type'].toLowerCase()){
	                	create_vpa('ADV_FLASH4');
	                }
	                create_vpa('-VIEW_OP_MARK2');
	                //create_vpa('ADV_FLASH2');
	                //create_vpa('ADV_FLASH3');
	
	                $g_current_vpa = create_vpa('VIEW_MESSAGE', 'show_message_thanks', '-1', '-1');
	                
	                //create_vpa(new_vpa);
	                
	
	
	                // $ga_buttbig[1]['v'].Show(1, 1);
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                // ошибка проверки номера
	                // !!! only after 'checking.html'
	                // соотв, только при online-режиме
	                case 'full-screen-message.html':
	                add_term_info();
	                create_vpa('VIEW_OP_MARK2');
	                create_vpa('VIEW_MESSAGE_ERROR', 'show_message_error', '-1', '-1');
	
	                mj_RT.Add('goto_Main', 7000, 0);
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                // service menu
	                // !!!! only after 'data-entry.html'
	                case 'service.html':
	                add_term_info();
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                // shows terminal error
	                // !!! поговорить с Колей - насчет выхода в сервисное меню
	                case 'terminal_error.html':
	                add_term_info();
	                create_vpa('VIEW_MESSAGE', 'show_message_termerror', '-1', '-1');
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                // ??? communal payments
	
	                case 'complat_message.html':
	                add_term_info();
	                create_vpa('VIEW_MESSAGE', 'show_message_complat', '-1', '-1');
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                case 'message.html':
	                add_term_info();
	                create_vpa('VIEW_MESSAGE_MESSAGE', 'show_message_message', '-1', '-1');
	                break;
	
	                //Вспомогательная страница для Ответа сервера в онлайне
	                case 'addinfo.html':
	                add_term_info();
	                create_vpa('VIEW_MESSAGE', 'show_message_addinfo', '-1', '-1');
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                case 'pb.html':
	                if ((getParameter('pb', '-1') == 'error') && (getParameter('errornumber', '-1') == 'showaddinfo')) {
	                    add_term_info();
	                    create_vpa('VIEW_MESSAGE', 'show_message_addinfo', '-1', '-1');
	                }
                    break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                case 'message_no_get_change.html':
	                add_term_info();
	                create_vpa('VIEW_PRINTER_NO_GET_CHANGE', 'show_message_nochange', '-1', '-1');
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                case 'printererror.html':
	                add_term_info();
	                create_vpa('VIEW_PRINTER_PRINTER_ERROR', 'show_message_prnerror', '-1', '-1');
	                break;
	
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                case 'cyberrules.html':
	                add_term_info();
	                create_vpa('VIEW_CYBERRULES', 'show_cyberrles', '-1', '-1');
	                break;
	                
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                case 'mt.html':
	                main_MT();
	                break;
	
	                //Вспомогательная страница для Налогов
	                case 'addinfo_taxes.html':
	                add_term_info();
	                create_vpa('VIEW_MESSAGE', 'show_addinfo', '-1', '-1');
	                break;
	                
	                // - - - - - - - - - - - - - - - - - - - - - - - -
	                default:
	                // add_term_info();
	                goto_Loc('terminal_error.html?iferr=interface_no_page');
	                break;
	    				 } // switch(filename) - main interface switch
      		} 
} // main_loop()


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// get graphics primitive object, based on MJ_V from global array
function MJGA($__index) {
        return mj_G._a[$__index]['i'];
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_sound_a($__v) {
        // alert($__v);

        var $v = $g_vpa['SOUND_CONTROL']['v'];
        // MJGA($v.o.i).innerHTML = 'test version!!!<br /><br />not for commercial use';
        // MjCRIODA($v);
        // wahhh!
}
$gIntroFlashVPA = '';
function createIntro($__v) {
		var $__s = $g_vpa[$__v]['s'];
    var $v = $g_vpa[$__v]['v'];
		
		$g_current_vpa = $__v;
		$introSKP = SKP('INTRO');
		
		$gIntroFlashVPA = create_vpa('INTRO_FLASH');
		
		$o  = $v.AddI($introSKP['pay'][0], $introSKP['pay'][1], $introSKP['pay'][2]);
		$o.SizeTo(1, $introSKP['pay'][3], $introSKP['pay'][4]);
		$o.AddHandler('goto_Loc("main_selection.html")');
		
		$o  = $v.AddI($introSKP['help'][0], $introSKP['help'][1], $introSKP['help'][2]);
		$o.SizeTo(1, $introSKP['help'][3], $introSKP['help'][4]);
		$o.AddHandler('route_to_vp("VIEW_HELP", "create_help", "-1", "'+$__v+'")');
	
		$o  = $v.AddI($introSKP['info'][0], $introSKP['info'][1], $introSKP['info'][2]);
		$o.SizeTo(1, $introSKP['info'][3], $introSKP['info'][4]);
		$o.AddHandler('route_to_vp("VIEW_INFO", "create_info", "-1", "'+$__v+'")');
}
function play_sound($__sound_id) {

        // play_sound(choose_operators)
        var $op_snd = LCP('sound_files')[$__sound_id];

        var $v = $g_vpa['SOUND_CONTROL']['v'];
        // MJGA($v.o.i).innerHTML = '\<BGSOUND SRC="sound/'+$op_snd+'" LOOP="1"\>';
        document.write('\<BGSOUND SRC="sound/'+$op_snd+'" LOOP="1"\>');

}


// view points name accociated array
/*
 format:
        'name' : {
                v: viewport object
                s: skin object in skin array
                p: parent's name, placed in array
                i: id, passed to create_vpa

                bl: button's left array
                br: button's right array
        },
        etc...

*/
var $g_vpa = new Array();
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// create VPA (view port in array) if not exists
/*
params:
$__skp - skin property in skp.js
$__func_subels - function, that generates VP subelements
$para - misc parameter for???? _ $__func_subels
*/

var $g_current_vpa = '';


function create_vpa($__skp, $__func_subels, $__id, $__parent)
{
                var $no_func_buttons_change = false;
                if ('-' === $__skp.substring(0, 1)) {
                        $no_func_buttons_change = true;
                        $__skp = $__skp.substring(1);
                }

        var $name = $__skp;
        if (arguments.length > 1) {
                $name += (($__id != -1) ? ('_'+$__id) : '');
        }

        $g_vpaincreation = $name;

        // if vpa was created - show+activate it and exit!
        if ('undefined' !== typeof($g_vpa[$name])) {
            if (!$g_enum_redraw) {
                $g_vpa[$name]['v'].Show(1, 1);
                return $name;
            }
        }

        $g_vpa[$name] = new Array();
        var $v = $g_vpa[$name];
        $v['s'] = SKP($__skp);

        // if image defined, creating image-based viewport
        if (false !== $v['s'].i) {
                $v['v'] = new MJ_VP(cvp_G, -1, $v['s'].r[0], $v['s'].r[1], $v['s'].r[2], $v['s'].r[3], 'IMG', $v['s'].i);
        }
        else {
                if ($v['s'].b !== false) {
                        $v['v'] = new MJ_VP(cvp_G, -1, $v['s'].r[0], $v['s'].r[1], $v['s'].r[2], $v['s'].r[3], 'DIV', $v['s'].b);
                }
                else {
                        $v['v'] = new MJ_VP(cvp_G, -1, $v['s'].r[0], $v['s'].r[1], $v['s'].r[2], $v['s'].r[3], 'DIV');
                }

                if ( ('undefined' !== typeof($v['s'].t)) && ('undefined' !== typeof($v['s'].t.allow_flash)) ) {
                // if ('undefined' !== typeof($v['s']['t'].allow_flash)) {
                        if ('undefined' !== typeof($v['s'].f)) {
                                MJGA($v['v'].o.i).innerHTML = '<embed src="../../skins/'+SkinName+'/i/'+$v['s'].f+'" width="'+GTX($v['s'].r[2])+'" height="'+GTY($v['s'].r[3])+'" autostart="true" quality="best" wmode="transparent">';
                        }
                } // if

        }
				/*
				for (k in buttons) {
					for (i in buttons[k]) {
						if ('undefined' === typeof($ga_buttbig[k])) {
								create_bigbut_new(k, i);
						}
					}
				}
				*/

        // - - - - - - create down buttons, if not exist
        if ('undefined' === typeof($ga_buttbig[0])) {
                create_bigbut(-2);
        }
        if ('undefined' === typeof($ga_buttbig[1])) {
                create_bigbut(-3);
        }

        if (!$no_func_buttons_change) {
                $ga_buttbig[0]['v'].Show(0, 1);
                $ga_buttbig[1]['v'].Show(0, 1);

                $v['bl'] = new Array();
                $v['bl']['s'] = 0;
                $v['bl']['n'] = '';
                $v['bl']['h'] = '';        // no handler

                $v['br'] = new Array();
                $v['br']['s'] = 0;
                $v['br']['n'] = '';
                $v['br']['h'] = '';        // no handler
        }

        // - - - - - - running subelement creation func
        if ( arguments.length > 1 ) {
                $v['p'] = $__parent;
                $v['i'] = $__id;
                eval($__func_subels+'($name);');
        } else {
                $v['p'] = '';
                $v['i'] = '';
        }

        //  - - - - - - debug info
        if ($is_debug) {
                set_rect_text($v['v'], [0, 0, 10, 10], SKP('DEBUG_MINI_TEXT'), $name);

        }
        // finalize viewport creation
        $v['v'].Finalize(1, 1);


        // alert('WOW!');

        return $name;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
var $a_srt = new Array();
/*
'm' - main text view element
'b' - background text view element
*/

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
$__v - link to a vpa object
$__r - rect array [x,y,w,h] 4 view text in this vpa
$__s - skp array with font parameters
$__text - text oto view
*/
function set_rect_text($__v, $__r, $__s, $__text, $__am) {
        var $abs_middle = false;
        if (arguments.length > 4) {
						if ('undefined' !== typeof($__am)) {
								$abs_middle = true;
            }
        }

        var $ia = $a_srt.length;
        $a_srt[$ia] = new Array();

        // add shadow text
        var $o = $__v.AddD($__r[0]+$__s.s[0], $__r[1]+$__s.s[0]);
        $o.SizeTo(1, GTX($__r[2]), $__r[3]);
        $a_srt[$ia]['b'] = $o;


        if ($abs_middle)  {
                var $style = "style='";
                $style += "font-family: "+$__s.f[0]+";";
                $style += "font-size: "+$__s.f[1]+'px'+";";
                $style += "font-weight: "+$__s.f[2]+";";
                $style += "color: "+$__s.s[1]+";";
                // $style += "background-color: #ff7777;";
                $style += "' ";

                mj_G._a[$o.i]['i'].innerHTML = '<table width="100%" height="100%"><tr><td '+$style+'valign="middle" align="center"></td></tr></table>';
        }
        else {
                //mj_G._a[$o.i]['i'].innerHTML = $__text;

                mj_G._a[$o.i]['i'].style["fontFamily"] = $__s.f[0];
                mj_G._a[$o.i]['i'].style["fontSize"] = $__s.f[1]+'px';
                mj_G._a[$o.i]['i'].style["fontWeight"] = $__s.f[2];

                mj_G._a[$o.i]['i'].style["color"] = $__s.s[1];
                mj_G._a[$o.i]['i'].style["textAlign"] = $__s.f[4];
								if ('undefined'!=typeof($__s.f[5])) mj_G._a[$o.i]['i'].style["lineHeight"] = $__s.f[5];
								if ('undefined'!=typeof($__s.f[6])) mj_G._a[$o.i]['i'].style["padding"] = $__s.f[6]+'px';
                if ('undefined' !== typeof($__s.b)) {
                        $o.SetBG(1, $__s.b);
                        // set_rect_text
                }
        }

        // add top text
        var $o = $__v.AddD($__r[0], $__r[1]);
//        $o.SizeTo(1, $__r[2], $__r[3]);
        $o.SizeTo(1, GTX($__r[2]), $__r[3]);
        $a_srt[$ia]['m'] = $o;


        if ($abs_middle)  {
                var $style = "style='";
                $style += "font-family: "+$__s.f[0]+";";
                $style += "font-size: "+$__s.f[1]+'px'+";";
                $style += "font-weight: "+$__s.f[2]+";";
                $style += "color: "+$__s.f[3]+";";
                $style += "' ";

                mj_G._a[$o.i]['i'].innerHTML = '<table width="100%" height="100%"><tr><td '+$style+'valign="middle" align="center">'+$__text+'</td></tr></table>';
        }
        else {
                mj_G._a[$o.i]['i'].innerHTML = $__text;

                mj_G._a[$o.i]['i'].style["fontFamily"] = $__s.f[0];
                mj_G._a[$o.i]['i'].style["fontSize"] = $__s.f[1]+'px';
                mj_G._a[$o.i]['i'].style["fontWeight"] = $__s.f[2];
                mj_G._a[$o.i]['i'].style["color"] = $__s.f[3];
                mj_G._a[$o.i]['i'].style["textAlign"] = $__s.f[4];
 								if ('undefined'!=typeof($__s.f[5])) mj_G._a[$o.i]['i'].style["lineHeight"] = $__s.f[5];
 								if ('undefined'!=typeof($__s.f[6])) mj_G._a[$o.i]['i'].style["padding"] = $__s.f[6]+'px';
        }
        // alert($__v);
        if ($is_debug) {
                $o.Border(1, '1px dotted maroon');
        }
        return $ia;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
$__v - link to a vpa object
$__r - rect array [x,y,w,h] 4 view text in this vpa
$__s - skp array with font parameters
$__text - text oto view
*/
function set_rect_text_in_textArea($__v, $__r, $__s,caption, $__text, $__am) {
        var $abs_middle = false;
        if (arguments.length > 4) {
                if ('undefined' !== typeof($__am)) {
                        $abs_middle = true;
                }
        }

        var $ia = $a_srt.length;
        $a_srt[$ia] = new Array();

        // add shadow text
        var $o = $__v.AddD($__r[0]+$__s.s[0], $__r[1]+$__s.s[0]);
        $o.SizeTo(1, GTX($__r[2]), $__r[3]);
        $a_srt[$ia]['b'] = $o;

        if ($abs_middle)  {
                var $style = "style='";
                $style += "font-family: "+$__s.f[0]+";";
                $style += "font-size: "+$__s.f[1]+'px'+";";
                $style += "font-weight: "+$__s.f[2]+";";
                $style += "color: "+$__s.s[1]+";";
                $style += "' ";

                mj_G._a[$o.i]['i'].innerHTML = '<table width="100%" height="100%"><tr><td '+caption+'valign="middle" align="center">'+$__text+'</td></tr></table>';
        }
        else {
                mj_G._a[$o.i]['i'].innerHTML = caption;

                mj_G._a[$o.i]['i'].style["fontFamily"] = $__s.f[0];
                mj_G._a[$o.i]['i'].style["fontSize"] = $__s.f[1]+'px';
                mj_G._a[$o.i]['i'].style["fontWeight"] = $__s.f[2];
                mj_G._a[$o.i]['i'].style["color"] = $__s.s[1];
                mj_G._a[$o.i]['i'].style["textAlign"] = $__s.f[4];

                if ('undefined' !== typeof($__s.b)) {
                        $o.SetBG(1, $__s.b);
                }
        }
        //alert(document.body.innerHTML);

        var $o = $__v.AddD($__r[0]+2, $__r[1]+40);
        $o.SizeTo(1, GTX($__r[2])-4, $__r[3]);
        $a_srt[$ia]['m'] = $o;

        if ($abs_middle)  {
                var $style = "style='";
                $style += "font-family: "+$__s.f[0]+";";
                $style += "font-size: "+$__s.f[1]+'px'+";";
                $style += "font-weight: "+$__s.f[2]+";";
                $style += "color: "+$__s.f[3]+";";
                $style += "' ";

                mj_G._a[$o.i]['i'].innerHTML = '<table width="100%" height="100%"><tr><td '+$style+'valign="middle"><textarea style="width=100%;height=100%">'+$__text+'</textarea></td></tr></table>';
        }
        else {
                mj_G._a[$o.i]['i'].innerHTML = '<textarea id="tArea" rows="15" style="width=88%; height=250px; overflow: hidden;">'+$__text+'</textarea>';
                var sArea="'tArea'";
                mj_G._a[$o.i]['i'].innerHTML+='<img id="image1" src="'+$mj_g_img_pth+'button_up.gif" onClick="up('+sArea+')" style="position:absolute; left: 605px; top: 20px"></img>';
                mj_G._a[$o.i]['i'].innerHTML+='<img id="image2" src="'+$mj_g_img_pth+'button_down.gif" onClick="down('+sArea+')" style="position:absolute; left: 605px; top: 140px"></img>';
                mj_G._a[$o.i]['i'].style["fontFamily"] = $__s.f[0];
                mj_G._a[$o.i]['i'].style["fontSize"] = $__s.f[1]+'px';
                mj_G._a[$o.i]['i'].style["fontWeight"] = $__s.f[2];
                mj_G._a[$o.i]['i'].style["color"] = $__s.f[3];
        }
        if ($is_debug) {
                $o.Border(1, '1px dotted maroon');
        }

        return $ia;
}

position=0;
incrRow=1;
data=new Array();

function initArea(name,$__text){
    var tArea=document.getElementById(name);
    if(tArea){
        for(var i=0;i<$__text.length;i++){
            data[i]=$__text[i];
        }
        //incrRow=Math.floor($__text.length/tArea.rows);
        //incrRow--;
        incrRow = tArea.rows;
        position = 0;
        viewArea(name);
        /*
        if(incrRow < 1)
            incrRow=Math.floor(tArea.rows/2);
        else
            incrRow=incrRow+tArea.rows;
            */
    }
}

function up(name){
	var range=document.getElementById(name).createTextRange();
	position-=incrRow;
	viewArea(name);
}

function down(name){
	var tArea=document.getElementById(name);
	position+=incrRow;
	viewArea(name);
}

function viewArea(name){
	var tArea=document.getElementById(name);
	tArea.value="";
	if(position < 0){
		position=0;
	}
	else if((position+tArea.rows) >= data.length){
		position=data.length-tArea.rows;
	}
	if(position < 0){
		position=0;
	}
	for(var i=position;i<(position+tArea.rows);i++){
	
		if(i >= data.length)
			break;
		tArea.value+=data[i];
		if(i<(position+tArea.rows))
		    tArea.value+="\n";
	}
}
//_Igor
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function change_srt_text($__id, $__text, skp) {
	mj_G._a[$a_srt[$__id]['m'].i]['i'].innerHTML = $__text;
  if ('undefined' != typeof(skp)) {
			mj_G._a[$a_srt[$__id]['m'].i]['i'].style["fontFamily"] = skp[0];
      mj_G._a[$a_srt[$__id]['m'].i]['i'].style["fontSize"] = skp[1]+'px';
      mj_G._a[$a_srt[$__id]['m'].i]['i'].style["fontWeight"] = skp[2];
      mj_G._a[$a_srt[$__id]['m'].i]['i'].style["color"] = skp[3];
      mj_G._a[$a_srt[$__id]['m'].i]['i'].style["textAlign"] = skp[4];
      if ('undefined' != typeof(skp['5'])) {
      	mj_G._a[$a_srt[$__id]['m'].i]['i'].style["paddingTop"] = skp[5];
      }
  }
}
function changeButtonImage(idButton, newImage) {
        var $iii = MJGA(idButton.i);
        $iii.src = $mj_g_img_pth+buttons[newImage];
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
var $g_ia_text_timer = -1;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

function create_current_time($__v) {
        var $__s = $g_vpa[$__v]['s'];
        var $v = $g_vpa[$__v]['v'];
        $g_ia_text_timer = set_rect_text($v, [0, 0, $__s.r[2], $__s.r[3]] , $__s, GetTimeA());
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function timer_refresh() {
        if ($g_ia_text_timer >= 0) {
                change_srt_text($g_ia_text_timer, GetTimeA());
        }

}




// -------------------------------------------------------
// -------------------------------------------------------
var $g_ia_text_backtimer = -1;
function create_BACKTIMER($__v) {

        var $__s = $g_vpa[$__v]['s'];
        var $v = $g_vpa[$__v]['v'];
        $g_ia_text_backtimer = set_rect_text($v, [0, 0, $__s.r[2], $__s.r[3]] , $__s, GetBackTimer());

        //         alert($g_ia_text_timer);
}

var $StartDateInHtml = new Date();

function GetBackTimer(){
        var $TimeDiff = new Date() - $StartDateInHtml;
/*
        var timeout = ga_jcfg['config']['timeout_payment'];
        if ('undefined' === typeof(timeout)){
             timeout = 120;
        }else{
             timeout = (timeout > 120) ? 120 : timeout;
        }
        alert("timeout="+timeout);
*/
        var ret = parseInt((120000-$TimeDiff)/1000);
        var s = ret%60;
        s = (s>9) ? s : '0'+s;
        var mi = (ret-s)/60;
        ret = (ret > 0 ) ? (mi+':'+s)  : '00:00';
        return ret;
}

function backtimer_refresh() {

        if ($g_ia_text_backtimer >= 0) {
                change_srt_text($g_ia_text_backtimer, GetBackTimer());
        }

}
// -------------------------------------------------------
// -------------------------------------------------------




// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_supphone($__v) {
        var $__s = $g_vpa[$__v]['s'];
        var $v = $g_vpa[$__v]['v'];
		SupportString = SupportString.replace(/\[/g, '<');
		SupportString = SupportString.replace(/\]/g, '>');
        set_rect_text($v, [0, 0, $__s.r[2], $__s.r[3]] , $__s, SupportString);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_termnunmber($__v) {
        var $__s = $g_vpa[$__v]['s'];
        var $v = $g_vpa[$__v]['v'];
        set_rect_text($v, [0, 0, $__s.r[2], $__s.r[3]] , $__s, LCP('terminal_no') + TerminalName);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_menu_germany($__v) {

        var $__s = $g_vpa[$__v]['s'];
        var $v = $g_vpa[$__v]['v'];
        set_rect_text($v, [5, 15, $__s.r[2], 70], $__s, LCP('choose_operator'));

        var $s = '';

        var $sk = SKP('OPERATOR_ICON_GER');
        /*
        var $w = $sp.bd[0] + 40;
        var $h = $sp.bd[1] + 80;
        */

        // getting max colums put into container
        var $yofs = 85;

        var $maxcols = 2; //Math.floor( $__s.r[2] / $sk.bd[0] );
        var $maxrows = 2; // Math.floor( ($__s.r[3] - $yofs) / $sk.bd[1] );


        var $xofs = Math.floor( ($__s.r[2] - $sk.bd[0]*$maxcols)/2 );
        // alert($maxcols);

        // var ndMenu=ndRoot.getElementsByTagName("menu").item(0);

        var $opofsx = parseInt(($sk.bd[0] - $sk.d[0])/2);
        var $opofsy = parseInt(($sk.bd[1] - $sk.d[1])/2);

        var $col = 0;
        var $row = 0;

        ndRoot=xmldoc.getElementsByTagName("root").item(0);
        ndOperators=ndRoot.getElementsByTagName("operators").item(0);

        // return;
        for (var i=0; i<ndOperators.childNodes.length; i++) {
                var ndOperator=ndOperators.getElementsByTagName("operator").item(i);
                if (ndOperator != null){

                        var opId = ndOperator.attributes.getNamedItem('id').text;

                        var opNme = ndOperator.getElementsByTagName("name").item(0).text;
                        var opImg = ndOperator.getElementsByTagName("image").item(0).text;


                        var $o = $v.AddI('germain/'+$sk.bi, $xofs + $col*$sk.bd[0], $yofs + $row*$sk.bd[1]);
                        // alert($sk.bi);
                        $o.AddHandler('goto_Loc("data-entry.html?recepient='+opId+'")');
                        $v.AddI('germain/'+opImg, $xofs + $col*$sk.bd[0] + $opofsx, $yofs + $row*$sk.bd[1] + $opofsy +2);

                        $col++;
                        if ($col >= $maxcols) {
                                $col = 0;
                                $row++;
                        }
                        if ($row >= $maxrows) {
                                break;
                        }
                }
        }



}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_marquee($__v) {
        var $__s = $g_vpa[$__v]['s'];
        var $v = $g_vpa[$__v]['v'];
        $s_text = encodeURIComponent($gs_marquee_text);
        MJGA($v.o.i).innerHTML = '<marquee style="width='+$__s.r[2]+'px; height='+$__s.r[3]+'px; font-family: '+$__s.f[0]+'; font-size: '+$__s.f[1]+'px;">'+$gs_marquee_text+'</marquee>';
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// adds operator logo or 'no_picture' image with text annotation
//
function gfxa_ins_oper_logo($v, $i, $x, $y, $minx, $miny, $t) {
				var $skp = SKP('OPERATOR_ICON');
        // loading
        var $y_logo = $y;
        
        var srcImage = pathToOperators+detOpIco($i);
        var $o_opimg = '';
        
        //Если надо рисуем подложку под кнопку
     		var opId = getParameter('recepient','-1');
     		if (opId!=-1) {
						var op = jcfg_findOperatorById(opId);
						if (typeof(op['backing'])!='undefined' && op['backing']==1) {
				       	$backing = $v.AddI($skp.bi, $x-($skp.bd[0]-$skp.d[0])/2, $y_logo-($skp.bd[1]-$skp.d[1])/2);
								$backing.SizeTo(1, $skp.bd[0], $skp.bd[1]);
						}
				}
				
				//Рисуем сам логотип
        if ((/\.png$/i).test(srcImage)) {
	        $o_opimg = $v.AddD($x, $y_logo);
					$o_opimg.SizeTo(1, $skp.d[0], $skp.d[1]);
	        var $iii = MJGA($o_opimg.i);
					$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+pathToOperators+detOpIco($i)+'\', sizingMethod=\'scale\')';
      	} else {
	        $o_opimg = $v.AddI(srcImage, $x, $y_logo);
					$o_opimg.SizeTo(1, $skp.d[0], $skp.d[1]);
      	}
        $t = $t.replace(/\[/g, '<');
        $t = $t.replace(/\]/g, '>');
			 
        return $o_opimg;
}

var $gs_cybrules = '_____';
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

function check_cybrules($__n) {
        $gs_cybrules = $gs_cybrules.substring(1, 5)+$__n;
        // alert($gs_cybrules);

        if ('12112' === $gs_cybrules) {
                goto_Loc("cyberrules.html");
        }

}


var timerMainPage = 0;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_menu_icons($__v) {
			
        var $__s = $g_vpa[$__v]['s'];
        var $v = $g_vpa[$__v]['v'];
        var $n_keybKHMask = 0;
        
        

        //  - - - - - recalc & draw 'TRINITY GROUP' or '999 operator'
        var $maxcols=0;
        var $xofs=0;
        var $yofs=0;
        var $skp_tri = SKP('OPERATORS_TRINITY_ICON');
        var $skp_tri_intr = SKP('TRINITY_INTRINSIC_ICON');
        var $show_group_100 = true;
        var pb = getParameter("pb","undefined");
        if (pb != "undefined") $show_group_100 = false;
        if($skp_tri.r[3] < 1) $show_group_100 = false;
          
        var $spg = SKP('GROUP_ICON');
        var $mnu = $ga_jcfg['menu']['0'];

        // - - - - - new TRINIRY icon redrawer
        var $col = 0;
        var $row = 1;

        var $mnu100 = jcfg_findMenuById(100);
        //MjCRIODA($mnu);
				$maxcols = Math.floor($skp_tri.r[2] / $skp_tri_intr.bd[0]);
				if ($show_group_100){
						var $x0 = 20;
						var $y0 = 20;
		        var $xofs_loc = Math.floor( ($skp_tri_intr.bd[0] - $skp_tri_intr.d[0] )/2 );
		        var $yofs_loc = Math.floor( ($skp_tri_intr.bd[1] - $skp_tri_intr.d[1] )/2 );
			               	 		for (var $mi in $mnu100) {
			                        //GROUP
			                        if (_jcfg_mnuIsGroupId($mi) !== -1) {
																			$col++;
																			if ($col > $maxcols) {
																				   $col = 1;
																				   $row++;
																			}
			                                var mTitImg = pathToOperators+$mnu100[$mi]['image'];
			                                var $o;
																			if($skp_tri_intr.bi) {
			                                		$o  = $v.AddI($skp_tri_intr.bi, $x0+($col-1)*$skp_tri_intr.bd[0], $y0+($row-1)*$skp_tri_intr.bd[1]);
																					$o.SizeTo(1, $skp_tri_intr.bd[0], $skp_tri_intr.bd[1]);

					                                $oimg=$v.AddD($x0+($col-1)*$skp_tri_intr.bd[0]+$xofs_loc, $y0+($row-1)*$skp_tri_intr.bd[1]+$yofs_loc);
																					$oimg.SizeTo(1, $skp_tri_intr.d[0], $skp_tri_intr.d[1]);
																					var $iii = MJGA($oimg.i);
																					$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+mTitImg+'\', sizingMethod=\'scale\')';
			                                } else {
					                                $o=$v.AddD($x0+($col-1)*$skp_tri_intr.bd[0]+$xofs_loc, $y0+($row-1)*$skp_tri_intr.bd[1]+$yofs_loc);
																					$o.SizeTo(1, $skp_tri_intr.d[0], $skp_tri_intr.d[1]);
																					var $iii = MJGA($o.i);
																					$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+mTitImg+'\', sizingMethod=\'scale\')';
																			}	
			                                $o.AddHandler("route_to_vp('VIEW_OPERATORS', 'create_submenu_icons', '"+$mi+"', '"+$__v+"')", $skp_tri_intr.bd[0], $skp_tri_intr.bd[1]);
			                                $o.AddKHMask(mjScanCodeById($n_keybKHMask++));
			                        } 
			                        	else 
																	if (_jcfg_mnuIsMenuOperatorId($mi) !== -1) {
			                                //OPERATOR
																			if ($mi == 'op_999') {
																					var $opId = '999';
					                                var $op = jcfg_findOperatorById($opId);
					                                var image999 = new Array();
					                                image999 = $mnu100[$mi]['image'];
					                                for (k in image999) {
																							$col++;
																							if ($col > $maxcols) {
																								   $col = 1;
																								   $row++;
																							}
																							var mTitImg = ''
																							if (image999[k] != '---') mTitImg = pathToOperators+image999[k];
							                                else mTitImg = pathToOperators+$op['image'];
							                                var $o;
																							if($skp_tri_intr.bi){
						                                        $o  = $v.AddI($skp_tri_intr.bi, $x0+($col-1)*$skp_tri_intr.bd[0], $y0+($row-1)*$skp_tri_intr.bd[1]);
																										$o.SizeTo(1, $skp_tri_intr.bd[0], $skp_tri_intr.bd[1]);
																										$oimg=$v.AddD($x0+($col-1)*$skp_tri_intr.bd[0]+$xofs_loc, $y0+($row-1)*$skp_tri_intr.bd[1]+$yofs_loc);
																										$oimg.SizeTo(1, $skp_tri_intr.d[0], $skp_tri_intr.d[1]);
																										var $iii = MJGA($oimg.i);
																										$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+mTitImg+'\', sizingMethod=\'scale\')';
							                                } else {
								                                $o=$v.AddI($x0+($col-1)*$skp_tri_intr.bd[0]+$xofs_loc, $y0+($row-1)*$skp_tri_intr.bd[1]+$yofs_loc);
																								$o.SizeTo(1, $skp_tri_intr.d[0], $skp_tri_intr.d[1]);
																								var $iii = MJGA($o.i);
																								$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+mTitImg+'\', sizingMethod=\'scale\')';
																							}
							                                var $goto_locdoc = 'data-entry';
							                                if($ga_jcfg['config']['NoChangeMessage'] == 1) $goto_locdoc = 'message_no_get_change';
							                                $o.AddHandler('goto_Loc("'+$goto_locdoc+'.html?recepient='+$opId+'")', $skp_tri_intr.bd[0], $skp_tri_intr.bd[1]);
							                                $o.AddKHMask(mjScanCodeById($n_keybKHMask++));
							                            }
																			} else {
																					$col++;
																					if ($col > $maxcols) {
																						   $col = 1;
																						   $row++;
																					}
					                                var $opId = _jcfg_mnuIsMenuOperatorId($mi);
					                                var $op = jcfg_findOperatorById($opId);
					                                var mTitImg = pathToOperators+$op['image'];
					                                var $o;
																					if($skp_tri_intr.bi){
				                                        $o  = $v.AddI($skp_tri_intr.bi, $x0+($col-1)*$skp_tri_intr.bd[0], $y0+($row-1)*$skp_tri_intr.bd[1]);
																								$o.SizeTo(1, $skp_tri_intr.bd[0], $skp_tri_intr.bd[1]);
																								$oimg=$v.AddD($x0+($col-1)*$skp_tri_intr.bd[0]+$xofs_loc, $y0+($row-1)*$skp_tri_intr.bd[1]+$yofs_loc);
																								$oimg.SizeTo(1, $skp_tri_intr.d[0], $skp_tri_intr.d[1]);
																								var $iii = MJGA($oimg.i);
																								$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+mTitImg+'\', sizingMethod=\'scale\')';
					                                } else {
						                                $o=$v.AddD($x0+($col-1)*$skp_tri_intr.bd[0]+$xofs_loc, $y0+($row-1)*$skp_tri_intr.bd[1]+$yofs_loc);
																						$o.SizeTo(1, $skp_tri_intr.d[0], $skp_tri_intr.d[1]);
																						var $iii = MJGA($o.i);
																						$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+mTitImg+'\', sizingMethod=\'scale\')';
																					}
					                                var $goto_locdoc = 'data-entry';
					                                if($ga_jcfg['config']['NoChangeMessage'] == 1) $goto_locdoc = 'message_no_get_change';
					                                $o.AddHandler('goto_Loc("'+$goto_locdoc+'.html?recepient='+$opId+'")', $skp_tri_intr.bd[0], $skp_tri_intr.bd[1]);
					                                $o.AddKHMask(mjScanCodeById($n_keybKHMask++));
			                              }
				                        }
			              }
        }
				
        //  - - - - - draw da MAIN GROUPS in da main menu
        // getting max colums put into container
        $maxcols = Math.floor($__s.r[2] / $spg.bd[0] );
				var $x0 = 20;
				var $y0 = 170;

        $col = 1;
        $row = 1;

        var $i = -1;
        for (var $mi in $mnu) {
                if (_jcfg_mnuIsMenuItemId($mi) === -1) {
                        continue;
                }
                $i++;

                var $grId = _jcfg_mnuIsGroupId($mi);

                // ignoring TRINITY operators
                if ($grId === '100') {
                        continue;
                }
               if ($grId !== -1) {
                    if((jcfg_enumerateMenuItems(jcfg_findMenuById($grId))) == 1) {
                        var localImage = $mnu[$mi]['image'];
                        if (pb == 'undefined' || localImage.indexOf('_mt')<=0) {
                            var $o = $v.AddI(localImage, $x0+($col-1)*$spg.bd[0], $y0+($row-1)*$spg.bd[1]);
                            $o.SizeTo(1, $spg.bd[0], $spg.bd[1]);
                            currentMenu = jcfg_findMenuById($grId)
                            for (var menuItem in currentMenu) {
                                var $opId =_jcfg_mnuIsMenuOperatorId(menuItem);
                                var $op = jcfg_findOperatorById($opId);
                                if ($opId !== -1) {
                                    if ('cyberplat_mt' === $op['processor']['type'].toLowerCase()) {
                                        $o.AddHandler('goto_Loc("mt.html?recepient='+$opId+'")', $spg.bd[0], $spg.bd[1]);
                                    }else{
							                        	var $goto_locdoc = 'data-entry.html?recepient='+$opId;
				                        				if ($ga_jcfg['config']['NoChangeMessage'] == 1) $goto_locdoc = 'message_no_get_change.html?recepient='+$opId;
                                				if ($mj_g_docnme == 'pb.html') $goto_locdoc = 'pb.html?pb=enteritem&recepient='+$opId;
																				$o.AddHandler('goto_Loc("'+$goto_locdoc+'")', $spg.bd[0], $spg.bd[1]);
                                    }
                                } else {
                                    $o.AddHandler("route_to_vp('VIEW_OPERATORS', 'create_submenu_icons', '"+$grId+"', '"+$__v+"')", $spg.bd[0], $spg.bd[1]);
                                    $o.AddKHMask(mjScanCodeById($n_keybKHMask++));
                                }
                            }
                        }
                    } else {
                    				menuTree[$grId] = 0;
                            var $o = $v.AddI($mnu[$mi]['image'], $x0+($col-1)*$spg.bd[0], $y0+($row-1)*$spg.bd[1]);
                            $o.SizeTo(1, $spg.bd[0], $spg.bd[1]);
                            $o.AddHandler("route_to_vp('VIEW_OPERATORS', 'create_submenu_icons', '"+$grId+"', '"+$__v+"')", $spg.bd[0], $spg.bd[1]);
                            $o.AddKHMask(mjScanCodeById($n_keybKHMask++));
                    }
                        
	               	 
	               	      $col++;
                        if ($col > $maxcols) {
                                $col = 1;
                                $row++;
                        }

                } 

        } // for in mnu

        $v['bl'] = new Array();
        $v['bl']['s'] = 1;
        $v['bl']['n'] = '[BL]';
        $v['bl']['h'] = '';        // no handler

				
        // - - - - - information and help addtional buttons
        mainMenuBigButton = SKP('MAINPAGEBUTBIG');
        if ($mj_g_docnme!='pb.html') {
	        if ($intro) {
	  		  	$ga_buttbig[0]['v'].Show(1, 1);
	          changeButtonImage($ga_buttbig[0]['o'], 'main');
	          $ga_buttbig[0]['o'].AddHandler("goto_Main()");
	        } else {
	      	  create_bigbut_static_vp($__v, -2, mainMenuBigButton['leftbut'], LCP('button_information'), "$g_n_secsInHelpOrInfo=0; route_to_vp('VIEW_INFO', 'create_help', '-1', '"+$__v+"')");
			  		create_bigbut_static_vp($__v, -3, mainMenuBigButton['rightbut'], LCP('button_instruction'), "$g_n_secsInHelpOrInfo=0; route_to_vp('VIEW_HELP', 'create_info', '-1', '"+$__v+"')");        		
	          mj_RT.Add('check_AmInHelpOrInfo()', 1000, 0);
	          
	      	}
					if (ShowPB == '1') {
				      	//кнопка записной книжки
				      	var pb_skp = SKP('PB_BOOK');
				      	$o = $v.AddI(pb_skp.i, pb_skp.coord[0], pb_skp.coord[1]);
				        $o.SizeTo(1, pb_skp.d[0], pb_skp.d[1]);
				      	$o.AddHandler('goto_Loc("pb.html?pb=login")', pb_skp.d[0], pb_skp.d[1]);
					}
        } else {
  		  	$ga_buttbig[0]['v'].Show(1, 1);
			var pb = getParameter("pb","undefined");
			if(pb == 'login')
				changeButtonImage($ga_buttbig[0]['o'], 'main');
			else
				changeButtonImage($ga_buttbig[0]['o'], 'back');
            $ga_buttbig[0]['o'].AddHandler('goto_Loc("pb.html?pb=getpb")');
      	}
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
var $g_n_secsInHelpOrInfo=0;

function check_AmInHelpOrInfo() {
                if ( ($g_current_vpa != 'VIEW_HELP') && ($g_current_vpa != 'VIEW_INFO') ) {
                        return;
                }

                $g_n_secsInHelpOrInfo++;
                if ($g_n_secsInHelpOrInfo > 20) {
                route_to_vp('VIEW_MAINMENU', "", "-1", "-1");
                }
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// creating submenu items with icons
//
function create_submenu_icons($__v) {
				clearTimeout(timerMainPage);
				timerMainPage = setTimeout("goto_Main()",90000);
	
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];
        var $id = $g_vpa[$__v]['i'];
        var $a_id =  $id.split('_');
        var $n_keybKHMask = 0;
        //  - - - - - - perpage offset
        var $__id = $a_id[0]; // $g_vpa[$__v]['i'];
        $perp_ofs = parseInt(('undefined' === typeof($a_id[1])) ? 0 : $a_id[1]);

        var $sk = SKP('OPERATOR_ICON');

        //Размеры области
        var $width_full = $__s.r[2];
        var $heigth_full = $__s.r[3];
        
        //Размеры кнопок
        var $width_loc = $sk.bd[0];
        var $heigth_loc = $sk.bd[1];
       
        // getting max colums to put into container
        var $maxcols = Math.floor($width_full/$width_loc);
        var $maxrows = Math.floor($heigth_full/$heigth_loc);
        
        var dx = ($sk.bd[0] - $sk.d[0])/2;
        var dy = ($sk.bd[1] - $sk.d[1])/2;
				
        // - - - - - - - generate menu items (operators, groups...)

        var $mnu = jcfg_findMenuById($__id);
        var $mnu_items_count = jcfg_enumerateMenuItems($mnu);

        x0 = 20;
        y0 = 20;
        var $col = 0;
        var $row = 0;
        var $items_outputed = 0;
        var $i = -1;
        for (var $mi in $mnu) {
                if (_jcfg_mnuIsMenuItemId($mi) === -1) {
                        continue;
                }
            if(!(($mi == 'op2501') && ($mj_g_docnme == 'pb.html'))) {
                $i++;
                if ($i < $perp_ofs) {
                        continue;
                }

                $items_outputed++;
	              var $o = $v.AddI($sk.bi, x0+$col*$width_loc, y0+$row*$heigth_loc);
                $o.SizeTo(1, $width_loc, $heigth_loc);
                //$o.Border(1, '1px dotted red');
								
                // - - - - - group
                var $grId = _jcfg_mnuIsGroupId($mi);
                if ($grId !== -1) {
         								menuTree[$grId] = $id;
                        $o.AddHandler("route_to_vp('VIEW_OPERATORS', 'create_submenu_icons', '"+$grId+"', '"+$__v+"')", $sk.bd[0], $sk.bd[1]);
                        $o.AddKHMask(mjScanCodeById($n_keybKHMask++));
                        gfxa_ins_oper_logo($v, $mnu[$mi]['image'], x0+$col*$width_loc+dx,  y0+$row*$heigth_loc+dy, $sk.d[0], $sk.d[1], $mnu[$mi]['name']);
                }
                // - - - - - operator
                else  {
                        var $opId = _jcfg_mnuIsMenuOperatorId($mi);
                        var $op = jcfg_findOperatorById($opId);
                        if ($op == -1) {
                                continue;
                        }

                        if ($g_forced_mobile_selection === true) {
                                $o.AddHandler('goto_ForcedCheck("'+$opId+'")', $sk.bd[0], $sk.bd[1]);
                                $o.AddKHMask(mjScanCodeById($n_keybKHMask++));
                        }
                        else {
                                  if ('cyberplat_mt' === $op['processor']['type'].toLowerCase()) {
                                      $o.AddHandler('goto_Loc("mt.html?recepient='+$opId+'")', $sk.bd[0], $sk.bd[1]);
                                  }else{
							                        var $goto_locdoc = 'data-entry.html?recepient='+$opId;
							                        if ($ga_jcfg['config']['NoChangeMessage'] == 1) $goto_locdoc = 'message_no_get_change.html?recepient='+$opId;
			                                if ($mj_g_docnme == 'pb.html') $goto_locdoc = 'pb.html?pb=enteritem&recepient='+$opId;
							                        $o.AddHandler('goto_Loc("'+$goto_locdoc+'")', $sk.bd[0], $sk.bd[1]);
                                  }
                                  $o.AddKHMask(mjScanCodeById($n_keybKHMask++));
                        }
                        gfxa_ins_oper_logo($v, $op['image'], x0+$col*$width_loc+dx, y0+($row*$heigth_loc)+dy, $sk.d[0], $sk.d[1], $op['name']);
                }


                $col++;
                if ($col >= $maxcols) {
                        $col = 0;
                        $row++;
                }
                if ($row >= $maxrows) {
                        break;
                }
            }
        }
        if ($mnu_items_count > $maxcols*$maxrows) {
      			pp = SKP('PERPAGE');
      			
      			var countPages = 0;
      			countPages = ($mnu_items_count % ($maxcols*$maxrows)==0) ? Math.floor($mnu_items_count/($maxcols*$maxrows)) : Math.floor($mnu_items_count/($maxcols*$maxrows))+1;
      			var currentPage = parseInt($perp_ofs/($maxcols*$maxrows));
      			
      			x0 = 516;
      			y0 = 640;
      			w =  pp.d[0];
      			h =  pp.d[1];

      			for (i=0; i<countPages; i++) {
      				 var xx = x0 + i*w;
      				 var yy = y0;
      				 if (i==currentPage) var imageName = pp.i+'_'+(i+1)+'_current'+pp.ext;
      				 else var imageName = pp.i+'_'+(i+1)+pp.ext;

      				 
      				 var $o = $v.AddI(imageName,xx,yy);
      				 $o.SizeTo(1, w, h);
      				 $s = $__id+'_'+(i*$maxcols*$maxrows);
      				 $o.AddHandler("route_to_vp('VIEW_OPERATORS', 'create_submenu_icons', '"+$s+"', '"+$__v+"')");
      			}
      	}
        
        
} // function


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// creating subelements with operators icons
/*
function create_operator_icons($__v) {
// function create_operator_icons($__v, $__s, $__i, $__p, $__id) {

        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];
        var $__id = $g_vpa[$__v]['i'];
        // alert($__v);
        // alert($__id);
        set_rect_text($v, [5, 15, $__s.r[2], 70], $__s, LCP('choose_operator'));

        $sk = SKP('OPERATOR_ICON');

        // getting max colums put into container
        var $yofs = 100;

        var $maxcols = Math.floor( $__s.r[2] / $sk.bd[0] );
        var $maxrows = Math.floor( ($__s.r[3] - $yofs) / $sk.bd[1] );

        var $xofs = Math.floor( ($__s.r[2] - $sk.bd[0]*$maxcols)/2 );
        // alert($maxcols);

        // var ndMenu=ndRoot.getElementsByTagName("menu").item(0);


        var $col = 0;
        var $row = 0;
        //alert($__id);
        ndOperators = GetMenu($__id,ndMenu);
        // ndOperators = GetUpMenu($__id,ndMenu);

        alert(ndOperators.childNodes.length);


        for (var $i=0; $i<ndOperators.childNodes.length; $i++) {
                var ndOperatorId=ndOperators.getElementsByTagName("operator_id").item($i);
                var opId = ndOperatorId.attributes.getNamedItem('id').text;

                var ndOperator=GetNodeById(opId);
                if (ndOperator != null){

                        // var opId = ndOperator.attributes.getNamedItem('id').text;

                        var opNme = ndOperator.getElementsByTagName("name").item(0).text;
                        var opImg = ndOperator.getElementsByTagName("image").item(0).text;
                        // alert(opNme);


                        // var $o = $__v.AddI($spg.bi, $xofs + $col*$spg.bd[0], $yofs + $row*$spg.bd[1]);

                        var $o = $v.AddI($sk.bi, $xofs + $col*$sk.bd[0], $yofs + $row*$sk.bd[1]);
                        $o.AddHandler('goto_Loc("data-entry.html?recepient='+opId+'")');
                        $v.AddI('op/'+opImg, $xofs + $col*$sk.bd[0] + 12, $yofs + $row*$sk.bd[1] + 18);

                        $col++;
                        if ($col >= $maxcols) {
                                $col = 0;
                                $row++;
                        }
                        if ($row >= $maxrows) {
                                break;
                        }
                }
        }


}
*/

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// , $__t, $__tid



function route_to_vp($__skp, $__func_subels, $__id, $__parent) {
				
        var parent_id = 0;
        if (typeof(menuTree[$__id])!='undefined') parent_id = menuTree[$__id];
        
        PlaySoundA('click1');
        // - - - - - disable current vpa
        $g_vpa[$g_current_vpa]['v'].Show(0, 1);
        // - - - - - disable relatives
        if ('undefined' !== typeof($g_vpa[$g_current_vpa]['relative_vpa'])) {
		        var $parent_vpa = $g_vpa[$g_current_vpa]['relative_vpa'];
            $g_vpa[$parent_vpa]['v'].Show(0, 1);
        }
				
        // - - - - - enable current vpa
        $g_current_vpa = create_vpa($__skp, $__func_subels, $__id, $__parent);
        // - - - - - buttons overcreation test
        if ($ga_buttbig.length > 2) {
                alert('route_to_vp() : buttons overcreation test - fail!');
        }
				
        // - - - - - buttons-vpa-logical view
        var $v = $g_vpa[$g_current_vpa];
        change_srt_text($ga_buttbig[0]['txi'], $v['bl']['n']);
        change_srt_text($ga_buttbig[1]['txi'], $v['br']['n']);

        $ga_buttbig[0]['v'].Show($v['bl']['s'], 1);
        $ga_buttbig[1]['v'].Show($v['br']['s'], 1);

        $ga_buttbig[0]['o'].AddHandler($v['bl']['h']);
        $ga_buttbig[1]['o'].AddHandler($v['br']['h']);

        // MjCRIODA($g_vpa[$g_current_vpa]);

        // - - - - - additional buttons logic
        
        if ('-1' !== $g_vpa[$g_current_vpa]['p']) {
					$ga_buttbig[0]['v'].Show(1, 1);
          
          // кнопка "назад"
          changeButtonImage($ga_buttbig[0]['o'], 'main');
    	  var destination = 'main.html';
          if ($intro == true) destination = 'main_selection.html';
          $ga_buttbig[0]['o'].AddHandler("goto_Loc('"+destination+"')");

          if (parent_id != 0) {
          	changeButtonImage($ga_buttbig[0]['o'], 'back');
          	$ga_buttbig[0]['o'].AddHandler("route_to_vp('VIEW_OPERATORS', 'create_submenu_icons', '"+parent_id+"', 'VIEW_OPERATORS_"+parent_id+"')");
          }
          
          var pb = getParameter("pb","undefined");
          if (pb.toLowerCase()=='additem') {
          	changeButtonImage($ga_buttbig[0]['o'], 'back');
          	$ga_buttbig[0]['o'].AddHandler('goto_Loc("pb.html?pb=additem")');
          }

        } else $ga_buttbig[0]['v'].Show(0, 1);
                
        // - - - - - enable relatives
        if ('undefined' !== typeof($g_vpa[$g_current_vpa]['relative_vpa'])) {
		        var $parent_vpa = $g_vpa[$g_current_vpa]['relative_vpa'];
            $g_vpa[$parent_vpa]['v'].Show(1, 1);

            if ('VIEW_ACCOUNT_DETAILS' === $g_current_vpa) {
    		        activate_field($g_fields_active);
                verify_fields();
            }
        }
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function goto_Loc($__s) {
        PlaySoundA('click1');
        document.location.href = $__s;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function goto_LocNos($__s) {
        document.location.href = $__s;

}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function goto_Main() {
        goto_Loc("main.html");
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// $s__s parameter ==== ???
function goto_Check($__s) {
        PlaySoundA('click2');
        if ('undefined' !== typeof($goto_check_overload)) {
  			    eval($goto_check_overload)($__s);
            return;
        }

        $__s = 'checking.html'+'?recepient=' + Parameters['recepient'];
				if (isOperator999 == 1) $__s+='&isOperator999=1';
				
        var opId = Parameters['recepient'];
        var $op = jcfg_findOperatorById(opId);
        var $pr = $op['processor']['type'];

        if ($pr.toLowerCase() == 'cyberplat_mt') {
            $__s+="&recepient_mt="+getParameter("recepient_mt","");
        }
        if ($pr === 'Cyberplat_PIN') {
                // GetCardsInfo(opId);
                var $opId = getParameter('recepient', 'undefined');
                var $l_cards = $ga_jcfg['details']['pin_info'][$opId]['cards'];

                $cid = $ga_fields[$g_fields_active]['val'];
                $__s += '&cardname='+$l_cards[$cid]['name'];
                $__s += '&cardvalue='+$l_cards[$cid]['amount'];
                $__s += '&cardid='+$cid;

        }
        else {
                for (var $i=0; $i<$ga_fields.length; $i++)  {
                        $__s += '&field'+$ga_fields[$i]['id']+'='+$ga_fields[$i]['val'];
                }
                if("half_pin" == $pr.toLowerCase()){
                        var $opId = getParameter('recepient', 'undefined');
                        var $l_cards = $ga_jcfg['details']['pin_info'][$opId]['cards'];

                        $cid = $ga_fields[$g_fields_active]['val'];
                        $__s += '&cardname='+$l_cards[$cid]['name'];
                        $__s += '&cardvalue='+$l_cards[$cid]['amount'];
                        $__s += '&cardid='+$cid;
                }
        }
       //alert($__s);
       document.location.href = $__s; 
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function goto_ForcedCheck($__n) {
        PlaySoundA('click1');
        Parameters['recepient'] = $__n;
        goto_Check();
}

function isArray(obj) {
   if (obj.constructor.toString().indexOf("Array") == -1)
      return false;
   else
      return true;
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_field_comment($__v, $__ofs, $__f) {

        //  - - - - - activate field
				if (create_field_comment.arguments.length == 2) var $ae = $ga_fields[$g_fields_active];
				else var $ae = $ga_fields[$__f];

        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];

        //  - - - - - - - comment
        var $skp_comm = SKP('VIEW_ACCOUNT_DETAILS_FCOMMENT');
        $ae['commf'] = $v.AddD(0, 0);
		var $commFontSize = parseInt($skp_comm.comissionSize);
		var $skp_comm = SKP('VIEW_ACCOUNT_DETAILS_FCOMMENT');
		var $OpId = getParameter('recepient','-1');
		var $op = jcfg_findOperatorById($OpId);
		var localCms = jcfg_getCommission($OpId);
		var local_commission_offset = 0;
		var is_pb_dataentry = ($mj_g_docnme == 'pb.html') && (getParameter("pb","undefined") == 'dataentry');
		var i_localCms = 0;

		if (isArray(localCms))
			for (var k in localCms)
				if (k != 0) 
					i_localCms++;
		if (i_localCms > 1)
			local_commission_offset += 1.2*$commFontSize*i_localCms;
		if (typeof($op['comment'])!='undefined')
			if ($op['comment'] != '')
				local_commission_offset += 2.2*$commFontSize;

		var len = parseInt($ae['commft'].length);
		var $off_y = (is_pb_dataentry && (len > 250) && (typeof($op['comment'])!='undefined')) ? 120 : 90;

		$off_y -= local_commission_offset*0.8;
        $ae['commf'].MoveTo(1, GTX($skp_comm.r[0]), GTY($skp_comm.r[1]) - $off_y);
        $ae['commf'].SizeTo(1, $skp_comm.r[2], $skp_comm.r[3]);
        var $online = '';
        if($op !== '-1') {
        		if($op['processor']['offline'] === '0' && "cyberplat_mt"!=$op['processor']['type'].toLowerCase()) {
		          	$online = '<p>'+LCP('comment_online')+'</p>';
    	      }
        }
        
		var fontsize = $skp_comm.f[1];
		
		     if (len > 250) fontsize = 18;
		else if (len > 170) fontsize = 20;
		else if (len > 100) fontsize = 24;

		var coeff = 12;
		if (is_pb_dataentry && 
			($ae['commft'] != '') && 
			(len > 250))
				coeff = 7;
		coeff = (coeff - (parseInt(local_commission_offset)/$commFontSize))/coeff;

		if (len > 100)
			if ((is_pb_dataentry) && (local_commission_offset != 0))
				fontsize = Math.ceil(fontsize*coeff);
			else 
				fontsize -= 3;

        mj_G._a[$ae['commf'].i]['i'].innerHTML = $ae['commft'] + $online;// + operatorComment + $cms;
        mj_G._a[$ae['commf'].i]['i'].style["fontFamily"] = $skp_comm.f[0];
        mj_G._a[$ae['commf'].i]['i'].style["fontSize"] = fontsize+'px';
        mj_G._a[$ae['commf'].i]['i'].style["fontWeight"] = $skp_comm.f[2];
        mj_G._a[$ae['commf'].i]['i'].style["color"] = $skp_comm.f[3];
        mj_G._a[$ae['commf'].i]['i'].style["textAlign"] = $skp_comm.f[4];
}

function showCommision($__v, $__ofs, $__f) {
			
        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];

        var $OpId = getParameter('recepient','-1')
        var $op = jcfg_findOperatorById($OpId);
				var $skp_comm = SKP('VIEW_ACCOUNT_DETAILS_FCOMMENT');


				//Комиссия
        var $cms = '';
				if ($OpId != '999' && "cyberplat_mt" != $op['processor']['type'].toLowerCase()) {
					var localCms = jcfg_getCommission($OpId);
					if (isArray(localCms)) {
						$cms = '<p><span style="font-size:'+$skp_comm.comissionSize+'px">'+LCP('commission_prefix')+'</span>';
						for (k in localCms) {
								if (k!=0) $cms+=', ';
								$cms+= '<nobr><font color="'+$skp_comm.comissionColor+'"><span style="font-size:'+$skp_comm.comissionSize+'px">'+localCms[k]['val']+'</font>';
								if (localCms[k]['txt']!='') $cms+=' от '+localCms[k]['txt']+'</nobr>';
						}	
						$cms+='</p>'
					} else $cms = LCP('commission_none');
				}
       
        //Комментарий на оператора
        var operatorComment = '';
        if (typeof($op['comment'])!='undefined') operatorComment = $op['comment'];
				var txt = $cms + operatorComment;
				set_rect_text($v, [45, 200, 200, 50], $skp_comm, txt);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_pinpad($__v) {
		pinpadActive = true;
	
        var cvp_PP = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        var pb = getParameter('pb', 'undefined');
        if (pb.toLowerCase() != 'login' && pb.toLowerCase() != 'enterpin') create_field_comment($__v, 0);

        var $title = $ga_fields[$g_fields_active]['title'];
        if(1 === $fields_count) $title = '';
        //title in field name
        //set_rect_text(cvp_PP, [5, 10, $__s.r[2], 50], $__s, $title );
				
				var opId = getParameter('recepient', 'undefined');
	  		var $op = jcfg_findOperatorById(opId);
	  		
	  		$a_pin = ['1', '2', '3', '|', '4', '5', '6',  '|', '7', '8', '9', '|', 'c', '0', 'b'];
	  		if ($op['fix']!==0 && $op['fix']!==1) {
							if (($ga_fields[$g_fields_active]['id']) == $op['fix']) {
									$a_pin = ['1', '2', '3', '|', '4', '5', '6',  '|', '7', '8', '9', '|', 'z', '0', 'b'];					
							}
				}
				
        
        var $x = 0;
        var $y = 0;
        var $xo = 15;
        var $yo = 70;

				pinPadButton = SKP('PINPADBUTTON');
				dx = pinPadButton.d[0];
				dy = pinPadButton.d[1];
        for (var $v=0; $v<$a_pin.length; $v++) {
                if ('|' !== $a_pin[$v]) {
                				if ($a_pin[$v]!=' ') {
                        	var $o = cvp_PP.AddI(pinPadButton['i']+'_'+$a_pin[$v]+'.'+pinPadButton['ext'], $xo + dx*$x, $yo + dy*$y);
                        	$o.SizeTo(1, dx, dy);
                        	var $o1 = cvp_PP.AddI(pinPadButton['i']+'_'+$a_pin[$v]+'_click.'+pinPadButton['ext'], $xo + dx*$x, $yo + dy*$y, true);
                        	$o1.SizeTo(1, dx, dy);

	                        $o.AddHandler('cvp_PP_PinPress("'+$o.i+'", "'+$o1.i+'", "'+$a_pin[$v]+'")', 120, 120);
	                        if ($a_pin[$v] === 'c') {
	                                $o.AddKHMask(mjsc_Space);
	                        }
	                        else if ($a_pin[$v] === 'b') {
	                                $o.AddKHMask(mjsc_Delete);
	                        }
	                        else {
	                                $o.AddKHMask(mjsc_1+parseInt($a_pin[$v]-1));
	                                $o.AddKHMask(mjsc_1p+parseInt($a_pin[$v]-1));
	                        }
												}
                        $x++;
                }
                else {
                        $y++;
                        $x=0;
                }
        }
        var $__s_bg_enter_number = SKP('VIEW_ACCOUNT_INLET_NUMBER_FIELD');
        //$o_number1 = cvp_PP.AddI($__s_bg_enter_number.i, $__s_bg_enter_number.r[0], $__s_bg_enter_number.r[1]);
        //$o_number = cvp_PP.AddD(0, $__s_bg_enter_number.r[1]+20);
        //$o_number.SizeTo(1, $__s_bg_enter_number.r[2], $__s_bg_enter_number.r[3]);
        //$o = $o_number;


        var $spb = SKP('BUTBIG');

        //mj_G._a[$o.i]['i'].style["fontFamily"] = $__s_bg_enter_number.f[0];
        //mj_G._a[$o.i]['i'].style["fontSize"] = $__s_bg_enter_number.f[1];
        //mj_G._a[$o.i]['i'].style["fontWeight"] = $__s_bg_enter_number.f[2];

        //mj_G._a[$o.i]['i'].style["color"] = $__s_bg_enter_number.f[3];
        //mj_G._a[$o.i]['i'].style["textAlign"] = $__s_bg_enter_number.f[4];


        // if masked, update it!
        var $s_def = '';

        var $ae = $ga_fields[$g_fields_active];
        //mj_G._a[$o_number.i]['i'].innerHTML = $s_def; // *(***)***-**-**
        change_srt_text($ga_fields[$g_fields_active]['txi'],  $s_def);
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  - - - - - animated objects
var $g_vp_aobj = new Array();

function gfxa_aniobj_back() {
        for (var $i in $g_vp_aobj) {
                /*
                mj_G._a[ $g_vp_aobj[$i]['oi'] ]['o'].left = $g_vp_aobj[$i]['ox'];
                mj_G._a[ $g_vp_aobj[$i]['oi'] ]['o'].top = $g_vp_aobj[$i]['oy'];
                */
                var $to = $g_vp_aobj[$i];
                if ($to['c'] > 0) {
                        $to['c']--;
                        mj_G._a[$i]['o'].MoveTo(1, $to['ox']+3*$to['c'],$to['oy']+3*$to['c']);
                        $g_vp_aobj[$i] = $to;
                        // alert('updated!');
                }
        }
}

var $keyb_click_increment = 2;

function gfxa_aniobj_use($i) {
        // alert($i);

        // дребезг!
        var $to;
        if ('undefined' === typeof($g_vp_aobj[$i])) {
                // alert('here!');
                $to = new Array();
                $to['c'] = $keyb_click_increment;
                $to['ox'] = mj_G._a[$i]['o'].x;
                $to['oy'] = mj_G._a[$i]['o'].y;

        }

        else {
                // alert('defined!');
                $to = $g_vp_aobj[$i];
                $to['c'] += $keyb_click_increment;
        }


        mj_G._a[$i]['o'].MoveTo(1, $to['ox']+3*$to['c'],$to['oy']+3*$to['c']);
        $g_vp_aobj[$i] = $to;
        // alert($to['ox']);

}

//mj_RT.Add('gfxa_aniobj_back', 50, 0);


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
var $cvp_PP_apin = new Array();

var $s_number = '';

function maskFieldValidation(number, mask) {
  var output = new Array(); // переменная возвращающая значения.
	
	//"чистим" маску.
	var cleanMaskNumber = '';
	
	var reg=/!.+!|~.+~|\*+/g;
	var result=mask.match(reg);
	for (var i=0; i<result.length; i++) cleanMaskNumber+=result[i];

	reg=/!|~/g;
	cleanMaskNumber = cleanMaskNumber.replace(reg, "");

	//Добавляем проверку на корректный ввод кода оператора
	var correctString = false;
	var reg=/#.+#/g;
	if (reg.test(mask)) {
		correctString = true;
		var correctPrefix = new Array();
		result=mask.match(reg);		
		for (var i=0; i<result.length; i++) {
			reg=/#/g;
			correctPrefix[i] = result[i].replace(reg, "");
		}
	}
	
	//Определяем индексы звездочек в чистой маске
	var cleanMaskIndex = new Array();
	var k=0;
	for (i=0; i<cleanMaskNumber.length; i++) 
		if (cleanMaskNumber.charAt(i)=='*') {
			cleanMaskIndex[k] = i;
			k++;
		}
  //Определяем символы набранные пользователем.
	var userNumber = '';
	for (i in cleanMaskIndex) 
		if (number.charAt(cleanMaskIndex[i]) != '') {
			userNumber+=number.charAt(cleanMaskIndex[i]);
		}
	
	//Чистим номер если введен некорректный код оператора	
	if (correctString) {
		for (i in correctPrefix) {
			reg = new RegExp("^"+correctPrefix[i]);
			if (reg.test(userNumber)) { 
				userNumber=userNumber.replace(reg, '');
			}
		}
	}	
	
 	//Длина номера не должна привышать длину "чистой маски"
	if (number.length > cleanMaskNumber.length) number = number.substring(0, cleanMaskNumber.length);
	

	//Готовим номер на выход
	k=0;
	number = '';
	for (i=0; i<cleanMaskNumber.length; i++) {
		if (cleanMaskNumber.charAt(i)=='*' && userNumber.charAt(k)!='') {
			number+=userNumber.charAt(k);
			k++;
		}
		else number+=cleanMaskNumber.charAt(i);
	}
	if (number.indexOf('*')>-1) number=number.substring(0, number.indexOf('*'));
	
	output['number'] = number;

	//---------------------------------------------------------------------

	//Если длина номера равна длине чистой маски включаем кнопку "Далее"
	if (number.length == cleanMaskNumber.length) output['pass'] = 1;
	else output['pass'] = 0;
	
	//---------------------------------------------------------------------
	//Теперь подготовим внешний вид.
	//Чистим маску: убираем !, вырезаем ~строка~, вырезаем #строка# 
	reg=/!|~.+~|#.+#/g;
	cleanMaskView = mask.replace(reg, "");
	//накладываем на эту маску строку символов, введенных пользователем.
	k=0;
	var view = '';
	for (i=0; i<cleanMaskView.length; i++) {
		if (cleanMaskView.charAt(i)=='*' && userNumber.charAt(k)!='') {
			view+=userNumber.charAt(k);
			k++;
		}
		else {
			if (cleanMaskView.charAt(i) == '*') view+='•';
			else view+=cleanMaskView.charAt(i);
		}
	}
	
	//Отправляем представление на на выход
	output['view'] = view;
 
 	//---------------------------------------------------------------------
 	// Ну а теперь небольшая хитрость.
 	// Обработаем здесь удаление символа
 	if (userNumber.length > 0) userNumber = userNumber.substring(0, userNumber.length-1);
 	k=0;
	var delLastSymbol = '';
	for (i=0; i<cleanMaskNumber.length; i++) {
		if (cleanMaskNumber.charAt(i)=='*' && userNumber.charAt(k)!='') {
			delLastSymbol+=userNumber.charAt(k);
			k++;
		}
		else delLastSymbol+=cleanMaskNumber.charAt(i);
	}
	if (delLastSymbol.indexOf('*')>-1) delLastSymbol=delLastSymbol.substring(0, delLastSymbol.indexOf('*'));
	output['delLastSymbol'] = delLastSymbol;
	return output;
}

function PP_PinBack($__a, $__b, $__f) {
	mj_G._a[$__b]['o'].Visi(1, 0);
	if ((!pinpadActive) || ($__f != $g_fields_active))
		return;
	mj_G._a[$__a]['o'].Visi(1, 1);
}

var isOperator999 = 0;

function cvp_PP_PinPress($__a, $__b, $__v) {
		pinpadActive = true;
		
        if ('undefined' !== typeof($__a))  {
          mj_G._a[$__a]['o'].Visi(1, 0);
		  mc_delay(3);
		  mj_G._a[$__b]['o'].Visi(1, 1);
		  setTimeout('PP_PinBack(' + $__a + ',' + $__b + ',' + $g_fields_active + ');', PPButtonShowDelay);
        }
        else $__v = '';

        var $ae = $ga_fields[$g_fields_active];
        $s_number = $ae['val'].toString();
      	
        //обработка нажатия кнопки "C"
        if ('c' == $__v) $s_number = '';

        //обработка нажатия кнопки "Назад"
        else if ('b' == $__v) {
	 					if ('m' === $ae['t']) {
								res = maskFieldValidation($s_number, $ae['attr']['mask']);
								$s_number = res['delLastSymbol'];
	          } 
						if ('i' === $ae['t'] || 'p' === $ae['t']) 
						if ($s_number.length > 0) $s_number = $s_number.substring(0, $s_number.length-1);
        } else if ('z' == $__v) {
        		if ($s_number.indexOf(".")==-1) $s_number += '.';
      	}
        // add number to field
        else {
                // - - - - - - - if masked field
                if ('m' === $ae['t']) $s_number += $__v;

                // - - - - - - - if password field
                if ('p' === $ae['t']) {
                		if ($s_number.length < $ae['attr']['max']) $s_number += $__v;
                }
                // - - - - - - - if integer field
                if ('i' === $ae['t']) 
                        if ($s_number.length < $ae['attr']['max']) 
                                $s_number += $__v;
        } 

        // FIELD VALIDAIONS
        // - - - - - - - if masked field
        if ('m' === $ae['t']) {
                var $s_mask = $ae['attr']['mask'];
								var res = maskFieldValidation($s_number, $s_mask);
								$s_number = res['number'];
								$s_r = res['view'];
								$ae['pass'] = res['pass'];
        }
        // - - - - - - - if integer field
        if ('i' === $ae['t']) {
                $s_r = $s_number;
                if ($s_number.length >= $ae['attr']['min']) {
                        $ae['pass'] = 1;
                }
                else {
                        $ae['pass'] = 0;
                }

        }
        // - - - - - - - if password field
        if ('p' === $ae['t']) {
                $s_r="";
                for (var $j=0; $j<$s_number.length; $j++) $s_r+="•";
                if ($s_number.length >= $ae['attr']['min'] && $s_number.length <= $ae['attr']['max']) {
                        $ae['pass'] = 1;
                } else {
                        $ae['pass'] = 0;
                }

        }
        // - - - - - - - verify on next
        $ae['val'] = $s_number;

        //  - - - - - - - checking in local base
        if ('999' === getParameter('recepient')) {
                Parameters['recepient_potent'] = '999';
        }

        //
        $g_forced_mobile_selection = false;

        if ('999' === getParameter('recepient_potent')) {
                if (!$gs_localbase_included) {
                        CreateBase('0123456789', 100000);
                        $gs_localbase_included = true;
                }
								
                if ($s_number.length >= 10) {
                        var $idx = GetIndex_InLocalBase($s_number);
                        if (($idx != -1) &&  (jcfg_findOperatorById(OpId_InLocalBase($idx)) != -1) ) {
                                // alert('operaror: '+OpName_InLocalBase($idx));

                                var $op = jcfg_findOperatorById(OpId_InLocalBase($idx));
                                //alert(jcfg_getCommission((OpId_InLocalBase($idx))));
                                var opNme = $op["name"];
                                var opImg = $op["image"];

                                var $iii = MJGA($go_operator_icon.i);
                       					$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+pathToOperators+detOpIco(opImg)+'\', sizingMethod=\'scale\')';
                                //$iii.src = $mj_g_img_pth+pathToOperators+opImg;

                                Parameters['recepient'] = OpId_InLocalBase($idx);
                                isOperator999 = 1;
                                $gs_global_check_handler = 'fields_preCheck()';
                                $skp_comm = SKP('VIEW_ACCOUNT_DETAILS_FCOMMENT');
                                
                                var localCms = jcfg_getCommission(OpId_InLocalBase($idx));
																if (isArray(localCms)) {
																	$cms = '<p><span style="font-size:'+$skp_comm.comissionSize+'px">'+LCP('commission_prefix')+'</span>';
																	for (k in localCms) {
																			if (k!=0) $cms+=', ';
																			$cms+= '<nobr><font color="'+$skp_comm.comissionColor+'"><span style="font-size:'+$skp_comm.comissionSize+'px">'+localCms[k]['val']+'</font>';
																			if (localCms[k]['txt']!='') $cms+=' от '+localCms[k]['txt']+'</nobr>';
																	}	
																	$cms+='</p>'
																} else $cms = LCP('commission_none');
                        }
                        else {
                                $g_forced_mobile_selection = true;
                                $ae['pass'] = 1;
                                $gs_global_check_handler = 'force_select_mobile_recepient()';

                                var $iii = MJGA($go_operator_icon.i);
                                //$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+pathToOperators+'logo_no_oper.png\', sizingMethod=\'scale\')';
                                $iii.src = $mj_g_img_pth+pathToOperators+'logo_no_oper.png';
																
																$cms = '<p>'+LCP('commission_variable')+'</p>';
                        }
                        
                }
                else {
                        $cms = '<p>'+LCP('commission_variable')+'</p>';
                        $ae['pass'] = 0;
                        var $iii = MJGA($go_operator_icon.i);
                        $iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+pathToOperators+'mob_999.png\', sizingMethod=\'scale\')';
                        //$iii.src = $mj_g_img_pth+pathToOperators+'mob_999.png';
                }
                
                var $online = '';
        				var $OpId = 999;
        				var $op = jcfg_findOperatorById($OpId);

				        if($op !== '-1') {
				        		if($op['processor']['offline'] === '0' && "cyberplat_mt"!=$op['processor']['type'].toLowerCase()) {
						          	$online = '<p>'+LCP('comment_online')+'</p>';
				    	      }
				        }
				        
				        mj_G._a[$ae['commf'].i]['i'].innerHTML = $ae['commft'] + $online + $cms;

        }
        PlaySoundA('click1');
        verify_fields();
        //  - - - - - - - update field text
        //mj_G._a[$o_number.i]['i'].innerHTML = $s_r;
        
        skp = changeFontSize($fields_count, $s_r.length);
        if ($s_r.length > 45 ) $s_r = $s_r.substr(0,45)+'…';
        change_srt_text($ga_fields[$g_fields_active]['txi'],  $s_r, skp);
		field_blink();
        setTimeout("nextActivateField()",100);
}
function changeFontSize(fieldCount, stringLength) {
	var skp = SKP('VIEW_ACCOUNT_DETAILS_FIELD_COPY');
	var skpLocalFont = skp.f;
	
	var word_h = {
		'single': {
			0: {'entry': 10, 'normal': 17},
			1: {'entry': 13, 'normal': 20},
			2: {'entry': 21, 'normal': 30}
		},
		'some': {
			0: {'entry': 13, 'normal': 20},
			1: {'entry': 21, 'normal': 30}
		},
		'much': {
			0: {'entry': 15, 'normal': 20},
			1: {'entry': 19, 'normal': 30}
		}
	};
	
	var size = 'normal';
	if (((($mj_g_docnme == 'pb.html') && (getParameter('pb') == 'dataentry')) || ($mj_g_docnme == 'data-entry.html')) &&
		((parseInt($g_fields_active) >= 0) && ($g_fields_active != 'undefined'))) {
			if (($ga_fields[$g_fields_active]['t'] == 't') || ($ga_fields[$g_fields_active]['t'] == 'tp'))
				size = 'entry';
	}

	if (fieldCount == 1) {
			var hs = word_h['single'];
			if (stringLength <= hs[0][size]) {
					skpLocalFont[1] = 54; 
					skpLocalFont[5] = 0;
			} else if (stringLength > hs[0][size] && stringLength <= hs[1][size]) {
					skpLocalFont[1] = 42; 
					skpLocalFont[5] = 8;
			} else if (stringLength > hs[1][size] && stringLength <= hs[2][size]) {
					skpLocalFont[1] = 27; 
					skpLocalFont[5] = 15;
			} else {
					skpLocalFont[1] = 19; 
					skpLocalFont[5] = 22;
			}
	} else if (fieldCount < maxFields) {
			var hs = word_h['some'];
			if (stringLength <= hs[0][size]) {
					skpLocalFont[1] = 42; 
					skpLocalFont[5] = 8;
			} else if (stringLength > hs[0][size] && stringLength <= hs[1][size]) {
					skpLocalFont[1] = 27; 
					skpLocalFont[5] = 15;
			} else {
					skpLocalFont[1] = 19; 
					skpLocalFont[5] = 22;
			}
	} else {
			var hs = word_h['much'];
			if (stringLength <= hs[0][size]) {
					skpLocalFont[1] = 27; 
					skpLocalFont[5] = 0;
			} else if (stringLength > hs[0][size] && stringLength <= hs[1][size]) {
					skpLocalFont[1] = 21; 
					skpLocalFont[5] = 5;
			} else {
					skpLocalFont[1] = 14;	
					skpLocalFont[5] = 10;	
			}
	}
	
	return skpLocalFont;

}
/*
function cvp_PP_PinBack() {
        for (var $i in $cvp_PP_apin) {
                if (0 == $cvp_PP_apin[$i][0]) {
                        mj_G._a[$i]['o'].Visi(1, 1);
                        mj_G._a[$cvp_PP_apin[$i][1]]['o'].Visi(1, 0);
                        $cvp_PP_apin[$i][0] = -1;
                }
                else {
                        $cvp_PP_apin[$i][0]--;
                }
        }
}

mj_RT.Add('cvp_PP_PinBack', 10, 0);
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function force_select_mobile_recepient() {
        /*
           var cvp_KP = $g_vpa[$__v]['v'];
    var $__s = $g_vpa[$__v]['s'];
    */
     // $g_vpa[$g_current_vpa]['v'].Show(0, 1);

     // create_vpa("VIEW_OPERATORS", 'create_submenu_icons', '101', '-1');
     route_to_vp("VIEW_OPERATORS", 'create_submenu_icons', '101', $g_current_vpa);
     // alert('created!');
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


var $keybs = {
        i:[],        // key images layout by [rows][cols](l1,l2,l3,l4) - array with buttons length (in )
                        //                                                          [acclen] - accumulated length of the row (in buton's dimensions)

        a:[],        // symbolic keys assocs by [layout][numkey][c] - char
                        //                                                                                   [t] - title
        o:[],         // [numkey] gfx objects associated to each key index
        r:[],         // rect inex for change_rect_text
        l:{
                lat:
                [
                '1234567890-/11',
                '@qwertyuiop[]',
                '22asdfghjkl;\'',
                '33zxcvbnm,.\\55',
                '44        '
                ],
                rus:
                [
                '1234567890-/11',
                'ёйцукенгшщзхъ',
                '22фывапролджэ',
                '33ячсмитьбю.55',
                '88        '
                ],
                latc:
                [
                '1234567890-/11',
                '@QWERTYUIOP[]',
                '77ASDFGHJKL;\'',
                '33ZXCVBNM,.\\55',
                '44        '
                ],
                rusc:
                [
                '1234567890-/11',
                'ЁЙЦУКЕНГШЩЗХЪ',
                '77ФЫВАПРОЛДЖЭ',
                '33ЯЧСМИТЬБЮ,55',
                '88        '
                ]
        }
};



var $g_layout = null;


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function parse_keyboard_layouts() {
        // $keybs

        // - - - - - calculating 'i' part of keyboard layots
        var $kk = $keybs['lat'];

        // key associations calculations
        for (var $layout in $keybs['l']) {
                //  - - - - by layouts
                // alert($layout);
        var $kk = $keybs['l'][$layout];
        var $abs_butpos = 0;
        $keybs['a'][$layout] = new Array();
                for (var $ir=0; $ir<$kk.length; $ir++) {
                        $x = 0;


                        // by 'lat' - get da keyboard's pics allocation
                        var $row_length_butims = 0;
                        if ($layout === 'lat') {
                                $keybs['i'][$ir] = new Array();
                                $keybs['i'][$ir]['cols'] = new Array();
                        }

                        var $butpos_inrow = 0;
                        var $spos = 0;
                        var $lastchar = -1;
                        var $butl = 1;

                        while ($spos < $kk[$ir].length) {
                                $lastchar = $kk[$ir].charAt($spos);
                                if ($lastchar != $kk[$ir].charAt($spos+1)) {
                                        // store button
                                        $keybs['a'][$layout][$abs_butpos] = new Array();
                                        if ($butl>1) {
                                                var $imt = '???';
                                                switch($lastchar) {
                                                        case '1': $imt = '<font face="Wingdings">&#223;</font>&nbsp;стереть'; break;
                                                        case '2': $imt = 'CAPS'; break;
                                                        case '3': $imt = '<font face="Wingdings">&#241;</font>&nbsp;Shift'; break;
                                                        case '4': $imt = 'РУС'; break;
                                                        case '5': $imt = '<font face="Wingdings">&#237;</font><br>Ввод'; break;
                                                        case '7': $imt = 'CAPS'; break;
                                                        case '8': $imt = 'ENG'; break;
                                                        default : $imt = 'ПРОБЕЛ'; break;
                                                }
                                                $keybs['a'][$layout][$abs_butpos]['c'] = '_'+$lastchar;        // служебная клавиша
                                                $keybs['a'][$layout][$abs_butpos]['t'] = $imt;
                                        }
                                        else {
                                                $keybs['a'][$layout][$abs_butpos]['c'] = $lastchar;
                                                $keybs['a'][$layout][$abs_butpos]['t'] = $lastchar;
                                        }

                                        if ($layout === 'lat') {
                                                $keybs['i'][$ir]['cols'][$butpos_inrow] = $butl;
                                                $butpos_inrow++;
                                                $row_length_butims += $butl;
                                        }

                                        $x+=$butl;
                                        $butl = 1;
                                        $abs_butpos++;

                                }
                                // increase button size
                                else {
                                        $butl++;
                                }

                                $spos++;

                    } // - - - - - row by chars
                    // MjCRIODA($keybs['i'][$ir]['cols']);
                    if ($layout === 'lat') {
                            $keybs['i'][$ir]['acclen'] = $row_length_butims;
                            // alert($keybs['i'][$ir]['acclen']);
                    }


                } // - - - - - layout by rows

        } // - - - - - by layouts

        $g_layout = 'rus';

        var $recepient_id = getParameter('recepient','-1')
        if ($recepient_id >= 0){
                // type of keyboard for text field
				if('undefined' != typeof(jcfg_findOperatorById($recepient_id)['fields'][$ga_fields[$g_fields_active]['id']]))
				{
					var $klava = jcfg_findOperatorById($recepient_id)['fields'][$ga_fields[$g_fields_active]['id']]['klava'];
					if ('undefined' !== $klava) {
							for (var $layout in $keybs['l']){
								  // if exists $klava in layers of keyboard
								  if($klava === $layout){
										 $g_layout = $klava;
										 break;
								  }
							}
					}
				}
        }


        // return;


}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// correct keyboard item with current locale
function keyboard_layout_redraw() {
        var $l = $keybs['o'].length;
        for (var $i=0; $i<$l; $i++) {
        	change_srt_text($keybs['r'][$i], $keybs['a'][$g_layout][$i]['t']);
        }


}



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function hideKeyboard($__v) {
		keybordActive = false;
		$g_vpa[$__v]['v'].Show(0, 1);
		
		if ($fields_count < maxFields)  $skc =SKP('VIEW_ACCOUNT_DETAILS_FIELD');
		else $skc = SKP('VIEW_ACCOUNT_DETAILS_FIELD_SMALL');
				
		for (i=0; i<$ga_fields.length; i++) {
				$ae = $ga_fields[i];
				$ae['txb'].OverloadDime($skc.r[2], $skc.r[3]);
		}
		
   	var $ae = $ga_fields[$g_fields_active];
 		var $iii = MJGA($ae['txb'].i);
 		$iii.src = $mj_g_img_pth+$skp.i; // серый		
		$ae['was_activated'] = 1;
}

function create_keyboard($__v) {
				keybordActive = true;
				pinpadActive = false;
        // alert('create_keyboard');
        if ($g_layout === null) {
                parse_keyboard_layouts();
        }

        // $g_vpa[$g_current_vpa]['v'].Show(0, 1);

        var cvp_KP = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];
        create_field_comment($__v, -450);

        var $title = $ga_fields[$g_fields_active]['title'];
				if ((getParameter('recepient', 'undefined') == '2500') && ($ga_fields[$g_fields_active]['id'] == 'comment_pb'))
				$title = LCP('comment_pb'); 
        set_rect_text(cvp_KP, [0, 60, $__s.r[2], 35], $__s, $title+':' );



                // alert('draw_keyboard...');
        var $y = 0;
        var $xo = 50;
        var $yo = 220;
        var $kw = 64;
        var $kh = 64;

        var $__s1 = SKP('BUTKEYBOARD');

        // draw the keyboard
        var $abs_butpos = 0;
        var keypad = SKP('KEYPAD');
        for (var $ir=0; $ir<$keybs['i'].length; $ir++) {
                // alert($keybs['i'][$ir]['acclen']);
                var $x = 0;
                var $xo_add = 0
                if ($ir==0) $xo_add = 0;
                else if ($ir==1) $xo_add = $kw/2;
                else if ($ir==2) $xo_add = 0;
                else if ($ir==3) $xo_add = $kw/2;
                else $xo_add = $kw;
                //$xo_add = (900 - ($keybs['i'][$ir]['acclen']*64)) / 2;
                for (var $ic=0; $ic<$keybs['i'][$ir]['cols'].length; $ic++) {
		                var $imi = '';
                    var $butl = $keybs['i'][$ir]['cols'][$ic];
                    switch ($butl) {
    		                case 1: $imi=keypad.keypad1; break;
                        case 2: $imi=keypad.keypad2; break;
                        case 4: $imi=keypad.keypad3; break;
                        case 8: $imi=keypad.keypad4; break;
                        case 9: $imi=keypad.keypad5; break;
                        default: $imi=keypad.keypad1; break;
                    }
                    if ($keybs['a'][$g_layout][$abs_butpos]['c'] == '_1') {
       	          		var $o = cvp_KP.AddI(keypad.keypad3, $xo + $xo_add + $kw*$x, $yo + $kh*$ir);
       	        			$o.SizeTo(0, $kw*$butl, $kh);
       	        			var $ri = set_rect_text(cvp_KP, [$xo + $xo_add + $kw*$x, $yo + $kh*$ir+14, $kw*$butl, $kh], $__s1, $keybs['a'][$g_layout][$abs_butpos]['t']);
               				$o.AddHandler('cvp_KP_PinPress("'+$__v+'", "'+$abs_butpos+'", "'+$ri+'");', $kw*$butl, $kh);	
                  	}else if ($keybs['a'][$g_layout][$abs_butpos]['c'] == '_5') {
                  		var $o = cvp_KP.AddI(keypad.keypad5, $xo + $xo_add + $kw*$x, $yo + $kh*$ir);
                  		$o.SizeTo(0, 96, 128);
                  		var $ri = set_rect_text(cvp_KP, [$xo + $xo_add + $kw*$x-10, $yo + $kh*$ir+60, $kw*$butl, $kh], $__s1, $keybs['a'][$g_layout][$abs_butpos]['t']);
                  		$o.AddHandler('cvp_KP_PinPress("'+$__v+'", "'+$abs_butpos+'", "'+$ri+'");', 96, 128);
                  	}else if ($keybs['a'][$g_layout][$abs_butpos]['c'] == '_ ') {
       	          		var $o = cvp_KP.AddI($imi, $xo + $xo_add + $kw*$x+3*$kw/4, $yo + $kh*$ir);
       	        			$o.SizeTo(0, $kw*$butl, $kh);
       	        			var $ri = set_rect_text(cvp_KP, [$xo + $xo_add + $kw*$x+3*$kw/4, $yo + $kh*$ir+14, $kw*$butl, $kh], $__s1, $keybs['a'][$g_layout][$abs_butpos]['t']);
               				$o.AddHandler('cvp_KP_PinPress("'+$__v+'", "'+$abs_butpos+'", "'+$ri+'");', $kw*$butl, $kh);	
                  	}
                    else {
       	          		var $o = cvp_KP.AddI($imi, $xo + $xo_add + $kw*$x, $yo + $kh*$ir);
       	        			$o.SizeTo(0, $kw*$butl, $kh);
       	        			var $ri = set_rect_text(cvp_KP, [$xo + $xo_add + $kw*$x, $yo + $kh*$ir+14, $kw*$butl, $kh], $__s1, $keybs['a'][$g_layout][$abs_butpos]['t']);
               				$o.AddHandler('cvp_KP_PinPress("'+$__v+'", "'+$abs_butpos+'", "'+$ri+'");', $kw*$butl, $kh);	
       	        		}
             				
                 		$keybs['o'][$abs_butpos] = $o;
                 		$keybs['r'][$abs_butpos] = $ri;		

            
                    $abs_butpos++;
                    $x += $butl;
                }
        }


        // - - - - - set the field values
        var $ae = $ga_fields[$g_fields_active];
        var $s_def = $ae['val'];

        var tmp = '';
 				if ($ae['t']=='tp') for (var i=0; i<$s_def.length; i++) tmp +='•';
 				if (tmp!='') $s_def = tmp;

        $o_number_k = cvp_KP.AddD(50, 130);
        $o_number_k.SizeTo(1, $__s.r[2]-100, 50);
        var $o = $o_number_k;

        var $spb = SKP('BUTKEYBOARD');
				
        mj_G._a[$o.i]['i'].style["fontFamily"] = $spb.f[0];
        mj_G._a[$o.i]['i'].style["fontSize"] = '50px';
        mj_G._a[$o.i]['i'].style["fontWeight"] = $spb.f[2];
        mj_G._a[$o.i]['i'].style["color"] = '#000000';
        mj_G._a[$o.i]['i'].style["textAlign"] = $spb.f[4];


        if ($s_def.length > 18) {
        	mj_G._a[$o_number_k.i]['i'].style["fontSize"] = '35px';
        	mj_G._a[$o_number_k.i]['i'].style["paddingTop"] = '10';
        } else {
        	mj_G._a[$o_number_k.i]['i'].style["fontSize"] = '50px';
        	mj_G._a[$o_number_k.i]['i'].style["paddingTop"] = '0';
      	}
				if ($s_def.length > 45 ) $s_def = $s_def.substr(0,45)+'…';        
				
        mj_G._a[$o_number_k.i]['i'].innerHTML = $s_def; // *(***)***-**-**
        change_srt_text($ga_fields[$g_fields_active]['txi'],  $s_def);

                // alert('draw_keyboard done.');

}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function keyboard_switch_caps() {
        var $bloc = $g_layout.substr(0,3);
        var $addl = $g_layout.substr(3);
        if ($addl === 'c') {
                $addl = '';
        }
        else {
                $addl = 'c';
        }
        $g_layout = $bloc+$addl;
        keyboard_layout_redraw();
}

var g_keyb_inshift = 0;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// $__v = viewport
// $__i = key absolute index
// $__r = rect index
var keybordActive = false;
/*
function keyboardBack() {
		if (!keybordActive) return
		else {
			var $l = $keybs['o'].length;
	    for (var $i=0; $i<$l; $i++) {
	    		$keybs['o'][$i].Visi(1, 1);
	    }
	  }
    
}
mj_RT.Add('keyboardBack()', 20, 0);
*/
function cvp_KP_PinPress($__v, $__i, $__r) {
        var $o = $keybs['o'][$__i];
      	$o.Visi(1, 0);
		if ($__i != '49')
			setTimeout('$keybs[\'o\'][' + $__i + '].Visi(1, 1);', KPButtonShowDelay);

        var $ae = $ga_fields[$g_fields_active];
        $s_number = $ae['val'];
        $s_r = $s_number;
        
 				var tmp = '';
 				if ($ae['t']=='tp') for (var i=0; i<$s_r.length; i++) tmp +='•';
 				if (tmp!='') $s_r = tmp;
 				
        var $c = $keybs['a'][$g_layout][$__i]['c'];
        if (($c == '_6') || ($c == '_ ')) $c = ' ';

        if ($c.length > 1) {
                switch($c.substr(1,1))
                {
                        // - - - - - del
                        case '1':
				                $s_r = $s_r.substring(0, $s_r.length-1);
				                $s_number = $s_number.substring(0, $s_number.length-1);
        	        			break;

                        // - - - - - caps
                        case '2':
                        // alert('wow!');
            						keyboard_switch_caps();
									break;
                        break;
                        
                        case '7':
                        // alert('wow!');
            						keyboard_switch_caps();
									break;
                        break;

						            // - - - - - shift
						            case '3':
						            keyboard_switch_caps();
						            g_keyb_inshift = 1;
						            break;

                        // - - - - - rus/lat
                        case '4':
                        var $bloc = $g_layout.substr(0,3);
                        var $addl = $g_layout.substr(3);
                        if ($bloc === 'rus') $bloc = 'lat';
                        else $bloc = 'rus';
                        $g_layout = $bloc+$addl;
                        keyboard_layout_redraw();
                        break;
                        
                        case '8':
                        var $bloc = $g_layout.substr(0,3);
                        var $addl = $g_layout.substr(3);
                        if ($bloc === 'rus') $bloc = 'lat';
                        else $bloc = 'rus';
                        $g_layout = $bloc+$addl;
                        keyboard_layout_redraw();
                        break;

            						// - - - - - clear
                        case '5':
                				hideKeyboard($__v);
                				break;
                				
                        // - - - - - default
                        default:
                        alert($c);
                        break;
                }
        }
        else {
						$s_number += $c;
        				$s_r += $c;
                if (g_keyb_inshift) {
                        keyboard_switch_caps();
                        g_keyb_inshift = 0;
                }
        }

 				tmp = '';
 				if ($ae['t']=='tp') for (var i=0; i<$s_r.length; i++) tmp +='•';
 				if (tmp!='') $s_r = tmp;
        if ($s_r.length > 0) $ae['pass'] = 1;
        else $ae['pass'] = 0;
		if($mj_g_docnme == 'pb.html')
		    $ae['pass'] = 1;
        $ae['val'] = $s_number;

        skp = changeFontSize($fields_count, $s_r.length);
        if ($s_r.length > 18) {
        	mj_G._a[$o_number_k.i]['i'].style["fontSize"] = '35px';
        	mj_G._a[$o_number_k.i]['i'].style["paddingTop"] = '10';
        } else {
        	mj_G._a[$o_number_k.i]['i'].style["fontSize"] = '50px';
        	mj_G._a[$o_number_k.i]['i'].style["paddingTop"] = '0';
      	}
				if ($s_r.length > 45 ) $s_r = $s_r.substr(0,45)+'…';        
        //alert($s_r);
        
        mj_G._a[$o_number_k.i]['i'].innerHTML = $s_r;
        change_srt_text($ga_fields[$g_fields_active]['txi'],  $s_r, skp);
        //  - - - - - - - update field text
        
        verify_fields();
      	if ($c.substr(1,1)=='5')	{
	      	for (k in $ga_fields) {
						if ($ga_fields[k]['pass']!=1 && parseInt(k)!=$g_fields_active) {
							activate_field(k);
							break;
						}
					}
				}

}


function nectActivateField() {
	var localId = -1;
	for (k in $ga_fields) {
		if ($ga_fields[k]['pass']!=1 && parseInt(k)!=$g_fields_active) {
			localId = k;
			break;
		}
	}
	return localId;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_error($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        var opId = Parameters['recepient'];
        var $op = jcfg_findOperatorById(opId);
        var $pr = $op['processor']['type'];

				
        $ga_buttbig[0]['v'].Show(1, 1);
        changeButtonImage($ga_buttbig[0]['o'], 'back');
        if ($pr.toLowerCase() == 'cyberplat_taxes') {
        	$ga_buttbig[0]['o'].AddHandler('goto_Loc("data-entry.html?recepient='+opId+'")');
        } else {
        	isOperator999 = getParameter('isOperator999', 0);
        	if (isOperator999 == 1) Parameters['recepient'] = 999;
      		$ga_buttbig[0]['o'].AddHandler('goto_Loc("data-entry.html?'+goto_get_allurlparams()+'")');
      	}

        if ($pr.toLowerCase() === 'Cyberplat_PIN') {
                var $txt = LCP('card_amount_notexists');
                set_rect_text($v, [40, 0, 660, 621], $__s, $txt, true);
        }
        else if ('avia_center'==$pr.toLowerCase()) {
                txt = '';
                var tmp = getParameter('error', -1);
                if(33==parseInt(tmp)) $txt=getParameter('msg', '');
                else 	$txt=GetMessageDbError(getParameter('error', -1));
        				set_rect_text($v, [40, 0, 660, 621], $__s, $txt, true);
        }
        else set_rect_text($v, [40, 0, 660, 621], $__s, GetMessageDbError(getParameter('error', -1)), true);
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_thanks($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        var opId = Parameters['recepient'];
        var $op = jcfg_findOperatorById(opId);
        var $pr = $op['processor']['type'];
        var $cmd = getParameter('cmd', 'none');
        var $rest = getParameter('rest', '');
        var $end = getParameter('end', 'none');
		var $sum = parseFloat(getParameter('sum', '0'));
		var $cms = parseFloat(getParameter('cms', '0'));
		var $mps = parseFloat(getParameter('mps', '0'));
		var $maxsum = parseFloat(getParameter('maxsum', '0'));

				if("undefined" == typeof($ga_buttbig[0]))
            create_bigbut(-2);
		
				if("undefined" == typeof($ga_buttbig[1]))
            create_bigbut(-3);

				if("cyberplat_mt"==$pr.toLowerCase()){
        		if('askuser'!=$cmd && 'notsend'!=$cmd && 'send'!=$cmd  && 'askretry'!=$cmd && '1'!=$end && 'retry'!=$cmd && 'clear'!=$cmd){
        				set_rect_text($v, [5, 100, $__s.r[2], 70], $__s, LCP('thank_you_mt'));
        		} else {
        				set_rect_text($v, [5, 10, $__s.r[2], 70], $__s, "");
        		}
        }else if('askretry'!=$cmd && 'retry'!=$cmd && 'askuser'!=$cmd){
        	//set_rect_text($v, [5, 10, $__s.r[2], 70], $__s, LCP('thank_you'));
        } else{
        	set_rect_text($v, [5, 10, $__s.r[2], 70], $__s, "");
        }
        $__s1 = SKP('VIEW_TEXT2');

        var arData = new Array();

        if ('undefined' === typeof(Parameters['processing'])) {
        				//alert('!');
        				
                if('showinfo'!=$cmd && 'askuser'!=$cmd && 'askretry'!=$cmd && 'retry'!=$cmd){
                        //$v.AddI('printer1.gif', ($__s.r[2]-184)/2, GTY(350));
                }
                // asking 4 retry // cmd=askretry

                //  - - - - - сообщение юзеру
                var $msg = getParameter('msg', '');
                var $txt = '';

                if ( ($g_is_metro_recepient) && ('stored' == $msg) ) {
                        // ('undefined' !== typeof($ga_metro_menu['Message']))
                        //alert('Here!');
                        $txt = $ga_metro_menu['Message'];
                }
                else {
                        $txt = $msg;
                }

                //  - - - - - ask retry?
                if ('askretry' === $cmd) {
                        // если все-таки msg пуст....
                if ($txt === '') {
		        				if("cyberplat_mt"==$pr.toLowerCase()) $txt = LCP('ask_retry_mt');
                    else $txt = LCP('ask_retry_payment');
                }
                  $ga_buttbig[1]['v'].Show(1, 1);
                	//change_srt_text($ga_buttbig[1]['txi'], LCP('button_yes'));
                	changeButtonImage($ga_buttbig[1]['o'], 'yes');
                	$ga_buttbig[1]['o'].AddHandler('goto_Loc("paymentcomplete.html?recepient='+opId+'&cmd=retry")');

                  $ga_buttbig[0]['v'].Show(1, 1);
                	//change_srt_text($ga_buttbig[0]['txi'], LCP('button_no'));
                	changeButtonImage($ga_buttbig[0]['o'], 'no');
        					if("cyberplat_mt"==$pr.toLowerCase()){
                        	$ga_buttbig[0]['o'].AddHandler('goto_Loc("PaymentComplete.html?recepient='+opId+'&cmd=askuser")');                        	
                  }else{
                   		   	$ga_buttbig[0]['o'].AddHandler('goto_Loc("paymentcomplete.html?recepient='+opId+'&cmd=cancel")');
                  }
                  mj_RT.Add('goto_Loc("paymentcomplete.html?recepient='+opId+'&cmd=cancel")', 60000, 0);
                	new_vpa="-VIEW_OP_MARK3_WITHOUT_FLASH"
                }
            	else if('askuser' == $cmd){
                    	$txt=login_info['first_name']+" "+login_info['middle_name']+",<br> к сожалению, в данный момент отсутствует связь с сервером системы. <br><br>Вы можете выбрать 'Далее' для того, чтобы перевод был отправлен в систему автоматически после восстановления связи либо 'Назад' для того, чтобы отменить перевод и вернуть деньги. Для возврата денег, необходимо обратиться в отделение банка, адрес которого находится на чеке. <br>Пожалуйста, выберите 'Далее' или 'Назад'";
                			$ga_buttbig[1]['v'].Show(1, 1);
                			changeButtonImage($ga_buttbig[1]['o'], 'next');
                    	$ga_buttbig[1]['o'].AddHandler('goto_Loc("paymentcomplete.html?cmd=send&recepient=' + opId + '")');

                			$ga_buttbig[0]['v'].Show(1, 1);
                			changeButtonImage($ga_buttbig[0]['o'], 'back');
                    	$ga_buttbig[0]['o'].AddHandler('goto_Loc("paymentcomplete.html?cmd=notsend&recepient=' + opId + '")');
                    	mj_RT.Add('goto_Loc("paymentcomplete.html?cmd=send&processing=1', 60000, 0);
            	}
            	else if('' != $rest){
                        var $resttxt = LCP('processrest1') + ':&nbsp;<font style="font-size:larger;color:red">' + $rest + '</font> ' + $ga_jcfg['config']['CurrencyName'] + '<br><br>' + LCP('processrest2');
                        //set_rect_text_in_textArea($v, [10, 75, 680, 80], $__s1, $resttxt, data);
                        set_rect_text($v, [0, 50, 744, 70], $__s1, $resttxt);
                        //alert($resttxt);
                        
                        //Yes: data-entry.html
                        $ga_buttbig[1]['v'].Show(1, 1);
                		    changeButtonImage($ga_buttbig[1]['o'], 'yes');
                		    $ga_buttbig[1]['o'].AddHandler('goto_Loc("data-entry.html?recepient=999")');
                	    
                		    //No: main.html
                        $ga_buttbig[0]['v'].Show(1, 1);
                		    changeButtonImage($ga_buttbig[0]['o'], 'no');
                		    $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');
                	    
                	   	mj_RT.Add('goto_Main', 20000, 0);
            	}
                else if('showinfo'==$cmd){
                        var data=getData();
                        data=data.replace(/\[T]/g,"\t");
                        data=data.replace(/\[CR]/g,"");
                        data=data.replace(/\[LF]/g,"\n");                
                        arData=data.split("\n");
                        if ($txt === '') {
                                $txt = LCP('take_tha_rezippt');
                        }
                        $ga_buttbig[1]['v'].Show(1, 1);
                        //change_srt_text($ga_buttbig[1]['txi'], '');
                        changeButtonImage($ga_buttbig[1]['o'], 'ok');
                        $ga_buttbig[1]['o'].AddHandler('goto_Loc("main.html")');
                        PlaySoundA('rezzipt');
                        //('goto_Main', 30000, 0);
                        set_rect_text_in_textArea($v, [15, 0, 680, 0], $__s1, $txt,data);
                        initArea("tArea",arData);
                				new_vpa="-VIEW_OP_MARK3_WITHOUT_FLASH";
                }
                else if('retry' == $cmd){
                	if("cyberplat_mt"==$pr.toLowerCase()) $txt = LCP('please_wait_proc_payment_mt');
                	else $txt = LCP('please_wait_proc_payment');
                	$__s1.f[3] = '#ff3333';
                	new_vpa="-VIEW_OP_MARK3_WITHOUT_FLASH";
                }
                else {
                				new_vpa="-VIEW_OP_MARK3_WITHOUT_FLASH";
                        // если все-таки msg пуст....
                        if ($txt === '') {
                                $txt = LCP('take_tha_rezippt');
                        }
						var is_payment_will_pass = ($mps == $maxsum) ? ($sum >= $maxsum) : (($sum - $cms) >= $mps);
						if ((!is_payment_will_pass) && 
							(!((getParameter('pb', 'undefined') == 'error') && (getParameter('errornumber', '-1') == '-8'))))
								$txt = LCP('no_payment_go_support');
                        $ga_buttbig[1]['v'].Show(1, 1);
                        //change_srt_text($ga_buttbig[1]['txi'], 'OK');
                        changeButtonImage($ga_buttbig[1]['o'], 'ok');
                        $ga_buttbig[1]['o'].AddHandler('goto_Loc("main.html")');

                        PlaySoundA('rezzipt');
                        mj_RT.Add('goto_Main', 10000, 0);
                }

        }
        else {
                //убираем кнопки, если они есть
                $ga_buttbig[0]['v'].Show(0, 1);
                $ga_buttbig[1]['v'].Show(0, 1);
        				
                var $txt = LCP('please_wait_proc_payment');
                if("cyberplat_mt"==$pr.toLowerCase()){
                	$txt = LCP('please_wait_proc_payment_mt');
                }
                $__s1.f[3] = '#ff3333';
        }
				if('askuser'==$cmd){
            set_rect_text($v, [15, 35, 700, 70], $__s, $txt);
				}
        else if('showinfo'!=$cmd){
            set_rect_text($v, [35, 110, 600, 70], $__s1, $txt);
        }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_termerror($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        create_bigbut(-2);
        create_bigbut(-3);

        set_rect_text($v, [5, 15, $__s.r[2], 70], $__s, LCP('out_of_service_header'));

        $__s1 = SKP('VIEW_TEXT1');
        $__s1.f[3] = '#ff3333';

        var $txt = LCP('out_of_service_message');
        set_rect_text($v, [25, 210, 600, 70], $__s1, $txt);

}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_complat($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        //MjCRIODA($__s);
        var $OpId = getParameter('recepient',-1);
        var $o = $v.AddI(pathToOperators+'plategka'+$OpId+'.jpg', 0, 0);
        $o.MoveTo(1, $__s.r[0]+($__s.r[2]-$o.w)/2, $__s.r[1]+($__s.r[3]-$o.h)/2);

        bigbutA('left',LCP('button_back'),'goto_Main()');
        bigbutA('right',LCP('button_next'),'goto_Loc("data-entry.html?recepient='+$OpId+'")');

        mj_RT.Add('goto_Main', 30000, 0);
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_message($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        create_bigbut(-2);
        create_bigbut(-3);

        // no quitance!!!
        // buttons!!!

        var $printer_ok_only = false;
        var $op = jcfg_findOperatorById(getParameter('recepient', -1));
        if (($op !== -1) && ($op['printer_ok_only'] == 1)) {
                $printer_ok_only = true;
        }
		var error = getParameter('error', 0);
		var $txt = '';
		if ((error == '-9') || 
			(error == '-10') ||
			(error == '-11')) {
				$txt = GetMessageDbError(error);
				var $s = '';
				for ($k in Parameters) {
				if (($k != 'act') && 
					($k != 'error') &&
					($k != 'errornumber') &&
					($k != 'pb')
					)
					$s += (($s !== '') ? '&' : '') + $k+'='+Parameters[$k];
				}
				timerMainPage = setTimeout('goto_Loc("Payment.html?end=1&'+$s+'")',5000);
		} else {
			if (!$printer_ok_only) {
                var $txt = LCP('prn_cantprint_message_message');

                $ga_buttbig[1]['v'].Show(1, 1);
                changeButtonImage($ga_buttbig[1]['o'], 'yes');
                $ga_buttbig[1]['o'].AddHandler('goto_Loc("message.html?res=1")');
	        }
	        else
                $txt = LCP('prn_cantprint_message_fail');

                $ga_buttbig[0]['v'].Show(1, 1);
                changeButtonImage($ga_buttbig[0]['o'], 'no');
                $ga_buttbig[0]['o'].AddHandler('goto_Main()');
        }

        set_rect_text($v, [40, 0, 660, 621], $__s, $txt, true);
		
		var addInfo = getParameter("showAddInfo","0");
		if (addInfo == "1"){
			$ga_buttbig[1]['o'].AddHandler('goto_Loc("addinfo.html?'+goto_get_allurlparams()+'")');
		}

		
        mj_RT.Add('goto_Main', 60000, 0);


}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_addinfo($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        create_bigbut(-2);
        create_bigbut(-3);

        set_rect_text($v, [5, 15, 690, 70], $__s, LCP('attention'));


        $__s1 = SKP('VIEW_TEXT1');
				$__s1.f[1] = '15';
        var $txt = '';
        var k = '';
        
        
        for (k in $ga_taxes['tax_data']) $txt+=$ga_taxes['tax_data'][k]['name']+':'+$ga_taxes['tax_data'][k]['val']+'<br><br>';
        
        set_rect_text($v, [5, 70, 690, 70], $__s1, $txt);

        var $rc = getParameter('recepient', '0');
        $gotodoc = 'payment.html?'+document.location.search;
        
        $ga_buttbig[1]['v'].Show(1, 1);
        change_srt_text($ga_buttbig[1]['txi'], '');
        $ga_buttbig[1]['o'].AddHandler('goto_Loc("'+$gotodoc+'")');

        $ga_buttbig[0]['v'].Show(1, 1);
        change_srt_text($ga_buttbig[0]['txi'], '');
        $ga_buttbig[0]['o'].AddHandler('goto_Main()');

        mj_RT.Add('goto_Main', 10000, 0);

}
function show_message_addinfo($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        create_bigbut(-2);
        create_bigbut(-3);

        $__s1 = SKP('VIEW_TEXT1');
        $__s1.f[3] = '#ff3333';

        var $addinfodata = getData();
        $addinfodata=$addinfodata.replace(/\[T]/g,"\t");
        $addinfodata=$addinfodata.replace(/\[CR]/g,"");
        $addinfodata=$addinfodata.replace(/\[LF]/g,"\n");                
        set_rect_text($v, [25, 210, 600, 70], $__s1, $addinfodata);



        var $gotodoc = 'payment.html?'+goto_get_allurlparams();

        $ga_buttbig[1]['v'].Show(1, 1);
        //change_srt_text($ga_buttbig[1]['txi'], LCP('button_yes'));
        changeButtonImage($ga_buttbig[1]['o'], 'yes');
        $ga_buttbig[1]['o'].AddHandler('goto_Loc("'+$gotodoc+'")');

        $ga_buttbig[0]['v'].Show(1, 1);
        //change_srt_text($ga_buttbig[0]['txi'], LCP('button_no'));
        changeButtonImage($ga_buttbig[0]['o'], 'no');
        $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');

        mj_RT.Add('goto_Main', 10000, 0);

}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_nochange($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];
				var CurrentRecepient = getParameter('recepient', 'undefined');
			

        create_bigbut(-2);
        create_bigbut(-3);
				
				
        $__s1 = SKP('VIEW_TEXT1');

        var $txt = LCP('not_issue_change');
        set_rect_text($v, [0, 0, 744, 621], $__s, $txt, true);
        
        
        var $gotodoc = 'data-entry.html?recepient='+CurrentRecepient;
        
        $ga_buttbig[0]['v'].Show(1, 1);
        changeButtonImage($ga_buttbig[0]['o'], 'no');
        $ga_buttbig[0]['o'].AddHandler('goto_Main()');

        $ga_buttbig[1]['v'].Show(1, 1);
        changeButtonImage($ga_buttbig[1]['o'], 'yes');
        $ga_buttbig[1]['o'].AddHandler('goto_Loc("'+$gotodoc+'")');

        mj_RT.Add('goto_Main', 10000, 0);

}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_prnerror($__v) {
        var $rest = getParameter('rest', '');
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        //create_bigbut(-2);
        //create_bigbut(-3);

        var data=getData();
        //if(""==data) set_rect_text($v, [5, 15, $__s.r[2], 70], $__s, LCP('prn_fail_message_header'));
		    //else set_rect_text($v, [5, 15, $__s.r[2], 70], $__s, LCP('prn_avia_fail_message_header'));

        $__s1 = SKP('VIEW_TEXT1');
        $__s1.f[3] = '#ff3333';

        if($rest == '') {
            create_bigbut(-2);
            create_bigbut(-3);
            var $txt = LCP('prn_fail_message_message');
            var opId = Parameters['recepient'];
            var $op = jcfg_findOperatorById(opId);
            var $pr = $op['processor']['type'].toLowerCase();
            if("" == data || $pr != 'avia_center'){
                set_rect_text($v, [0, 0, 744, 621], $__s, $txt, true);
            }
            else{        
                data=data.replace(/\[T]/g,"\t");
                data=data.replace(/\[CR]/g,"");
                data=data.replace(/\[LF]/g,"\n");
                
                arData=data.split("\n");
                set_rect_text_in_textArea($v, [10, -10, 690, 620], $__s1, "",data);
                initArea("tArea",arData);
            }
            // printer error!!!
			timerMainPage = setTimeout("goto_Main()",3000);
            
            mj_RT.Add('goto_Main', 30000, 0);
        } else {
            var $txt = LCP('prn_fail_message_message') + '<br />' + '<br />' + LCP('not_issue_change_nospace');
            $txt += '<br><br>'+LCP('processrest1') +' '+ $rest + ' ' + $ga_jcfg['config']['CurrencyName'] +'<br>'+ LCP('processrest2');
            set_rect_text($v, [0, 70, 744, 621], $__s, $txt, true);
            
            //$__s1.f[3] = '#3E64AF';
            //var $resttxt = 
            //set_rect_text($v, [25, 320, 600, 70], $__s1, $resttxt);
            
            //Yes: data-entry.html
            $ga_buttbig[1]['v'].Show(1, 1);
            changeButtonImage($ga_buttbig[1]['o'], 'yes');
            $ga_buttbig[1]['o'].AddHandler('goto_Loc("data-entry.html?recepient=999")');
                	    
            //No: main.html
            $ga_buttbig[0]['v'].Show(1, 1);
            changeButtonImage($ga_buttbig[0]['o'], 'no');
            $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');
            
            mj_RT.Add('goto_Main', 30000, 0);
        }
}




// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_checking($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        $__s1 = SKP('VIEW_TEXT1');
        $sk = SKP('OPERATOR_ICON');

        var opId = Parameters['recepient'];
        var $op = jcfg_findOperatorById(opId);
        var $pr = $op['processor']['type'];

        var opNme = $op["name"];
        var opImg = $op["image"];

        gfxa_ins_oper_logo($v, opImg, 82, 92, $sk.d[0], $sk.d[1], opNme);
        var $valmusthave = 0;

        // ------------------------------------------------
        if ($pr.toLowerCase() === 'cyberplat_pin') {
                set_rect_text($v, [255, 100, 420, 70], $__s1, LCP('card') + ': '+unescape(Parameters['cardname']));
                set_rect_text($v, [205, 180, 400, 70], $__s1, LCP('summ') + ': '+Parameters['cardvalue']+' '+LCP('currency')[CurrencyType]);
                $valmusthave = Parameters['cardvalue'];
        }
        else if ("cyberplat_mt"==$pr.toLowerCase()) {
                set_rect_text($v, [25, 210, 650, 70], $__s1, "Пожалуйста, подождите ответа сервера.");
        }
        else {
                // GetFieldsInfo(opId);
                var $fa = new Array();
                var $ii = 0;
                for (var $ff in $op['fields']) {
                        $fa[$ii] = $op['fields'][$ff];
                        $fa[$ii]['id'] = $ff;
                        $fa[$ii]['vali'] = getParameter('field'+$ff, '');
                        if ($pr.toLowerCase() === 'cyberplat_taxes') $fa[$ii]['showval'] = $fa[$ii]['vali']
                        else $fa[$ii]['showval'] = parse_fld_name($op['fields'][$ff]);
                        $ii++;
                }
								
								var txt = '';
                for (var $ii=0; $ii<$fa.length; $ii++) {
                        if ($ii > 1) break;
                     		$fa[$ii]['name'] = $fa[$ii]['name'].replace(/\[/g, '<');
												$fa[$ii]['name'] = $fa[$ii]['name'].replace(/\]/g, '>');
                        txt += $fa[$ii]['name']+'<br>'+'<font color="'+$__s.numberColor+'">'+$fa[$ii]['showval']+'</font><br><br>'; 
                }
                set_rect_text($v, [300, 100, 440, 200], $__s, txt);
        }
}

function show_comission_in_payment($__v) {
        var opId = Parameters['recepient'];
        var $op = jcfg_findOperatorById(opId);
        var $pr = $op['processor']['type'];

        var opNme = $op["name"];
        var opImg = $op["image"];
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];
               
        $__s1 = SKP('VIEW_TEXT1');
        $sk = SKP('OPERATOR_ICON');
				
        var $x_logo_ofs = 0;
        var $y_logo_ofs = 60;

        gfxa_ins_oper_logo($v, opImg, $x_logo_ofs, $y_logo_ofs, $sk.d[0], $sk.d[1], opNme);
        
				/**Выводим номера счетов**/        
        if ($pr === 'Cyberplat_PIN') {
                set_rect_text($v, [255, 100, 420, 70], $__s1, LCP('card') + ': '+unescape(Parameters['cardname']));
                set_rect_text($v, [205, 180, 400, 70], $__s1, LCP('summ') + ': '+Parameters['cardvalue']+' '+LCP('currency')[CurrencyType]);
                $valmusthave = Parameters['cardvalue'];
        }
        else if("cyberplat_mt" == $pr.toLowerCase()){
        /*
        		var $sum = parseFloat( getParameter('sum', 0) );
        		var $mps = parseFloat( getParameter('mps', 0) );
            set_rect_text($v, [15, 100, 670, 70], $__s1, unescape(Parameters['field100']));
        		if ($sum >= $mps && $sum > 0) {
            		var $o = $v.AddI('nbut1_g0_blue.gif', 140, 220);//keypad2.gif
              	$o.AddHandler('calculate();');
                $o.SizeTo(1,400,64);
                set_rect_text($v, [140, 230, 400, 64], $__s1, 'Рассчитать комиссию');
            }
        /**/
        } 
        else {
            var $fa = new Array();
            var $ii = 0;
            $sLocal = SKP('VIEW_COMISSION_TEXT');
            for (var $ff in $op['fields']) {
		            $fa[$ii] = $op['fields'][$ff];
                $fa[$ii]['id'] = $ff;
                $fa[$ii]['vali'] = getParameter('field'+$ff, '');
                $fa[$ii]['showval'] = parse_fld_name($op['fields'][$ff]);
                $ii++;
            }
								
						var txt = '';
            for (var $ii=0; $ii<$fa.length; $ii++) {
            		if ($ii > 1) break;
            		$fa[$ii]['name'] = $fa[$ii]['name'].replace(/\[/g, '<');
								$fa[$ii]['name'] = $fa[$ii]['name'].replace(/\]/g, '>');
                txt += $fa[$ii]['name']+'<br>'+'<font style="color:'+$sLocal.numberColor+'; font-size:'+$sLocal.numberFontSize+'">'+$fa[$ii]['showval']+'</font><br><br>'; 
            }
            set_rect_text($v, [$x_logo_ofs, 195, $__s.r[2], 200], $sLocal, txt);
        }        
        
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_payment($__v) {
				
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        $__s = SKP('VIEW_TEXT');
        $__s1 = SKP('VIEW_TEXT1');
        $sk = SKP('OPERATOR_ICON');

        var opId = Parameters['recepient'];
        var $op = jcfg_findOperatorById(opId);
        var $pr = $op['processor']['type'];

        var opNme = $op["name"];
        var opImg = $op["image"];

        var $x_logo_ofs = 230;
        var $y_logo_ofs = 5;

				
        var $valmusthave = 0;
        var arData=new Array();
        // ------------------------------------------------
        if ($pr === 'Cyberplat_PIN') {
			      set_rect_text($v, [255, 100, 420, 70], $__s1, LCP('card') + ': '+unescape(Parameters['cardname']));
            set_rect_text($v, [205, 180, 400, 70], $__s1, LCP('summ') + ': '+Parameters['cardvalue']+' '+LCP('currency')[CurrencyType]);
            $valmusthave = Parameters['cardvalue'];
        }
        else if($pr.toLowerCase() == 'avia_center') {
            var data="";
            data=getData();
            data=data.replace(/\[T]/g,"\t");
            data=data.replace(/\[CR]/g,"");
            data=data.replace(/\[LF]/g,"\n");
            arData=data.split("\n");
            set_rect_text_in_textArea($v, [0, -30, $__s.r[2]-10, 70], $__s1, '',data);
        }
        else if("cyberplat_mt" == $pr.toLowerCase()){
        	var $sum = parseFloat( getParameter('sum', 0) );
        	var $mps = parseFloat( getParameter('mps', 0) );
          set_rect_text($v, [15, 100, 670, 70], $__s1, unescape(Parameters['field100']));
        	if ($sum >= $mps && $sum > 0) {
          		var $o = $v.AddI('nbut1_g0_blue.gif', 140, 200);//keypad2.gif
              $o.AddHandler('calculate();');
              $o.SizeTo(1,400,64);
              //set_rect_text($v, [140, 230, 400, 64], $__s1, "");
          }
        } else {
        				$sLocal = SKP('VIEW_PAYMENT_TEXT_HEADER');
			        	set_rect_text($v, [0, 68, $__s.r[2], 200], $sLocal, LCP('enter_money'));
	      }
        
        // ------------------------------------------------
        var $sum = parseFloat( getParameter('sum', 0) );
        var $cms = parseFloat( getParameter('cms', 0) );
		var $mps = parseFloat(getParameter('mps', 0) );
		var $maxsum = parseFloat(Parameters['maxsum']);
        var $para_end = parseFloat( getParameter('end', 0) );

        if ($sum == 0) PlaySoundA('validator');
        
        /*****************************СКОЛЬКО ВВЕЛИ*********************************/     
        var $mps = parseFloat( getParameter('mps', 0) );
        if("cyberplat_mt"==$pr.toLowerCase()){
    		    commission_text = LCP('money_commission') + ': ' + getParameter('money_commission', LCP('not_calculate'));
        }
        else{
            commission_text=LCP('money_commission')+': '+parseFloat(Parameters['cms']/100);
        }
        
        
        if($pr.toLowerCase() == 'avia_center'){        
            $sLocal = SKP('VIEW_PAYMENT_MONEY_ACCEPTED');
            txt1 = LCP('money_accepted');
            txt2 = '<font size="'+$sLocal.sumFontSize+'">'+parseFloat($sum/100)+'</font> '+LCP('currency')[CurrencyType];
            $sLocal.f = $sLocal.f1;
            set_rect_text($v, [5, 420, 600, 62], $sLocal, txt1);
            $sLocal.f = $sLocal.f2;
            set_rect_text($v, [300, 420, 300, 62], $sLocal, txt2);
            
            $sLocal = SKP('VIEW_PAYMENT_MONEY_COMMISION');
            jcfg_getCommission(opId);
            txt1 = LCP('money_commission');
            txt2 = '<font size="'+$sLocal.sumFontSize+'">'+jcfg_getCommission(opId)+'</font>';
            $sLocal.f = $sLocal.f1;
            set_rect_text($v, [5, 482, 600, 62], $sLocal, txt1);
            $sLocal.f = $sLocal.f2;
            set_rect_text($v, [300, 482, 300, 62], $sLocal, txt2);
            
            $sLocal = SKP('VIEW_PAYMENT_MONEY_CREDITING');
            txt1 = LCP('money_crediting');
            txt2 = '<font size="'+$sLocal.sumFontSize+'">'+parseFloat( (Parameters['sum'] - Parameters['cms'])/100 )+'</font> '+ LCP('currency')[CurrencyType];
            $sLocal.f = $sLocal.f1;
            set_rect_text($v, [5, 544, 600, 62], $sLocal, txt1);
            $sLocal.f = $sLocal.f2;
            set_rect_text($v, [300, 544, 300, 62], $sLocal, txt2);
        } else if($pr.toLowerCase() == "cyberplat_mt"){
            var buf=$__s.f[1];
            $__s.f[1]=34;
            set_rect_text($v, [5, 155, $__s.r[2], 70], $__s, LCP('money_accepted')+': '+parseFloat($sum/100)+' '+LCP('currency')[CurrencyType]);
            $__s.f[1]=buf;
        }
        else if($pr.toLowerCase() == "half_pin"){
            if ($sum >= $mps) {
                set_rect_text($v, [5, 285, $__s.r[2], 70], $__s, LCP('money_accepted')+': '+parseFloat($sum/100)+' '+LCP('currency')[CurrencyType]);
                set_rect_text($v, [5, 335, $__s.r[2], 70], $__s, commission_text+' '+ LCP('currency')[CurrencyType]);
            }
				} 
				
				else if("cyberplat_taxes"==$pr.toLowerCase()) {
            set_rect_text($v, [5, 325, $__s.r[2], 70], $__s, LCP('money_accepted')+': '+parseFloat($sum/100)+' '+LCP('currency')[CurrencyType]);
            set_rect_text($v, [5, 375, $__s.r[2], 70], $__s, LCP('commission_prefix_taxes')+' '+parseFloat(Parameters['cms']/100)+' '+ LCP('currency')[CurrencyType]);
        } else {
            $sLocal = SKP('VIEW_PAYMENT_MONEY_ACCEPTED');
            txt1 = LCP('money_accepted');
            txt2 = '<font size="'+$sLocal.sumFontSize+'">'+parseFloat($sum/100)+'</font> '+LCP('currency')[CurrencyType];
            $sLocal.f = $sLocal.f1;
            set_rect_text($v, [5, 314, 350, 62], $sLocal, txt1);
            $sLocal.f = $sLocal.f2;
            set_rect_text($v, [300, 314, 350, 62], $sLocal, txt2);
            
            $sLocal = SKP('VIEW_PAYMENT_MONEY_COMMISION');
            var localCms = jcfg_getCommission(opId);
            
            txt1 = LCP('money_commission');
            
            $sLocal.f = $sLocal.f1;
            
            var iCount=1;
            if (isArray(localCms)) for (k in localCms) iCount++;
            
						
						var startPos = 0;
						//Два или больше
						if (iCount>2) {
								for (k=1; k<iCount-1; k++) {
									if ( (localCms[k-1]['sum']<parseInt($mps/100) ) && (localCms[k]['sum']> parseInt($mps/100))) {
											startPos = k-1;
											break;
									}
								}
						}
						set_rect_text($v, [5, 376, 350, 62*(iCount-startPos)], $sLocal, txt1);

						
          	$sLocal.f = $sLocal.f2;
						if (isArray(localCms)) {
							txt2 = '';
							i = 0;
							for (k = startPos; k < iCount-1; k++) {
									txt2 = '<nobr><span style="font-size:'+$sLocal.sumFontSize+'">'+localCms[k]['val']+'</font>';
									if (localCms[k]['txt']!='') txt2+= ' от '+localCms[k]['txt']+'</nobr>';
									i++;
									set_rect_text($v, [300, 314+62*i, 350, 62], $sLocal, txt2);
							}	
						}

						
            $sLocal = SKP('VIEW_PAYMENT_MONEY_CREDITING');
            txt1 = LCP('money_crediting');
						
	          //alert(Parameters['sum']);
            //alert(Parameters['cms']);

						var validSum = 0;
						if ($mps == $maxsum) {
							if($sum >= $maxsum) {
									validSum = ($maxsum - $cms)/100;
							}
						}
						else validSum = ($sum - $cms)/100;
							
						txt2 = '<font size="'+$sLocal.sumFontSize+'">' + validSum + '</font> '+ LCP('currency')[CurrencyType];

						i++;
            $sLocal.f = $sLocal.f1;
            set_rect_text($v, [5, (314+i*62), 350, 62], $sLocal, txt1);
            
            $sLocal.f = $sLocal.f2;
            set_rect_text($v, [300, (314+i*62), 350, 62], $sLocal, txt2);

        }
				/*КОНЕЦ СКОЛЬКО ВВЕЛИ*/     
				
				
				
				/*****************************МИН - МАКС*********************************/     
        var $mon_prefix = '<span style="color: '+$__s1.s[1]+';">';
        var $mon_postfix = '</span>';

        var $mps = parseFloat(getParameter('mps', 0) );
        var $maxsum = parseFloat( getParameter('maxsum', 0) );
        
        $sLocal = SKP('VIEW_PAYMENT_TEXT_MINMAX_SUMM');
        if($pr.toLowerCase() == 'avia_center'){
            if ($mps !== $maxsum) {
            				txt = LCP('ticket_cost')+' <b>'+parseFloat($mps/100)+$mon_postfix+'</b> '+LCP('currency')[CurrencyType];
            				set_rect_text($v, [5, 282, $__s.r[2]+40, 70], $sLocal, txt);
                    txt = LCP('min_summ_prefix')+' <b>'+parseFloat($mps/100)+$mon_postfix+'</b> '+LCP('currency')[CurrencyType]+' '+LCP('min_summ_suffix')+' <b>'+ parseFloat($maxsum/100)+$mon_postfix+'</b> '+LCP('currency')[CurrencyType]+'<br>'+LCP('ned_summ_plus');
                    set_rect_text($v, [5, 320, $__s.r[2]+40, 70], $sLocal, txt);
            } else {
            				//Минимальная сумма внесения <...> 
            				//Сдача будет переведена на мобильный телефон
            				txt = LCP('ned_summ')+': <b><font color="'+$sLocal.needSummColor+'">'+parseFloat($mps/100)+' '+LCP('currency')[CurrencyType]+'</font></b><br>'+LCP('ned_summ_plus');
                    set_rect_text($v, [5, 320, $__s.r[2]+40, 70], $sLocal, txt);
            }
        } 
        else if ($pr.toLowerCase() == 'cyberplat_mt'){
            if ($mps !== $maxsum) {
            		txt = LCP('min_summ_prefix')+' <b>'+parseFloat($mps/100)+$mon_postfix+'</b> '+LCP('currency')[CurrencyType]+' '+LCP('min_summ_suffix')+' <b>'+ parseFloat($maxsum/100)+$mon_postfix+'</b> '+LCP('currency')[CurrencyType];
            		set_rect_text($v, [5, 260, $__s.r[2], 50], $sLocal, txt);
            } else {
            		txt = LCP('ned_summ')+': <b><font color="'+$sLocal.needSummColor+'">'+parseFloat($mps/100)+' '+LCP('currency')[CurrencyType]+'</font></b><br>'+LCP('ned_summ_plus');
                set_rect_text($v, [5, 240, $__s.r[2], 50], $sLocal, txt);
            }
        }
        else {
            if ($mps !== $maxsum) {
            		// Вы можете заплатить от <...> до <...>
            		txt = LCP('min_summ_prefix')+' <b>'+parseFloat($mps/100)+$mon_postfix+'</b> '+LCP('currency')[CurrencyType]+' '+LCP('min_summ_suffix')+' <b>'+ parseFloat($maxsum/100)+$mon_postfix+'</b> '+LCP('currency')[CurrencyType];
            		set_rect_text($v, [5, 240, $__s.r[2], 50], $sLocal, txt);
            } else {
         				// Минимальная сумма внесения <...> 
         				// Сдача будет переведена на мобильный телефон
            		txt = LCP('ned_summ')+': <b><font color="'+$sLocal.needSummColor+'">'+parseFloat($mps/100)+' '+LCP('currency')[CurrencyType]+'</font></b><br>'+LCP('ned_summ_plus');
                set_rect_text($v, [5, 220, $__s.r[2], 50], $sLocal, txt);
            }
        }
				/**********************КОНЕЦ  *МИН - МАКС*********************************/  
				
				
				/*****************************КНОПКИ*********************************/   
        create_bigbut(-2);
        create_bigbut(-3);
        if (($sum <= 0) || (getParameter('rest', '-1') == '0')) {
               	$ga_buttbig[0]['v'].Show(1, 1);
               	changeButtonImage($ga_buttbig[0]['o'], 'cancel');
        				if($pr.toLowerCase() == 'cyberplat_mt') { 
        					$ga_buttbig[0]['o'].AddHandler('goto_Loc("mt.html?recepient='+opId+'&state=start_menu")');
        				} else {
					       	isOperator999 = getParameter('isOperator999', 0);
					       	if (isOperator999 == 1) Parameters['recepient'] = 999;
                	$ga_buttbig[0]['o'].AddHandler('goto_Loc("data-entry.html?'+goto_get_allurlparams()+'")');
                	Parameters['recepient'] = opId;
                }
        }
		var is_forward_button = ($mps == $maxsum) ? ($sum >= $maxsum) : (($sum - $cms) >= $mps);
        if (is_forward_button) {
                $ga_buttbig[1]['v'].Show(1, 1);
                changeButtonImage($ga_buttbig[1]['o'], 'payment');
                if (getParameter('act', '-1') != '-1')
                		$ga_buttbig[1]['o'].AddHandler('goto_Loc("Payment.html?end=1&'+goto_get_allurlparams()+'")');
                if("cyberplat_mt"==$pr.toLowerCase()) {
                        var buf=$__s.f[1];
                        $__s.f[1]=34;
                        var system_commission = getParameter('system_commission', '-1');
                        var rent_commision = getParameter('rent_commission', '-1');
                        var amount = getParameter('amount', '-1');

                        if("-1" == system_commission || "-1" == amount){
                            system_commission = "";
                            rent_commision = "";
                            amount = "";
                        }
                        else{
                            system_commission = LCP('money_commission') + ': ' + system_commission;
                            rent_commision = LCP('money_rest')+': ' + rent_commision;
                            amount = LCP('money_crediting_mt')+': ' + amount;
                        }                        
                        set_rect_text($v, [5, 310, $__s.r[2], 70], $__s, amount);
  				        			set_rect_text($v, [5, 370, $__s.r[2], 70], $__s, system_commission);
                        set_rect_text($v, [5, 430, $__s.r[2], 70], $__s, rent_commision);
                        $__s.f[1]=buf;
                }
                else if("half_pin" == $pr.toLowerCase()){
                		var money_crediting=Parameters['money_crediting'];
                    var rent_commision = getParameter('rent_commission', '0');
                		if(! money_crediting) money_crediting=parseFloat( (Parameters['sum'] - Parameters['cms'])/100 );
                    set_rect_text($v, [5, 440, $__s.r[2], 70], $__s, LCP('money_crediting')+': '+ money_crediting +' '+ LCP('currency')[CurrencyType]);
                    set_rect_text($v, [5, 390, $__s.r[2], 70], $__s, LCP('money_rest')+': ' + rent_commision +' '+ LCP('currency')[CurrencyType]);
                } 
//            }
        } else if($sum > 0){
        		$sLocal = SKP('VIEW_PAYMENT_MONEY_NOT_ANOUGH');
            if($pr.toLowerCase() == 'avia_center'){
                set_rect_text($v, [5, 645, $__s.r[2], 50], $sLocal, LCP('money_not_anough'));
            } else if ($pr.toLowerCase() == 'cyberplat_mt') {
          		set_rect_text($v, [5, 645, $__s.r[2], 50], $sLocal, LCP('money_not_anough'));
          	}
            else{
                set_rect_text($v, [5, 540+(iCount-2)*62, $__s.r[2]+50, 50], $sLocal, LCP('money_not_anough'));
            }
        }
        if($pr.toLowerCase() == 'avia_center'){
            initArea("tArea",arData);
        }

}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
var $ga_fields = new Array();        // fields array

var $g_fields_active = -1;                // current active field

var $g_n_blinkcnt = 0;


function field_blink() {
				//alert('!');
	
				if ($fields_count < maxFields) $skp =SKP('VIEW_ACCOUNT_DETAILS_FIELD');
        else $skp =SKP('VIEW_ACCOUNT_DETAILS_FIELD_SMALL');
        
        if ($g_fields_active < 0) return;
				
        for (i=0; i<$fields_count; i++) {
        	var $ae = $ga_fields[i];
        	//alert($ae['second_activated']);
        	if (i == $g_fields_active) {
       			if ($ae['t'] == 't') {
        			if ($ae['was_activated'] == 1 && $ae['pass']!=1) {
		        		var $iii = MJGA($ae['txb'].i);
		        		$iii.src = $mj_g_img_pth+$skp.i_wrong;
		        	} else {
		        		var $iii = MJGA($ae['txb'].i);
	        			$iii.src = $mj_g_img_pth+$skp.i; // серый
		        	}
			      } else {
	        		if ($ae['second_activated'] == 1) {
								if ($ae['pass']!=1)	{
				        	var $iii = MJGA($ae['txb'].i);
	            		$iii.src = $mj_g_img_pth+$skp.i_current_wrong; 
								} else {
			        		var $iii = MJGA($ae['txb'].i);
		        			$iii.src = $mj_g_img_pth+$skp.i_blink;
								} 
        			} else {
		        		var $iii = MJGA($ae['txb'].i);
		        		$iii.src = $mj_g_img_pth+$skp.i_blink;
        			}
        	}
        	} else {
	        	if ($ae['was_activated'] == 1 && $ae['pass']!=1) {
	        		var $iii = MJGA($ae['txb'].i);
	            $iii.src = $mj_g_img_pth+$skp.i_wrong; //красный
	        	}	else {
	        		var $iii = MJGA($ae['txb'].i);
	        		$iii.src = $mj_g_img_pth+$skp.i; // серый
	        	}
	        }
        }
        //var $ae = $ga_fields[$g_fields_active];
     		//var $iii = MJGA($ae['txb'].i);
     		//$iii.src = $mj_g_img_pth+$skp.i_blink;
}

mj_RT.Add('field_blink', 350, 0);

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// fields array:
/*
['txb'] - background object
['txi'] - object to set_rect_text with [m] and [b] text elements
['v'] - viewport control to enter data into field !!!
['t'] - type of the field (-1)
['a'] - array of attr of the field
['id'] - id of the field
['title'] - title of the field
['commft'] - comments field text

*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// create new field
function create_field($__v, $__title, $__id, $__type, $__attr, $__comment, $__def) {
        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];

        var $f_id = $ga_fields.length;
        $ga_fields[$f_id] = new Array();
        var $ae = $ga_fields[$f_id];

				//парсим ББ коды в заголовке
				$__title = $__title.replace(/\[/g, '<');
				$__title = $__title.replace(/\]/g, '>');
				var ppp = '';
				//Размеры кнопок
        y0 = $v['yofs'];
        if ($fields_count < maxFields)  {
        	$skc = SKP('VIEW_ACCOUNT_DETAILS_FIELD');
        	//для центрирования по высоте текста в кнопке
        	k1 = 12;
        	//для позиционирования текста над кнопками относильно самой кнопки
        	k2 = 28;
        	//приращение текста+кнопки
        	dy = 49;
        	//смещение надписи над кнопками относительно кнопок по горизонтали
        	dx = 18;
        	//координа X вывода кнопки
        	x0 = 310;
        	//Длина имени поля
        	textWidth = $skc.r[2];
        	//Длина значения поля
        	valueWidth = parseInt($skc.r[2]-22);
        } else {
        	$skc = SKP('VIEW_ACCOUNT_DETAILS_FIELD_SMALL');
        	//для центрирования по высоте текста в кнопке
        	k1 = 10;
        	//для позиционирования текста над кнопками относильно самой кнопки
        	k2 = -20;
        	//приращение текста+кнопки
        	dy = 0;
        	//смещение надписи над кнопками относительно кнопок по горизонтали
        	dx = -225;
        	//координа X вывода кнопки
        	x0 = 450;
        	//
					//Длина имени поля
        	textWidth = '220';
        	//Длина значения поля
        	valueWidth = parseInt($skc.r[2]-17);
			y0 = y0 - 60;
        }
        $o = $v.AddI($skc.i, x0, y0);
        //$o = $v.AddI('s', x0, y0);
        $o.SizeTo(1, $skc.r[2], $skc.r[3]);
				$o.AddHandler('activate_field("'+$f_id+'")', $skc.r[2], $skc.r[3]);
        $ae['txb'] = $o;
        
        $ae['v'] = -1;
        $ae['t'] = $__type;
        $ae['a'] = $__attr;
        $ae['id'] = $__id;
        $ae['title'] = $__title;
        $ae['pass'] = 0;
        $ae['val'] = ''; // -1;
        $ae['was_activated'] = 0; 
        $ae['second_activated'] = 0; 
        if ('undefined' !== typeof($__def)) {
                $ae['val'] = $__def;
        }
        $ae['attr'] = $__attr;
        var txt = '';

        //Надписи в кнопках
        if ($ae['t'] == 'e') { 
	       	var opId = getParameter('recepient', 'undefined');
	        var $op = jcfg_findOperatorById(opId);
					var ekey = $ae['val'];
					if (ekey!='')  for(var ei in $op['fields'][$__id]['enum']) if (ekey == ei) txt = $op['fields'][$__id]['enum'][ei];
					else
						for(var ej in $op['fields'][$__id]['enum'][ei])
							if (ekey == ej) txt = $op['fields'][$__id]['enum'][ei][ej];
							else 
								for(var ek in $op['fields'][$__id]['enum'][ei][ej])
									if (ekey == ek) txt = $op['fields'][$__id]['enum'][ei][ej][ek];
        } else if ($ae['t'] == 'm') {
      		var res = maskFieldValidation($ae['val'], $ae['a']['mask']);
      		txt = res['view'];
      		
      	} else txt = $ae['val'];
				
        if (txt.length > 45 ) txt = txt.substr(0,45)+'…';
        $ae['txi'] = set_rect_text($v, [x0, (y0+k1), valueWidth, $skc.r[3]], $skc, txt);

        //Надписи НАД кнопками
        if ($fields_count < maxFields) $skf = SKP('VIEW_ACCOUNT_DETAILS_FINFO');
      	else var $skf = SKP('VIEW_ACCOUNT_DETAILS_FINFO_SMALL');
      	
      	set_rect_text($v, [x0+dx, y0-k2, textWidth, $skc.r[3]], $skf, $__title);
        $v['yofs'] += $skc.r[3] + dy;
        	
        $ae['commft'] = $__comment;
        
        if (($ae['t'] === 'i')) {
        		if ($ae['attr']['min'] <= $ae['val'].length) $ae['pass'] = 1;
        } else if ($ae['t'] === 'm') {
        		tmp = maskFieldValidation($ae['val'], $ae['attr']['mask'])
        		$ae['pass'] = tmp['pass']
        } else if ($ae['t'] === 'e') {
      			if ($ae['val']!='') $ae['pass'] = 1;
      	} else if ($ae['t'] === 't') {
      			if ($ae['val'].length > 0) $ae['pass'] = 1;
      	} else {
      	
      	}
        return $f_id;
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// create all fields due to current operator

//Массив данных с внешнего источника (сканер)
$ext_var = new Array();
$ext_id = new Array();
timerId = 0;

function create_account_fields($__v) {
				//запускаем таймер
				//alert('Убиваем таймер: '+timerId);
				clearTimeout(timerId);
				timerId = setTimeout("goto_Main()",60000);
				//alert('Страница: '+ document.location.href + '  Таймер:'+timerId);
				
        // $g_vpa[$g_current_vpa]['v'].Show(0, 1);
        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];

        //set_rect_text($v, [5, 5, $s.r[2], 3], $s, LCP('title_requizites') );

        // var opId = Parameters['recepient'];
        var opId = getParameter('recepient', 'undefined');

        var $op = jcfg_findOperatorById(opId)
        var opNme = $op['name'];
        if (opId != '999') var opImg = $op['image'];
        else var opImg = 'logo_no_oper.png';

				showCommision($__v, 0);
				
        $gs_marquee_text = jcfg_getCommissionText(opId);
        //var opComment = ndOperator.getElementsByTagName("comment").item(0).text;
        //opComment = opComment.replace(']', 'W');


        // !!!!!
        var $xofs = 50;
        var $yofs = 85;

        var $sk = SKP('OPERATOR_ICON');
        //var $o = $v.AddI($sk.bi, $xofs - Math.floor(($sk.bd[0]-$sk.d[0])/2), $yofs - Math.floor(($sk.bd[1]-$sk.d[1])/2));
        // $go_operator_icon = $v.AddI('op/'+opImg, $xofs, $yofs);
        $go_operator_icon = gfxa_ins_oper_logo($v, opImg, $xofs, $yofs, $sk.d[0], $sk.d[1], opNme);

        $v['yofs'] = 78;
				
				var flag = 0;
				var fields_count = 0;
				
				
        // create and detect fields
        var $pr = $op['processor']['type'].toLowerCase();
        if ($pr === 'cyberplat' || $pr == 'avia_center' || $pr == 'half_pin' || $pr == 'cyberplat_taxes' || 'external' == $pr) {
                for (var $fi in $op['fields']) {
                        $fields_count++;
                }
				
				if($mj_g_docnme == 'pb.html') $fields_count++;
			 	        for (var $fi in $op['fields']) {
								$ext_var[$fi] = getParameter('field'+$fi, '');
					        if (flag==0 && $ext_var[$fi]!='') fields_count++
									else flag=1;
								}
                for (var $fi in $op['fields']) {
                        var $cf = $op['fields'][$fi];
                        var $fcomment = '';
                        if ('undefined' !== typeof($cf['comment'])) {
                                $fcomment = $cf['comment'];
                        }
                        // FieldComment[$i];
                        $fcomment = $fcomment.replace(/\[/g, '<');
                        $fcomment = $fcomment.replace(/\]/g, '>');
						
						// name 
                        $cf['name'] = $cf['name'].replace(/\[/g, '<');
                        $cf['name'] = $cf['name'].replace(/\]/g, '>');
						
												var txtLocal = '';
                        var $f_type = $cf['type']; // .toUpperCase();
                        if ($f_type == 'masked') {
	                      		create_field($__v, $cf['name'], $fi, 'm', {mask: $cf['mask']}, $fcomment, $ext_var[$fi]);                        	
	                      		if ($ext_var[$fi]!='') {
	                      				var res = maskFieldValidation($ext_var[$fi], $cf['mask']);
			                      		txtLocal = res['view'];
	                      		}
                      	} else if ($f_type == 'integer') {
                      			var linked = 0;
                      			(typeof($cf['link'])=='undefined') ? linked = 0 : linked = $cf['link'];
                            if ('undefined' === typeof($cf['minlength'])) $cf['minlength'] = 1;
                            create_field($__v, $cf['name'], $fi, 'i', {min: $cf['minlength'], max: $cf['maxlength'], link : linked}, $fcomment, $ext_var[$fi]);
                            if ($ext_var[$fi]!='') txtLocal = $ext_var[$fi];
						} 
						else if ($f_type == 'pwd') {
                            if ('undefined' === typeof($cf['minlength'])) $cf['minlength'] = 1;
                            create_field($__v, $cf['name'], $fi, 'p', {min: $cf['minlength'], max: $cf['maxlength']}, $fcomment, $ext_var[$fi]);
                            if ($ext_var[$fi]!='') txtLocal = $ext_var[$fi];
                      	} else if ($f_type == 'enum') {
                            if ($ext_var[$fi] !='') create_field($__v, $cf['name'], $fi, 'e', {en: $cf['enum']}, $fcomment, $ext_var[$fi]);
                            else create_field($__v, $cf['name'], $fi, 'e', {en: $cf['enum']}, $fcomment);
														if ($ext_var[$fi]!='') {
																var opIdLocal = getParameter('recepient', 'undefined');
		                            var opLocal = jcfg_findOperatorById(opIdLocal);
									var ekey = $ext_var[$fi];
									if (ekey!='') 
										for(var ei in $op['fields'][$fi]['enum'])
											if (ekey == ei) 
												txtLocal = $op['fields'][$fi]['enum'][ei];
											else
												for(var ej in $op['fields'][$fi]['enum'][ei])
													if (ekey == ej) 
														txtLocal= $op['fields'][$fi]['enum'][ei][ej];
													else
														for(var ek in $op['fields'][$fi]['enum'][ei][ej])
															if (ekey == ek) 
																txtLocal = $op['fields'][$fi]['enum'][ei][ej][ek];
									//txtLocal = $op['fields'][$fi]['enum'][$ext_var[$fi]];                            	
									}
                      	} else if ($f_type == 'text') {
                            create_field($__v, $cf['name'], $fi, 't', null, $fcomment, $ext_var[$fi]);
                            if ($ext_var[$fi]!='') txtLocal = $ext_var[$fi];
                      	} else if ($f_type == 'textpwd') {
							var str="";
							for(var i=0;i<$ext_var[$fi].length;i++)
								str+="•";
                            create_field($__v, $cf['name'], $fi, 'tp', null, $fcomment, $ext_var[$fi]);
							txtLocal = str;
                      	} else {
                      			CybIf_Logger(CYBIF_LOG_ERR, 'iface_runtime.js: create_account_fields() - unknown field type: '+$f_type+' in operator id: '+opId);
                      	}
                      	if ($ext_var[$fi]!='') {
		        								aeLocal = $ga_fields[$ga_fields.length-1];
                        		skpLocal = changeFontSize($fields_count, txtLocal.length);
                        		if (txtLocal.length > 45) txtLocal = txtLocal.substr(0,45)+'…';
        										change_srt_text(aeLocal['txi'], txtLocal, skpLocal);
        								}
       								
                }
				if($mj_g_docnme == 'pb.html') {
				  	var pb_comm = LCP('comment_pb');
						if (opId == '2500') pb_comm = LCP('comment2_pb');
						create_field($__v, pb_comm, 'comment_pb', 't', null, '', getParameter('name', ''));
				}
        }
        else if ("cyberplat_pin" == $pr) {
                jcfg_getCardsInfo();
                create_field($__v, LCP('nominal'), '', 'c', '');
        }
        if("half_pin" == $pr){
        				$fields_count++;
                jcfg_getCardsInfo();
                create_field($__v, LCP('nominal'), '', 'c', '');

 								aeLocal = $ga_fields[$ga_fields.length-1];
             		skpLocal = changeFontSize($fields_count, '10');
								change_srt_text(aeLocal['txi'], '', skpLocal);
        }
        if('cyberplat' != $pr && 'avia_center' != $pr && "cyberplat_pin" != $pr && "external" != $pr && "half_pin" != $pr && "cyberplat_taxes" != $pr){
                CybIf_Logger(CYBIF_LOG_ERR, 'iface_runtime.js: create_account_fields() - invalid processor type: '+$pr+' in operator id: '+opId);
        }
	
				
				$ga_buttbig[0]['v'].Show(1, 1);
				changeButtonImage($ga_buttbig[0]['o'], 'main');
        if ('undefined' !== typeof($goto_back_overload)) {
         	$ga_buttbig[0]['o'].AddHandler($goto_back_overload);
        } else {
        	$ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');
        }

        //кнопка удалить
	var pb = getParameter('pb', 'undefined');
	if (pb == 'dataentry') {
			var rec_id = getParameter('rec_id', '-1');
			var o = $v.AddI(buttons['del'], 20, 520);
      o.AddHandler('goto_Loc("pb.html?pb=delete&recepient='+opId+'&rec_id='+rec_id+'")');
	}
  	activate_field(0);
}

var $g_enum = new Array();
var $g_enum_id = 0;
var $g_enum_redraw = false;
var $gEnumImage = new Array();
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_enum($__v) {
        var enumCount = 0;
		pinpadActive = false;
        
        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];
        var $title = $ga_fields[$g_fields_active]['title'];
        //set_rect_text($v, [5, 5, $s.r[2], 70], $s, $title);

        var $ae = $ga_fields[$g_fields_active];
				create_field_comment($__v, 0);
				
        // ParentId
        if (typeof($ae['a']['en']['enum1'])!='undefined') {
      			if (typeof($ae['a']['en']['enum1']['parentId'])!='undefined') {
      				var temp = new Array();
      				for(i=0; i<$g_fields_active; i++) { 
      					temp[$ga_fields[i]['id']] = $ga_fields[i]['val'];
      				}
      				var loc = $ae['a']['en'];
      				for (var $ff in loc) {
      					if (typeof(temp[loc[$ff]['parentId']])!='undefined' && temp[loc[$ff]['parentId']] == loc[$ff]['parentValue']) break;
      				}
      				tmp = $ae['a']['en'][$ff]['values'];
      				$g_enum[$ae['id']] = $ff;
      			} else {
							if (typeof($g_enum[$ae['id']]) == 'undefined') tmp = $ae['a']['en'];
							else tmp = $ae['a']['en'][$g_enum[$ae['id']]]['values'];
      			}
      	} else {
						if (typeof($g_enum[$ae['id']]) == 'undefined') tmp = $ae['a']['en'];
						else tmp = $ae['a']['en'][$g_enum[$ae['id']]]['values'];
      	}
      	for (var $ff in tmp) enumCount++;
        
        var $skc = '';
				if (enumCount > maxEnum) $skc = SKP('ENUM_ICON_SMALL');
				else $skc = SKP('ENUM_ICON');
        var $yofs = 70;
				var k = 1;
				var tmpFlag = 0;
				$gEnumImage[$g_fields_active] = new Array();
        for (var $ff in tmp) {
        				if (enumCount > maxEnum) {
        					maxEnumLength = 22;
        					
        					if (k > parseInt(enumCount/2)) {
        						locX = $s.r[2]/2;
        						if (tmpFlag == 0) {
        							$yofs = 70;
        							tmpFlag = 1;
        						}
        						locY = $yofs;
        					}
	       					else {
	       						locX = ($s.r[2]-2*$skc.d[0])/2;
										locY = $yofs;
									}
									buttonWidth = $skc.d[0];
									buttonHeight = $skc.d[1];
        				} else {
        					maxEnumLength = 45;

									locX = ($s.r[2]-$skc.d[0])/2;
									locY = $yofs;
									buttonWidth = $skc.d[0];
									buttonHeight = $skc.d[1];
        				}
        				if ($ae['val'] == $ff) localImage = $skc.bi_current;
        				else localImage = $skc.bi
        				
                var $o = $v.AddI(localImage, locX,  locY);
                $o.SizeTo(1, buttonWidth, buttonHeight);
								
								if ($ff.indexOf("enum")!=-1) {
	                	$o.AddHandler('createSubEnum("'+$__v+'", "'+$ff+'")');
	                	localSKP = changeEnumFontSize(enumCount, tmp[$ff]['name'].length);
	                	dy = dyEnum(localSKP.f[1]);
	                	txt = tmp[$ff]['name'];
	                	if (txt.length > maxEnumLength) txt = txt.substr(0,maxEnumLength)+'…';
                		set_rect_text($v, [locX, locY+dy, buttonWidth, buttonHeight], $s, txt);
								} else {
                    $o.AddHandler('select_fpar_enum("'+$ff+'", "'+enumCount+'")');
                    localSKP = changeEnumFontSize(enumCount, tmp[$ff].length);
                    dy = dyEnum(localSKP.f[1]);
										txt = tmp[$ff];
										if (txt.length > maxEnumLength) txt = txt.substr(0,maxEnumLength)+'…';
                		set_rect_text($v, [locX, locY+dy, buttonWidth, buttonHeight], localSKP, txt);
                }
                
                $gEnumImage[$g_fields_active][$ff] = $o;
             		$yofs += $skc.d[1];	
             		k++;
              	
        }
}
function createSubEnum($__v, $start) {
        var $ae = $ga_fields[$g_fields_active];
        $g_enum_id = 1000;
        $g_vpa[$ae['v']]['v'].Show(0, 1);
        $g_enum[$ae['id']] = $start;
        $g_enum_redraw = true;
        create_vpa('VIEW_ACCOUNT_INLET', 'create_enum', $g_enum_id, '-1');
        $g_enum_redraw = false;
        $ga_buttbig[0]['v'].Show(1, 1)
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function select_fpar_enum($__s, enumCount) {
				if (enumCount > maxEnum) $skc = SKP('ENUM_ICON_SMALL');
				else $skc = SKP('ENUM_ICON');
				
				for (var $ff in $gEnumImage[$g_fields_active]) {
					var $iii = MJGA($gEnumImage[$g_fields_active][$ff].i);
        	$iii.src = $mj_g_img_pth+$skc.bi;
				}
				var $iii = MJGA($gEnumImage[$g_fields_active][$__s].i);
        $iii.src = $mj_g_img_pth+$skc.bi_current;

        var $ae = $ga_fields[$g_fields_active];
        $ae['val'] = $__s;
        if (typeof($g_enum[$ae['id']]) == 'undefined') {
        		tmp = $ae['a']['en'][$__s];
        } else {
						tmp = $ae['a']['en'][$g_enum[$ae['id']]]['values'][$__s];
				}
				skp = changeFontSize($fields_count, tmp.length);
        if (tmp.length > 45) tmp = tmp.substr(0,45)+'…';
        change_srt_text($ae['txi'],  tmp, skp);
        $ae['pass'] = 1;
        verify_fields();
        nextActivateField();
        field_blink();
}
function changeEnumFontSize(enumCount, stringLength) {
	skp = SKP('VIEW_ACCOUNT_INLET');
	if (enumCount > maxEnum) {
		if (stringLength <= 15) skp.f[1] = 21; 
		else skp.f[1] = 14; 
	} else { 
		if (stringLength <= 20) skp.f[1] = 27; 
		else if (stringLength > 20 && stringLength <= 30) skp.f[1] = 21; 
		else skp.f[1] = 14; 		
	}
	return skp;	
}
function dyEnum (fontEnumSize){
	if (fontEnumSize == 27) ret = 10;
	else if (fontEnumSize == 21) ret = 15;
	else ret = 20;
		
	return ret;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_card($__v) {
        // alert('wow!');
        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];

        var $title = $ga_fields[$g_fields_active]['title'];

        set_rect_text($v, [5, 15, $s.r[2], 70], $s, $title);

        var $ae = $ga_fields[$g_fields_active];

        var $skc = SKP('CARD_ICON1');

        var $yofs = 70;
        /*
        for (var $i=0; $i<CardsCount; $i++) {
                var $o = $v.AddI($skc.bi, ($s.r[2]-$skc.d[0])/2, $yofs );
                // $o.SizeTo(1, $skc.d[0], $skc.d[1]);
                // $o.SetBG(1, '#555599');

                $o.AddHandler('select_fpar_card("'+$i+'")');

                set_rect_text($v, [($s.r[2]-$skc.d[0])/2, $yofs+40, $skc.d[0], $skc.d[1]], $s, CardName[$i]);
                set_rect_text($v, [($s.r[2]-$skc.d[0])/2, $yofs+90, $skc.d[0], $skc.d[1]], $s, CardValue[$i]+' euro');

                $yofs += $skc.d[1]+5;
        }
        */
        // alert('alive!');


        var $opId = getParameter('recepient', 'undefined');
        if ('undefined' === typeof($ga_jcfg['details']['pin_info'][$opId])) {
                return;
        }

        var $l_cards = $ga_jcfg['details']['pin_info'][$opId]['cards'];

        if ($l_cards.length < 1) {
                var $o = $v.AddI($skc.bi, ($s.r[2]-$skc.d[0])/2, $yofs );
                set_rect_text($v, [($s.r[2]-$skc.d[0])/2, $yofs+25, $skc.d[0], $skc.d[1]], $s, LCP('card_no_avialable'));
        }

        else {
                for (var $cid in $l_cards) {
                        var $o = $v.AddI($skc.bi, ($s.r[2]-$skc.d[0])/2, $yofs );
                        $o.AddHandler('select_fpar_card("'+$cid+'")', $skc.d[0], $skc.d[1]);
                        set_rect_text($v, [($s.r[2]-$skc.d[0])/2, $yofs+25, $skc.d[0], $skc.d[1]], $s, $l_cards[$cid]['name']+'<br>'+$l_cards[$cid]['amount']+' '+LCP('currency')[CurrencyType]);
                        $yofs += $skc.d[1]+$skc.btw[0];
                }
        }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function select_fpar_card($__s) {
        var $ae = $ga_fields[$g_fields_active];

        $ae['val'] = $__s;

        var $opId = getParameter('recepient', 'undefined');
        var $l_cards = $ga_jcfg['details']['pin_info'][$opId]['cards'];

        change_srt_text($ae['txi'], $l_cards[$__s]['name'] );

        $ae['pass'] = 1;
        verify_fields();
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//проверка на наличие vpa в массиве
function is_vpa_exist($name) {
	var res_exist = false;
	
	for(var er in $g_vpa)
		if (er === $name) {
			res_exist = true;
			break;
		}
return res_exist;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// activate field with field id
var delta = 0;
function activate_field($__id) {
				var opId = getParameter('recepient', 'undefined');
				/**************/			
        
        if ($fields_count < maxFields)  $skc =SKP('VIEW_ACCOUNT_DETAILS_FIELD');
        else $skc = SKP('VIEW_ACCOUNT_DETAILS_FIELD_SMALL');
        	
				for (i=0; i<$ga_fields.length; i++) {
					$ae = $ga_fields[i];
					$ae['txb'].OverloadDime($skc.r[2], $skc.r[3]);
				}
				/**************/


        // Если родительский vpa еще длоконца не создан, но тута уже начинается движуха по сходанию дочерних
        var $parent_vpa = $g_current_vpa;
        if ($parent_vpa == '') $parent_vpa = $g_vpaincreation;
        //  - - - - - check if exists
        if ('undefined' === typeof($ga_fields[$__id])) {
                CybIf_Logger(CYBIF_LOG_ERR, 'field no'+$__id+' is undefined!!!');
        }
		
		var $ae0 = $ga_fields[0];
		var inlet0 = 'VIEW_ACCOUNT_INLET';
		var $name0 = inlet0 + '_' + $ae0['id'];
		
		if (((($ae0['t'] == 't') || ($ae0['t'] == 'tp')) && ($pb_entry == 1))   &&   (($mj_g_docnme == 'pb.html') && (pb == 'dataentry'))) {
			if (($__id != 0)  && ($g_vpa[$name0]['v'].is_show == 1))
			$g_vpa[$name0]['v'].Show(0, 0);
			$g_vpa[$name0]['v'].Redraw();
		}

        //  - - - - - deactivate current active
        if ($g_fields_active >= 0) {
			if (is_no_keyboard)
                $g_vpa[$ga_fields[$g_fields_active]['v']]['v'].Show(0, 1);
        }
        if ($g_enum_id > 0) {
                $g_vpa['VIEW_ACCOUNT_INLET_1000']['v'].Show(0, 1);
        }
        //  - - - - - - set current active field
        delta = $__id - $g_fields_active;
        $g_fields_active = $__id;

        //  - - - - - activate field
        var $ae = $ga_fields[$__id];
        if ($ae['was_activated'] == 1) $ae['second_activated'] = 1;
        $ae['was_activated'] = 1;
		var pb = getParameter('pb', 'undefined');
		$after_entry = false;
		if ((($pb_entry == 1) && ($mj_g_docnme == 'pb.html') && (pb == 'dataentry')) || 
		    (($mj_g_docnme == 'pb.html') && (pb != 'dataentry')) ||
			($mj_g_docnme != 'pb.html')) {
				$after_entry = true;
                switch($ae['t']) {
                        // - - - - - - - -
                        case 'm':
							var inlet = 'VIEW_ACCOUNT_INLET';
							$ae['v'] = create_vpa(inlet, 'create_pinpad', $ae['id'], '-1');
							field_blink();
							is_no_keyboard = true;
	                        cvp_PP_PinPress();
                        break;
                        // - - - - - - - -
                        case 'i':
	                        $ae['v'] = create_vpa('VIEW_ACCOUNT_INLET', 'create_pinpad', $ae['id'], '-1');
							field_blink();
							is_no_keyboard = true;
	                        cvp_PP_PinPress();
                        break;

                        case 'p':
                        var inlet = 'VIEW_ACCOUNT_INLET';
	                        $ae['v'] = create_vpa(inlet, 'create_pinpad', $ae['id'], '-1');
							field_blink();
							is_no_keyboard = true;
	                        cvp_PP_PinPress();
                        break;
                        // - - - - - - - -
                        case 'e':
	                        if (delta > 0 && $ae['id'] == 108) $g_enum_redraw = true;
	                        $ae['v'] = create_vpa('VIEW_ACCOUNT_INLET', 'create_enum', $ae['id'], '-1');
							field_blink();
	                        $g_enum_redraw = false;
							is_no_keyboard = true;
                        break;

                        // - - - - - - - -
                        case 'c':
	                        $ae['v'] = create_vpa('VIEW_ACCOUNT_INLET', 'create_card', 'card', '-1');
							field_blink();
							is_no_keyboard = true;
                        break;

                        // - - - - - - - -
                        case 't':
                 			for (i in $ga_fields) $ga_fields[i]['txb'].OverloadDime(0, 0);
                 			$g_enum_redraw = true;
	                        $ae['v'] = create_vpa('VIEW_ACCOUNT_INLET_KEYS', 'create_keyboard', $ae['id'], '-1');
							field_blink();
	                        $g_enum_redraw = false;
							is_no_keyboard = false;
                        break;

                        // - - - - - - - -
                        case 'tp':
                			for (i in $ga_fields) $ga_fields[i]['txb'].OverloadDime(0, 0);
                			$g_enum_redraw = true;
	                        $ae['v'] = create_vpa('VIEW_ACCOUNT_INLET_KEYS', 'create_keyboard', $ae['id'], '-1');
							field_blink();
	                        $g_enum_redraw = false;
							is_no_keyboard = false;
                        break;

                        // - - - - - - - -
                        default:
							alert('attempt to create/active undefined field edit control, type: '+$ae['t']);
                        break;
                }
			} else {
				$pb_entry = 1;
				is_no_keyboard = false;
			}

        var $ae = $ga_fields[$g_fields_active];
        $s_number = $ae['val']; // = $s_r;

        $g_vpa[$parent_vpa]['relative_vpa'] = $ae['v'];
				
				
   	    $ga_buttbig[0]['v'].Show(1, 1);
			  changeButtonImage($ga_buttbig[0]['o'], 'back');
	      if ('undefined' !== typeof($goto_back_overload)) {
         		$ga_buttbig[0]['o'].AddHandler($goto_back_overload);
        } else {
				var pb = getParameter('pb', 'undefined');
				if($mj_g_docnme == 'pb.html'){
				    if(pb == 'enteritem')
						$ga_buttbig[0]['o'].AddHandler('goto_Loc("pb.html?pb=addItem")');
					if(pb == 'login')
						changeButtonImage($ga_buttbig[0]['o'], 'main');
				}
				else{
					var destination = 'main.html';
					var $pstate = getParameter('pstate', 'ok');
					if ($intro == true) destination = 'main_selection.html';

					var $state = getParameter('state', '-1');
					if ($state == 'pb') destination = makeStateURL();

					if (opId == 'undefined') $ga_buttbig[0]['o'].AddHandler('goto_Loc("'+destination+'")');
					else $ga_buttbig[0]['o'].AddHandler('goto_Loc("'+destination+((($state != 'pb') || ($pstate == 'vld')) ? '?' : '&')+'recepient='+opId+'")');
				}
        }
        verify_fields();
		//убираем комментарий старого филда
		if (($mj_g_docnme == 'pb.html') && (pb == 'dataentry') || ((pb == 'enteritem'))) {
			var $is_all_passed = true;
			
			for(var iz = 0; iz < $ga_fields.length; iz++) {
				if ((($ga_fields[iz]['pass'] == 0) || (($ga_fields[iz]['id'] == 'comment_pb') && ($ga_fields[iz]['val'] == ''))) &&
					(iz != $__id)) {
					$is_all_passed = false;
				}

				if ($after_entry) {
					var inlet = 'VIEW_ACCOUNT_INLET';
					var $name = inlet + '_' + $ga_fields[iz]['id'];

					if (is_vpa_exist($name)) {
						//create_field_comment($name, 0);
						$g_vpa[$name]['v'].Redraw();
					}
				}
			}
		}
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
var $g_is_check_played = false;

function nextActivateField() {
	if ($ga_fields[$g_fields_active]['pass'] == 1) {
		if ($ga_fields[$g_fields_active]['second_activated'] != 1) {
			for (k in $ga_fields) {
				if ($ga_fields[k]['pass']!=1 && parseInt(k)!=$g_fields_active) {
					if ($ga_fields[$g_fields_active]['t']=='i' || $ga_fields[$g_fields_active]['t']=='p') {
						if ($ga_fields[$g_fields_active]['val'].length == $ga_fields[$g_fields_active]['attr']['max']){
						    activate_field(k);
						}
					} 
					else activate_field(k);
					break;
				}
			}
		}
	}
}

function changePbText() {
	change_srt_text(getpinMessage, getpinAllMessages[2][0]);
}

function verify_fields($__f) {
		if (!timeInfoExist) addTimeInfo();
		clearTimeout(timerId);
		timerId = setTimeout("goto_Main()",120000);
		if (Parameters['recepient'] == 0) var opId = 0;
		else var opId = getParameter('recepient', 'undefined');
		
	  var $op = jcfg_findOperatorById(opId);
	  var pb = getParameter('pb', 'undefined');
	  if (pb == 'undefined')  var $pr = $op['processor']['type'];	  
  
   	if (pb.toLowerCase() == 'enteritem') {

   		var str = 'pb.html?pb=add&recepient='+opId;
   		for (k in $ga_fields) str+='&field'+$ga_fields[k]['id']+'='+$ga_fields[k]['val'];
   		str = str.replace(/\\/g, '%5C');
		str = str.replace(/"/g, '%22');
   		$gs_global_check_handler = 'goto_Loc("'+str+'")';
  		changeButtonImage($ga_buttbig[1]['o'], 'add');
  		$ga_buttbig[1]['v'].Show(1, 1);	
   		$ga_buttbig[1]['o'].AddHandler($gs_global_check_handler);

		//отрисовка комментария для 1-го филда, если он с клавиатурой
		if ($after_entry) {
			var $is_need_show_comment = false;
			var $is_all_passed = true;
			
			for(var iz = 0; iz < $ga_fields.length; iz++) {
				if ($ga_fields[iz]['pass'] == 0)
					$is_all_passed = false;
			}
			
			for(var iz = 0; iz < $ga_fields.length; iz++) {
				if ((($ga_fields[$g_fields_active]['t'] == 't') || ($ga_fields[$g_fields_active]['t'] == 'tp')) &&
					($is_all_passed) && 
					!keybordActive) 
						$is_need_show_comment = true;
			}
			
			if ($is_need_show_comment) {
				var inlet = 'VIEW_ACCOUNT_INLET';
				var $name = inlet + '_' + $ga_fields[0]['id'];
				
				//если нет vpa, создаем
				if (!is_vpa_exist($name)) {
					$g_vpa[$name] = new Array();
					var $v = $g_vpa[$name];
					$v['s'] = SKP(inlet);

					// if image defined, creating image-based viewport
					if (false === $v['s'].i){
						if ($v['s'].b !== false) {
							$v['v'] = new MJ_VP(cvp_G, -1, $v['s'].r[0], $v['s'].r[1], $v['s'].r[2], $v['s'].r[3], 'DIV', $v['s'].b);
						}
						else {
							$v['v'] = new MJ_VP(cvp_G, -1, $v['s'].r[0], $v['s'].r[1], $v['s'].r[2], $v['s'].r[3], 'DIV');
						}

						if (('undefined' !== typeof($v['s'].t)) && 
							('undefined' !== typeof($v['s'].t.allow_flash)) 
							('undefined' !== typeof($v['s'].f))) {
								MJGA($v['v'].o.i).innerHTML = '<embed src="../../skins/'+SkinName+'/i/'+$v['s'].f+'" width="'+GTX($v['s'].r[2])+'" height="'+GTY($v['s'].r[3])+'" autostart="true" quality="best" wmode="transparent">';
						}
					}
				}
				create_field_comment($name, 0, 0);
			}
			}
		} else if(pb == 'login') {
				var flag = '1';
				for (k in $ga_fields) {
					if ($ga_fields[k]['pass'] == 0) {
						flag = 0;
						break;
					} 
				}	
				if (flag == 1) {
						$gs_global_check_handler = 'goto_Loc("pb.html?pb=checkaccount&number='+$ga_fields[0]['val']+'&pin='+$ga_fields[1]['val']+'")';
			  		changeButtonImage($ga_buttbig[1]['o'], 'next');
			  		$ga_buttbig[1]['o'].AddHandler($gs_global_check_handler);
			      $ga_buttbig[1]['v'].Show(1, 1);
				} else $ga_buttbig[1]['v'].Show(0, 1);
				
				if ($ga_fields[0]['pass'] == 1) { 	
					getpinAction = 'goto_Loc("pb.html?pb=sendsmspin&number='+$ga_fields[0]['val']+'")';
				} else {
					getpinAction = 'changePbText()';
				}
				getpin.AddHandler(getpinAction);
					
		} else if(pb == 'enterpin') {
				if ($ga_fields[$g_fields_active]['pass'] == 1) {
						$gs_global_check_handler = 'goto_Loc("pb.html?pb=checkaccount&pin='+$ga_fields[$g_fields_active]['val']+'")';
			  		changeButtonImage($ga_buttbig[1]['o'], 'next');
			  		$ga_buttbig[1]['o'].AddHandler($gs_global_check_handler);
			      $ga_buttbig[1]['v'].Show(1, 1);
				} else $ga_buttbig[1]['v'].Show(0, 1);
		} else if(pb == 'dataentry') {
				if ($op['fix']!==0 && $op['fix']!==1) {
							if (($ga_fields[$g_fields_active]['id']) == $op['fix']) {
									if ($op['limit']['max'] < parseFloat($ga_fields[$g_fields_active]['val'])){
											$ga_fields[$g_fields_active]['pass'] = 0;
									}
							}
					}
				
				var flag = '1';
				for (k in $ga_fields) {
					if (((($ga_fields[k]['t'] == 't') || ($ga_fields[k]['t'] == 'tp')) && ($ga_fields[k]['val'] == '') && ($ga_fields[k]['id'] != 'comment_pb')) ||
						((($ga_fields[k]['t'] != 't') && ($ga_fields[k]['t'] != 'tp')) && ($ga_fields[k]['pass'] == 0))) {
						flag = 0;						
						break;
					}
				}
				
				
				//Кнопка оплатить
				var msg_name = getParameter('name', '');
				if (flag == 1) {
						var rec_id = getParameter('rec_id', '-1');
						var str = 'pb.html?pb=checking&recepient='+opId+'&rec_id='+rec_id+'&msg='+msg_name;
						for (k in $ga_fields) {
							var $tvalue = $ga_fields[k]['val'];
							if ($ga_fields[k]['t'] == 'm') {
								var $tmask = $ga_fields[k]['attr']['mask'];
								var value = maskFieldValidation($tvalue, $tmask);
								tvalue = value['number'];
							}
							str+='&field'+$ga_fields[k]['id']+'='+$tvalue;
						}
						str = str.replace(/\\/g, '%5C');
						str = str.replace(/"/g, '%22');
						$gs_global_check_handler = 'goto_Loc("'+str+'")';
			  		changeButtonImage($ga_buttbig[1]['o'], 'payment');
			  		$ga_buttbig[1]['o'].AddHandler($gs_global_check_handler);
			      $ga_buttbig[1]['v'].Show(1, 1);
				} else $ga_buttbig[1]['v'].Show(0, 1);
				
				//Кнопка Сохранить и выйти
				var rec_id = getParameter('rec_id', '-1');
				var str = 'pb.html?pb=update&recepient='+opId+'&rec_id='+rec_id+'&msg='+msg_name;
				for (k in $ga_fields) {
					var $tvalue = $ga_fields[k]['val'];
					if ($ga_fields[k]['t'] == 'm') {
						var $tmask = $ga_fields[k]['attr']['mask'];
						var value = maskFieldValidation($tvalue, $tmask);
						tvalue = value['number'];
					}
					str+='&field'+$ga_fields[k]['id']+'='+$tvalue;
				}
				str = str.replace(/\\/g, '%5C');
				str = str.replace(/"/g, '%22');
				$gs_global_check_handler = 'goto_Loc("'+str+'")';
				if (rec_id != -1) {
						changeButtonImage($ga_buttbig[0]['o'], 'back');
						$ga_buttbig[0]['v'].Show(1, 1);
						$ga_buttbig[0]['o'].AddHandler($gs_global_check_handler);
				} else $ga_buttbig[0]['v'].Show(0, 1);
				for (var ik in $ga_fields) {
					if ($ga_fields[ik]['t'] == 't') {
						skp = changeFontSize($ga_fields.length, $ga_fields[ik]['val'].length);
						if ($ga_fields[ik]['val'].length > 45 ) $ga_fields[ik]['val'] = $ga_fields[ik]['val'].substr(0,45)+'…';
						change_srt_text($ga_fields[ik]['txi'], $ga_fields[ik]['val'], skp);
					}
				}
				
				//отрисовка комментария для 1-го филда, если он с клавиатурой

				if ($after_entry) {
					var $is_need_show_comment = false;
					
					for(var iz = 0; iz < $ga_fields.length; iz++) {
						if ((($ga_fields[$g_fields_active]['t'] == 't') || ($ga_fields[$g_fields_active]['t'] == 'tp')) &&
							($ga_fields[iz]['pass'] != 0) && 
							!keybordActive) {
								$is_need_show_comment = true;
						}
					}

					if ($is_need_show_comment) 
						create_field_comment('VIEW_ACCOUNT_INLET' + '_' + $ga_fields[0]['id'], 0, 0);
				}
		} else {
					if (pb == 'undefined') {
					    if('undefined' != typeof($op['fields']) && $pr.toLowerCase() == 'cyberplat'){        
						    	var $cf = $op['fields'][$ga_fields[$g_fields_active]['id']];
		        			if('undefined' != typeof($cf['regexp'])){
		        					var $s_number = $ga_fields[$g_fields_active]['val'];
		            			if(null==$s_number.match($cf['regexp'])) $ga_fields[$g_fields_active]['pass'] = 0;
		            			else $ga_fields[$g_fields_active]['pass'] = 1;
		    					}
							}	
					}
					if ($op['fix']!==0 && $op['fix']!==1) {
							if (($ga_fields[$g_fields_active]['id']) == $op['fix']) {
									if ($op['limit']['max'] < parseFloat($ga_fields[$g_fields_active]['val']))	
											$ga_fields[$g_fields_active]['pass'] = 0;
							}
					}
					
					
					flag = -1;
					var flag2 = -1;
	 				for (i=0; i<$ga_fields.length; i++) {
						if ($ga_fields[i]['t']=='i' && $ga_fields[i]['attr']['link']!='0') {
							for (j=0; j<$ga_fields.length; j++) {
									if ($ga_fields[j]['id'] == $ga_fields[i]['attr']['link']) {
										if ($ga_fields[i]['pass']!=1 && $ga_fields[j]['pass']!=1) flag2 = i; 
										break;
									}
							}
							if (flag2 != -1) {
								flag = flag2;
								break;
							}
							
						} else {
								if ($ga_fields[i]['pass']!=1) {
			    					flag = i;
			     					break
			   				}
			   		}
	   			}
	   			changeButtonImage($ga_buttbig[1]['o'], 'next');
	    		if (flag == -1) {
	  				$ga_buttbig[1]['o'].AddHandler($gs_global_check_handler);
	      		$ga_buttbig[1]['v'].Show(1, 1);
					} else {
						$ga_buttbig[1]['v'].Show(0, 1);
					}
	
			    var $rr = false;
	    		if ($ga_fields[$g_fields_active]['pass'] == 1) {
	    				$rr = true;
	        		if (!$g_is_check_played) {
	        				$g_is_check_played = true;
	            		PlaySoundA('confirm_number');
	        		}
	    		}
    }
	  $pb_entry = 1;
    return $rr;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function fields_preCheck() {
        var $opId = parseInt( getParameter('recepient', -1) );
        // $g_vpa[
        switch ($opId) {
                // - - - - - - - - - - - - - - -
                case 401:
                // MjCRIODA($ga_fields[0]);
                // MjCRIODA($g_vpa[$g_current_vpa]['v']);
                var $sfval = $ga_fields[0]['val'];
                var $v1 = ( parseInt($sfval.charAt(0)) + parseInt($sfval.charAt(1))*3 + parseInt($sfval.charAt(2))*7 + parseInt($sfval.charAt(3))*9 ) % 10;
                var $v2 = ( parseInt($sfval.charAt(4)) + parseInt($sfval.charAt(5))*3 + parseInt($sfval.charAt(6))*7 + parseInt($sfval.charAt(7))*9 ) % 10;
                var $vc = $v1 +''+ $v2;
                var $ve = $sfval.substring(8);

                if ($vc !== $ve) {
                        route_to_vp('VIEW_MESSAGE' , 'show_message_precheckerr', 'precheckerr', $g_current_vpa);
                }
                else {
                        goto_Check();
                }
                break;

                // - - - - - - - - - - - - - - -
                default:
                goto_Check();
                break;
        }

        // alert('fields_preCheck()');
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function show_message_precheckerr($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];

        set_rect_text($v, [5, 15, $__s.r[2], 70], $__s, LCP('achtung'));

        $__s1 = SKP('VIEW_TEXT1');
        $__s1.f[3] = '#ff3333';

        var $txt = LCP('entered_code_controldigits_err');
        set_rect_text($v, [55, 210, 600, 70], $__s1, $txt);

}



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// dohlaya!!!
function show_acc_fields($__v) {
        // alert($__v);
        // $__v, $__s, $__i, $__p, $__id

        // MjCRIODA($g_vpa[$__v]);

        var $v = $g_vpa[$__v]['v'];
        var $__s = $g_vpa[$__v]['s'];


        set_rect_text($v, [5, 15, $__s.r[2], 70], $__s, LCP('title_requizites') );

        $sk = SKP('OPERATOR_ICON');


        // MjCRIODA( Parameters );
        var opId = Parameters['recepient'];

        var ndOperator=GetNodeById(opId);
        var opNme = ndOperator.getElementsByTagName("name").item(0).text;
        var opImg = ndOperator.getElementsByTagName("image").item(0).text;
        // alert(opNme);


        // var $o = $__v.AddI($spg.bi, $xofs + $col*$spg.bd[0], $yofs + $row*$spg.bd[1]);
        var $xofs = 100;
        var $yofs = 170;

        var $o = $v.AddI($sk.bi, $xofs, $yofs);
        // $o.AddHandler('goto_Loc("data-entry.html?recepient='+opId+'")');
        $v.AddI(pathToOperators+opImg, $xofs + 12, $yofs +  18);


        $skt = SKP('FORM_TEXT');
        set_rect_text($v, [30, 80, 450, 100], $skt, opNme);



        // alert();
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// create big buttons
var $ga_buttbig = new Array();
var $varBut = SKP('BUTTONS');
var buttons =  new Array();

for (k in $varBut) buttons[k] = $varBut[k];

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_bigbut($__x, label) {
        // alert('wow');
        //  - - - - - -
        var $spb = SKP('BUTBIG');

        var $id = $ga_buttbig.length;
        $ga_buttbig[$id] = new Array();
        var $ae = $ga_buttbig[$id];

        //  - - - - next button
        $ae['v'] = new MJ_VP(cvp_G, -1, -1, -1, $spb.d[0], $spb.d[1], 'DIV');
        var $v = $ae['v'];

        var $x_ofs = 20;
        var $y_ofs = 0;
        if ('undefined' !== typeof($spb.r)) {
            $x_ofs = $spb.r[0];
            $y_ofs = $spb.r[1];
        }

        if ($__x == -2) {
        		$v.o.MoveTo(1, $x_ofs, GTY(1014 - $spb.d[1] - $y_ofs));
        } else {
            $v.o.MoveTo(1, GTX(1280 - $spb.d[0]) - $x_ofs , GTY(1014 - $spb.d[1] - $y_ofs));
        }

        // cvp_PP_Next.o.Border(1, '1px dotted red');
        var $imgf = $spb['i'];

        if ('undefined' !== typeof($spb.i)) {
                $imgf = $spb.i;
        }
        if (('undefined' !== typeof($spb.i_r)) && (-3 === $__x)) {
                $imgf = $spb.i_r;
        }
				
        var $o = $v.AddI($imgf, 0, 0);
        $o.SizeTo(1, $spb.d[0], $spb.d[1])
        $ae['o'] = $o;

        //by default BUTTON_NEXT
        $ae['txi'] = set_rect_text($v, [0, ($spb.d[1]-$spb.f[1])/2+$spb.s[0]-10, $spb.d[0], $spb.f[1]], $spb, '');

        $o.AddHandler('void(0)', $spb.d[0], $spb.d[1]);
        if ($__x == -2) {
                $o.AddKHMask(mjsc_Esc);
                $o.AddKHMask(mjsc_l4);
        }
        else if ($__x == -3) {
                $o.AddKHMask(mjsc_Enter);
                $o.AddKHMask(mjsc_r4);
        }
        $v.Finalize(0, 1);
        return $id;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function bigbutA($__type, $__txt, $__handler) {
        var $id = -1;
        if($__type === 'left'){
              $id = 0;
        }else if($__type === 'right'){
              $id = 1;
        }else{
           return;
        }
        $ga_buttbig[$id]['v'].Show(1, 1);
        change_srt_text($ga_buttbig[$id]['txi'], $__txt);
        $ga_buttbig[$id]['o'].AddHandler($__handler);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_bigbut_static_vp($__v, $__x, $__img, $__text, $__handler) {
        var $spb = SKP('BUTBIG');
        var $v = $g_vpa[$__v]['v'];

        var $x = $v.o.x+$v.o.w-$spb.d[0]-500;
        var $y = $v.o.y+$v.o.h-$spb.d[1]+107;

        var $x_ofs = 20;
        var $y_ofs = 0;
        if ('undefined' !== typeof($spb.r)) {
            $x_ofs = $spb.r[0];
            $y_ofs = $spb.r[1];
        }


        var $x_koeff = 1280/mj_G.Wpx;
        var $y_koeff = 1024/mj_G.Hpx;
        if ($__x == -2) {
                $x = $x_ofs;
                $y = 1014 - $spb.d[1] - $y_ofs;
        }
        else if ($__x == -3) {
                $x = 1280 - $spb.d[0] - $x_ofs;
                $y = 1014 - $spb.d[1] - $y_ofs;
        }
                else if ($__x == -4) {
                $x = (1280 - $spb.d[0])/2;
                $y = 1014 - $spb.d[1] - $y_ofs;
        }
        else if ($__x == -10) {
                $x = 1020;
                $y = 3;
        }
        var $o = $v.AddI($__img, $x-$v.o.x, $y-parseInt($v.o.y*$y_koeff));
        $o.SizeTo(1, $spb.d[0], $spb.d[1]);
        //set_rect_text($v, [$x-$v.o.x, $y-parseInt(($v.o.y-23)*$y_koeff), $spb.d[0], $spb.f[1]], $spb, $__text);
        $o.AddHandler($__handler, $spb.d[0], $spb.d[1]);
        if ($__x == -2) {
                $o.AddKHMask(mjsc_Esc);
                $o.AddKHMask(mjsc_l4);
        }
        else if ($__x == -3) {
                $o.AddKHMask(mjsc_Enter);
                $o.AddKHMask(mjsc_r4);
        }
}



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function goto_get_allurlparams()
{
        var $s = '';
        for ($k in Parameters) {
            if ($k != 'act')
                $s += (($s !== '') ? '&' : '') + $k+'='+Parameters[$k];
        }
        return $s;

}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_info($__v) {
    var $v = $g_vpa[$__v]['v'];
    var $s = SKP('OPERATOR_ICON');
    if ($intro) $g_vpa[$gIntroFlashVPA]['v'].Show(0, 1);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_help($__v) {
		if ($intro) $g_vpa[$gIntroFlashVPA]['v'].Show(0, 1);
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_metro_menu_level() {

}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_metro_message($__v) {

        var $v = $g_vpa[$g_current_vpa]['v'];
        var $s = $g_vpa[$g_current_vpa]['s'];
        set_rect_text($v, [500, 450, $s.r[2], 3], $s, LCP('metro_insert_card') );

        $ga_buttbig[0]['v'].Show(1, 1);
        //change_srt_text($ga_buttbig[0]['txi'], LCP('button_cancel'));
        changeButtonImage($ga_buttbig[0]['o'], 'cancel');
        $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');


}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_metro_menu($__v) {
        // $g_vpa[$g_current_vpa]['v'].Show(0, 1);

        var $metro_styles = '<style>'+SKP('METRO_STYLES').s+'</style>';

        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];

       var $id = $g_vpa[$__v]['i'];
       // prev element
       var $id_prev = 0;
       if ($id != -1) {
                       $id_prev = $ga_metro_menu['Menu'][$id][1];
       }
       // alert($id);


        var $skp_i = SKP('METRO_MENU_ITEM');

        set_rect_text($v, [500, 25, 475, 3], $s, 'Номер карты: ' + $ga_metro_menu['CardNum'] );
        set_rect_text($v, [15, 25, 475, 3], $s, 'Статус карты: ' + $metro_styles+$ga_metro_menu['Message'] );


        var $ai = 0;
        var $l = $ga_metro_menu['Menu'].length;
        var $y_ofs = 260;
        var $x_ofs = 20;

        // alert( $ga_metro_menu['Menu'].length );
        var $iy = 0;
        for (var $i=0; $i<$l; $i++) {
                // alert($ga_metro_menu['Menu'][$i][2]);
                if ($ga_metro_menu['Menu'][$i][1] != $id_prev)  continue;

                var $wx = $skp_i.d[0];
                var $wy = $skp_i.d[1];
                        var $x = $x_ofs + ($wx+10)*($iy%2);
                        var $y = $y_ofs + ($wy+5)*(Math.floor($iy/2));

                        var $o = $v.AddI($skp_i.i, $x, $y);
                set_rect_text($v, [$x, $y+5, $wx, $wy], $skp_i, $ga_metro_menu['Menu'][$i][2]);
                if ($ga_metro_menu['Menu'][$i][4] !== 0) {
                                $o.AddHandler("route_to_vp('VIEW_METRO_MENU', 'create_metro_menu', '"+$ga_metro_menu['Menu'][$i][0]+"', '"+$__v+"')");
                }
                else {
                                // $o.AddHandler("goto_Check('VIEW_METRO_MENU', 'create_metro_menu', '"+$ga_metro_menu['Menu'][$i][0]+"', '"+$__v+"')");
                        $__s = 'checking.html'+'?recepient=' + Parameters['recepient'] + '&field100='+$ga_metro_menu['Menu'][$i][0];
                        $o.AddHandler('goto_Loc("'+$__s+'")');
                }
                        // $o.AddHandler("alert('test!')");
                         //

                $iy++;

                // set_rect_text($v, [$x_ofs, $y_ofs, $skp_i.d[0], $skp_i.d[1]], $skp_i, $ga_metro_menu['Menu'][$i][2]);
                // $y_ofs += 18;

                // $i++;
        }
        // alert($i);



        // !!!!!
        var $xofs = 50;
        var $yofs = 100;

        /*
var $ga_metro_menu = {
     CardNum: 92001112201,
     CardStatus: '<div class="elInfoNormal">ПД многоразовый на 90 дней<br>Остаток поездок: 7<br>Начало действия: 09.03.2004<br>Окончание действия: 07.06.2004<br>Багаж: 1</div><div class="elInfoBlack">Доплата разрешена</div>',
     ItemsNum: 19,
     Menu: [[1, 0, 'ПБ многоразовый по тарифу', 0, 1],[2, 1, '5 Поездок', 60, 0],[3, 1, '10 Поездок', 120, 0],[4, 1, '20 Поездок', 240, 0],[5, 1, '30 Поездок', 360, 0],[6, 0, 'ПБ на 50 поездок и 30 дней', 456, 0],[7, 0, 'ПБ на 25 поездок и 15 дней', 233, 0],[8, 0, 'ПБ на 40 поездок и 30 дней', 368, 0],[9, 0, 'ПБ на 20 поездок и 15 дней', 188, 1],[10, 0, 'ПБ на 10 поездок и 7 дней', 98, 0],[11, 0, 'ПБ многоразовый на 90 дней', 0, 1],[12, 11, '3 Поездки', 36, 0],[13, 11, '20 Поездок', 240, 0],[14, 11, '42 Поездки', 493, 0],[15, 11, '60 Поездок', 690, 0],[16, 0, 'Единый на месяц на БСК', 750, 0],[17, 0, 'Проверка 0123456789 123456789 123456789 123456789', 140, 0],[18, 0, 'Проверка 0123456789 123456789 123456789', 280, 0],[19, 0, 'Проверка 0123456789 123456789', 420, 0]]
}

        */

        // var
        /*
        set_rect_text($v, [5, 5, $s.r[2], 3], $s, LCP('title_requizites') );
        set_rect_text($v['v'], [5, 10, $__s.r[2], 70], $__s, getParameter('cmd', 'unknown') );
        */
        // var $sp = SKP('PINPAD');


        $ga_buttbig[0]['v'].Show(1, 1);
        //change_srt_text($ga_buttbig[0]['txi'], LCP('button_back'));
        // $ga_buttbig[0]['o'].AddHandler('goto_Loc("message.html?res=1")');
        $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');

}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function PlaySoundA($__s) {
        var $vol  = 10; // from config
        if ($vol > 0) {
                var $fname = '';
                if ('undefined' !== typeof(LCP('sound_files')[$__s])) {
                        $fname = LCP('sound_files')[$__s];
                }
                if ($fname.length > 1) {
                        // alert($fname.length);
                        goto_LocNos('command.html?cmd=playsound&parm='+$fname);
                }
        }

}



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function create_locale_select($__v) {
        var $v = $g_vpa[$__v]['v'];
        var $s = $g_vpa[$__v]['s'];



        var $s_dstloc = 'ru';
        if ($mj_g_locale == 'ru') {
                $s_dstloc = 'en';
        }

        // var $o = $v.AddI('locale_ru.jpg', 0, 0);
        Parameters['locale'] = $s_dstloc;
        var $o = $v.AddI('locale_'+$s_dstloc+'.jpg', 0, 0);
        $o.AddHandler('goto_Loc("'+$mj_g_docnme+'?'+goto_get_allurlparams()+'")', 160, 80);

        /*
        // var $o = $v.AddI('locale_'+$mj_g_locale+'.jpg', 200, 0);
        var $o = $v.AddI('locale_'+$mj_g_locale+'.jpg', 200, 0);
        Parameters['locale'] = 'ru';
        $o.AddHandler('goto_Loc("'+$mj_g_docnme+'?'+goto_get_allurlparams()+'")');
        // $o.AddHandler("route_to_vp('VIEW_METRO_MENU', 'create_metro_menu', '"+$ga_metro_menu['Menu'][$i][0]+"', '"+$__v+"')");

        */
        /*
var $o = $v.AddI($skp_i.i, $x, $y);
                set_rect_text($v, [$x, $y+5, $wx, $wy], $skp_i, $ga_metro_menu['Menu'][$i][2]);
                if ($ga_metro_menu['Menu'][$i][4] !== 0) {
                                $o.AddHandler("route_to_vp('VIEW_METRO_MENU', 'create_metro_menu', '"+$ga_metro_menu['Menu'][$i][0]+"', '"+$__v+"')");

                                */
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function calculate(){
        document.location=document.location+"&action=calculate_commission";
}

// [ file include done. ]
$iface_runtime_js = true;

function is_field_exist(oper_id, field_id) {
	var $op = jcfg_findOperatorById(payment_book['groups'][k]['records'][j]['operator_id']);
	var result = false;

	for (var f_id in $op['fields'])
		if (f_id == field_id) {
			result = true;
			break;
		}

	return result;
}

// MjCRIODA($xml1);

function makeStateURL() {
var pstate = getParameter('pstate', 'ok');
var currentURL;

if (pstate == 'vld')
	currentURL = 'main.html';
else if (pstate == 'ok')
	currentURL = 'pb.html?pb=getpb';
	
return currentURL;
}
