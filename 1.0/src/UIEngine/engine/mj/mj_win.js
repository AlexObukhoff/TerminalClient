// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
* MeG JavaScript extension lib
* '2006-2007
*
* misc windows work
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*
// ==========================================================================
open new browser window with no bars and menus

note:  W,H used only, if you specify it
returns: handle to opened window
// ==========================================================================
*/
function MjOpenWindowNN($__fname, $__w, $__h) {
     $res = null;     // result

     if (3 > arguments.length) {
          $res = window.open($__fname, '_blank', 'toolbar=no,location=no,status=no,menubar=no,scrollbars=yes,resizable=no');
     }
     else {
          $res = window.open($__fname, '_blank', 'width='+$__w+',height='+$__h+',toolbar=no,location=no,status=no,menubar=no,scrollbars=yes,resizable=no');
     }

     return $res;
}

// пример:
// MjOpenWindowNN('1.html', 100, 300);