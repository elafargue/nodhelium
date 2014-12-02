nodhelium
=========

This is a work in progress Node library that wraps [libhelium](https://github.com/helium/libhelium).

This version only implements basic "open", "close", "subscribe" and "unsubscribe" calls. No "send" for now.

Check test/example.js to see how you can use this addon. Once you have subscribed to a module, you will get "message" events containing the MAC and the actual message in a Buffer object.

Very little testing so far, this probably requires quite a lot of additional work to be really useful. In particular, MAC is passed as two 16bit ints, which is pretty ugly. A better Javascript wrapper should be put in place (helium.js).

Help very welcome!

Ed Lafargue