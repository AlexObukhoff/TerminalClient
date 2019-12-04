//.pragma library

var Idle = {
	Name: "idle",
	Event: {
		Back: "back",
		Platru: "platru",
		Info: "info",
		Language: "lang",
		Search: "search",
		UserAssistant: "user_assistant",
		Timeout: "timeout",
		Stop: "stop_ui"
	}
};

var Fill = {
	Name: "fill_bill",
	NameMultistage: "fill_multistage"
};

var Payment = {
	Name: "payment",
	Event: {
		Forward: "forward",
		Back: "back",
		Abort: "abort",
		Retry: "retry",
		Timeout: "timeout",
		SendEmail: "email",
		TopupPlatru: "topup",
		AmountUpdated: "amount",
		HIDUpdated: "hid",
		ReceiptPrinted: "receipt_printed",
		AmountDispensed: "amount_dispensed",
		PinUpdated: "pin",
	},
	Parameters: {
		Amount: "AMOUNT",
		AmountAll: "AMOUNT_ALL",
		MinAmount: "MIN_AMOUNT",
		MaxAmount: "MAX_AMOUNT",
		MaxAmountAll: "MAX_AMOUNT_ALL",
		Fee: "FEE",
		Change: "CHANGE",
		Provider: "PROVIDER",
		Error: "SERVER_ERROR",
		ErrorMessage: "ERROR_MESSAGE",
		AddInfo: "ADDINFO",
		AddFields: "ADD_FIELDS",
		ReceiptPrinted: "RECEIPT_PRINTED",
		MNPGatewayIn: "GATEWAY_IN",
		MNPGatewayOut: "GATEWAY_OUT",
		Cheated: "CHEATED"
	},
	Error: {
		ConnectionLost: -1,
		SessionAlreadyExist: 1,
		BadSD: 2,
		BadAP: 3,
		BadOP: 4,
		BadSessionCodeFormat: 5,
		BadSignature: 6,
		BadAmount: 7,
		BadNumberFormat: 8,
		BadAccountFormat: 9,
		BadDocumentFormat: 10,
		WrongIP: 12,
		OperatorNumberReject: 23,
		BadRetryPayment: 32,
		ErrorDate: 34
	},
	ProcessorType: {
		Cyberplat: "cyberplat",
		CyberplatPin: "cyberplat_pin",
		Multistage: "multistage"
	},
	ProcessError: {
		OK: 0,
		LowMoney: 1,
		OfflineIsNotSupported: 2,
		BadPayment: 3
	},
	ReceiptState: {
		Default: "default",
		LowMoney: "low_money",
		Error: "error",
		ValidatorError: "validator_error"
	},
	ReceiptTemplate: {
		"default": "payment",
		"low_money": "payment_not_enough_money",
		"error": "payment_error",
		"validator_error": "validator_error",
		"change": "payment_change"
	},
	ReceiptType: {
		Payment: "payment",
		NoFiscal: "no_fiscal",
	},
	ProcessTryCount: 3
};

var StateTimeout = {
	Finish: 60
}

var Platru = {
	Name: "Platru",
	Event: {
		Login: "login",
		SendPin: "send_pin",
		AddEntry: "add_entry",
		EditEntry: "edit_entry",
		DeleteEntry: "delete_entry",
		FillAmount: "fill_amount",
		Refresh: "refresh",
		Pay: "pay",
		Topup: "topup",
		GetHistory: "get_history",
		Back: "back",
		Abort: "abort"
	},
	TopupProvider: "5088"
};

var ServiceMenu = {
	Name: "service_menu"
};

var Event = {
	Resume: "resume",
	Back: "back"
};

var Result = {
	Back: "back",
	OK: "ok",
	Abort: "abort"
};

// Провайдеры услуги Киберсдача/Киберплатёж
var CyberService = {
	Providers: [88888, 88889, 88810],
	RassvetParking: 777778,
	ChangebackProvider: 888
};

var Sound = {
	Click1: "common/click1.wav",
	Click2: "common/click2.wav",
	ChooseOperator: "narrator/choose_operator.wav",
	EnterNumber: "narrator/enter_number.wav",
	InsertMoney: Core.graphics.ui["use_short_insert_money_hint"] === "true" ? "narrator/insert_money_dispense.wav" : "narrator/insert_money.wav",
	TakeReceipt: "narrator/take_receipt.wav"
};

