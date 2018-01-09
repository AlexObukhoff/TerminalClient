/*
* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
* Cyberplat interface v2.0 skinning file
* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
* all coordiantes must be specifyed for 1280x1024 screen resolution
*
* for any lowest screen resolutions it will be automatically recalculated.
*
* - - - - - - - - - - - - - - - - - - -
*/

$ga_skp = {

        MEG : 'bingo!',

        DEBUG_MINI_TEXT : {
                r: [-1, -1, 100, 10],
                b: '#cc0000',
                i: false,

                f: ['Trebuchet MS', '9', 'bold', '#ffffff', 'left'],
                s: [0, '#ff0000']
        },
        VIEW_ACCOUNT_NONE : {
                r: [0, 0, 0, 0],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: false,        // background image (false, if not used)

                f: ['Verdana', '17', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        DEBUG_TOP_BIG : {
                d: [1200, 49],  // rectangle for panel
                b: '#ffffff',                        // background color for panel, not used if background image specifyed
                i: false, // 'op_bg_metro.gif',                // background image (false, if not used)

                f: ['Trebuchet MS', '21', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#ffffff']                                                                        // text shadow (offset and color)
        },


        SOUND_CONTROL : {
                r: [10, 10, 20, 20],  // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: false,                // background image (false, if not used)

                f: ['Trebuchet MS', '37', 'bold', '#ddddff', 'center'],        // text style in head of panel
                s: [3, '#333333']                                                                        // text shadow (offset and color)
        },

        MAIN : {
                i: 'bg.jpg'
        },

        SSAVER : {

                t: {                                        // type-dependent parameters
                        allow_flash : false
                },

                        r: [100, 100, 1000, 800],
                b: false, // '#aaaacc',
                i: false,
                f: 'mark/rek.swf'
        },

        // an a view for current terminal number information
        VIEW_TERMINFO_TERMNUMBER: {
                r: [0, 200, 330, 24],        // current time on terminal
                b: '#FFFFFF',                        // background color for panel, not used if background image specifyed
                i: false, //'bg_opvp2.bmp',                // background image (false, if not used)

                f: ['Trebuchet MS', '18', 'bold', '#A3A3A3', 'center'],        // text style
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        // an a view for current terminal support phone information
        VIEW_TERMINFO_SUPPPHONE: {
                r: [330, 200, 620, 24],        // current time on terminal
                b: '#FFFFFF',                        // background color for panel, not used if background image specifyed
                // b: '#113311',                        // background color for panel, not used if background image specifyed
                i: false, //'bg_opvp2.bmp',                // background image (false, if not used)

                f: ['Trebuchet MS', '18', 'bold', '#A3A3A3', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        // an a view for current terminal time information
        VIEW_TERMINFO_CURRENT_TIME: {
                r: [950, 200, 330, 24],        // current time on terminal
                b: '#FFFFFF',                        // background color for panel, not used if background image specifyed
                // b: '#553311',                        // background color for panel, not used if background image specifyed
                i: false, //'bg_opvp2.bmp',                // background image (false, if not used)
                f: ['Trebuchet MS', '18', 'bold', '#A3A3A3', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        VIEW_BACKTIMER: {
                r: [790, 255, 300, 30],        // current time on terminal
                b: false, // '#113311',                        // background color for panel, not used if background image specifyed
                // b: '#553311',                        // background color for panel, not used if background image specifyed
                i: false, //'bg_opvp2.bmp',                // background image (false, if not used)

                f: ['Verdana', '31', 'normal', '#ff0000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        // viewpanel for operators and groups selection
        VIEW_MARQUEE : {
                r: [315, 940, 650, 50],        // current time on terminal
                b: false, //'#ddddff',                        // background color for panel, not used if background image specifyed
                i: false, // 'bg_marquee.gif',                // background image (false, if not used)

                f: ['Trebuchet MS', '27', 'bold', '#ffffff', 'center'],        // text style in head of panel
                s: [1, '#333366']                                                                        // text shadow (offset and color)
        },

        // viewpanel for operators and groups selection
        VIEW_MAINMENU : {
                r: [0, 224, 1280, 640],  // rectangle for panel
                b: false, //'#cccccc',                        // background color for panel, not used if background image specifyed
                i: false, //'bg_opvp1.bmp',                // background image (false, if not used)

                f: ['Trebuchet MS', '27', 'normal', '#ddddff', 'center'],        // text style in head of panel
                s: [3, '#333333']                                                                        // text shadow (offset and color)
        },


        OPERATORS_TRINITY_ICON : {
                r: [0, 0, 1280, 170],  // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: false,//'main_999.gif',                // background image (false, if not used)

                f: ['Trebuchet MS', '17', 'bold', '#ddddff', 'center'],        // text style in head of panel
                s: [3, '#333333']                                                                        // text shadow (offset and color)
        },

        TRINITY_INTRINSIC_ICON : {
                d: [185, 80],                        // operator images dimensions
                bd: [248, 150],                        // operator background image dimensions
                bi: 'button.gif',                // operator background image
                f: ['Trebuchet MS', '13', 'normal', '#777', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0'] // ,                                                                        // text shadow (offset and color)
                // b: '#ff0000'
        },

        // viewpanel for operators and groups selection
        VIEW_OPERATORS : {
                r: [0, 224, 1280, 640],  // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: false,//'bg_opvp2.bmp',                // background image (false, if not used)

                f: ['Trebuchet MS', '30', '', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        VIEW_METRO_MENU : {
                r: [-1, 266, 1000, 600],  // rectangle for panel
                b: false, // '#933',                        // background color for panel, not used if background image specifyed
                i: 'bg_metro.bmp',                // background image (false, if not used)

                f: ['Trebuchet MS', '25', '', '#000', 'left'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        METRO_MENU_ITEM : {
                d: [451, 49],  // rectangle for panel
                b: false, // '#aaa',                        // background color for panel, not used if background image specifyed
                i: 'op_bg_metro.gif',                // background image (false, if not used)

                f: ['Trebuchet MS', '21', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#ffffff']                                                                        // text shadow (offset and color)
        },


        VIEW_CYBERRULES : {
                r: [-1, 270, 450, 600],  // rectangle for panel
                b: false, // '#aaaaaa', // false,                        // background color for panel, not used if background image specifyed
                i: 'cyber_rulez.bmp',//'bg_opvp2.bmp',                // background image (false, if not used)

                f: ['Trebuchet MS', '10', '', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        // viewpanel for operators and groups selection
        VIEW_INFO : {
                t: {                                        // type-dependent parameters
                        allow_flash : true
                },
                r: [0, 224, 1280, 640],  // rectangle for panel
                b: false, // '#ffcccc',                        // background color for panel, not used if background image specifyed
                i: false, //'bg_opvp1.bmp',                // background image (false, if not used)

                // f: ['Trebuchet MS', '27', 'normal', '#ddddff', 'center'],        // text style in head of panel
                // s: [3, '#333333']                                                                        // text shadow (offset and color)
                f: '../about.swf'                // flash component for panel, used if 'i'==false
        },

        // viewpanel for operators and groups selection
        VIEW_HELP : {
                t: {                                        // type-dependent parameters
                        allow_flash : true
                },
                r: [0, 224, 1280, 640],  // rectangle for panel
                b: false, // '#ccffcc',                        // background color for panel, not used if background image specifyed
                i: false, //'bg_opvp1.bmp',                // background image (false, if not used)

                // f: ['Trebuchet MS', '27', 'normal', '#ddddff', 'center'],        // text style in head of panel
                // s: [3, '#333333']                                                                        // text shadow (offset and color)
                f: '../help.swf'                // flash component for panel, used if 'i'==false
        },




        // marketing block no1
        VIEW_OP_MARK1 : {
                t: {                                        // type-dependent parameters
                        allow_flash : true
                },
                r: [0, 0, 1280, 200],        // rectangle for panel
                b: false,//'#aaaacc',                        // background color for panel, not used if image or flash component specifyed
                i: false,                                 // image for panel
                f: '../top1.swf'                // flash component for panel, used if 'i'==false
        },

        // marketing block no2 (see syntax in marketing block no1)
        VIEW_OP_MARK2 : {
                t: {                                        // type-dependent parameters
                        allow_flash : true
                },
                r: [0, 0, 1280, 200],
                b: false,//'#aaaacc',
                i: false,
                f: '../top2.swf'
        },

        // panel for account details (all input fields)
        VIEW_ACCOUNT_DETAILS : {
                r: [0, 224, 850, 640],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: false,        // background image (false, if not used)

                f: ['Trebuchet MS', '21', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        // each field info in panel
        VIEW_ACCOUNT_DETAILS_FCOMMENT : {
                r: [25, 570, 240, 485],      // rectangle for panel
                b: false, //'#ccccFF',                        // background color for panel, not used if background image specifyed
                i: false,//'bg_vp_accdet.bmp',        // background image (false, if not used)
                f: ['Trebuchet MS', '19', 'normal', '#FFFFFF', 'center'],        // text style in head of panel
                s: [3, '#333333'],
                comissionSize : '30',
                comissionColor : '#4A73C2'
        },

        // each field info in panel
        VIEW_ACCOUNT_DETAILS_FINFO : {
                r: [20, 0, 100, 35],        // rectangle for panel
                b: false, // '#cccccc',                        // background color for panel, not used if background image specifyed
                i: false,//'bg_vp_accdet.bmp',        // background image (false, if not used)

//                f: ['Trebuchet MS', '21', 'normal', '#eeeeff', 'left'],        // text style in head of panel
                f: ['Trebuchet MS', '21', 'normal', '#000', 'left'],        // text style in head of panel
//                s: [1, '#333366']                                                                        // text shadow (offset and color)
                s: [1, '#b0b0b0']                                                                        // text shadow (offset and color)
        },
				VIEW_ACCOUNT_DETAILS_FINFO_SMALL : {
                r: [20, 0, 100, 35],        // rectangle for panel
                b: false, // '#cccccc',                        // background color for panel, not used if background image specifyed
                i: false,//'bg_vp_accdet.bmp',        // background image (false, if not used)
                f: ['Trebuchet MS', '21', 'normal', '#000', 'right', '16px'],        // text style in head of panel
                s: [1, '#b0b0b0']                                                                        // text shadow (offset and color)
        },
		

        FIELD_BLINKER: {
                bp: '#000000',        // field passive
                bf: '#3E64AF',        // field data failed
                bd: '#FFFFFF',        // field data done.
                i1: 'spc.gif',
                i2: 'field1_current.png'
        },

        // each field content in panel
        VIEW_ACCOUNT_DETAILS_FIELD_COPY : {
                f: ['Trebuchet MS', '42', 'normal', '#000', 'center']        // text style in head of panel
        },
        VIEW_ACCOUNT_DETAILS_FIELD : {
                r: [296, 10, 535, 92],        // rectangle for panel
                b: false, //'#9999cc',                        // background color for panel, not used if background image specifyed
                i: 'field1.gif', // 'bg_vp_accdet.bmp',        // background image (false, if not used)
								i_blink : 'field1_current.gif',
								i_nonblink : 'spc.gif',
								i_wrong : 'field1_wrong.gif',
								i_current_wrong : 'field1_current_wrong.gif',
                f: ['Trebuchet MS', '42', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#d0d0d0']                                                                        // text shadow (offset and color)
        },
				VIEW_ACCOUNT_DETAILS_FIELD_SMALL : {
                r: [296, 10, 408, 58],        // rectangle for panel
                b: false, //'#9999cc',                        // background color for panel, not used if background image specifyed
                i: 'field2.gif', // 'bg_vp_accdet.bmp',        // background image (false, if not used)
								i_blink : 'field2_current.gif',
								i_nonblink : 'spc.gif',
								i_wrong : 'field2_wrong.gif',
								i_current_wrong : 'field2_current_wrong.gif',
                f: ['Trebuchet MS', '27', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#d0d0d0']                                                                        // text shadow (offset and color)
        },

        // panel for current account filed input area (pinpad, etc...)
        VIEW_ACCOUNT_INLET : {
                r: [850, 224, 430, 640],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: false, //'bg_vp_accinl2.bmp',        // background image (false, if not used)

                f: ['Trebuchet MS', '21', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

// panel for current account filed input area (pinpad, etc...)
        VIEW_ACCOUNT_INLET_NUMBER_FIELD : {
                r: [0, 45, 360, 64],        // rectangle for 8(***)***
                bd: [269, 157],                        // enter field background image dimensions
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'bg_enter_number.png',        // background image (false, if not used)

                f: ['Trebuchet MS', '35', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        // panel for current account filed input area (pinpad, etc...)
        VIEW_ACCOUNT_INLET_KEYS : {
                r: [265, 245, 1000, 780],        // rectangle for panel
                b: false, // '#aaaaaa',                        // background color for panel, not used if background image specifyed
//                i: 'bg_keypad.bmp', // false, // 'bg_vp_accinl2.bmp',        // background image (false, if not used)
                i: 'keyboard.gif', // false, // 'bg_vp_accinl2.bmp',        // background image (false, if not used)

                f: ['Trebuchet MS', '35', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },
				
				KEYPAD : {
      			keypad1 : 'keypad1.gif',
      			keypad2 : 'keypad2.gif',
      			keypad3 : 'keypad3.gif',
      			keypad4 : 'keypad4.gif',
      			keypad5 : 'keypad5.gif'
      	},	
      	
        PINPADBUTTON : {
                d: [120, 120],        
                i: 'pad',        
                ext: 'gif' 
        },


        // messaging panel (info messages, account properties, bye message, etc...)
        VIEW_PRINTER_NO_GET_CHANGE : {
                r: [268, 244, 744, 620],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'message_warning.gif',        // background image (false, if not used)

                f: ['Trebuchet MS', '35', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },
        VIEW_MESSAGE_MESSAGE : {
                r: [268, 244, 744, 620],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'message_warning.gif',        // background image (false, if not used)
                f: ['Trebuchet MS', '35', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },
        VIEW_PRINTER_PRINTER_ERROR : {
                r: [268, 244, 744, 620],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'message_warning.gif',        // background image (false, if not used)
                f: ['Trebuchet MS', '35', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },
        VIEW_MESSAGE_ERROR : {
                r: [268, 244, 744, 620],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'message_warning.gif',        // background image (false, if not used)
                f: ['Trebuchet MS', '35', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },
        VIEW_MESSAGE_CHECKING : {
                r: [268, 244, 744, 620],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'message.gif',        // background image (false, if not used)
                f: ['Trebuchet MS', '25', 'normal', '#FFFFFF', 'left'],        // text style in head of panel
                s: [1, '#D0D0D0'],
                numberColor : '#496EC7',
                numberSize : '35' 
        },
        VIEW_MESSAGE_CHECKING_FLASH : {
                t: {                                        // type-dependent parameters
                        allow_flash : true
                },
                r: [330, 544, 620, 300],        // rectangle for panel
                b: false,//'#aaaacc',                        // background color for panel, not used if image or flash component specifyed
                i: false,                                 // image for panel
                f: '../loading.swf'                // flash component for panel, used if 'i'==false
        },

        
        VIEW_MESSAGE : {
                r: [268, 244, 744, 620],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'message.gif',        // background image (false, if not used)

                f: ['Trebuchet MS', '21', 'normal', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },

        VIEW_MESSAGE_PAYMENT : {
                r: [432, 245, 768, 650],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
//                i: 'bg_message.bmp',        // background image (false, if not used)
                i: false,        // background image (false, if not used)

                f: ['Trebuchet MS', '25', 'bold', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                        // text shadow (offset and color)
        },
				VIEW_PAYMENT_TEXT_HEADER : {
                f: ['Trebuchet MS', '75', 'normal', '#000000', 'left', '0.9'],        // text style in head of panel
                s: [1, '#D0D0D0']
				},
				VIEW_PAYMENT_TEXT_MINMAX_SUMM : {
                f: ['Trebuchet MS', '30', 'normal', '#FFFFFF', 'left'],        // text style in head of panel
                s: [1, '#D0D0D0'],
                needSummColor: '#FB0500'
				},
				VIEW_PAYMENT_MONEY_NOT_ANOUGH : {
                f: ['Trebuchet MS', '44', 'normal', '#FD0000', 'left'],        // text style in head of panel
                s: [1, '#D0D0D0']
				},
				VIEW_PAYMENT_MONEY_ACCEPTED : {
                f : '',
                f1: ['Trebuchet MS', '36', 'normal', '#1C1C1C', 'left', '1', '12 0 0 35'],        // text style in head of panel
                f2: ['Trebuchet MS', '36', 'normal', '#1C1C1C', 'left', '1', '2 0 0 35'],        // text style in head of panel
                s: [1, '#D0D0D0'],
                b: '#FFFFFF',
                sumFontSize : '49'
				},
				VIEW_PAYMENT_MONEY_COMMISION : {
                f : '',
                f1: ['Trebuchet MS', '36', 'normal', '#1C1C1C', 'left', '1', '12 0 0 35'],        // text style in head of panel
                f2: ['Trebuchet MS', '36', 'normal', '#1C1C1C', 'left', '1', '2 0 0 35'],        // text style in head of panel
                s: [1, '#D0D0D0'],
                b: '#ECECEC',
                sumFontSize : '49'
				},
				VIEW_PAYMENT_MONEY_CREDITING : {
                f : '',
                f1: ['Trebuchet MS', '36', 'normal', '#1C1C1C', 'left', '1', '12 0 0 35'],        // text style in head of panel
                f2: ['Trebuchet MS', '36', 'normal', '#1C1C1C', 'left', '1', '2 0 0 35'],        // text style in head of panel
                s: [1, '#D0D0D0'],
                b: '#FFFFFF',
                sumFontSize : '49'
				},

        VIEW_MESSAGE_COMISSION : {
                r: [62, 245, 370, 350],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
//                i: 'bg_message.bmp',        // background image (false, if not used)
                i: false,        // background image (false, if not used)

                f: ['Trebuchet MS', '25', 'bold', '#000', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0']                                                                       // text shadow (offset and color)
        },
        VIEW_COMISSION_TEXT : {
                f: ['Trebuchet MS', '20', 'normal', '#FFFFFF', 'left'],        // text style in head of panel
                s: [1, '#D0D0D0'],
                numberColor : '#496EC7',
                numberFontSize : '29'
      	},

        // operators buttons parameters

        // germany operator icon
        OPERATOR_ICON_GER : {
                d: [200, 120],                        // operator images dimensions
                bd: [248, 190],                        // operator background image dimensions
                btw: [0, 0],                        // operator between distance
                bi: 'op_bg2.png'                        // operator background image
        },


        OPERATOR_ICON : {
                d: [185, 80],                        // operator images dimensions
                bd: [248, 150],                        // operator background image dimensions
                btw: [0, 0],                        // operator between distance
                bi: 'button.gif',                // operator background image

                f: ['Trebuchet MS', '13', 'normal', '#070707', 'center'],        // text style in head of panel
                s: [1, '#D0D0D0'] // ,                                                                        // text shadow (offset and color)
                // b: '#ff0000'
        },


        // operators buttons parameters
        CARD_ICON : {
                d: [200, 120],                        // operator images dimensions
                btw: [0, 0],
                // bd: [260, 190],                        // operator background image dimensions
                bi: 'op_bg2.png',                        // operator background image

                f: ['Trebuchet MS', '37', 'bold', '#ddddff', 'center'],        // text style in head of panel
                s: [3, '#333333']                                                                        // text shadow (offset and color)
        },


        // operators buttons parameters
        CARD_ICON1 : {
                d: [280, 100],
                btw: [0, 0],                      // operator images dimensions
                // bd: [260, 190],                        // operator background image dimensions
                bi: 'card_bg.gif',                        // operator background image

                f: ['Trebuchet MS', '37', 'bold', '#ddddff', 'center'],        // text style in head of panel
                s: [3, '#333333']                                                                        // text shadow (offset and color)
        },

        // operators buttons parameters
        ENUM_ICON : {
                d: [392, 60],                        // operator images dimensions
                bi: 'select.gif',                        // operator background image
                bi_current : 'select_current.gif', 
                f: ['Trebuchet MS', '11', 'bold', '#ddddff', 'center'],        // text style in head of panel
                f_small: ['Trebuchet MS', '19', 'bold', '#ddddff', 'center'],        // text style in head of panel
                s: [3, '#333333']                                                                        // text shadow (offset and color)
        },
        
        ENUM_ICON_SMALL : {
                d: [196, 60],                        // operator images dimensions
                bi: 'select_short.gif',              // operator background image
                bi_current : 'select_short_current.gif', 
                f: ['Trebuchet MS', '11', 'bold', '#ddddff', 'center'],        // text style in head of panel
                f_small: ['Trebuchet MS', '19', 'bold', '#ddddff', 'center'],        // text style in head of panel
                s: [3, '#333333']                                                                        // text shadow (offset and color)
        },

        // !!! temporary deprecated !!!
        // operators buttons parameters
        OPERATOR_ICON_OLD : {
                d: [185, 64],                        // operator images dimensions
                bd: [210, 100],                        // operator background image dimensions
                bi: 'op_bg3.gif'                // operator background image
        },


        // group buttons parameters
        GROUP_ICON : {
                bd: [413, 225],                        // group background image dimensions
                btw: [0, 0],                        // group between distance
                bi: false,//'gbut_clean1.gif',        // group background image

                f: ['Trebuchet MS', '31', 'bold', '#ffffff', 'center'],        // text style on button
                s: [3, '#333333']                                                                        // text shadow (offset and color)

        },

        // big user-control buttons
        BUTBIG : {
                d: [248, 150],                                                                                // button image size
                f: ['Trebuchet MS', '31', 'normal', '#ffffff', 'center'],        // text style on button
                s: [1, '#535353'],
                i: 'button_menu.gif'
                                                                     // text shadow (offset and color)
        },
        
        MAINPAGEBUTBIG : {
							  leftbut: 'button_about.gif',
							  rightbut: 'button_help.gif'
        },

				BUTTONS : {
							  back 	: 'button_back.gif',
								cancel 	: 'button_cancel.gif',
								no 			: 'button_no.gif',
								main 		: 'button_menu.gif',
								next		: 'button_next.gif',
								ok			: 'button_ok.gif',
								payment : 'button_pay.gif',
								yes			: 'button_yes.gif'
        },

        // big user-control buttons
        BUTKEYBOARD : {

                d: [290, 100],                                                                                // button image size
                f: ['Trebuchet MS', '21', 'normal', '#FFFFFF', 'center'],        // text style on button
                s: [1, '#ffffff']                                                                        // text shadow (offset and color)
        },

        // temp!!!1
        FORM_TEXT : {
                f: ['Trebuchet MS', '33', 'bold', '#ffffff', 'left'],        // text on buttons
                s: [3, '#333333']        // text shadow
        },



        // deprecated
        // viewport (окно отображения) для выбора оператора
        VP_OPSEL        : {
                r: [-1, -1, 600, 500],  //
                i: false
        },
        FLDI : {
                d: [400, 50],
                f: ['Trebuchet MS', '31', 'bold', '#000033', 'right'],
                bc: '#999999'
        },

        // messaging panel (info messages, account properties, bye message, etc...)
        VIEW_TEXT : {
                r: [-1, -1, 700, 500],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'spc.gif',        // background image (false, if not used)

                f: ['Trebuchet MS', '43', 'normal', '#444455', 'center'],        // text style in head of panel
                s: [0, '#222222']                                                                        // text shadow (offset and color)
        },

        // messaging panel (info messages, account properties, bye message, etc...)
        VIEW_TEXT1 : {
                r: [-1, -1, 700, 500],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'bg_message.bmp',        // background image (false, if not used)

                f: ['Trebuchet MS', '29', 'normal', '#444455', 'center'],        // text style in head of panel
                s: [0, '#000000']          // used 4 currency highlighting
        },

        VIEW_TEXT2 : {
                r: [-1, -1, 700, 500],        // rectangle for panel
                b: false,                        // background color for panel, not used if background image specifyed
                i: 'bg_message.bmp',        // background image (false, if not used)

                f: ['Trebuchet MS', '35', 'normal', '#000', 'center'],        // text style in head of panel
                s: [0, '#000000']          // used 4 currency highlighting
        },


        METRO_STYLES : {
                s: '.elError { font-family: Trebuchet MS, Helvetica, sans-serif; font-size: 90%; color: #FF0000; } .elInfoNormal { font-family: Trebuchet MS, Helvetica, sans-serif; font-size: 90%; } .elInfoBlack { font-family: Trebuchet MS, Helvetica, sans-serif; font-size: 90%; } .elInfoRed { font-family: Trebuchet MS, Helvetica, sans-serif; font-size: 90%; color: #FF0000; text-decoration: blink; }'
        },

				PERPAGE : {
						    r: [600, 905, 300, 100],        // current time on terminal
                b: false,                        // background color for panel, not used if background image specifyed
                i: false, //'bg_opvp2.bmp',                // background image (false, if not used)
                f: ['Trebuchet MS', '30', 'normal', '#000000', 'left'],        // text style
                s: [1, '#D0D0D0'],
                i: 'page',
                ext: '.gif',
                d : [124, 150]
				},
				
        ADV_FLASH2 : {
                t: {                                        // type-dependent parameters
                        allow_flash : true
                },
                r: [21, 250, 248, 600],        // rectangle for panel
                b: false,//'#aaaacc',                        // background color for panel, not used if image or flash component specifyed
                i: false,                                 // image for panel
                f: '../left.swf'                // flash component for panel, used if 'i'==false
        },
				ADV_FLASH3 : {
                t: {                                        // type-dependent parameters
                        allow_flash : true
                },
                r: [1011, 250, 248, 600],        // rectangle for panel
                b: false,//'#aaaacc',                        // background color for panel, not used if image or flash component specifyed
                i: false,                                 // image for panel
                f: '../right.swf'                // flash component for panel, used if 'i'==false
        },
				ADV_FLASH4 : {
                t: {                                        // type-dependent parameters
                        allow_flash : true
                },
                r: [330, 550, 630, 300],        // rectangle for panel
                b: false,//'#aaaacc',                        // background color for panel, not used if image or flash component specifyed
                i: false,                                 // image for panel
                f: '../print.swf'                // flash component for panel, used if 'i'==false
        },
						PB_BOOK : {
					coord : [330, 640],
        	        d: [620, 160],        // rectangle for panel
    	            i: 'pb_banner.gif'        // background image (false, if not used)
        },

        MEG : 'rulezzzz!'
};


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//  - - - - - всегда в true, для динамического include файла
$inc_skp = true;

