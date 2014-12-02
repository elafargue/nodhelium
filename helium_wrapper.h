/*
 * Copyright (C) 2014 Edouard Lafargue.
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */ 

#ifndef HEL_WRAPPER_H
#define HEL_WRAPPER_H

#define DEBUG true

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
// exception, borrowed from https://github.com/node-hid/node-hid
// which is MIT licensed as well.
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


// //////////////////////////////////////////////////////////////////
// The main wrapper
// //////////////////////////////////////////////////////////////////

class Helium : public node::ObjectWrap {

public:
	static void Init(v8::Handle<v8::Object> exports);


	int subscribe(uint64_t mac, char* base64);
	int unsubscribe(uint64_t mac);
	int open();
	void close();

protected:
	explicit Helium();
	~Helium();

	// Callback for incoming messages fron Helium
	static void incoming_msg(const helium_connection_t *conn, uint64_t mac, char * const message, size_t count);
	static void HandleMessageDone(uv_work_t *req);

	void sendCallback(uint64_t mac, char * const message, size_t count);

	// Convert a message into a Node Buffer object
	static void convertMessageToJS(char* message, size_t count,  v8::Local<v8::Value> argv[]);

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

private:
	// ///////
	// Our internal variables
	// ///////
	helium_connection_t *conn_;
	v8::Handle<v8::Object> self;

	/**
     * Passes a WiiMote event from libcwiid's thread to the Nodejs's thread
     */
    struct helium_message {
      Helium* helium;
      uint64_t mac;
      size_t count;
      char* message;
    };

};


#endif
