/*
 * Copyright (C) 2014 Edouard Lafargue.
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */ 
var helium = require(__dirname + '/build/Release/helium');


// We extend the Helium object with the events interface to emit events (see in examples)
var Helium = helium.Helium;
var events = require('events');

inherits(Helium, events.EventEmitter);

exports.Helium = Helium;

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}
