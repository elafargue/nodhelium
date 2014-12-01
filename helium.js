var helium = require('./build/Release/helium');

// Will work
var obj = new helium.Helium();

obj.open();

var token = "/jLFloewvLYX9Z9O8uu73w==";
// Javascript cannot handle 64bit numbers
var mac_h = 0x00212eff
var mac_l = 0xff005979;

obj.subscribe(mac_h, mac_l, token);

while(1) {

}


obj.unsubscribe(mac);

obj.close();

// Will fail
try {
	var obj2 = new helium.Helium();
} catch (err) {
	console.log("Error thrown as expected: " + err);
}

