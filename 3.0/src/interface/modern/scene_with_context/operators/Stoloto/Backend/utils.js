/* @file Вспомогательный набор функций для работы с билетами */

Qt.include("../../../../scripts/gui.js");

var Games = {
	"rusloto": "7103", // - Русское лото
	"zhil": "7105",    // - Жилищная лотерея
	"zp": "7115",    // - Золотая подкова
	"6x36": "7101",    // - 6 из 36
	"bingo75": "7175",  //- Бинго 75

	gameId: function() {
		var provider = Core.payment.getProvider(GUI.props("operator_id"))
		return provider.receiptParameters["GAME_ID"]
	}
}

function zeroArray() {
	return Array.apply(null, Array(9)).map(Number.prototype.valueOf, 0);
}

function fillNumbers(value) {
	var result = [];
	var row_result = zeroArray();
	var prev = 0;


	value.map(function(item) {
		// Новый ряд
		if (item < prev) {
			prev = 0;

			if (row_result.length) {
				result.concat(row_result);
				result = result.concat(row_result);
			}

			row_result = zeroArray();
		}

		var index = Math.floor(item / 10);
		if (item <= 90 && item >= 80) {
			index = 8;
		}

		row_result[index] = item;
		prev = item;
	});

	// Последний ряд
	result = result.concat(row_result);

	return result;
}


function createNumbersModel(nums) {
	var result = [];

	nums.map(function(item){
		result.push({value: item});
	});

	return result;
}

function getLotoTickets(aParams) {
	var result = [];

	for (var i in aParams.combinations) {
		var ticket = aParams.combinations[i]

		var numbers = fillNumbers(ticket.numbers);

		result.push({
						selected: false,
						barCode: ticket.barCode,
						numbersField1: createNumbersModel(numbers.slice(0, 27)),
						numbersField2: createNumbersModel(numbers.slice(27, 54)),
					})
	}

	return result;
}

function getBingo75Tickets(aParams) {
	var result = [];

	for (var i in aParams.combinations) {
		var ticket = aParams.combinations[i]
		ticket.numbers.splice(12, 0, "0")
		var numbers = ticket.numbers

		result.push({
						selected: false,
						barCode: ticket.barCode,
						numbers: createNumbersModel(numbers)
					})
	}

	return result;
}

function createNumbersModel6x36(nums) {
	var result = [];

	for (var i = 1; i <= 36; i++) {
		var num = i < 10 ? "0%1".arg(i) : "%1".arg(i)
		result.push({value: num, disabled: nums.indexOf(num) != -1});
	}

	return result;
}

function get6x36Tickets(aParams) {
	var result = [];

	for (var i in aParams.combinations) {
		var ticket = aParams.combinations[i]

		result.push({
						selected: false,
						barCode: ticket.barCode,
						numbers: createNumbersModel6x36(ticket.numbers)
					})
	}

	return result;
}
