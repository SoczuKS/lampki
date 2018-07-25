var ledController = function(baseUrl) {
	var requests = [];
	var buffer = "";
	this.settings = {
		sektory: 0,
		kolory: 0
	};
	this.loadState = () = > {
		getJSON("state", (response) = > {
			this.settings = response.settings;

			var colors = ["red", "blue", "yellow", "green", "white"];
			var div = document.getElementById("controller_diodes");
			div.innerHTML = "";

			var table = document.createElement('table'),
				tbody = document.createElement('tbody');

			table.appendChild(tbody);

			for (var x = 0; x < this.settings.sektory; x++) {
				tbody.insertRow(x);
				for (var y = 0; y < this.settings.kolory; y++) {
					tbody.rows[x].insertCell(y);
					tbody.rows[x].cells[y].classList.add("diode_" + colors[y]);
					tbody.rows[x].cells[y].id = 'diode_' + x + '_' + y;
					tbody.rows[x].cells[y].onclick = (e) = > {
						controller.setLED(e.target.parentElement.rowIndex, e.target.cellIndex, (e.target.classList.contains('diode_on') ? 0 : 1));
					};
				}
			}

			div.appendChild(table);

			response.leds.forEach((diode) = > {
				console.log(diode);
				document.getElementById('diode_' + diode.sektor + '_' + diode.kolor).classList.add("diode_on");
			});
		});
	};
	this.update = () = > {
		if (requests.length == 0) return;
		var toSend = requests.map((r) = > {
			return (r.x + "_" + r.y + "_" + r.z);
		}).join("&");
		getJSON(toSend, (response) = > {
			response.good.forEach((diode) = > {
				if (diode.stan === 1) {
					document.getElementById('diode_' + diode.sektor + '_' + diode.kolor).classList.add("diode_on");
				} else {
					document.getElementById('diode_' + diode.sektor + '_' + diode.kolor).classList.remove("diode_on");
				}
			});
		});
		requests = [];
	};
	this.setLED = (x, y, z) = > {
		requests.push({
			x: x,
			y: y,
			z: z
		});
	};
	this.test = () = > {
		getJSON("test");
	};
	this.resetDiodes = () = > {
		getJSON("reset");
	};
	this.quit = () = > {
		getJSON("quit");
	};
	this.randomize = () = > {
		getJSON("rand");
	};
	this.police = () = > {
		getJSON("997");
	};
	this.xmas = () = > {
		getJSON("xmas");
	};

	function getJSON(url, callback) {
		var xmlHttp = new XMLHttpRequest();
		xmlHttp.onreadystatechange = function() {
			if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
				console.log(xmlHttp.responseText);
				if (typeof callback === "function") {
					callback(JSON.parse(xmlHttp.responseText));
				}
			}
		};
		xmlHttp.open("GET", baseUrl + url, true);
		xmlHttp.send(null);
	}
};
