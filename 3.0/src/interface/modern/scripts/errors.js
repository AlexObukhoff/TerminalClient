/* Описания ошибок*/

var _errors = [
			{ value: -1, description: QT_TR_NOOP("errors#network_error") },
			{ value: 7, description: QT_TR_NOOP("errors#wrong_amount") },
			{ value: 23, description: QT_TR_NOOP("errors#operator_number_reject") },
			{ value: 223, description: QT_TR_NOOP("errors#card_not_activated") },
			{ value: 32, description: QT_TR_NOOP("errors#bad_retry_number") }
		];

function getDescription(aErrorCode) {
	for (var i in _errors) {
		if (_errors[i].value === Number(aErrorCode)) {
			return _errors[i].description;
		}
	}

	return QT_TR_NOOP("errors#cannot_check_payment");
}
