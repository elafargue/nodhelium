/*
 * Copyright (C) 2014 Edouard Lafargue.
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */ 
var helium = require('../helium');

var net = require('net');
var server = net.createServer(function(c){
	console.log('server connected');
	c.on('end',function (){
		console.log('server disconnected');
	});
	c.write('Hello\n');
	c.pipe(c)
});

// Will work
obj = new helium.Helium();

obj.open();

var token = "PbOkU4Jo+NObbPe27MJGNQ==";
// Javascript cannot handle 64bit numbers, so we
// split the MAC address in two 32bit numbers.
// Example for 0x0011223344556677 as the full MAC:
var mac_h = 0x000000ff; // The MSB
var mac_l = 0xfff00002; // LSB

obj.subscribe(mac_h, mac_l, token);

obj.on( 'message', function(data) {
	console.log("Message ");
	console.log(data);
});

server.listen(8124, function() {
	console.log('server bound');
});
