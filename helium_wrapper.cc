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
#include <vector>

using namespace v8;
using node::ObjectWrap;

// http://stackoverflow.com/questions/10507323/shortest-way-one-liner-to-get-a-default-argument-out-of-a-v8-function
char *get(v8::Local<v8::Value> value, const char *fallback = "") {
    if (value->IsString()) {
        v8::String::AsciiValue string(value);
        char *str = (char *) malloc(string.length() + 1);
        strcpy(str, *string);
        return str;
    }
    char *str = (char *) malloc(strlen(fallback) + 1);
    strcpy(str, fallback);
    return str;
}

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
	helium_logging_start();
	// Initialize the connection:
	conn_ = helium_alloc();
	
	std::cout << "Created Helium object\n";

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

/**
 * This method is called whenever a message arrives from a modem.
 * It generates a Node callback that is meant to be caught by the Javascript
 * layer of this addon, to dispatch to listeners - the philosophy is that it is
 * considered as best practice to do as little as necessary in C++ and let the JS
 * layer handle as much as possible.
 */
void Helium::incoming_msg(const helium_connection_t *conn, uint64_t mac, char * const message, size_t count)
{
	std::cout << "Step 0\n";
	NanScope();

	std::cout << "Step 1\n";
    Local<Value> args[2];
	std::cout << "Step 2\n";
	args[0] = *Undefined();
  	args[1] = *Undefined();

    printf("Connection %p received message from device %" PRIx64 ".\n", (void *)conn, mac);
    hexdump(message,count);

	convertMessageToJS(message, args);
	args[0] = String::New("MAC");

    node::MakeCallback(constructor, "on", 2, args);

}

/**
 * Converts a Helium message (a char*) into a proper JS "Buffer" object in Node.
 * See http://www.samcday.com.au/blog/2011/03/03/creating-a-proper-buffer-in-a-node-c-addon/
 *
 */
void Helium::convertMessageToJS(char* msg, v8::Local<v8::Value> args[]) {

	int length = strlen(msg);

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
    args[1] = buf;
}



Helium::~Helium() {
	close();
}

void Helium::Init(Handle<Object> target) {

	NanScope();

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
		Helium* obj;

		obj = new Helium();
	    obj->Wrap(args.This());
	    return args.This();

	} catch (const JSException& e) {
      return e.asV8Exception();
  	}
}

NAN_METHOD(Helium::Open) {
	NanScope();

	std::cout << "Helium Open\n";

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());

	int err = obj->open();

	NanReturnValue(Number::New(err));

}

NAN_METHOD(Helium::Close) {
	NanScope();

	std::cout << "Helium Close\n";
	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());
	obj->close();

	NanReturnValue(Number::New(43));
}


NAN_METHOD(Helium::Subscribe) {
	NanScope();

	if (args.Length() != 3) {
		return ThrowException(String::New("Helium subscribe requires 3 arguments (MAC H/L and token)"));
	}

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());

	// We expect Address as 1st argument (64bit uint)
	// and token as string for second argument.
	uint64_t mac_h = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
	uint64_t mac_l = args[1]->IsUndefined() ? 0 : args[1]->NumberValue();
	uint64_t mac = (mac_h << 32) + mac_l;
	char* base64 = get(args[2], "0");

	printf("MAC: %" PRIx64 ", Token: %s \n", mac, base64);

	int err = obj->subscribe(mac, base64);

	NanReturnValue(Number::New(err));
}

NAN_METHOD(Helium::Unsubscribe) {
	NanScope();

	if (args.Length() != 1) {
		return ThrowException(String::New("Helium unsubscribe requires one argument (MAC)"));
	}

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());
	uint64_t mac = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
	int err = obj->unsubscribe(mac);
    helium_dbg("unsubscribe result %d\n", err);

	NanReturnValue(Number::New(err));

}