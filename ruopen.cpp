#include "ruopen.h"

using namespace std;

Curl curl;
static Info info;
static ListDept spot;

#include "utils.cpp"
/*
void cleanup()
{
	curl_slist_free_all(curl.headers.json);
	curl_easy_cleanup(curl.handle);
	curl_global_cleanup();
	while (!spot.empty()) {
		Department *dept = spot.front();
		while (!(dept->courses.empty())) {
			Course *course = dept->courses.front();
			cout << "Freeing course " << course->course << endl;
			delete course;
			dept->courses.pop_front();
		}
		cout << "Freeing department " << dept->dept << endl;
		delete dept;
		spot.pop_front();
	}
}
*/
bool init()
{
	curl.handle = NULL;
	curl.respLen = 0;
	curl.cookiejar = "cookiejar.txt";
	curl_global_init(CURL_GLOBAL_ALL);
	curl.handle = curl_easy_init();
	if (!curl.handle)
		return false;
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
	//curl_easy_setopt(curl.handle, CURLOPT_VERBOSE, 1L);
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
	setCampus("new brunswick");
	return setSemester(getCurrentSemester());
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

string getCurrentSemester()
{
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.json);
	curl_easy_setopt(curl.handle, CURLOPT_URL, "http://sis.rutgers.edu/soc/current_term_date.json");
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://sis.rutgers.edu/soc");
	curl.res = curl_easy_perform(curl.handle);

	if (curl.res != CURLE_OK)
		return "NULL";
	else {
		Json::Value jsonroot;
		Json::Reader jsonreader;
		if (!jsonreader.parse(curl.response, jsonroot)) {
			cerr << "ERROR: Json parser errored on parsing semester information." << endl;
			return "NULL";
		}
		int year = jsonroot.get("year", -1).asInt();
		int term = jsonroot.get("term", -1).asInt();
		if (year == -1 || term == -1) {
			cerr << "ERROR: Wrong semester information gathered." << endl;
			return "NULL";
		}
		stringstream ss;
		ss << term << year; //rutgers semester code is just term + year
		curl.response.clear();
		return semesterCodeToString(ss.str());
	}
}

string createParams(string dept = "NULL")
{
	stringstream ss;
	ss << "semester=" << info.semester << "&campus=" << info.campus << "&level=U,G";
	if (dept != "NULL")
		ss << "&subject=" << dept;
	return ss.str();
}

Json::Value *getDepartments()
{
	string params = createParams();
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.json);
	curl_easy_setopt(curl.handle, CURLOPT_URL, string("http://sis.rutgers.edu/soc/subjects.json?").append(params).c_str());
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://sis.rutgers.edu/soc");
	curl.res = curl_easy_perform(curl.handle);
	if (curl.res != CURLE_OK)
		return NULL;

	Json::Value *depts = new Json::Value();
	Json::Reader jsonreader;
	if (!jsonreader.parse(curl.response, *depts)) {
		cerr << "ERROR: Json parser errored on Departments." << endl;
		return NULL;
	}

	curl.response.clear();
	return depts;
}

Json::Value *getCourses(string deptcode)
{
	string params = createParams(deptcode);
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.json);
	curl_easy_setopt(curl.handle, CURLOPT_URL, string("http://sis.rutgers.edu/soc/courses.json?").append(params).c_str());
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://sis.rutgers.edu/soc");
	curl.res = curl_easy_perform(curl.handle);
	if (curl.res != CURLE_OK)
		return NULL;

	Json::Value *courses = new Json::Value();
	Json::Reader jsonreader;
	if (!jsonreader.parse(curl.response, *courses)) {
		cerr << "ERROR: Json parser errored on Departments." << endl;
		return NULL;
	}

	curl.response.clear();
	return courses;
}

bool setSemester(string semesterString)
{
	string code;
	if (semesterString == "NULL" || (code = semesterStringToCode(semesterString)) == "NULL")
		return false;

	info.semester = code;
	info.semesterString = semesterCodeToString(code);
	return true;
}

bool setCampus(string campus)
{
	transform(campus.begin(), campus.end(), campus.begin(), ::tolower);
	if (campus == "new brunswick") {
		info.campus = "NB";
		info.campusString = "New Brunswick";
	} else if (campus == "newark") {
		info.campus = "NK";
		info.campusString = "Newark";
	} else if (campus == "camden") {
		info.campus = "CM";
		info.campusString = "Camden";
	} else
		return false;
	return true;
}

bool spotCourse(string &deptcode, string &coursecode, string &sectioncode)
{
	//Validate Department Input
	Json::Value *dept_json = getDepartments();
	Json::ValueIterator it, it2, it3;
	for (it = dept_json->begin(); it != dept_json->end(); ++it) {
		if ((*it).get("code", "NULL").asString() == deptcode)
			break;
	}
	if (it == dept_json->end()) {
		delete dept_json;
		return false;
	}
	Department dept;
	dept.dept = (*it).get("description", "NULL").asString();
	dept.deptCode = deptcode;
	delete dept_json;

	//Validate Course Input
	Json::Value *course_json = getCourses(dept.deptCode);
	int c;
	for (c = 0; c < (*course_json).size(); ++c) {
		if ((*course_json)[c].get("courseNumber", "NULL").asString() == coursecode)
			break;
	}
	if (c == (*course_json).size()) {
		delete course_json;
		return false;
	}
	Course course;
	course.course = (*course_json)[c].get("title", "NULL").asString();
	course.courseCode = coursecode;

	//Validate Section Input
	Json::Value sections = (*course_json)[c]["sections"];
	int s;
	for (s = 0; s < sections.size(); ++s) 
	{
		if (sections[s].get("number", "NULL").asString() == sectioncode)
			break;
	}
	if (s == sections.size()) {
		delete course_json;
		return false;
	}
	Section section;
	section.courseIndex = sections[s].get("index", "NULL").asString();
	section.section  = sections[s].get("number", "NULL").asString();
	delete course_json;

	course.sections.push_back(section);
	dept.courses.push_back(course);
	spot.push_back(dept);
	return true;
}

void createConfFile()
{
	ofstream conf;
	conf.open(CONFFILE);
	conf << "# NOTE* If not using a setting or want the default, you can also remove the setting entirely or have the value be an empty line.\n" <<
		"# [CAMPUS] Values: New Brunswick, Newark, Camden.\n# Default: New Brunswick\n\n" <<
		"# [SEMESTER] Values: <SEASON> <YEAR>\n# Examples: Spring 2014, Summer 2013, Fall 2012, Winter 2011, etc...\n# Default: <EMPTY LINE> signifying to use current semester (determined automatically).\n\n" <<
		"# [COURSES] Values: <DEPARTMENT>:<COURSE>:<SECTION>\n# Examples: 198:111:03 or 640:250:01\n# Default: <EMPTY LINE> signifying not to spot any courses (can be set within program)." <<
		"#     NOTE* Have each course on a separate line with no empty lines inbetween.\n\n"
		"[CAMPUS]\nNew Brunswick\n\n[SEMESTER]\n\n[COURSES]\n\n";
	conf.close();
}

void printSpotting()
{
	cout << "Currently spotting:" << endl;
	for (ListDept::iterator dept = spot.begin(); dept != spot.end(); ++dept) {
		for (ListCourses::iterator course = (*dept).courses.begin(); course != (*dept).courses.end(); ++course) {
			for (ListSections::iterator section = (*course).sections.begin(); section != (*course).sections.end(); ++section) {
				cout << '[' << (*section).courseIndex << "] " << (*dept).deptCode << ":" << (*course).courseCode << ":" << (*section).section << (*course).course << endl;
			}
		}
	}
}

int main(int argc, char **argv)
{
	if (!init())
		return 1; //error printed from call

	//Read configuration file and apply all custom settings
	ifstream conf;
	conf.open(CONFFILE);
	if (!conf.is_open())
		createConfFile();

	string line;
	int linecount = 0;
	while (getline(conf, line)) {
		++linecount;
		if (line == "[COURSES]") {
			while (getline(conf, line)) {
				++linecount;
				if (line.length() == 0) //blank line, end course list
					break;
				boost::cmatch what; // what[1]=dept, what[2]=course, what[3]=section
				boost::regex re("^\\s*([0-9]{3})[\\s:]+([0-9]{3})[\\s:]+([0-9]){1,2}\\s*$");
				if (!boost::regex_match(line.c_str(), what, re)) {
					cerr << "ERROR: Configuration file is formatted incorrectly on line " << linecount << ": '" << line << "'." << endl;
					cerr << "\tExample Format: 198:111:01 or 198 111 01 where 198 is department, 111 is course, and 01 is section." << endl;
					cerr << "\tTo restore the default conf file, run " << argv[0] << " --create-conf" << endl;
					return 1;
				}
				string dept(what[1].first, what[1].second);
				string course(what[2].first, what[2].second);
				string section(what[3].first, what[3].second);
				if (section.length() == 1)
					section.insert(0, "0");
				if (!spotCourse(dept, course, section))
					cout << "ERROR: " <<dept<<':'<<course<<':'<<section<< " is invalid." << endl;
			}
		} else if (line == "[SEMESTER]") {
			getline(conf, line);
			++linecount;
			if (line.length() == 0) //blank line because setting is optional, ignore
				continue;
			if (!setSemester(line)) {
				cerr << "ERROR: Invalid semester provided in configuration file on line " << linecount << ": '" << line << "'." << endl;
				cerr << "\tExample Format: \"fall 2013\" or \"spring 2014\" (without quotes)." << endl;
				cerr << "\tLeaving the setting blank will use the most current/upcoming semester." << endl;
				cerr << "\tDue to error, using the most current semester instead: \"" << info.semesterString << "\"." << endl;
			}
		} else if (line == "[CAMPUS]") {
			getline(conf, line);
			++linecount;
			if (line.length() == 0) //blank line because setting is optional, ignore
				continue;
			if (!setCampus(line)) {
				cerr << "ERROR: Invalid campus provided in configuration file on line " << linecount << ": '" << line << "'." << endl;
				cerr << "\tAcceptable Format: \"new brunswick\", \"newark\", or \"camden\" (without quotes)." << endl;
				cerr << "\tLeaving the setting blank will use the default: \"New Brunswick\"." << endl;
				cerr << "\tDue to error, using the default: \"New Brunswick\"." << endl;
			}
		} else
			continue;
	}
	conf.close();

	cout << "RUopen Version 1.0" << endl << "Semester: " << info.semesterString << endl << "Campus: " << info.campusString << endl;
	printSpotting();

/*
	Json::Value *depts = getDepartments();
	if (depts == NULL) {
		cerr << "ERROR: Unable to retrieve subjects" << endl;
		return 1;
	}
	int jsonsize = depts->size();
	cout << "number of departments: " << jsonsize << endl;
	for (int i = 0; i < jsonsize; ++i) {
		string subj = (*depts)[i].get("description", "NULL").asString();
		string subjcode = (*depts)[i].get("code", "NULL").asString();
		cout << "Subject: (" << subjcode << ") " << subj << endl;
	}

	delete depts;
*/
/* JUST FOR REFERENCE NOW
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
	curl_slist_free_all(curl.headers.json);
	curl_easy_cleanup(curl.handle);
	curl_global_cleanup();
}

