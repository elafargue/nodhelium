// -*- C++ -*-

#include <node.h>
#include "helium_wrapper.h"

#include <iostream>

using namespace v8;

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


Helium::Helium(uint64_t address, char* base64) {

	// Initialize our variables for the connection:
	conn_ = helium_alloc();
	helium_base64_token_decode((const unsigned char *)base64, strlen(base64), token_);

	address_ = address;

	std::cout << "Created Helium object\n";

}

void Helium::close() {

	helium_close(conn_);
 	helium_free(conn_);	
}

Helium::~Helium() {
	close();
}


// First arg is address , second arg is token
Handle<Value>  Helium::New(const Arguments& args) {
	if (!args.IsConstructCall()) {
		return ThrowException(String::New("Helium function can only be used as a constructor"));
	}

	if (args.Length() < 1 || args.Length() > 2) {
		return ThrowException(String::New("Helium constructor requires two arguments (address and token)"));
	}

	HandleScope scope;

	try {
		Helium* obj;

		uint64_t add = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
		char* base64 = get(args[1], "0");

		obj = new Helium(add, base64);
	    obj->Wrap(args.This());
	    return args.This();

	} catch (const JSException& e) {
      return e.asV8Exception();
  	}
}


void Helium::Init(Handle<Object> target) {

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(Helium::New);
  tpl->SetClassName(String::New("Helium"));

  // How many methods our class has
  tpl->InstanceTemplate()->SetInternalFieldCount(2);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "open", Open);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);

  target->Set(String::NewSymbol("Helium"), tpl->GetFunction());

}


Handle<Value> Helium::Open(const v8::Arguments& args) {
	HandleScope scope;

	std::cout << "Helium Open\n";

	return scope.Close(Number::New(42));

}

Handle<Value> Helium::Close(const v8::Arguments& args) {
	HandleScope scope;

	std::cout << "Helium Close\n";

	return scope.Close(Number::New(43));
}