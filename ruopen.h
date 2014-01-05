#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <string>
#include <json/json.h>

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
	int semester;
	string semesterString;
	string campus;
};

void init();
int writeCallback(char *, size_t, size_t, string *);
string *getSubjects();
string *getCourses();

void debug();
void __attribute__ ((destructor)) dtor();
