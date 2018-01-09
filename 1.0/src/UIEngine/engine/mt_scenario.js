/*
control functions:

toPage <page_name>
toState <state> [|<reload>]

? fillTerms
? fillComment
fillParm <parm>


_act <action>
make action

_chk [<criteria>, <action_if_true>, <action_if_false>]
check and make action
<criteria> - [1] if

_evt <action>
make action on object event (click/press)


*/

$ga_mt_scen = {
	main: {
		_act: 'toState|auth'
	},

	//  - - - -  авторизация юзера
	auth: {
		c_str_uid: {
			id: 'login',
			type: 'm',
			mask: '**** **** **** ****',
			desc: 'Номер карточки'
		},
		c_str_pwd: {
			id: 'password_md5',
			desc: 'Пароль',
			type: 'p'
		},
		c_lfb: {	// toState
			desc: '',//LCP('button_back'),
			_evt: 'toPage|main'
		},
		c_rfb: {	// toState
			desc: '',//LCP('button_next'),
			_evt: 'toState|auth_check|reload'
		},
		_txt_: {
			id: 'comment',
			r: [330, 300, 450, 100],
			_act: 'fillParm|comment_to_passw'
		},
		_txt_1: {
			id: 'comment',
			r: [130, 400, 650, 300],
			_act: 'fillParm|comment_for_place'
		},
		_act_1: 'leavParm|login',


		stub: ''
	},

	// - - - -
	auth_check: {
		_chk: ['error', 'toState|show_error_and_return', ''],
		_act: 'toState|hello',

		stub: ''
	},

	// - - - -
	hello: {
		_act_1: 'leavParm',

		_txt_: {
			r: [80, 35, 1140, 800],
			_act: 'fillParm|hello'
		},
		c_lfb: {	// toState
			desc: '',
			_evt: 'toPage|main'
		},
		c_rfb: {	// toState
			desc: '',
			_evt: 'toState|start_menu'
		},


		stub: ''
	},

	//  - - - - начальное меню
	start_menu: {
		_act_1: 'leavParm',

		c_btn_new: {
			desc: 'Отправить перевод',
			_evt: 'toState|create_new|reload'
		},
		c_btn_chpasswd: {
			desc: 'Изменить пароль',
			_evt: 'toState|pwd_change|reload'
		},
		c_btn_info: {
			desc: 'Информация о Системах Денежных переводов',
			_evt: 'toState|terms'
		},
		c_btn_return: {
			desc: 'Отмена',
			_evt: 'toPage|main'
		},


		stub: ''
	},

    // Информационная страница
    info_page: {
		_txt_: {
			r: [60, 20, 1160, 400],
			_act: 'fillParm|info_text'
		},

		c_rfb: {	// toState
			desc: '',//LCP('button_next'),
			_evt: 'toPage|checking'
		},
		c_lfb: {	// toState
			desc: '',//LCP('button_back'),
			_evt: 'toState|create_new'
		},
		r_btn_calc: {
			desc: 'Предварительный расчет комиссии',
			r: [450, 500, 400, 75],
			_evt: 'toState|calculate'
		},

		stub: ''
    },

	//  - - - - show error
	show_error: {

		_txt_: {
			r: [60, 300, 1160, 200],
			_act: 'fillParm|error_text',
			align: 'center'
		},

		c_rfb: {	// toState
			desc: '',
			_evt: 'toState|start_menu',
			align: 'center'
		},

		stub: ''
	},


	//  - - - - show error
	show_error_and_return: {

		_txt_: {
			r: [60, 300, 1160, 200],
			_act: 'fillParm|error_text',
			align: 'center'
		},

		c_rfb: {	// toState
			desc: '',//LCP('button_next'),
			_evt: 'toState|auth'
		},

		stub: ''
	},


	//  - - - -
	pwd_change: {
		c_str_opwd: {
			id: 'old_passwd_md5',
		      desc: 'Введите текущий пароль',
		      type: 'p'
		},
		c_str_npwd: {
			id: 'new_passwd_md5',
		      desc: 'Введите новый пароль',
		      type: 'p'
		},
		c_str_npwd1: {
			id: 'new_passwd1_md5',
		      desc: 'Подтвердите введенный пароль',
		      type: 'p'
		},
		_txt_: {
			id: 'comment',
			r: [330, 450, 450, 100],
			_act: 'fillParm|comment_to_passw'
		},

		c_lfb: {	// toState
			desc: '',//LCP('button_back'),
			_evt: 'toState|start_menu|reload'
		},
		c_rfb: {	// toState
			desc: '',//LCP('button_next'),
			_evt: 'toState|pwd_change_check|reload'
		},


		stub: ''
	},

	pwd_change_first_time: {
		c_str_npwd: {
			id: 'new_passwd_md5',
		      desc: 'Введите новый пароль',
		      type: 'p'
		},
		c_str_npwd1: {
			id: 'new_passwd1_md5',
		      desc: 'Подтвердите введенный пароль',
		      type: 'p'
		},
		_txt_: {
			id: 'comment',
			r: [330, 400, 450, 100],
			_act: 'fillParm|comment_to_passw'
		},

		c_lfb: {	// toState
			desc: '',//LCP('button_back'),
			_evt: 'toState|main'
		},
		c_rfb: {	// toState
			desc: '',//LCP('button_next'),
			_evt: 'toState|pwd_change_check|reload'
		},


		stub: ''
	},

	// - - - -
	pwd_change_forced: {
		c_str_opwd: {
			id: 'old_passwd_md5',
		      desc: 'Введите текущий пароль',
		      type: 'p'
		},
		c_str_npwd: {
			id: 'new_passwd_md5',
		      desc: 'Введите новый пароль',
		      type: 'p'
		},
		c_str_npwd1: {
			id: 'new_passwd1_md5',
		      desc: 'Подтвердите введенный пароль',
		      type: 'p'
		},
		_txt_: {
			id: 'comment',
			r: [330, 400, 450, 100],
			_act: 'fillParm|comment_to_passw'
		},

		c_lfb: {	// toState
			desc: '',//LCP('button_back'),
			_evt: 'toState|main'
		},
		c_rfb: {	// toState
			desc: '',//LCP('button_next'),
			_evt: 'toState|pwd_change_check|reload'
		},		


		stub: ''
	},


	//  - - - -
	pwd_change_check: {
		_chk: ['pwd_change_failed', 'toState|pwd_change', ''],
		_act: 'toState|say_change_good',

		stub: ''
	},
	// - - - -
	say_change_good: {
		_txt_: {
			r: [400, 200, 400, 100],
			_act: 'fillParm|pwd_change_good'
		},
		c_rfb: {	// toState
			desc: '',//LCP('button_next'),
			_evt: 'toState|start_menu'
		}
	},
	//  - - - -
	pwd_confirm_check: {
		_chk: ['pwd_change_failed', 'toState|pwd_change', ''],
		_act: 'toState|say_confirm_bad',

		stub: ''
	},
	// - - - -
	say_confirm_bad: {
		_txt_: {
			r: [400, 200, 500, 100],
			_act: 'fillParm|pwd_confirm_bad'
		},
		c_rfb: {
			desc: '',
			_evt: 'toState|start_menu'
		}
	},
	
	//  - - - -
	pwd_change_forced_check: {
		_chk: ['pwd_change_failed', 'toState|pwd_change_forced', ''],
		_act: 'toState|pwd_change_ok',

		stub: ''
	},

	//  - - - -
	pwd_change_ok: {
		_txt_: {
			id: 'comment',
			_evt: 'fillComment'
		},

		c_rfb: {	// toState
			desc: '',//LCP('button_next'),
			_evt: 'toState|start_menu|reload'
		},


		stub: ''
	},


	terms: {
		c_txt: {
			r: [60, 20, 1140, 600],
			_act: 'fillParm|terms'

		},
		c_lfb: {	// left button
			desc: '',//LCP('button_back'),
			_evt: 'toState|start_menu'
		},


		stub: ''
	},

	calculate:{
		c_str_enter_summ: {
			r: [60, 20, 1080, 600],
			type: 'i',
			desc: 'Сумма',
			id: 'enter_summ'

		},
		c_lfb: {
			desc: '',//LCP('button_back'),
			_evt: 'toState|info_page|reload'
		},
		c_rfb: {
			desc: '',//LCP('button_next'),
			_evt: 'toState|calculate|reload'
		},
		_txt_: {
			r: [210, 250, 580, 200],
			_act: 'fillParm|calculate',
			//_act: 'fillParm|calculate|reload',
			last_init: 'true',
			id: 'show_summ'
		},


		stub: ''
	},
	//  - - - - создать перевод
	create_new: {
		_act_1: 'leavParm|use_last',
		_act_2: 'fillParm|check_use_last',

		_txt_: {
			r: [380, 40, 540, 100],
			align: 'center',
			_act: 'fillParm|info_text_in_create'
		},
		c_inf_data: {
			id: 'data',
			desc: '',
			type: 't',
			_act: 'fillParm|data'
		},
		c_lfb: {	// toState
			desc: '',//LCP('button_back'),
			_evt: 'toState|start_menu|reload'
		},


		stub: ''
	},

	//  - - - -
	state_stub: {
		stub: ''
	},


	stub: ''
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// [ file include done. ]
$mt_scenario_js = true;
// [EOF]

