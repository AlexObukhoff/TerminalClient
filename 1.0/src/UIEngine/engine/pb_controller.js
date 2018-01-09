var timerId = 0;

function controller() {
		//alert('Убиваем таймер: '+timerId);
		clearTimeout(timerId);
		timerId = setTimeout("goto_Main()",60000);
		//mj_RT.Add('document.focus', 20000, 0);
		var currentPage = getParameter('pb', 'login');
		//alert('Страница: '+ currentPage + '  Таймер:'+timerId);
		add_term_info();
		create_vpa('VIEW_FLASH');
		switch (currentPage.toLowerCase()) {
				case 'login':
		  			createLogin();
		  			break;
				case 'getpin':
		  			getPin();
		  			break;
		  		case 'enterpin':
		  			enterPin();
		  			break;
		  		case 'getpb':
		  			getPB();
		  			break;
				case 'additem':
		  			mainMenu();
		  			break;		  			
				case 'enteritem':
		  			dataEntry();
		  			break;		  			
		  		case 'dataentry':
		  			dataEntry();
		  			break;	
				case 'checking':
		  			checking();
		  			break;			  				  			
				case 'payment':
		  			payment();
		  			break;
				case 'thanks':
		  			thanks();
		  			break;		
				case 'loading':
		  			loading();
		  			break;		  			
				case 'error':
		  			showError();
		  			break;			 		  				  						  			
	  			default :
			  		//alert('Page not found: '+currentPage);
			  		def();
		  			break;
	  } 
}
//=======================================================================
// [ file include done. ]
$pb_controller_js = true;