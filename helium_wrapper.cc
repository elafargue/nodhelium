// -*- C++ -*-

#include <node.h>
#include "helium_wrapper.h"

#include <iostream>

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

void my_callback(const helium_connection_t *conn, uint64_t mac, char * const message, size_t count)
{
    printf("Connection %p received message from device %" PRIx64 ".\n", (void *)conn, mac);
    hexdump(message,count);
}



Persistent<Function> Helium::constructor;


Helium::Helium() {


	helium_logging_start();
	// Initialize the connection:
	conn_ = helium_alloc();
	
	std::cout << "Created Helium object\n";

}

void Helium::close() {

	helium_close(conn_);
 	helium_free(conn_);	
}

int Helium::subscribe(uint64_t mac, char* base64) {

	helium_token_t token;
	helium_base64_token_decode((const unsigned char *)base64, strlen(base64), token);

	return helium_subscribe(conn_, mac, token);

}

int Helium::unsubscribe(uint64_t mac) {

	return helium_unsubscribe(conn_, mac);
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
	helium_open(obj->conn_, "r01.sjc.helium.io", my_callback);

	NanReturnValue(Number::New(42));

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

	if (args.Length() < 1 || args.Length() > 2) {
		return ThrowException(String::New("Helium subscribe requires two arguments (MAC and token)"));
	}

	Helium* obj = ObjectWrap::Unwrap<Helium>(args.Holder());

	// We expect Address as 1st argument (64bit uint)
	// and token as string for second argument.
	uint64_t mac = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
	char* base64 = get(args[1], "0");

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