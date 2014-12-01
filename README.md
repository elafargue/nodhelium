nodhelium
=========

This is a work in progress Node library that wraps [libhelium](https://github.com/helium/libhelium).

This is not functional just yet. The basic "open", "close", "subscribe" and "unsubscribe" calls are implemented in the wrapper.

The next step is to implement a callback mechanism in the wrapper and finalize helium.js which will implement the correct eventing mechanism (right now helium.js is merely an example, and should be moved to tests).

The callback will be along those lines:

~~~c
  // in Init() 
  Persistent<Object> context_obj = Persistent<Object>::New(Object::New()); 
  target->Set(String::New("context"), context_obj); 

  // elsewhere in your code 
  Local<Value> args[] = { String::New("ping") }; 
  node::MakeCallback(context_obj, "on", ARRAY_SIZE(args), args); 
~~~

And the JS shim: 

~~~c
  var bindings = require('./bindings'); // the .node file 
  bindings.context.on = function(name) { 
    console.log(name); // prints "ping" 
    // now invoke the user's callbacks 
  }; 

~~~

Help very welcome!

Ed Lafargue