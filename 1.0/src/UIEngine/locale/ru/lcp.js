// сообщения

$ga_lcp = {

        // RU: основные сообщения интерфейса
        // EN: main interface messages
        achtung                                : 'Внимание!',
        attention                              : 'Внимание!',

        out_of_service                : 'Терминал временно не работает',


        // - - - - - - - -  service menu button labels
        svmnu_title                        :'CYBERPLAT.COM :: Сервисное меню',

        terminal_no                                                        :'Терминал №',
        svmnu_configtool                      :'НАСТРОЙКА ПО',

        svmnu_incass                :'Инкассация',
        svmnu_conn2cyber        :'Связь с CyberPlat',
        svmnu_balance                :'Баланс',
        svmnu_monserver                :'Связь с сервером мониторинга',
        svmnu_zreport                :'Z-отчет',
        svmnu_exit                        :'Выход',
        svmnu_simbalance        :'Баланс Sim-карты',
        svmnu_termreset                :'Перезагрузка терминала',
        svmnu_back                        :'Вернуться',
        svmnu_closewindow                   :'Свернуть окно',
        svmnu_printunprintedchecks          :'Печать ненапечатанных чеков' ,
        svmnu_generatekeys                   : 'Сгенерировать ключи по команде с сервера' ,
        svmnu_generatekeysbyhands             : 'Настройка конфигурации' ,


        svmnu_m_BillsCount                                        :'Кол-во купюр',
        svmnu_m_BillsSum                                        :'Сумма',
        svmnu_m_ValidatorState                                :'Состояние купюроприёмника',
        svmnu_m_PrinterState                                :'Состояние принтера',
        svmnu_m_UnprocPaymentsCount                        :'Непроведенных платежей',
        svmnu_m_UnprocStatPacketsCount                :'Непроведенных пакетов',
        svmnu_m_SIMBalance                                        :'Баланс SIM',
        svmnu_m_GSMSignalQuality                        :'Уровень GSM сигнала',
        svmnu_m_LastPaymentReceived                        :'Дата внесения последнего платежа',
        svmnu_m_LastPaymentProcessed                :'Дата проведения последнего платежа',


        svmnu_m_states        : {
                0: 'OK',
                1: 'Устройство недоступно',
                2: 'Нет бумаги',
                3: 'Бумага заканчивается',
                4: 'Бумага застряла',
                5: 'Некритическая ошибка',
                6: 'Стэкер открыт',
                7: 'Стэкер полон',
                8: 'Купюра застряла',
                9: 'Взлом',
                10: 'Критическая ошибка',
                11: 'Кассета установлена',
                255: 'Неопределённое состояние'
        },

        svmnu_m_states_unrecognized : 'Нераспознанное состояние',

        // menu headers
        nominal									:'Номинал',
        choose_operator         :'Выберите оператора',
        title_requizites        :'Поля реквизитов',
        title_tel_number        :'Номер телефона',

        // button lables
        button_back                        :'Назад',
        button_next                        :'Далее',
        button_more                        :'Вперед',
        button_main                        :'Главная',
        button_to_main                     : 'В меню',
        button_payment                :'Оплатить',
        button_cancel                :'Отменить',
        button_yes                                                :'Да',
        button_no                                                :'Нет',
        button_information                        :'Информация',
        button_instruction                        :'Инструкция',
        metro_insert_card        :'Пожалуйста, приложите Вашу карту к кардридеру',

        title_select_card        :'Выбор карты',
        title_select_var        :'Выбрите вариант',

        terminal_number                :'Терминал №',
        comment_online                :'Моментальное зачисление платежа',

        checking_req                :'Проверка реквизитов',
        please_wait                        :'Пожалуйста, подождите',
        checking_what                :'Проверка',
        checking_error               : 'Ошибка проверки',
        payment_enter            :'Внесение средств',
        take_tha_rezippt        :'Ваш платеж принят. <br /> Возьмите, пожалуйста, чек.',
        thank_you                        :'Спасибо!',
        ned_summ                                :'Минимальная сумма внесения',
        ned_summ_plus							: 'Сдача будет переведена на мобильный телефон',
        min_summ                                :'Минимальная сумма',
        max_summ                                :'Максимальная сумма',
        ticket_cost 														: 'Стоимость билета',
        min_summ_prefix													: 'Вы можете заплатить от',
        min_summ_suffix													: 'до',
        
				processrest1            :'Сдача',
				processrest2            :'Перевести на мобильный телефон?',

        enter_money 						: 'Внесите деньги <br>в купюроприемник!',
        commission_prefix                                                :'Комиссия: ',
        commission_prefix_taxes                                          :'Оплата информационных услуг: ',
        commission_postfix                                                :'',
        commission_none                                                :'За данную операцию комиссия не взимается',
        commission_variable                                                :'',

        money_accepted                :'Принято ',
        money_commission        :'Комиссия',
        money_crediting                :'К зачислению',
        summ                                :'Сумма',
        card                                                        :'Карта',

        money_not_anough                                :'Вы внесли недостаточную сумму!',
        ask_retry_payment                                :'Произошла ошибка при проведении платежа. Повторить попытку?',
        please_wait_proc_payment        :'Пожалуйста, подождите - идет проведение платежа...',
		no_payment_go_support			:'Платеж не был проведен. Пожалуйста, сохраните чек и обратитесь в техподдержку.',
        card_amount_notexists                        :'Такого номинала карт не существует', // This amount does not exist!
        card_no_avialable                        :'Номиналы карт не загружены',
        number_check_error                                :'Ошибка проверки номера.<br />Попробуйте позже.',
				
        entered_code_controldigits_err                                :'Ошибка правильности ввода кода потребителя',




        not_issue_change                               :'<br><br>Терминал сдачу не выдает.<br>Вы желаете продолжить?',
        not_issue_change_nospace                       :'Терминал сдачу не выдает.<br />Вы желаете продолжить?',
        prn_fail_message_header                        :'Ошибка принтера',
        prn_fail_message_message             				   :'Аппаратный сбой принтера',
        out_of_service_header                    		   :'Терминал не работает',
        out_of_service_message                      	 :'Терминал временно не работает',
        prn_cantprint_message_header     						   :'Ошибка печати чека',
        prn_cantprint_message_message      						 :'Внимание! По техническим причинам печать чека не возможна, но платеж будет проведен. Продолжить?',
        prn_cantprint_message_fail                     :'По техническим причинам печать чека невозможна. Платеж не может быть принят.',




        // currency                        :'рублей',

        day_names                        :['Воскресенье', 'Понедельник', 'Вторник', 'Среда', 'Четверг', 'Пятница', 'Суббота'],
        month_names                        :['Январь', 'Февраль', 'Март', 'Апрель', 'Май', 'Июнь', 'Июль', 'Август', 'Сентябрь', 'Октябрь', 'Ноябрь', 'Декабрь'],
        month_names2                :['Января', 'Февраля', 'Марта', 'Апреля', 'Мая', 'Июня', 'Июля', 'Августа', 'Сентября', 'Октября', 'Ноября', 'Декабря'],
        year_postfix                :' г.',



        sound_files : {
                 choose_operators                :'choose_operators.wav',
                 confirm_number                  :'confirm_number.wav',
                 enter_number                    :'enter_nomer.wav',
                 click1                          :'ni_click1.wav',
                 click2                          :'ni_click2.wav',
                 error                           :'Err.wav',
                 validator                       :'kupurop.wav',
                 rezzipt                         :'kvitanc.wav',
                 ok                              :'ok.wav'
        },

        currency:        {
                RUR                :'руб.',
                EUR                :'евро',
                USD                :'долл.США',
                KZT                :'тенге',
                UAH                :'грн.',
                UZS                :'сум'
        },


/*
0 - space
1 - стереть/назад/del
2 - caps/заглавные
3 - shift
4 - rus/lat
5 - clear/del all
7 - cancel
8 - next
*/

        keyb: [
                '`1234567890-=\\',
                'qwertyuiop[]11',
                '22asdfghjkl;\'44',
                '33zxcvbnm,./33',
                '66555577 88'
        ],


        calculate:'Рассчитать',
        not_calculate:'Не рассчитано',
        thank_you_mt                        :'ПЕРЕВОД ОТПРАВЛЕН',
        money_crediting_mt                :'Сумма перевода',
        ask_retry_mt                                 :'Произошла ошибка при проведении перевода. Повторить попытку?',
        please_wait_proc_payment_mt     :'Пожалуйста, подождите. Ваш перевод отправляется...',
        pb_no_items : 'В настоящее время в Вашей записной книжке нет ни одной записи. Для создания новой записи нажмите на кнопку "Добавить"',
        comment_pb : 'Комментарий к записи',
	comment2_pb : 'Комментарий<br />к записи',
	not_exist_recepient_mt : 'К сожалению, отправка переводов невозможна. Вам нужно обратиться в банк для внесения изменений в регистрационные данные.',
	can_not_connect_to_server_mt : 'Отсутствует связь с сервером',
	hello_user_mt : 'Здравствуйте, ',
	rule_mt : ".<br><br>Денежные переводы осуществляются  в соответствии с действующим законодательством и Правилами работы Систем денежных переводов.<br><br>Нажимая кнопку 'ДАЛЕЕ',  Вы подтверждаете, что:<br><ul><li>С  Правилами работы Систем по осуществлению денежных переводов физических лиц без открытия банковских счетов ознакомлены, согласны и присоединяетесь к ним.</li><br><li>Совершаемый денежный перевод не связан с осуществлением предпринимательской и инвестиционной деятельности или приобретением прав на недвижимое имущество.</li><br><li>Согласны с тем, что остаток, образующийся при отправлении переводов, Клиенту не возвращается.</li></ul>",
	password_change_success_mt : 'Пароль удачно изменен.',
	password_change_bad_mt : 'Подтверждение пароля не прошло.',
	comment_to_password_mt : 'Вводимый пароль должен быть от 4 до 12 цифр',
	registration_place_mt : 'Регистрация клиентов проводится по адресу: ',
	accept_mt : 'Принято : ',
	not_calculate_mt : 'Не рассчитано',
	sum_transfer_mt : '<br>Сумма перевода : ',
	commission_mt : '<br>Комиссия : ',
	rent_mt : '<br>Остаток : ',
	info_transfer_system_mt : 'Система переводов : ',
	info_sender_mt : 'Отправитель : ',
	info_reciver_mt : 'Получатель : ',
	info_destination_mt : 'Пункт назначения перевода : ',
	choose_reciver_mt : 'Выберите получателя перевода',
	calculate_commission_mt : 'Рассчитать комиссию',
	money_rest : 'Остаток',
        stub : ''
};
