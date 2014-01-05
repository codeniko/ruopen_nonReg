#include "ruopen.h"

using namespace std;

Curl curl;
static Info info;

#include "utils.cpp"

void init()
{
	curl.handle = NULL;
	curl.respLen = 0;
	curl.cookiejar = "cookiejar.txt";
	curl_global_init(CURL_GLOBAL_ALL);
	curl.handle = curl_easy_init();
	curl.headers.json = NULL; // init to NULL is important 
	curl.headers.text = NULL; // will reset to text/plain
	curl.headers.json = curl_slist_append(curl.headers.json, "Accept: application/json");  
	curl.headers.json = curl_slist_append(curl.headers.json, "Content-Type: application/json");
	curl.headers.json = curl_slist_append(curl.headers.json, "charsets: utf-8");

	curl_easy_setopt(curl.handle, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:17.0) Gecko/20130917 Firefox/17.0 Iceweasel/17.0.9");
	curl_easy_setopt(curl.handle, CURLOPT_NOPROGRESS, 1); // turn of progress bar
	// curl_easy_setopt(curl.handle, CURLOPT_FAILONERROR, 1); // fail on error
	curl_easy_setopt(curl.handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl.handle, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curl.handle, CURLOPT_WRITEDATA, &curl.response);
	curl_easy_setopt(curl.handle, CURLOPT_AUTOREFERER, 1);
	curl_easy_setopt(curl.handle, CURLOPT_VERBOSE, 1L);
	// curl_easy_setopt(curl.handle, CURLOPT_CUSTOMREQUEST, "PUT");
	//	curl_easy_setopt(curl.handle, CURLOPT_HEADER, 1); // A parameter set to 1 tells the library to include the header in the body output.
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.text);
	curl_easy_setopt(curl.handle, CURLOPT_WRITEHEADER, &curl.responseHeader);
	curl_easy_setopt(curl.handle, CURLOPT_POSTREDIR, 1);
	curl_easy_setopt(curl.handle, CURLOPT_COOKIEJAR, curl.cookiejar.c_str());
	// curl_easy_setopt(curl.handle, CURLOPT_POST, 1);
	curl_easy_setopt(curl.handle, CURLOPT_URL, "http://www.codeniko.net"); // default
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://www.codeniko.net"); //default
	// curl_easy_setopt(curl.handle, CURLOPT_POSTFIELDS, jsonput.c_str());
	// curl_easy_setopt(curl.handle, CURLOPT_POSTFIELDSIZE, jsonput.length());
}

int writeCallback(char* buf, size_t size, size_t nmemb, string *in)
{ //callback must have this declaration
	//buf is a pointer to the data that curl has for us
	//size*nmemb is the size of the buffer
	
	if (in != NULL)
	{
		curl.respLen = size*nmemb;
		in->append(buf, size*nmemb);
		return size*nmemb; //tell curl how many bytes we handled
	}
	curl.respLen = 0;
	return 0; //tell curl how many bytes we handled
}

bool setSemester()
{
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.json);
	curl_easy_setopt(curl.handle, CURLOPT_URL, "http://sis.rutgers.edu/soc/current_term_date.json");
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://sis.rutgers.edu/soc");
	curl.res = curl_easy_perform(curl.handle);

	if (curl.res != CURLE_OK)
		return false;
	else {
		Json::Value jsonroot;
		Json::Reader jsonreader;
		if (!jsonreader.parse(curl.response, jsonroot)) {
			cerr << "ERROR: Json parser errored on parsing semester information." << endl;
			return false;
		}
		int jsonsize = jsonroot.size();
		for (int i = 0; i < jsonsize; ++i) {
			int year = jsonroot.get("year", -1).asInt();
			int term = jsonroot.get("term", -1).asInt();
			if (year == -1 || term == -1) {
				cerr << "ERROR: Wrong semester information gathered." << endl;
				return false;
			}
			stringstream ss;
			ss << term << year; //rutgers semester code is just term + year
			info.semester = ss.str(); 
			info.semesterString = semesterCodeToString(info.semester);
			if (info.semesterString == "NULL") {
				cerr << "ERROR: Wrong semester information gathered." << endl;
				return false;
			}
		}
		return true;
	}
}

string *getSubjects()
{
	stringstream ss;
	ss << "semester=" << info.semester << "&campus=" << info.campus << "&level=U,G";
	string params = ss.str();
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.json);
	curl_easy_setopt(curl.handle, CURLOPT_URL, string("http://sis.rutgers.edu/soc/subjects.json?").append(params).c_str());
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://sis.rutgers.edu/soc");
	curl.res = curl_easy_perform(curl.handle);

	if (curl.res != CURLE_OK)
		return NULL;
	else
		return &curl.response;
}

string *getCourses(string subject)
{
	stringstream ss;
	ss << "semester=" << info.semester << "&campus=" << info.campus << "&level=U,G&subject=" << subject;
	string params = ss.str();
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.json);
	curl_easy_setopt(curl.handle, CURLOPT_URL, string("http://sis.rutgers.edu/soc/courses.json?").append(params).c_str());
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://sis.rutgers.edu/soc");
	curl.res = curl_easy_perform(curl.handle);

	if (curl.res != CURLE_OK)
		return NULL;
	else
		return &curl.response;
}

int main(int argc, char **argv)
{
	init();

	//info.semester = 12014;
	bool semset = setSemester();
	if (!semset) {
		return 1; //errored, printed in setSemestered()
	}
	info.campus = "NB";
	cout << "Semester is : " << info.semester << endl;
	cout << "Semester is : " << info.semesterString << endl;
	

/*
	string *subjects_json = getSubjects();
	if (subjects_json == NULL) {
		cerr << "ERROR: Unable to retrieve subjects" << endl;
		return 1;
	}
	curl.response.clear();
	string *courses_json = getCourses("010");
	if (courses_json == NULL) {
		cerr << "ERROR: Unable to retrieve courses" << endl;
		return 1;
	}
	*/

	return 0;
}

void __attribute__ ((destructor)) dtor() 
{
	curl_easy_cleanup(curl.handle);
	curl_slist_free_all(curl.headers.json);
	curl_global_cleanup();
}

