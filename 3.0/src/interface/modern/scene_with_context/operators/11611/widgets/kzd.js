/*var $ = {
	o: User.getObject("KZD"),

	model: null,//User.getObject("KZD").model,

	setFrom: function (aFrom) {
		return User.getObject("KZD").setFrom(aFrom);
	},

	setTo: function(aTo) {
		return  User.getObject("KZD").setTo(aTo);
	},

	search: function() {
		User.getObject("KZD").search();
	},

	get: function(aParameters) {
		User.getObject("KZD").get(aParameters);
	},

	result: function() {
		return User.getObject("KZD").getSearchResult();
	}
}*/

Qt.include("../../../../scripts/gui.js")

var $ = {
	ticket: function(aKey) {
		var ticket = Core.userProperties.get("ticket");
		return aKey ? ((ticket && ticket.hasOwnProperty(aKey) && ticket[aKey]) ? ticket[aKey] : 0) : ticket;
	},

	updateTicket: function(aKey, aValue) {
		var ticket = Core.userProperties.get("ticket") ? Core.userProperties.get("ticket") : {};
		ticket[aKey] = aValue;

		if (ticket.train) {
			ticket["trainNumber"] = this.trainModel()[ticket.train].Number;
			ticket["depDate"] = this.trainModel()[ticket.train].DepartureDateTime;
			ticket["depTime"] = this.trainModel()[ticket.train].DepartureDateTime.split("T")[1];
			ticket["timeInWay"] = this.trainModel()[ticket.train].TimeInWay;
		}

		if (ticket.car) {
			for (var i in this.carModel()) {
				if (this.carModel()[i].Number == ticket.car) {
					ticket["carNumber"] = this.carModel()[i].Number;
					ticket["carClassService"] = this.carModel()[i].ClassService.Type;
					break;
				}
			}
		}

		Core.userProperties.set("ticket", ticket);

		log("#updateTicket", aKey, aValue, ticket);
	},

	resetTicket: function() {
		Core.userProperties.set("ticket", {});
		log("#reset ticket")
	},

	o51: function() {
		var result; try { result = JSON.parse(Backend$KZD.lastTrainSearch) } catch(e) { Core.log.error("KZD request parse error: %1".arg(e.message)) } return result
	},

	o52: function() {
		var result; try { result = JSON.parse(Backend$KZD.lastCarSearch) } catch(e) { Core.log.error("KZD request parse error: %1".arg(e.message)) } return result
	},

	trains: function(aForward) {
		aForward = aForward && aForward !== "undefined" ? aForward : true;

		// Поиск по одним суткам
		return this.o51()[aForward ? "ForwardDirection" : "BackwardDirection"]["Trains"][0]["TrainsCollection"];
	},

	cars: function(aForward) {
		aForward = aForward && aForward !== "undefined" ? aForward : true;

		return this.o52()[aForward ? "ForwardDirectionDto" : "BackwardDirectionDto"]["Trains"][0]["Train"];
	},

	trainModel: function() {
		var result = [];
		var trains = this.trains();

		for(var i in trains) {
			var $ = trains[i];
			result.push({
										Number: $.Number,
										Type: $.Type,
										DepartureDateTime: $.DepartureDateTime,
										ArrivalDateTime: $.ArrivalDateTime,
										TimeInWay: $.TimeInWay,
										FreeSeats: this.freeSeats(),
										Tariffs: this.tariffs(),
										value: result.length
									});
		}

		return result;
	},

	typeModel: function() {
		var result = [];
		var train = this.trains()[this.ticket("train")];

		for(var i in train.Cars) {
			var $ = train.Cars[i];
			result.push({
										Type: $.Type,
										Type2: "",
										FreeSeats: $.FreeSeats,
										TariffMin: this.tariffs()[$.Type].MinValue,
										TariffMax: this.tariffs()[$.Type].MaxValue,
										value: $.Type
									});
		}

		return result;
	},

	carModel: function() {
		var result = [];
		var car = this.cars();

		var types = ["", "Общий", "Сидячий", "Плацкартный", "Купе", "Мягкий", "Люкс"];

		for(var i in car.Cars) {
			var $ = car.Cars[i];

			if (types[this.ticket("type")] !== $.Type) continue;

			for (var k in $.Cars) {
				var $$ = $.Cars[k];

				result.push({
											Type: $.Type,
											Tariff: $.Tariff,
											ClassService: { Type: $.ClassService.Type, Value: $.ClassService.Value },
											Owner: $.Owner.Type,
											Number: $$.Number,
											ElRegPossible: $$.hasOwnProperty("ElRegPossible"),
											SeatsDn: $$.Seats.SeatsDn,
											SeatsUp: $$.Seats.SeatsUp,
											SeatsLateralDn: $$.Seats.SeatsLateralDn,
											SeatsLateralUp: $$.Seats.SeatsLateralUp,
											SpecialCarDetails: $$.SpecialCarDetails,
											Places: $$.Places,											
											value: $$.Number
										});
			}
		}

		result.sort(function(a,b) {return (a.Number > b.Number) ? 1 : ((b.Number > a.Number) ? -1 : 0);} );

		return result;
	},

	placesModel: function() {
		var result = [];

		for (var i in this.carModel()) {
			if (this.ticket("car") !== this.carModel()[i].Number) continue;

			var $ = this.carModel()[i];

			result.push({
										Train: this.trains()[this.ticket("train")].Number,
										Number: $.Number,
										Type: $.Type,
										Tariff: $.Tariff,
										ClassServiceType: $.ClassService.Type,
										SpecialCarDetails: $.SpecialCarDetails ? $.SpecialCarDetails[0] : 0,
										Places: { Places: $.Places },
										value: ""
									});
		}

		return result;
	},

	freeSeats: function() {
		var result = {};

		this.trains()[this.ticket("train")]["Cars"].forEach(function(aCar){
			result[aCar.Type] = (result[aCar.Type] || 0) + Number(aCar.FreeSeats);
		});

		return result;
	},

	freeSeatsFull: function() {
		var result = {};

		this.trains()[this.ticket("train")]["Cars"].forEach(function(aCar){
			result[aCar.Type] = aCar.Seats;
		});

		return result;
	},

	tariffs: function() {
		var result = {};

		this.trains()[this.ticket("train")]["Cars"].forEach(function(aCar){
			var tariffs = []
			var minValue = aCar.Tariffs[0].TariffValue;
			var maxValue = aCar.Tariffs[0].TariffValue;

			aCar.Tariffs.forEach(function(aTariff){
				minValue = minValue > aTariff.TariffValue ? aTariff.TariffValue : minValue;
				maxValue = maxValue < aTariff.TariffValue ? aTariff.TariffValue : maxValue;
				tariffs.push({"Carrier": aTariff.Carrier.Name, Value: Number(aTariff.TariffValue).toFixed(2)});
			});
			result[aCar.Type] = {"MinValue": minValue, "MaxValue": maxValue, Carriers: tariffs};
		});
		return result;
	}
}

function formatDate(aTime) {
	var d = new Date((aTime || "").replace(/-/g,"/").replace(/[TZ]/g," "));

	function $(aValue) {
		return (aValue < 10) ? "0%1".arg(aValue) : aValue;
	}

	return {
		time: "%1:%2".arg($(d.getHours())).arg($(d.getMinutes())),
		date: "%1.%2.%3".arg($(d.getDate())).arg($(d.getMonth()+1)).arg($(d.getFullYear()))
	}
}

function formatValue(aNumber) {
	return aNumber ? aNumber : "---";
}

function formatPrice(aPrice) {
	return "%1 %2".arg(aPrice ? aPrice.MinValue : "---").arg(aPrice ? "₸" : "");
}

String.prototype.firstToUp = function() {
	return this.charAt(0).toUpperCase() + this.slice(1);
}

