function stopError() {return true;}
window.onerror = stopError;

// Константы типов сообщений
var mkError = "error";
var mkGeneric = "generic";

var kbA=65;
var kbB=66;
var kbC=67;
var kbD=68;
var kbE=69;
var kbF=70;
var kbG=71;
var kbH=72;
var kbDel=46;

var kbF1=112;
var kbF2=113;
var kbF3=114;
var kbF4=115;
var kbF5=116;
var kbF6=117;
var kbF7=118;
var kbF8=119;
var kbEsc=27;
var kbEnter=13;
var kbBs=8;
var kb0=48;
var kb1=49;
var kb2=50;
var kb3=51;
var kb4=52;
var kb5=53;
var kb6=54;
var kb7=55;
var kb8=56;
var kb9=57;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function GetItemNumFK(FKeyCode) {
var iret=0;
switch (FKeyCode){
  case kbA:
    iret = 0;
    break;
  case kbB:
    iret = 2;
    break;
  case kbC:
    iret = 4;
    break;
  case kbE:
    iret = 1;
    break;
  case kbF:
    iret = 3;
    break;
  case kbG:
    iret = 5;
    break;
  default:
    iret = 0;
    break;
}
return iret;
}

function kbL1(FKeyCode) {
var iret=false;
if(FKeyCode == kbA){
iret = true;
}
return iret;
}

function kbL2(FKeyCode) {
var iret=false;
if(FKeyCode == kbB){
iret = true;
}
return iret;
}

function kbL3(FKeyCode) {
var iret=false;
if(FKeyCode == kbC){
iret = true;
}
return iret;
}

function kbL4(FKeyCode) {
var iret=false;
if(FKeyCode == kbD){
iret = true;
}
return iret;
}

function kbR1(FKeyCode) {
var iret=false;
if(FKeyCode == kbE){
iret = true;
}
return iret;
}

function kbR2(FKeyCode) {
var iret=false;
if(FKeyCode == kbF){
iret = true;
}
return iret;
}

function kbR3(FKeyCode) {
var iret=false;
if(FKeyCode == kbG){
iret = true;
}
return iret;
}

function kbR4(FKeyCode) {
var iret=false;
if(FKeyCode == kbH){
iret = true;
}
return iret;
}



var monthNames = new Array("Январь",
        "Февраль",
        "Март",
        "Апрель",
        "Май",
        "Июнь",
        "Июль",
        "Август",
        "Сентябрь",
        "Октябрь",
        "Ноябрь",
        "Декабрь");

var monthNames2 = new Array("января",
        "февраля",
        "марта",
        "апреля",
        "мая",
        "июня",
        "июля",
        "августа",
        "сентября",
        "октября",
        "ноября",
        "декабря");

var dayNames = new Array("Воскресенье", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота");

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function GetTime() {
webe = new Date();
var s=webe.getSeconds();
var mi=webe.getMinutes();
var h=webe.getHours();
var y=webe.getFullYear();
var d=webe.getDate();
var m = webe.getMonth();
//var m2 = m + 1
//return d + "/" + m2 + "/" +y + ".  " + h + ":" + m + ":" + s;
//if (m2<10)
//  m2="0"+m2;
if (d<10)
  d="0"+d;
if (mi<10)
  mi="0"+mi;
if (s<10)
  s="0"+s;

return d + " " + monthNames2[m] + " " +y+" г.   " + h + ":" + mi;
}



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function ShowTimeTimer()
{
eval("document.all.datetime1.innerHTML = '" + GetTime() + "<strong>" + GetTime() + "</strong>'");
document.focus();
my_timedProcess = setTimeout("ShowTimeTimer() ", 20000);
}


/******************************************************
  ИНИЦИАЛИЗАЦИЯ РАБОЧИХ ПЕРЕМЕННЫХ ДЛЯ СТРАНИЦ
******************************************************/
var Parameters = new Array();
{

        var args = location.search.substring(1, location.search.length).split('&');
        for (var i = 0; i < args.length; i++) {
                var valuePair = args[i].split('=');
                Parameters[valuePair[0]] = unescape(valuePair[1]);
        }
}

function getParameter(parameterName, defaultValue) {
        if ((Parameters[parameterName] == null)
                        || (Parameters[parameterName] == "")) {
                return defaultValue;
        } else {
               var $_s = Parameters[parameterName].toString();
               $_s = $_s.replace(/%2F/g, '/');
               $_s = $_s.replace(/%20/g, ' ');
               return $_s;
        }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function getParametersAllAsUrl() {
        var $s = '';
        for ($k in Parameters) {
                // if
                if ($k !== '') {
                        $s += (($s !== '') ? '&' : '') + $k+'='+Parameters[$k];
                }
        }
        return $s;
}




// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
function min(a,b) {
if (a<b)
        return a;
        else
        return b;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// MeG's enchanced
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

function GetTimeA() {
webe = new Date();
var s=webe.getSeconds();
var mi=webe.getMinutes();
var h=webe.getHours();
var y=webe.getFullYear();
var d=webe.getDate();
var m = webe.getMonth();
if (d<10)  d="0"+d;
if (mi<10) mi="0"+mi;
if (s<10)  s="0"+s;

return d + " " + LCP('month_names2')[m] + " " +y+ LCP('year_postfix')+" "+ h + ":" + mi + ":" + s;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
