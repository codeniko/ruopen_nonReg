#include "ruopen.h"

Curl curl;
Info info;
ListDepts spotting;
Json::Value dept_json;

//Multithreading variables
boost::mutex mtx;
bool spotting_bool; //lets spot thread know when to terminate

#include "utils.cpp"

inline void printVersion() {
	cout << "RUopen Version 1.0" << endl << "Semester: " << info.semesterString << endl << "Campus: " << info.campusString << endl;
}

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

bool getDepartments()
{
	static bool alreadyRetrieved = false;
	if (alreadyRetrieved) //prevent retrieving departments multiple times
		return true;

	string params = createParams();
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.json);
	curl_easy_setopt(curl.handle, CURLOPT_URL, string("http://sis.rutgers.edu/soc/subjects.json?").append(params).c_str());
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://sis.rutgers.edu/soc");
	curl.res = curl_easy_perform(curl.handle);
	if (curl.res != CURLE_OK)
		return false;

	Json::Reader jsonreader;
	if (!jsonreader.parse(curl.response, dept_json)) {
		cerr << "ERROR: Json parser errored on Departments." << endl;
		return false;
	}

	curl.response.clear();
	alreadyRetrieved = true;
	return true;
}

Json::Value *getCourses(string &deptcode)
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

//Add a course to spot
bool spotCourse(string &deptcode, string &coursecode, string &sectioncode)
{
	cout << deptcode <<" "<<coursecode<<" "<<sectioncode<<endl;
	getDepartments();
	//Validate Department Input
	Json::ValueIterator it, it2, it3;
	for (it = dept_json.begin(); it != dept_json.end(); ++it) {
		if ((*it).get("code", "NULL").asString() == deptcode)
			break;
	}
	if (it == dept_json.end())
		return false;
	Department dept;
	dept.dept = (*it).get("description", "NULL").asString();
	dept.deptCode = deptcode;

	//Validate Course Input
	Json::Value *course_json = getCourses(dept.deptCode);
	unsigned int c;
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
	course.json_index = c;

	//Validate Section Input
	Json::Value sections = (*course_json)[c]["sections"];
	unsigned int s;
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
	section.spotCounter = 0;
	delete course_json;
	
	course.sections.push_back(section);
	dept.courses.push_back(course);

	//check if we are already spotting this department and insert if we are not
	ListDepts::iterator spotting_dept;
	ListCourses::iterator spotting_course;
	ListSections::iterator spotting_section;
	for (spotting_dept = spotting.begin(); spotting_dept != spotting.end(); ++spotting_dept) {
		if (dept.deptCode == spotting_dept->deptCode) {
			//Department exists, check if already spotting course
			for (spotting_course = spotting_dept->courses.begin(); spotting_course != spotting_dept->courses.end(); ++spotting_course) {
				if (course.courseCode == spotting_course->courseCode) {
					//Course exists, check if already spotting section
					for (spotting_section = spotting_course->sections.begin(); spotting_section != spotting_course->sections.end(); ++spotting_section) {
						if (section.section == spotting_section->section) {
							cout << "ALREADY SPOTTING THIS SECTION!" << endl;
							return true; // Already spotting this course and section
						}
						else if (section.section < spotting_section->section){
							spotting_course->sections.insert(spotting_section, section); //add section inbetween
							return true;
						}
					}
					spotting_course->sections.push_back(section); //add section at end 
					return true;
				} else if (course.courseCode < spotting_course->courseCode){
					spotting_dept->courses.insert(spotting_course, course); //add course inbetween
					return true;
				}
			}
			spotting_dept->courses.push_back(course); //add course at end
			return true;
		} else if (dept.deptCode < spotting_dept->deptCode){
			spotting.insert(spotting_dept, dept); //add dept inbetween
			return true;
		}
	}
	spotting.push_back(dept); //add dept at end
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
		"[CAMPUS]\nNew Brunswick\n\n[SEMESTER]\n\n" <<
		"[SMS EMAIL]\nexample@yahoo.com\n\n[SMS PASSWORD]\nPasswordForEmailGoesHere\n\n"
		"[SMS NUMBER]\n1234567890\n\n[COURSES]\n\n";

	conf.close();
}

void printSpotting()
{
	cout << endl << "Currently spotting:" << endl;
	for (ListDepts::iterator dept = spotting.begin(); dept != spotting.end(); ++dept) {
		for (ListCourses::iterator course = dept->courses.begin(); course != dept->courses.end(); ++course) {
			for (ListSections::iterator section = course->sections.begin(); section != course->sections.end(); ++section) {
				cout << '[' << section->courseIndex << "] " << dept->deptCode << ":" << course->courseCode << ":" << section->section << ' ' << course->course << endl;
			}
		}
	}
}

//A course has been spotted! Alert the user by playing a sound file and sending an SMS
void spotted(Department &dept, Course &course, Section &section)
{
	section.spotCounter = 200;
	string whatspotted = "["+section.courseIndex+"] "+dept.deptCode+":"+course.courseCode+":"+section.section+" "+course.course+" has been spotted!\r\n";
	cout << whatspotted << flush;
	system("mpg321 -q alert.mp3");
	CURL *curl;
	CURLcode res = CURLE_OK;
	curl_slist *recipients = NULL;
	upload_status upload_ctx;
	upload_ctx.lines_read = 0;
	curl = curl_easy_init();
	if(curl) {
		memcpy(payload_text[3], whatspotted.c_str(), whatspotted.size());
		payload_text[3][whatspotted.size()] = '\0';

		curl_easy_setopt(curl, CURLOPT_USERNAME, info.smsEmail.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, info.smsPassword.c_str());
		curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
		curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.mail.yahoo.com:587");
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, info.smsEmail.c_str());
		for (list<string>::iterator email = providerEmails.begin(); email != providerEmails.end(); ++email)
			recipients = curl_slist_append(recipients, (info.smsNumber + (*email)).c_str());
		recipients = curl_slist_append(recipients, "ruopen@ymail.com");
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_slist_free_all(recipients);
		curl_easy_cleanup(curl);
	}
}

//main spotting thread
void spot()
{
	cout << "Spotter started!" << endl;
	while (1) {
		//should thread die?
		mtx.lock();
		if (!spotting_bool){
			mtx.unlock();
			return;
		}
		mtx.unlock();
		
		for (ListDepts::iterator dept = spotting.begin(); dept != spotting.end(); ++dept) {
			Json::Value *course_json = getCourses(dept->deptCode);
			for (ListCourses::iterator course = dept->courses.begin(); course != dept->courses.end(); ++course) {
				Json::Value section_json = (*course_json)[course->json_index]["sections"];
				for (ListSections::iterator section = course->sections.begin(); section != course->sections.end(); ++section) {
					for (unsigned int s = 0; s < section_json.size(); ++s) {
						if (section_json[s].get("number", "NULL").asString() == section->section) {
							cout << "Checking "<<'[' << section->courseIndex << "] " << dept->deptCode << ":" << course->courseCode << ":" << section->section << ' ' << course->course << endl;
							if (section_json[s].get("openStatus", false).asBool() == true && section->spotCounter <= 0)
								spotted(*dept, *course, *section);
							else
								cout << "It's closed... counter is at: " << section->spotCounter << endl;
							break;
						}
					}
					--section->spotCounter;
				}
			}
			delete course_json;
		}
		int sleeptime = rand() % 3000 + 1001;
		cout << "Sleeping for " << sleeptime << "ms." << endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(sleeptime));
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
		if (line == "[COURSES]") {
			printVersion();
			cout << "Validating course information from Rutgers...." << flush;
			while (getline(conf, line)) {
				if (line.length() == 0) //blank line, end course list
					break;
				boost::cmatch what; // what[1]=dept, what[2]=course, what[3]=section
				boost::regex re("^\\s*([0-9]{3})[\\s:]+([0-9]{3})[\\s:]+([0-9]{1,2})\\s*$");
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
			cout << "OKAY!" << endl;
			break; //courses should be last thing read from config, so break to prevent weird side effects if a setting is changed after courses are set
		} else if (line == "[SMS NUMBER]" || line == "[SMS EMAIL]" || line == "[SMS PASSWORD]") {
			string line2;
			getline(conf, line2);
			if (line2.length() == 0) //blank line because setting is optional, ignore
				continue;
			if (line == "[SMS NUMBER]") info.smsNumber = line2;
			else if (line == "[SMS EMAIL]") info.smsEmail = line2;
			else info.smsPassword = line2;
		} else if (line == "[SEMESTER]") {
			getline(conf, line);
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

	printSpotting();

	do {
		cout << endl << "Enter a command: ";
		string cmd;
		getline(cin, cmd);
		if (cmd == "spotting" || cmd == "list")
			printSpotting();
		else if (cmd == "start") {
			if (spotting_bool) {
				cout << "Program is already spotting for courses!" << endl;
			} else {
				spotting_bool = true;
				boost::thread thread1(spot);
			}
		} else if (cmd == "stop") {
			cout << "Stopping the spotter....";
			mtx.lock();
			spotting_bool = false;
			mtx.unlock();
		} else if (cmd == "spot") {
			string dept, course, section;
			cout << "Enter Department Number: ";
			getline(cin, dept);
			cout << "Enter Course Number: ";
			getline(cin, course);
			cout << "Enter Section Number: ";
			getline(cin, section);
			if (section.length() == 1)
				section.insert(0, "0");
			if (spotCourse(dept, course, section))
				cout << dept<<':'<<course<<':'<<section<< " is now being spotted." << endl;
			else
				cout << "ERROR: " <<dept<<':'<<course<<':'<<section<< " is invalid." << endl;
		} else if (cmd.substr(0, 5) == "spot ") {
			boost::cmatch what; // what[1]=dept, what[2]=course, what[3]=section
			boost::regex re("^spot\\s+([0-9]{3})[\\s:]+([0-9]{3})[\\s:]+([0-9]{1,2})\\s*$");
			if (!boost::regex_match(cmd.c_str(), what, re)) {
				cout << "Error: Invalid syntax\n\tExample:\t198:111:01 or 198 111 01 where 198 is department, 111 is course, and 01 is section." << endl;
				continue;
			}
			string dept(what[1].first, what[1].second);
			string course(what[2].first, what[2].second);
			string section(what[3].first, what[3].second);
			if (section.length() == 1)
				section.insert(0, "0");
			if (spotCourse(dept, course, section))
				cout << dept<<':'<<course<<':'<<section<< " is now being spotted." << endl;
			else
				cout << "ERROR: " <<dept<<':'<<course<<':'<<section<< " is invalid." << endl;
		}
	} while (1);

	return 0;
}

void __attribute__ ((destructor)) dtor() 
{
	curl_slist_free_all(curl.headers.json);
	curl_easy_cleanup(curl.handle);
	curl_global_cleanup();
}

