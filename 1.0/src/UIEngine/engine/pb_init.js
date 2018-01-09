/*
основной файл генерации страниц интерфейса
*/



var $is_debug = false;

function parseBBCode($__s) {
        $__s = $__s.replace(/\[/g, '<');
        $__s = $__s.replace(/\]/g, '>');
        return $__s;
}

var SkinName = 'pb';
var MainMenuMarqueeString = parseBBCode($ga_jcfg['config']['MainMenuMarqueeString']);
var CurrencyType = $ga_jcfg['config']['CurrencyType'].toUpperCase();
var CurrencyName = $ga_jcfg['config']['CurrencyName'];
var TerminalName = parseBBCode($ga_jcfg['config']['TerminalName']);
var SupportString = parseBBCode($ga_jcfg['config']['SupportString']);
var pathToOperators = "../../logo/";
var maxFields = 6;
var maxEnum = 9;
var maxPB = 8;
var $intro = false;
//максимальное количество символов в текстовом поле на кнопке записной книжки...
var maxButtonTextLength = 20;		//из 1-го филда записи
var maxButtonCommentLength = 15;	//название записи



//  - - - - - инициализация глобальных переменных
// картинки шкурки
var $mj_g_img_pth = '../../skins/'+SkinName+'/';
var $mg_g_vp_bg = '#cccccc';
var $mg_g_vp_debug = $is_debug;        // под-отладка элементов отображения



//  - - - - - подготавливаем видимую область, устанавливаем клиппинг
//                           защищаем от возможных полос прокрутки
document.body.style['overflow'] = 'hidden';
document.body.style['border'] = '0px';
mj_G.Wpx = mj_G._domb.clientWidth;
mj_G.Hpx = mj_G._domb.clientHeight;
document.body.style['clip'] = 'rect(0px, '+mj_G.Wpx+'px, '+mj_G.Hpx+'px, 0px)';

// - - - - - - разбор текущего URI?
var $s_uri = document.location.href;
var $re = new RegExp('(.*?)/locale/([^/]+)/([^\\?]*)', 'gi');
$re.exec($s_uri);
var $mj_g_pathwc = RegExp.$1;        // path to webclient
var $mj_g_pathlc = $mj_g_pathwc + 'interface/locale/';        // path to locale
var $mj_g_locale = RegExp.$2;        // current locale
var $mj_g_docnme = RegExp.$3;        // document filename

// Загрузка библиотек
MjIncdyn('../../skins/'+SkinName+'/skp.js', '$inc_skp');
MjIncdyn('../../iface_runtime.js', '$iface_runtime_js');
MjIncdyn('../../iface_details.js', '$iface_details_js');
MjIncdyn('../../iface_jcfg.js', '$iface_jcfg_js');
MjIncdyn('../../pb_config.js', '$pb_config_js');
MjIncdyn('../../pb_controller.js', '$pb_controller_js');
MjIncdyn('../../pb_model.js', '$pb_model_js');

// - - - - - - run JS loader
MjIncdynGo('DynLoader_kok', 'DynLoader_kfa');

// - - - - - - JS loading OK!
function DynLoader_kok() {
	controller();
}

// - - - - - - JS loading failed!
function DynLoader_kfa($s) {
	alert($s);
}
// - - - - - -
cvp_G = new MJ_VP(-1, -1, 0, 0, 1280, 1024, 'IMG', 'bg.jpg');
cvp_G.Finalize(true);

