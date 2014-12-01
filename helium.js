/*
 * Copyright (C) 2014 Edouard Lafargue.
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */ 
var helium = require('./build/Release/helium');

// Will work
var obj = new helium.Helium();

obj.open();

var token = "[replace with token]";
// Javascript cannot handle 64bit numbers, so we
// split the MAC address in two 32bit numbers.
// Example for 0x0011223344556677 as the full MAC:
var mac_h = 0x00112233; // The MSB
var mac_l = 0x44556677; // LSB

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

