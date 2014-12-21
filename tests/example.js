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
var mac = "000000fffff00002";

obj.subscribe(mac, token);

obj.send(mac, token, "meow");

obj.on( 'message', function(data) {
	console.log("Message ");
	console.log(data);
});

server.listen(8124, function() {
	console.log('server bound');
});
