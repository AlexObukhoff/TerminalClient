//Создание глобальных переменных
//==============================
$ga_fields = new Array();
create_bigbut(-2);
create_bigbut(-3);

//переписали немного функции создания поля.
//Добавили координаты вывода
function createPBField(currentVP, title, id, type, attr, comment, def, x0, y0) {
	    var $v = $g_vpa[currentVP]['v'];
	    var $s = $g_vpa[currentVP]['s'];
        
        var $f_id = $ga_fields.length;
        $ga_fields[$f_id] = new Array();
        var $ae = $ga_fields[$f_id];
		
		//парсим ББ коды в заголовке
		title = title.replace(/\[/g, '<');
		title = title.replace(/\]/g, '>');

		//Размеры кнопок
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
       	//Длина имени поля
       	textWidth = $skc.r[2];
       	//Длина значения поля
       	valueWidth = parseInt($skc.r[2]-22);
		
        $o = $v.AddI($skc.i, x0, y0);
        $o.SizeTo(1, $skc.r[2], $skc.r[3]);
				$o.AddHandler('activate_field("'+$f_id+'")', $skc.r[2], $skc.r[3]);
        $ae['txb'] = $o;
        $ae['v'] = -1;
        $ae['t'] = type;
        $ae['a'] = attr;
        $ae['id'] = id;
        $ae['title'] = title;
        $ae['pass'] = 0;
        $ae['val'] = ''; // -1;
        $ae['was_activated'] = 0; 
        $ae['second_activated'] = 0; 
        if ('undefined' !== typeof(def)) $ae['val'] = def;
        $ae['attr'] = attr;
        var txt = $ae['val'];
        if (txt.length > 45 ) txt = txt.substr(0,45)+'…';
			
        $ae['txi'] = set_rect_text($v, [x0, (y0+k1), valueWidth, $skc.r[3]], $skc, txt);
        $skf = SKP('VIEW_ACCOUNT_DETAILS_FINFO');
        set_rect_text($v, [x0+dx, y0-k2, textWidth, $skc.r[3]], $skf, title);
        $ae['commft'] = comment;
        
        if ($ae['t'] === 'm') {
        		tmp = maskFieldValidation($ae['val'], $ae['attr']['mask'])
        		$ae['pass'] = tmp['pass']
        } 
        return $f_id;
}

//Страница: Ввод номера
//======================================
function createLogin() {
		create_vpa('PAGE_LOGIN', 'createLoginPage', '-1', '-1');
}

var getpin = '';
var getpinAction = 'void(0)';
var getpinMessage = '';
var getpinAllMessages = new Array();
function createLoginPage(currentVP) {
		var localSKP = $g_vpa[currentVP]['s'];
    var localVP = $g_vpa[currentVP]['v'];
    
    //Выводим всю статику
    for (k in localSKP.staticImage) {
	    var o  = localVP.AddI(localSKP.staticImage[k][0], localSKP.staticImage[k][1], localSKP.staticImage[k][2]);
			o.SizeTo(1, localSKP.staticImage[k][3], localSKP.staticImage[k][4]);
			if (typeof(localSKP.staticImage[k][5])!='undefined') o.AddHandler(localSKP.button[k][5]);
    }
	  //Текст
    for (k in localSKP.staticText) set_rect_text(localVP, [localSKP.staticText[k][1],localSKP.staticText[k][2],localSKP.staticText[k][3],localSKP.staticText[k][4]], localSKP, localSKP.staticText[k][0]);
    
		//Кнопка Получить Пин
    getpin  = localVP.AddI(localSKP.dynamicImage['getpin'][0], localSKP.dynamicImage['getpin'][1], localSKP.dynamicImage['getpin'][2]);
		getpin.SizeTo(1, localSKP.dynamicImage['getpin'][3], localSKP.dynamicImage['getpin'][4]);
		getpin.AddHandler(getpinAction);

		//Надписи
		for (k in localSKP.dynamicText) {
			getpinAllMessages[k] = new Array
			getpinAllMessages[k] = localSKP.dynamicText[k];
		}
		getpinMessage = set_rect_text(localVP, [getpinAllMessages[1][1],getpinAllMessages[1][2],getpinAllMessages[1][3],getpinAllMessages[1][4]], localSKP, getpinAllMessages[1][0]);			

		var error = getParameter('error', 'undefined');
		if (error == 'badpin') {
				set_rect_text(localVP, [getpinAllMessages[3][1],getpinAllMessages[3][2],getpinAllMessages[3][3],getpinAllMessages[3][4]], localSKP, getpinAllMessages[3][0]);
		}

		var number = getParameter('number', '');
    $fields_count++;
    createPBField(currentVP, localSKP.field_login[0], localSKP.field_login[1], localSKP.field_login[2], {mask: localSKP.field_login[3]}, localSKP.field_login[4], number, localSKP.field_login[5], localSKP.field_login[6]);
    $fields_count++;
    createPBField(currentVP, localSKP.field_pwd[0], localSKP.field_pwd[1], localSKP.field_pwd[2], {min: localSKP.field_pwd[3][0], max: localSKP.field_pwd[3][1]}, localSKP.field_pwd[4], '', localSKP.field_pwd[5], localSKP.field_pwd[6]);
    activate_field(0);
    
    //кнопка Назад
    $ga_buttbig[0]['v'].Show(1, 1);
    changeButtonImage($ga_buttbig[0]['o'], 'main');
    $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');
    
}


//Страница: Получения пина
//======================================
function getPin() {
		create_vpa('PAGE_GETPIN', 'createGetPinPage', '-1', '-1');
}

function createGetPinPage(currentVP) {
    var localVP = $g_vpa[currentVP]['v'];
		var localSKP = $g_vpa[currentVP]['s'];
		var skp = SKP('PAGE_LOGIN');
    //Выводим всю статику
    //Картинки
    for (k in localSKP.staticImage) {
    	var o  = localVP.AddI(localSKP.staticImage[k][0], localSKP.staticImage[k][1], localSKP.staticImage[k][2]);
		o.SizeTo(1, localSKP.staticImage[k][3], localSKP.staticImage[k][4]);
		if (typeof(localSKP.staticImage[k][5])!='undefined') o.AddHandler(localSKP.staticImage[k][5]);
    }
    
    //Текст
    for (k in localSKP.staticText) {
    	set_rect_text(localVP, [localSKP.staticText[k][1],localSKP.staticText[k][2],localSKP.staticText[k][3],localSKP.staticText[k][4]], localSKP, localSKP.staticText[k][0]);
    }
    //Динамические надписи
    number = getParameter('number', '');
    var res = maskFieldValidation(number, skp.field[3])
    txt = 'Номер телефона:<br><span style="font-size:40px">' + res['view'] + '</span>';
    set_rect_text(localVP, [517,100,450,100], localSKP, txt);
    
 		//кнопка Назад
    $ga_buttbig[0]['v'].Show(1, 1);
    changeButtonImage($ga_buttbig[0]['o'], 'main');
    $ga_buttbig[0]['o'].AddHandler('goto_Loc("pb.html?pb=login")');
}

//Страница: Ввод Пина
//======================================
function enterPin() {
		create_vpa('PAGE_ENTERPIN', 'createEnterPinPage', '-1', '-1');
}

function createEnterPinPage(currentVP) {
    var localVP = $g_vpa[currentVP]['v'];
    var localSKP = $g_vpa[currentVP]['s'];		

		var error = getParameter('error', 'undefined');
		errorTxt = '<span style="color:red;font-size:35px">Неправильный PIN-код</span>';
		if (error == 'badpin') {
			set_rect_text(localVP, [30,221,400,100], localSKP, errorTxt);
		}

    //Выводим всю статику
    //Картинки
    for (k in localSKP.staticImage) {
   	var o  = localVP.AddI(localSKP.staticImage[k][0], localSKP.staticImage[k][1], localSKP.staticImage[k][2]);
		o.SizeTo(1, localSKP.staticImage[k][3], localSKP.staticImage[k][4]);
		if (typeof(localSKP.staticImage[k][5])!='undefined') o.AddHandler(localSKP.staticImage[k][5]);
    }
    //Текст
    for (k in localSKP.staticText) {
    	set_rect_text(localVP, [localSKP.staticText[k][1],localSKP.staticText[k][2],localSKP.staticText[k][3],localSKP.staticText[k][4]], localSKP, localSKP.staticText[k][0]);
    }
    $fields_count++;
    createPBField(currentVP, localSKP.field[0], localSKP.field[1], localSKP.field[2], {min: localSKP.field[3][0], max: localSKP.field[3][1]}, localSKP.field[4], '', localSKP.field[5], localSKP.field[6]);
    activate_field(0);
    
    //кнопка Назад
    $ga_buttbig[0]['v'].Show(1, 1);
    changeButtonImage($ga_buttbig[0]['o'], 'main');
    $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');
    
    //Кнопка Вперед
}

//Страница: Записная книга
//======================================
function getPB() {
		create_vpa('PAGE_PB', 'createPBPage', '-1', '-1');
}

function createPBPage(currentVP) {
	var localSKP = $g_vpa[currentVP]['s'];
  var localVP = $g_vpa[currentVP]['v'];
  var operatorSKP = SKP('OPERATOR_ICON');
	var o = '';
	var txt = '';
	var pbConfigLength = 1;
  for (k in payment_book['groups']) {
			for (j in payment_book['groups'][k]['records'])	{
		    	//подложка
		    	o  = localVP.AddD(localSKP.itemsPos[pbConfigLength][0], localSKP.itemsPos[pbConfigLength][1]);
					o.SizeTo(1, localSKP.itemsPos[pbConfigLength][2], localSKP.itemsPos[pbConfigLength][3]);
	        var $iii = MJGA(o.i);
					$iii.style['filter'] = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src=\''+$mj_g_img_pth+localSKP.itemsImage[payment_book['groups'][k]['records'][j]['color']]+'\', sizingMethod=\'scale\')';
					
					var url= 'pb.html?pb=dataentry&rec_id='+j+'&recepient='+payment_book['groups'][k]['records'][j]['operator_id'];
					var txt='';
					txt = payment_book['groups'][k]['records'][j]['name'];
					if ("undefined" == typeof(txt))
						txt = "";
					
					if(txt.length >= maxButtonCommentLength)
						txt = txt.substring(0, maxButtonCommentLength - 2) + "…";

					var field_id_right = true;
					for (i in payment_book['groups'][k]['records'][j]['fields']) {
						//if (txt == '') { 
						var op_id = payment_book['groups'][k]['records'][j]['operator_id'];
						var op = jcfg_findOperatorById(op_id);
						var cf = op['fields'][i];

						if (field_id_right) {
							if (is_field_exist(op_id, i)) {
								field_id_right = false;
								var val = '';
								if (cf['type'] == 'masked') {
										var res = maskFieldValidation(payment_book['groups'][k]['records'][j]['fields'][i], cf['mask']);
										val = res['view'];
								} else if (cf['type'] == 'integer') val = payment_book['groups'][k]['records'][j]['fields'][i];
								else if (cf['type'] == 'enum') {
									var ekey = payment_book['groups'][k]['records'][j]['fields'][i];
									if (ekey!='') 
										for(var ei in cf['enum'])
											if (ekey == ei) 
												val = cf['enum'][ei];
											else
												for(var ej in cf['enum'][ei])
													if (ekey == ej) 
														val= cf['enum'][ei][ej];
													else
														for(var ek in cf['enum'][ei][ej])
															if (ekey == ek) 
																val = cf['enum'][ei][ej][ek];
									//val = cf['enum'][payment_book['groups'][k]['records'][j]['fields'][i]]
								}
								else if (cf['type'] == 'text') val = payment_book['groups'][k]['records'][j]['fields'][i]
								else alert('Нет такого поля')

								if ("undefined" == typeof(val))
								    val = "";

								if((val.length > maxButtonTextLength) && ((cf['type'] == 'text') || (cf['type'] == 'enum')))
								    val = val.substring(0, maxButtonTextLength - 2) + "…";
									
								txt += '<br/> <span style="font-weight:normal;font-size:smaller;">' + val + '</span>';								
							}
						}
						url+='&field'+ i + '='+payment_book['groups'][k]['records'][j]['fields'][i];
					}
					url+='&name='+payment_book['groups'][k]['records'][j]['name'];
					url = url.replace(/\\/g, '%5C');
					url = url.replace(/"/g, '%22');
					o.AddHandler('goto_Loc("'+url+'")');
					

					var opId = payment_book['groups'][k]['records'][j]['operator_id'];
					var op = jcfg_findOperatorById(opId);
					//логотип
					gfxa_ins_oper_logo(localVP, op['image'], localSKP.itemsPos[pbConfigLength][0]+localSKP.logoOffset[0], localSKP.itemsPos[pbConfigLength][1]+localSKP.logoOffset[1], operatorSKP.d[0], operatorSKP.d[1], '');
 					set_rect_text(localVP, [localSKP.itemsPos[pbConfigLength][0]+localSKP.textOffset[0], localSKP.itemsPos[pbConfigLength][1]+localSKP.textOffset[1], localSKP.textOffset[2], localSKP.textOffset[3]], localSKP, txt);
					pbConfigLength++;			
					
			}
  }	
		if (pbConfigLength == 0) {
				set_rect_text(localVP, [268, 0, 744, 620], localSKP, LCP('pb_no_items'), true);
		}
		
		
    //кнопка Назад
    $ga_buttbig[0]['v'].Show(1, 1);
    changeButtonImage($ga_buttbig[0]['o'], 'exit');
    $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');
    
    //кнопка Добавить
    if (pbConfigLength <= maxPB) {
		buttons['add']	= $varBut.add;
		$ga_buttbig[1]['v'].Show(1, 1);
    	changeButtonImage($ga_buttbig[1]['o'], 'add');
    	$ga_buttbig[1]['o'].AddHandler('goto_Loc("pb.html?pb=addItem")');
    }
}

//Страница: Главное меню
//======================================
function mainMenu() {
		$g_current_vpa = create_vpa('VIEW_MAINMENU', 'create_menu_icons', '-1', '-1');
}

//Страница: Ввод данных(data-entry)
//======================================
function dataEntry() {
		$g_current_vpa = create_vpa('VIEW_ACCOUNT_DETAILS', 'create_account_fields', '-1', '-1');
	  var localVP 	= $g_vpa[$g_current_vpa]['v'];
	  var localSKP 	= SKP($g_current_vpa);
		var currentPage = getParameter('pb', 'undefined');
		var recepient = getParameter('recepient', 'undefined');
		var o;
		
		//добавить
		if (currentPage.toLowerCase() == 'enteritem') {

			//назад на страницу с ЗК
			$ga_buttbig[0]['v'].Show(1, 1);
    	changeButtonImage($ga_buttbig[0]['o'], 'main');
    	$ga_buttbig[0]['o'].AddHandler('goto_Loc("pb.html?pb=getpb")');
    		
    	//кнопка добавить автоматом строиться в verify_field();
		} 
		
		//редактировать, удалить, платить
		else {
			
			//кнопка сохранить и выйти
			var rec_id = getParameter('rec_id', '-1');
			var opId = getParameter('recepient', 'undefined');
			var msg_name = getParameter('name', '');
			var str = 'pb.html?pb=update&recepient='+opId+'&rec_id='+rec_id+'&msg='+msg_name;
			//payment_book['groups'][k]['records'][j]['name']
			for (k in $ga_fields) {
				var $tvalue = $ga_fields[k]['val'];
				if ($ga_fields[k]['t'] == 'm') {
					var $tmask = $ga_fields[k]['attr']['mask'];
					$tmask = $tmask.replace(/[^*|!.+!]/g, '');
					$tmask = $tmask.replace(/!/g, '');

					var value = '';
					for(var i = 0; i < $tmask.length; i++)
						value += (($tmask.charAt(i) != '') && ($tvalue.charAt(i) == '')) ? $tmask.charAt(i) : $tvalue.charAt(i);
					$tvalue = value;
				}
				str+='&field'+$ga_fields[k]['id']+'='+$tvalue;
			}

			url = 'goto_Loc("'+str+'")';
			url = url.replace(/\\/g, '%5C');
			if (rec_id != -1) {
					changeButtonImage($ga_buttbig[0]['o'], 'back');
					$ga_buttbig[0]['v'].Show(1, 1);
					$ga_buttbig[0]['o'].AddHandler(url);
			}
			//кнопка оплатить автоматом строиться в verify_field;
		}
		if (($mj_g_docnme == 'pb.html') && 
			(getParameter('pb', 'undefined') == 'dataentry')) {
			if (($ga_fields[0]['t'] != 't') && 
				($ga_fields[0]['t'] != 'tp')) {
					activate_field(0);
					cvp_PP_PinPress();
			} else {
					var inlet = 'VIEW_ACCOUNT_INLET';
					var $name = inlet + '_' + $ga_fields[0]['id'];
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
				$g_vpa[$name]['v'].Show(1, 0);
				create_field_comment($name, 0);
			}
		}
}

function checking() {
	create_vpa('VIEW_MESSAGE_CHECKING', 'show_message_checking', '-1', '-1');
	$ga_buttbig[0]['v'].Show(1, 1);
 	changeButtonImage($ga_buttbig[0]['o'], 'main');
 	$ga_buttbig[0]['o'].AddHandler('goto_Loc("pb.html?pb=getpb")');

	/*
	var recepient = getParameter('recepient', 'undefined');
	$ga_buttbig[1]['v'].Show(1, 1);
  changeButtonImage($ga_buttbig[1]['o'], 'next');
  $ga_buttbig[1]['o'].AddHandler('goto_Loc("pb.html?pb=payment&recepient='+recepient+'&field100=7849565445")');
  */
	
}
function payment() {
    create_vpa('VIEW_MESSAGE_COMISSION', 'show_comission_in_payment', '-1', '-1');
    create_vpa('VIEW_MESSAGE_PAYMENT', 'show_message_payment', '-1', '-1');
    
   	var recepient = getParameter('recepient', 'undefined');
		var sum  = parseFloat(getParameter('sum', 0) );
		var $mps = parseFloat(getParameter('mps', 0) );
		var $maxsum = parseFloat(Parameters['maxsum']);
		var $fixcms = parseFloat(Parameters['fixcms']);
		var $cms = parseFloat(Parameters['cms']);

		if ((sum > 0) && (getParameter('rest', '-1') != '0')) {
			//Кнопка Назад - убрана
			$ga_buttbig[0]['v'].Show(0, 1);
		} else {
			//Кнопка Назад - появилась
			$ga_buttbig[0]['v'].Show(1, 1);
			changeButtonImage($ga_buttbig[0]['o'], 'main');
			$ga_buttbig[0]['o'].AddHandler('goto_Loc("' + makeStateURL() + '")');
		}
	var is_forward_button = ($mps == $maxsum) ? (sum >= $maxsum) : ((sum - $cms) >= $mps);
  	if (is_forward_button) {
  			//Кнопка Назад - убрана
	  		$ga_buttbig[0]['v'].Show(0, 1);

		   	//Кнопка Вперед - появилась
		   	var localUrl = 'pb.html?pb=payment&end=1';
		   	for (k in Parameters) {
		  			if (k!='pb') {
		  					localUrl+='&'+k+'='+Parameters[k];
		  			}
		  	}
			localUrl = localUrl.replace(/\\/g, '%5C');
		   	$ga_buttbig[1]['v'].Show(1, 1);
		    changeButtonImage($ga_buttbig[1]['o'], 'payment');
		    $ga_buttbig[1]['o'].AddHandler('goto_Loc("'+localUrl+'")');
  	} else {
    		//Кнопка Вперед - убрана
    		$ga_buttbig[1]['v'].Show(0, 1);
  	}
   	


}

function thanks() {
	$g_current_vpa = create_vpa('VIEW_MESSAGE', 'show_message_thanks', '-1', '-1');
	create_vpa('ADV_FLASH4');
	var recepient = getParameter('recepient', 'undefined');
	var rest = getParameter('rest', '-1');
	var $pstate = getParameter('pstate', 'ok');
	
	if (rest == '-1') {
		//ок
		$ga_buttbig[1]['v'].Show(1, 1);
	    changeButtonImage($ga_buttbig[1]['o'], 'ok');
	    $ga_buttbig[1]['o'].AddHandler('goto_Loc("' + makeStateURL() + '")');
	} else {
		//нет
		$ga_buttbig[0]['v'].Show(1, 1);
	    changeButtonImage($ga_buttbig[0]['o'], 'no');
	    $ga_buttbig[0]['o'].AddHandler('goto_Loc("' + makeStateURL() + '")');
	
		//да
		$ga_buttbig[1]['v'].Show(1, 1);
	    changeButtonImage($ga_buttbig[1]['o'], 'yes');
	    $ga_buttbig[1]['o'].AddHandler('goto_Loc("data-entry.html?recepient=999&state=pb&pstate=' + $pstate + '")');
	}
	
}

function showError() {
		create_vpa('VIEW_MESSAGE_ERROR', 'showErrorPage', '-1', '-1');
}

function showErrorPage(currentVP) {
	var localSKP = $g_vpa[currentVP]['s'];
  var localVP = $g_vpa[currentVP]['v'];
	var rest = getParameter('rest', '-1');
	
	var errorNumber = getParameter('errornumber', -1);

	var $printer_ok_only = false;
	var $op = jcfg_findOperatorById(getParameter('recepient', -1));
	if (($op !== -1) && ($op['printer_ok_only'] == 1))
		$printer_ok_only = true;
	var txt = GetMessageDbError(errorNumber);

	//кнопка Назад
	$ga_buttbig[0]['v'].Show(1, 1);
	var $btnTitile = 'back';
	if (errorNumber == '-7')
		$btnTitile = 'no';
	changeButtonImage($ga_buttbig[0]['o'], $btnTitile);
	$ga_buttbig[0]['o'].AddHandler('goto_Loc("' + makeStateURL() + '")');
	
	if (errorNumber == 'showaddinfo') {
		  txt = getData();

		  //кнопка Вперед
		  $ga_buttbig[1]['v'].Show(1, 1);
		  changeButtonImage($ga_buttbig[1]['o'], 'yes');
		  var s = '';
		  for (k in Parameters) {
             if ((k !== 'pb') && (k !== 'errornumber'))
				s += '&' + k + '=' + Parameters[k];
		  }
		  $ga_buttbig[1]['o'].AddHandler('goto_Loc("pb.html?pb=payment' + s + '")');
	} else if (errorNumber == -7) {
	  if (!$printer_ok_only) {
		  //кнопка Вперед
		  $ga_buttbig[1]['v'].Show(1, 1);
		  changeButtonImage($ga_buttbig[1]['o'], 'yes');
		  $ga_buttbig[1]['o'].AddHandler('goto_Loc("pb.html?pb=error&res=1")');
	  } else
		  txt = LCP('prn_cantprint_message_fail');
	} else if (	(errorNumber == -9) || 
				(errorNumber == -10) ||
				(errorNumber == -11)){
		  $ga_buttbig[0]['v'].Show(0, 1);
		  var $s = '';
	      for ($k in Parameters) {
			if (($k != 'act') && 
				($k != 'error') &&
				($k != 'errornumber') &&
				($k != 'pb')
				)
				$s += (($s !== '') ? '&' : '') + $k+'='+Parameters[$k];
	      }
		  timerMainPage = setTimeout('goto_Loc("pb.html?pb=payment&end=1&'+$s+'")',5000);
	} else if (errorNumber == -8) {
		  //убираем кнопку Назад
		  $ga_buttbig[0]['v'].Show(0, 1);
		  if (rest == '-1') {
			  timerMainPage = setTimeout('goto_Loc("' + makeStateURL() + '")',3000);
		  } else {
			  var s = '';
			  for (k in Parameters) {
                s += ((k !== 'pb') && (k !== 'errornumber')) ? (((s !== '') ? '&' : '') + k+'='+Parameters[k]) : '';
			  }
			  timerMainPage = setTimeout('goto_Loc("pb.html?pb=thanks&state=pb&' + s + '");',5000);
		  }
	} else if ((errorNumber == -5) || (errorNumber == -6)){
		  $ga_buttbig[0]['o'].AddHandler('goto_Loc("main.html")');
	}
	
	set_rect_text(localVP, [40, 0, 660, 621], localSKP, txt, true);
}

function loading() {
		create_vpa('VIEW_LOADING');
		create_vpa('VIEW_LOADING_FLASH');	
}

function def() {
	$ga_buttbig[0]['v'].Show(1, 1);
  changeButtonImage($ga_buttbig[0]['o'], 'main');
  $ga_buttbig[0]['o'].AddHandler('goto_Loc("pb.html?pb=getpb")');
}
//===================
$pb_model_js = true;


