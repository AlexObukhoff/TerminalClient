// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
* MeG JavaScript extension lib
* '2006-2007
*
* working with data/string - misc conversions
*/
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


// ------ Hexadecimal Conversion---------------------------------------------
// convert a single digit (0 - 15) into hex
function MjDig2HexChar($__digit)
{
	return("0123456789abcdef".charAt($__digit));
}

// ------ 
// convert a hex digit into decimal
function MjHexChar2Dig($__digit)
{
	return("0123456789abcdef".indexOf($__digit.toLowerCase()));
}

// ------ 
function MjString2HexSeq($__s) {
	$sr = '';
	for ($i=0; $i<$__s.length; $i++) {
		$cn = $__s.charCodeAt($i);
		$sr += MjDig2HexChar(($cn >> 4) & 0x0f) + MjDig2HexChar($cn & 0x0f);
	}
	return $sr;
}


// ------ Encoding String by Xor operation ----------------------------------
// $__s		: string to xor
// $__spw	: string 'xor by'
// return	: hex sequence string
function MjStringXorHexSeq($__s, $__spw) {
	$sr = '';
	$ipw = 0;

	for ($i=0; $i<$__s.length; $i++) {
		$sr += String.fromCharCode( ($__s.charCodeAt($i) & 0xff) ^  ($__spw.charCodeAt($ipw) & 0xff) );
		$ipw++;
		if ($ipw >= $__spw.length) {
			$ipw=0;
		}
	}

	// alert(escape($sr));
	return MjString2HexSeq($sr);
}

