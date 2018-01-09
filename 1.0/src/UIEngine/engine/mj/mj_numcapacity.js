$g_included_mj_numcapacity = true;	// uncluded in system
/**
* mobile 10-numbers base functionality

	#	$db_usrcapacity - user database (high prority)
	#	$db_numcapacity - cyberplat database
*/


$a_numcapa = new Array();
$a_numcapa_length = 0;

// - - - - - - -
// create database with service operator ID
function CreateBase($__service_num, $__service_op) {
	$a_numcapa[0] = [$__service_num, $__service_num, $__service_op, 'бУНД Б ЯЕПБХЯМНЕ ЛЕМЧ', '', ''];

	$a_numcapa = $a_numcapa.concat($db_usrcapacity);
	$a_numcapa = $a_numcapa.concat($db_numcapacity);

	$a_numcapa_length = $a_numcapa.length;
	MjCRIO($a_numcapa_length);
	// alert($a_numcapa_length);
}

// - - - - - - -
// returns: INDEX in base
function GetIndex_InLocalBase($__number) {
	$res = -1;

	for($i=0; $i<$a_numcapa_length; $i++) {
		if ($a_numcapa[$i].length < 6) continue;
		if (($__number >= $a_numcapa[$i][0]) && ($__number <= $a_numcapa[$i][1])) {
			$res = $i;
			break;
		}
	} // for

	return $res;
}

// - - - - - - -
// returns: OperatorID in base
function OpId_InLocalBase($__index) {
	return $a_numcapa[$__index][2];
}

// - - - - - - -
// returns: OperatorName in base
function OpName_InLocalBase($__index) {
	if ($__index < 0) return 'ме нопедекем';
	else return $a_numcapa[$__index][3];
}


// - - - - - - -
// returns: Region in base
function Region_InLocalBase($__index){
	if ($__index < 0) return 'ме нопедекем';
	else return $a_numcapa[$__index][5];
}

// - - - - - - -
// returns: Company in base
function Company_InLocalBase($__index){
	if ($__index < 0) return 'ме нопедекем';
	else return $a_numcapa[$__index][4];
}

// - - - - - - -
// [EOF]