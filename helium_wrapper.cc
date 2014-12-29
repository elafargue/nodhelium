// -*- C++ -*-
/*
 * Copyright (C) 2014 Edouard Lafargue.
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */ 

#include <node.h>
#include "helium_wrapper.h"

#include <iostream>
#include <stdio.h>

using namespace v8;
using node::ObjectWrap;

void UV_NOP(uv_work_t* req) { /* No operation */ }

 void hexdump(char *buffer, size_t length)
 {
  int i;
  for (i=0;i<(int)length;i++) {
		printf("%02x ",buffer[i] & 0xff);
   }
  printf("\nASCII: \n");
  for (i=0;i<(int)length;i++)
	{
	if (buffer[i] < 32) printf(".");
	else printf("%c",buffer[i]);
	}
  printf("\n");
 }


Persistent<Function> Helium::constructor;


/**
 * Create the Helium connection object. Just prepares the API
 * using libhelium
 */
Helium::Helium() {
	// helium_logging_start();
	// Initialize the connection:
	conn_ = helium_alloc();
	this->is_open = 0;
	helium_set_context(conn_, this);

}

/**
 * Opens the connection. Right now hardcodes an ipv4 proxy, but
 * this should be made optional before release.
 */
int Helium::open() {
	return helium_open(conn_, "r01.sjc.helium.io", incoming_msg);
}

/**
 * Close the connection and frees up resources.
 */
void Helium::close() {

	helium_close(conn_);
 	helium_free(conn_);	
}

/**
 * Subscribe to a modem: needs the MAC address and public token
 * for the call to be successful.
 */
int Helium::subscribe(uint64_t mac, char* base64) {

	helium_token_t token;
	helium_base64_token_decode((const unsigned char *)base64, strlen(base64), token);

	return helium_subscribe(conn_, mac, token);

}

/**
 * Unsubscribe from messages from a moden. Only the MAC address
 * is needed there.
 */
int Helium::unsubscribe(uint64_t mac) {

	return helium_unsubscribe(conn_, mac);
}

int Helium::send(uint64_t mac, char* base64, char *message, size_t msg_len) {

	helium_token_t token;
	helium_base64_token_decode((const unsigned char *)base64, strlen(base64), token);

	return helium_send(conn_, mac, token, (unsigned char*)message, msg_len);

}

/**
 * This method is called whenever a message arrives from a modem.
 * It generates a Node callback that is meant to be caught by the Javascript
 * layer of this addon, to dispatch to listeners - the philosophy is that it is
 * considered as best practice to do as little as necessary in C++ and let the JS
 * layer handle as much as possible.
 */
void Helium::incoming_msg(const helium_connection_t *conn, uint64_t mac, char * const message, size_t count)
{

	uv_work_t* uv = new uv_work_t;

  	Helium *self = const_cast<Helium*>(static_cast<const Helium*>(helium_get_context(conn)));

  	// We need to make a copy of the message for our use down the line:
	struct helium_message* req = (struct helium_message *)malloc(sizeof(*req));
	req->mac = mac;
	req->helium = self;
	req->count = count;
	req->message = (char *)malloc(count);
	memcpy(req->message, message, count);

	uv->data = req;

	// This will call HandleMessageDone in the nodejs thread, where we ca create V8 objects
	int r = uv_queue_work(uv_default_loop(), uv, UV_NOP, (uv_after_work_cb)HandleMessageDone);
	if (r != 0) {
    	std::cout << "error while queuing helium message callback";
    	delete uv;
  	}

}

/**
 * Callback after incoming_msg, but this one is within the NodeJS thread context, so we have
 * access to our scope, V8 objects, and so on.
 */
void Helium::HandleMessageDone(uv_work_t *req) {
	helium_message* msg = static_cast<helium_message* >(req->data);
	NanScope();
	msg->helium->sendCallback(msg->mac, msg->message, msg->count);
	// Don't forget to free our memory!
	free(msg->message);
	free(msg);
	delete req;
}

void Helium::sendCallback(uint64_t mac, char * const message, size_t count) {
    //printf("Received message from device %" PRIx64 ".\n", mac);
    //hexdump(message, count);

    // Print the MAC into a hex string
    char macstring[17];
    sprintf(macstring, "%016"PRIx64, mac);

	// First argument is the event name ("message")
	Local<Value> args[2] = { String::New("message"), *Undefined()};

	// Create a Javascript object like this:
	// { mac: "HEX String", message: Buffer}
	Local<Object> resp = Object::New();
	resp->Set(String::NewSymbol("mac"), String::New(macstring));
	Local<Value> buf[1];
	convertMessageToJS(message, count, buf);
	resp->Set(String::NewSymbol("message"), buf[0]);
	args[1] = resp;

  	node::MakeCallback(self, "emit", 2, args);

}

/**
 * Converts a Helium message (a char*) into a proper JS "Buffer" object in Node.
 * See http://www.samcday.com.au/blog/2011/03/03/creating-a-proper-buffer-in-a-node-c-addon/
 *
 */
void Helium::convertMessageToJS(char* msg, size_t length, v8::Local<v8::Value> arg[]) {

	// Allocate binary buffer:
	node::Buffer *slowBuffer = node::Buffer::New(length);
	memcpy(node::Buffer::Data(slowBuffer), msg, length);

    //Get "fast" node Buffer constructor
    Local<Function> nodeBufConstructor = Local<Function>::Cast(
      Context::GetCurrent()->Global()->Get(String::New("Buffer") )
    );
    //Construct a new Buffer
    // First argument is the JS object Handle for the SlowBuffer.
	// Second arg is the length of the SlowBuffer.
	// Third arg is the offset in the SlowBuffer we want the "Fast"Buffer to start at.
    Handle<Value> nodeBufferArgs[3] = { slowBuffer->handle_,
    									Integer::New(length),
    									Integer::New(0) };
    Local<Object> buf = nodeBufConstructor->NewInstance(3, nodeBufferArgs);
    arg[0] = buf;
}



Helium::~Helium() {
	close();
}

void Helium::Init(Handle<Object> target) {

	HandleScope scope;

  // Prepare constructor template
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(Helium::New);
  tpl->SetClassName(String::New("Helium"));

  // How many methods our class has
  tpl->InstanceTemplate()->SetInternalFieldCount(4);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "open", Open);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
  NODE_SET_PROTOTYPE_METHOD(tpl, "subscribe", Subscribe);
  NODE_SET_PROTOTYPE_METHOD(tpl, "unsubscribe", Unsubscribe);
  NODE_SET_PROTOTYPE_METHOD(tpl, "send", Send);

  NanAssignPersistent(constructor, tpl->GetFunction());
  target->Set(NanNew("Helium"), tpl->GetFunction());

}

// First arg is address , second arg is token
NAN_METHOD(Helium::New) {
	if (!args.IsConstructCall()) {
		return ThrowException(String::New("Helium function can only be used as a constructor"));
	}

	HandleScope scope;

	try {
		Helium* obj = new Helium();
	    obj->Wrap(args.This());

		obj->self = Persistent<Object>::New(args.This());

	    return scope.Close(args.This());

	} catch (const JSException& e) {
      return e.asV8Exception();
  	}
}

NAN_METHOD(Helium::Open) {
	NanScope();

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());
	if (obj->is_open == 1) {
		return ThrowException(String::New("Helium connection is already open"));
	}

	int err = obj->open();
	if (err == 0) {
		obj->is_open = 1;
	}
	NanReturnValue(Number::New(err));

}

NAN_METHOD(Helium::Close) {
	NanScope();

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());
	if (obj->is_open != 1) {
		return ThrowException(String::New("Helium connection was not opened"));
	}
	obj->close();
	obj->is_open = 0;
	NanReturnValue(Number::New(43));
}


NAN_METHOD(Helium::Subscribe) {
	NanScope();

	if (args.Length() != 2) {
		return ThrowException(String::New("Helium subscribe requires 2 arguments (MAC and token)"));
	}

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());

	if (obj->is_open != 1) {
		return ThrowException(String::New("Helium connection was not opened"));
	}

	if (!args[0]->IsString()) {
		return ThrowException(String::New("MAC was not a string"));
	}
	v8::String::AsciiValue string(args[0]);

	if (!args[1]->IsString()) {
		return ThrowException(String::New("Token was not a string"));
	}
	v8::String::AsciiValue base64(args[1]);

	// TODO check endptr
	uint64_t mac = (uint64_t)strtoll(*string, NULL, 16);

	int err = obj->subscribe(mac, *base64);
	NanReturnValue(Number::New(err));
}

NAN_METHOD(Helium::Unsubscribe) {
	NanScope();

	if (args.Length() != 1) {
		return ThrowException(String::New("Helium unsubscribe requires one argument (MAC)"));
	}

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());

	if (obj->is_open != 1) {
		return ThrowException(String::New("Helium connection was not opened"));
	}

	if (!args[0]->IsString()) {
		return ThrowException(String::New("MAC was not a string"));
	}
	v8::String::AsciiValue string(args[0]);

	// TODO check endptr
	uint64_t mac = (uint64_t)strtoll(*string, NULL, 16);

	int err = obj->unsubscribe(mac);
	helium_dbg("unsubscribe result %d\n", err);

	NanReturnValue(Number::New(err));

}

NAN_METHOD(Helium::Send) {
	NanScope();

	if (args.Length() != 3) {
		return ThrowException(String::New("Helium subscribe requires 3 arguments (MAC, token and message)"));
	}

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());

	if (obj->is_open != 1) {
		return ThrowException(String::New("Helium connection was not opened"));
	}

	if (!args[0]->IsString()) {
		return ThrowException(String::New("MAC was not a string"));
	}
	v8::String::AsciiValue macstring(args[0]);

	// TODO check endptr
	uint64_t mac = (uint64_t)strtoll(*macstring, NULL, 16);

	if (!args[1]->IsString()) {
		return ThrowException(String::New("Token was not a string"));
	}
	v8::String::AsciiValue base64(args[1]);

	if (!args[2]->IsString()) {
		return ThrowException(String::New("Message was not a string"));
	}
	v8::String::Utf8Value string(args[2]);

	int err = obj->send(mac, *base64, *string, string.length());
	NanReturnValue(Number::New(err));
}

