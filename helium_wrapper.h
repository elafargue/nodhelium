#ifndef HEL_WRAPPER_H
#define HEL_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
extern "C" {
	#include <helium.h>
	#include "helium_logging.h"
}

#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string>


#include <nan.h>

// //////////////////////////////////////////////////////////////////
// Throwable error class that can be converted to a JavaScript
// exception
// //////////////////////////////////////////////////////////////////
class JSException
{
public:
  JSException(const std::string & text) : _message(text) {}
  virtual ~JSException() {}
  virtual const std::string message() const { return _message; }
  virtual v8::Handle<v8::Value> asV8Exception() const { return ThrowException(v8::String::New(message().c_str())); }

protected:
  std::string _message;
};

/**
 * Need to implement a simple callback system:

  // in Init() 
  Persistent<Object> context_obj = Persistent<Object>::New(Object::New()); 
  target->Set(String::New("context"), context_obj); 

  // elsewhere in your code 
  Local<Value> args[] = { String::New("ping") }; 
  node::MakeCallback(context_obj, "on", ARRAY_SIZE(args), args); 

And the JS shim: 

  var bindings = require('./bindings'); // the .node file 
  bindings.context.on = function(name) { 
    console.log(name); // prints "ping" 
    // now invoke the user's callbacks 
  }; 

Hope that helps. 

*/



// //////////////////////////////////////////////////////////////////
// The main wrapper
// //////////////////////////////////////////////////////////////////

class Helium : public node::ObjectWrap {

public:
	static void Init(v8::Handle<v8::Object> exports);


	int subscribe(uint64_t mac, char* base64);
	int unsubscribe(uint64_t mac);
	void close();

private:
	explicit Helium();
	~Helium();

	/// ///////
	// The Node library calls:
	/// ///////

	static NAN_METHOD(New);

	/* Open/close a new Helium connection - passing ipv4 proxy as option */
	static NAN_METHOD(Open);
	static NAN_METHOD(Close);
    /* Subscribe to updates from an object (passes a callback) */
    static NAN_METHOD(Subscribe);
    static NAN_METHOD(Unsubscribe);

	static v8::Persistent<v8::Function> constructor;

	// ///////
	// Our internal variables
	// ///////
	helium_connection_t *conn_;

};


#endif
