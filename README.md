nodhelium
=========

This is a work in progress Node library that wraps [libhelium](https://github.com/helium/libhelium).

This is not functional just yet. The basic "open", "close", "subscribe" and "unsubscribe" calls are implemented in the wrapper.

The next step is to implement a callback mechanism in the wrapper and finalize helium.js which will implement the correct eventing mechanism (right now helium.js is merely an example, and should be moved to tests).

Help very welcome!

Ed Lafargue