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
#include <algorithm>
//#include <boost/thread/locks.hpp> 
//#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#define CONFFILE "ruopen.conf"

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
	string campusString;
	string smsNumber;
	string smsEmail;
	string smsPassword;
};

struct Department;
struct Course;
struct Section;
typedef list<Department> ListDepts;
typedef list<Course> ListCourses;
typedef list<Section> ListSections;


void createConfFile();
string createParams(string);
void __attribute__ ((destructor)) dtor();
Json::Value *getCourses(string &);
string getCurrentSemester();
bool getDepartments();
bool init();
void printSpotting();
inline void printVersion();
bool removeCourse(int);
bool setCampus(string);
bool setSemester(string);
void spot();
bool spotCourse(string &, string &, string &);
void spotted(Department &, Course &, Section &);
int writeCallback(char *, size_t, size_t, string *);


//utils.cpp
extern list<string> providerEmails;
extern char payload_text[][70];
struct upload_status;
size_t payload_source(void *, size_t, size_t, void *);
string semesterCodeToString(string);
string semesterStringToCode(string);
void debug();


#endif
