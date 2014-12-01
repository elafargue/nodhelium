var helium = require('./build/Release/helium');

// Will work
var obj = new helium.Helium();

obj.open();

var token = "/jLFloewvLYX9Z9O8uu73w==";
var mac = 0x00212effff005979;

obj.subscribe(mac, token);

obj.unsubscribe(mac);

obj.close();

// Will fail
try {
	var obj2 = new helium.Helium();
} catch (err) {
	console.log("Error thrown as expected: " + err);
}

