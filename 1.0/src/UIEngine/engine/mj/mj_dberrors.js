$g_included_mj_dberrors = true;        // uncluded in system

$a_dberrors = new Array();
$a_dberrors_length = 0;



function FindIndexError(CurError){
  var ret = -1;
  for($i = 0; $i < $db_errors.length; $i++){
    if( parseInt($db_errors[$i][0]) == parseInt(CurError) ){
      ret = $i;
      break;
    }
  }
  return ret;
}

function GetMessageDbError(CurError){
    var message = LCP('number_check_error');
    var IndexError = FindIndexError(CurError);
//    alert("IndexError="+IndexError);
    if(IndexError != -1){
      if($db_errors[IndexError][2] == 1){
        if(1==0 && ErrInList(CurError,$ga_jcfg['config']['ignored_check_errors_list'])  && $db_errors[IndexError][3] != '' ){
          bigbutA('right',LCP('button_yes'),'fields_preCheck()');
          bigbutA('left',LCP('button_no'),'goto_Loc("main.html")');
          message = $db_errors[IndexError][3];
        }else{
          message = $db_errors[IndexError][1];
        }
      }
    }
    if($is_debug){
        message = 'Error='+CurError+'<br>'+$db_errors[IndexError][1];
    }
    return message;

}


function ErrInList(CurError,ErrorsList){
        //alert("find... ErrInList("+CurError+",{"+ErrorsList+"})....");
        var ret = false;
        var ArrSplit = ErrorsList.split(",");
        for(i=0;i<ArrSplit.length;i++){
                if(parseInt(CurError) == parseInt(ArrSplit[i])){
                        return true;
                }
        }
        return ret;
}



function ContWithErrors(CurError) {
        ret = false;
//        alert($ga_jcfg['config']['ignored_check_errors_list']);
        if(ErrInList(CurError,$ga_jcfg['config']['ignored_check_errors_list'])){
          ret = true;
        }

        return ret;
}


