#ifndef _ruopen_h_
#define _ruopen_h_

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <string>
#include <json/json.h>
#include <boost/regex.hpp>
#include <list>

using namespace std;

struct Curl {
	CURL *handle;
	CURLcode res;
	string response; //response
	int respLen; //response length
	string responseHeader; // response header
	string cookiejar;
	struct Headers {
		struct curl_slist *json;
		struct curl_slist *text;
	} headers;
};

struct Info {
	bool isSemesterAutoSet; // was semester code given to program or parsed auto from json
	string semester;
	string semesterString;
	string campus;
};

void init();
int writeCallback(char *, size_t, size_t, string *);
bool setSemester();
string *getSubjects();
string *getCourses();
string semesterCodeToString(string);
string semesterStringToCode(string);
void debug();

void __attribute__ ((destructor)) dtor();

#endif
