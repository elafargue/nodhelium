var helium = require('./build/Release/helium');

// Will work
var obj = new helium.Helium(0x00212effff005979, "/jLFloewvLYX9Z9O8uu73w==");

obj.open();

obj.close();

// Will fail
try {
	var obj2 = new helium.Helium();
} catch (err) {
	console.log("Error thrown as expected: " + err);
}

