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
};

struct Department;
struct Course;
struct Section;
typedef list<Department> ListDept;
typedef list<Course> ListCourses;
typedef list<Section> ListSections;

struct Section {
	string section;
	string courseIndex;
};

struct Course {
	string course;
	string courseCode;
	ListSections sections;
};

struct Department {
	string dept;
	string deptCode;
	ListCourses courses;
};

bool init();
int writeCallback(char *, size_t, size_t, string *);
bool setSemester(string);
Json::Value *getDepartments();
Json::Value *getCourses(string);
string getCurrentSemester();
bool setCampus(string);
bool spotCourse(string &, string &, string &);
string semesterCodeToString(string);
string semesterStringToCode(string);
void debug();

void __attribute__ ((destructor)) dtor();

#endif
