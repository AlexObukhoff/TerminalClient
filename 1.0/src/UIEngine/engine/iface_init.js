 /**
* |+|----------------------------------------------------------------------|
* | [CYBERPLAT interface runtime filescript]
* |
* | |----------------------------------------------------------------------|
* | | initialization pre-run script
* | |
* | | ! does runtime initialization and dynamic loading curent skin
* | | ! and iface_runtime.js - main interface file
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
var $gs_iface_version = '2.0.0080.000 (20070921 13:52)';

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function parseBBCode($__s) {
        $__s = $__s.replace(/\[/g, '<');
        $__s = $__s.replace(/\]/g, '>');
        return $__s;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// logging debug and error constants and service functions
var CYBIF_LOG_MSG = 0;
var CYBIF_LOG_NFO = 16;
var CYBIF_LOG_WRN = 32;
var CYBIF_LOG_ERR = 128;

var CYBIF_LOG_TRANSA = new Array();
CYBIF_LOG_TRANSA[CYBIF_LOG_MSG] = 'Simply message';
CYBIF_LOG_TRANSA[CYBIF_LOG_NFO] = 'Notify message';
CYBIF_LOG_TRANSA[CYBIF_LOG_WRN] = '!WARNING message';
CYBIF_LOG_TRANSA[CYBIF_LOG_ERR] = '!!!ERROR message';

var CYBIF_MSG_CNT = 0;

function CybIf_Logger($__typ, $__msg) {
        CYBIF_MSG_CNT++;
        if ($__typ < CYBIF_LOG_ERR) return;
        alert('[Cyber Interface Logger]\n\n_________event cnt/type:_________\n' + CYBIF_MSG_CNT +'/'+ $__typ + ' (' + CYBIF_LOG_TRANSA[$__typ] + ')\n\n_________event context:_________\n' + $__msg);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
основной файл генерации страниц интерфейса

*/
var $is_debug = true;
var $is_debug = false;
var $is_show_hi = false;

//  - - - - - получаем текущую конфигурацию из config.xml
// GetConfigInfo();
if ('undefined' === typeof($ga_jcfg)) {
        CybIf_Logger(CYBIF_LOG_ERR, 'iface_init.js: iface_config.js inclusion is not detected!');
}


var SkinName = $ga_jcfg['config']['SkinName'];
var MainMenuMarqueeString = parseBBCode($ga_jcfg['config']['MainMenuMarqueeString']);
var CurrencyType = $ga_jcfg['config']['CurrencyType'].toUpperCase();
var CurrencyName = $ga_jcfg['config']['CurrencyName'];
var TerminalName = parseBBCode($ga_jcfg['config']['TerminalName']);
var SupportString = parseBBCode($ga_jcfg['config']['SupportString']);
var ShowPB = parseBBCode($ga_jcfg['config']['ShowPB']);
var pathToOperators = "../../logo/";
var maxFields = 5;
var maxEnum = 9;


// alert(CurrencyType);

//  - - - - - инициализация глобальных переменных
// картинки шкурки
var $mj_g_img_pth = '../../skins/'+SkinName+'/';

var $mg_g_vp_bg = '#cccccc';
var $mg_g_vp_debug = $is_debug;        // под-отладка элементов отображения

// $alert(SkinName);


//  - - - - - подготавливаем видимую область, устанавливаем клиппинг
//                           защищаем от возможных полос прокрутки
document.body.style['overflow'] = 'hidden';
document.body.style['border'] = '0px';

// creating global viewport
mj_G.Wpx = mj_G._domb.clientWidth;
mj_G.Hpx = mj_G._domb.clientHeight;


document.body.style['clip'] = 'rect(0px, '+mj_G.Wpx+'px, '+mj_G.Hpx+'px, 0px)';


// - - - - - - каков текущий документ + разбор текущего URI?
var $s_uri = document.location.href;

var $re = new RegExp('(.*?)/locale/([^/]+)/([^\\?]*)', 'gi');
$re.exec($s_uri);
// alert($s_uri);
var $mj_g_pathwc = RegExp.$1;        // path to webclient
var $mj_g_pathlc = $mj_g_pathwc + 'interface/locale/';        // path to locale
var $mj_g_locale = RegExp.$2;        // current locale
var $mj_g_docnme = RegExp.$3;        // document filename


MjIncdyn('../../skins/'+SkinName+'/skp.js', '$inc_skp');
MjIncdyn('../../iface_runtime.js', '$iface_runtime_js');
MjIncdyn('../../iface_details.js', '$iface_details_js');
MjIncdyn('../../iface_jcfg.js', '$iface_jcfg_js');

if ('undefined' !== typeof($g_is_mt_scenario)) {
	MjIncdyn('../../mt_scenario.js', '$mt_scenario_js');
}
// - - - - - - run JS loader
MjIncdynGo('DynLoader_kok', 'DynLoader_kfa');

function MjGetAL($ass_arr) {
        var $cnt = 0;
        for ($v in $ass_arr) $cnt++;
        return $cnt;
}

fly_x = 0;
fly_y = 800;
fly = '';
xmin = 0;
ymin= 0;
xmax= 1280;
ymax = 1024;
dx=5;
dy=-2;
flyWidth = 300;
flyHeight = 200;
ttime = 50;

function create_fly() {
	if ($mj_g_docnme!='main.html') return false;
	
	$s_add = '<embed src="../../skins/'+SkinName+'/fly.swf" width="'+flyWidth+'" height="'+flyHeight+'" autostart="true" quality="best" wmode="transparent">';
	$s_add = '<div id="meg_ny" name="meg_ny" style="position: absolute;">'+$s_add+'</div>'
  document.writeln($s_add);
  $oos = document.getElementById('meg_ny').style;

  $oos = document.getElementById('meg_ny').style;
  fly = document.getElementById('meg_ny');
  $oob = document.body;
  $oos.top = fly_y;
  $oos.left = fly_y;

  $snowf_xw = $oob.clientWidth;
  $snowf_yw = $oob.clientHeight;

  $oos.width = $snowf_xw+'px';
  $oos.height = $snowf_yw+'px';
  $oos['zIndex'] = '99999';
  $oos['overflow'] = 'hidden';
  $oos['clip'] = 'rect(0px, '+$snowf_xw+'px, '+$snowf_yw+'px, 0px)';
  
  setTimeout('fly_move()', ttime);
}

function fly_move() {
				if (fly_x > xmax-flyWidth || fly_x<xmin) {
					//$angle = Math.random()*1.57;
					dx=-dx;//*Math.cos($angle); 
				}
        if (fly_y > ymax-flyHeight || fly_y<ymin) {
        	///$angle = Math.random()*1.57;
        	dy=-dy;//*Math.cos($angle); ; 
        }
        fly_x+=dx;
        fly_y+=dy;
        fly.style['left'] = fly_x;
        fly.style['top'] = fly_y;
        setTimeout('fly_move()', ttime);
}

//create_fly();
// - - - - - - JS loading OK!
function DynLoader_kok() {
        main_loop();
}

// - - - - - - JS loading failed!
function DynLoader_kfa($s) {
        CybIf_Logger(CYBIF_LOG_ERR, 'KFA (Dynamic JS loading failed)!\n\n'+'fupath: ' + $mj_g_pathwc + ',\nlocale: ' + $mj_g_locale + ',\ndocnme: ' + $mj_g_docnme+'\n\n'+$s);
}


// - - - - - -
cvp_G = new MJ_VP(-1, -1, 0, 0, 1280, 1024, 'IMG', 'bg.jpg');
if ($mg_g_vp_debug && $is_show_hi) {
        cvp_G.o.Opacity(0, 0.7);
}
cvp_G.Finalize(true);


