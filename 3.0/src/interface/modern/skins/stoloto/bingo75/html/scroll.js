	jQuery(document).ready(function($) {
		// рисуем контролы
		$('body').wrapInner('<div class="container"></div>')
				.append('<div class="verticalScrollBar"><div class="verticalScrollTrack"><div class="verticalLine"></div><div class="verticalScrollDrag"></div></div></div>')
				.append('<a href="#" class="btn_up"><svg version="1.1" x="0px" y="0px" width="102px" height="102px" viewBox="0 0 102 102" enable-background="new 0 0 102 102" xml:space="preserve"><path class="back" opacity="0.5" d="M98,92c0,3.3-2.7,6-6,6H10c-3.3,0-6-2.7-6-6V10c0-3.3,2.7-6,6-6h82c3.3,0,6,2.7,6,6V92z"/><path class="arrow" fill="#F2F2F2" d="M69.106,49.011L53.894,34.488c-1.591-1.519-4.196-1.519-5.787,0L32.894,49.011C31.302,50.53,30,53.572,30,55.772v3.637V64v4l10.473-10.693l7.674-7.835c1.54-1.572,4.062-1.575,5.605-0.007l7.721,7.843L72,68v-4v-4.591v-3.637C72,53.572,70.698,50.53,69.106,49.011z"/></svg></a>')
				.append('<a href="#" class="btn_down"><svg version="1.1" x="0px" y="0px" width="102px" height="102px" viewBox="0 0 102 102" enable-background="new 0 0 102 102" xml:space="preserve"><path class="back" opacity="0.5" d="M98,92c0,3.3-2.7,6-6,6H10c-3.3,0-6-2.7-6-6V10c0-3.3,2.7-6,6-6h82c3.3,0,6,2.7,6,6V92z"/><path class="arrow" fill="#F2F2F2" d="M32.894,52.338l15.213,14.522c1.591,1.519,4.196,1.519,5.787,0l15.213-14.522C70.698,50.819,72,47.777,72,45.577V41.94v-4.591v-4L61.527,44.042l-7.674,7.835c-1.54,1.572-4.062,1.575-5.605,0.007l-7.721-7.843L30,33.349v4v4.591v3.637C30,47.777,31.302,50.819,32.894,52.338z"/></svg></a>');
		$('.container').append('<div class="spacer" style="height:0px"></div>');
		
		var frameHeight;  // Высота экрана прокрутки
		var allHeight;  // Высота за вычетом паддинга
		var lostHeight; // Высота последней страницы
		var contentHeight;  // Высота контента
		var totalPages;  // Всего страниц
		var currentPage = 1; // Текущая страница
		var verticalScrollDrag = $('.verticalScrollDrag'); // Ссылка на бегунок
		var verticalScrollTrack = $('.verticalScrollTrack');  // Ссылка на трэк
		var verticalDragHeight; //  
		var isScrollableV; // Нужен ли скроллинг
		var position = 0;  // Текущая позиция
		var moving = false; // Флаг - идет ли прокрутка контента
		var newHeight;
		var heightTrack;
		var overTrack;
		var overUp;
		var overDown;
		
		
		// изменение размеров скроллбара
		function resizeScrollBar() {
		  frameHeight = $('.container').height();
		  allHeight = $('.container')[0].scrollHeight;
		  lostHeight = allHeight % frameHeight;
		  $('.spacer').height(lostHeight + 'px');
		  contentHeight = allHeight - frameHeight;
		  totalPages = Math.ceil( contentHeight / frameHeight + 1 );
		  
		  verticalDragHeight = Math.floor( 1 / totalPages * verticalScrollTrack.height() );
		  isScrollableV = totalPages > 1;
		  
		  
		  if (verticalDragHeight > 12) {
			  verticalScrollDrag.height(verticalDragHeight + 'px');
			  heightTrack = verticalScrollTrack.height()-verticalDragHeight;
		 	  overTrack = heightTrack - verticalDragHeight*(totalPages-1);
				
		  } else {
			  // Коррекция высоты драг-элемента
			  verticalScrollDrag.height('12px');
			  newHeight = verticalScrollTrack.height() + verticalDragHeight;
			  //verticalDragHeight =  Math.floor( 1 / totalPages * newHeight );
			  verticalDragHeight = Math.floor( 1 / ( totalPages) *  ( verticalScrollTrack.height() )  );
			  /*overUp = Math.ceil(( verticalScrollTrack.height() - 6 - ( verticalDragHeight*(totalPages-1) ) ) / 2);
			  overDown = totalPages - overUp -1;*/
			  heightTrack = verticalScrollTrack.height()-verticalDragHeight-6;
		 	  overTrack = heightTrack - verticalDragHeight*(totalPages-1);
		  }
		  
		  
			  
		  overUp = (overTrack - overTrack % 2) / 2;
		  if (overUp != 0 ) {
			overDown = totalPages - overUp;
			overUp += overTrack % 2;
		  } else {
			overDown = totalPages +1;  
		  }
		}
		
		function getShift () {
			if ( currentPage <= overUp || currentPage >= overDown ) {
				return verticalDragHeight + 1;
			}
			return verticalDragHeight;
		}
		
		resizeScrollBar();
		
		// Открываем если нужно видеть значение переменных
		/*$('body').append('<ul class="logs">' + 
			'<li> verticalDragHeight:' + verticalDragHeight + '</li>' +
			'<li> TrackHeight:' + verticalScrollTrack.height() + '</li>' +
			'<li> frameHeight:' + frameHeight + '</li>' +
			'<li> DragHeight:' + verticalDragHeight*( totalPages - 1) + '</li>' +
			'<li> allHeight:' + allHeight + '</li>' +
			'<li> totalPages:' + totalPages + '</li>' +
			'<li> overTrack:' + overTrack + '</li>' +
			'<li> overUp:' + overUp + '</li>' +
			'<li> overDown:' + overDown + '</li>' +
			' </ul>');*/
		
		
		// скроллирование экрана
		function scroller() {
			$('.container').animate(
				{
					scrollTop : position
				},
				{
					duration:'slow',
					complete:function() { moving = false; }
				});	
			
		}
		
		// Нажатие кнопки "вниз"
		$('.btn_down').click(function(e) {
			if (moving) return false;
			if ( currentPage < totalPages ) {
				currentPage++;
				moving = true;
				position += frameHeight;
				scroller();
				verticalScrollDrag.animate({top : verticalScrollDrag.position().top + getShift()},{duration:'slow'});	
			}
			return false;
		});	
		
		// Нажатие кнопки "вверх"
		$('.btn_up').click(function(e) {
			if (moving) return false;
			if ( currentPage > 1 ) {
				currentPage--;
				moving = true;
				position -= frameHeight;
				scroller();
				verticalScrollDrag.animate({top : verticalScrollDrag.position().top - getShift()},{duration:'slow'});	
			}
			return false;
		 					
		});	

		
						
	});