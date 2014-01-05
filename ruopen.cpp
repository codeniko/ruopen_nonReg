#include "ruopen.h"

using namespace std;

static Curl curl;
static Info info;

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

//RETURN: Will return a pointer to a string containing the JSON of subjects or NULL if error
string *getSubjects()
{
	stringstream ss;
	ss << "semester=" << info.semester << "&campus=" << info.campus << "&level=U,G";
	string params = ss.str();
	cout << params << endl;
	curl_easy_setopt(curl.handle, CURLOPT_HTTPHEADER, curl.headers.json);
	curl_easy_setopt(curl.handle, CURLOPT_URL, string("http://sis.rutgers.edu/soc/subjects.json?").append(params).c_str());
	curl_easy_setopt(curl.handle, CURLOPT_REFERER, "http://sis.rutgers.edu/soc");
	curl.res = curl_easy_perform(curl.handle);

	if (curl.res != CURLE_OK)
		return NULL;
	else
		return &curl.response;

	/* //Print subject results to standard output
	Json::Value root;
	Json::Reader jsonreader;
	if (!jsonreader.parse(response, root)) {
		cout << "Json parser errored on subjects.";
		exit(1);
	}
	int jsonsize = root.size();
	for (int i = 0; i < jsonsize; ++i) {
		string subj = root[i].get("description", "NULL").asString();
		string subjcode = root[i].get("code", "NULL").asString();
		cout << "Subject: (" << subjcode << ") " << subj << endl;
	}
	*/
}

int main(int argc, char **argv)
{
	init();

	info.semester = 12014;
	info.campus = "NB";


	string *subjects = getSubjects();
	if (subjects == NULL) {
		cerr << "ERROR: Unable to retrieve subjects" << endl;
	}
	cout << "Response code: " << curl.res << endl;
	cout << "Response length: " << curl.respLen << endl;
	cout << "Response header: \n" << curl.responseHeader << endl;

	Json::Value root;
	Json::Reader jsonreader;
	if (!jsonreader.parse(*subjects, root)) {
		cerr << "ERROR: Json parser errored on subjects." << endl;
		return 1;
	}
	int jsonsize = root.size();
	for (int i = 0; i < jsonsize; ++i) {
		string subj = root[i].get("description", "NULL").asString();
		string subjcode = root[i].get("code", "NULL").asString();
		cout << "Subject: (" << subjcode << ") " << subj << endl;
	}

	return 0;
}

void __attribute__ ((destructor)) dtor() 
{
	curl_easy_cleanup(curl.handle);
	curl_slist_free_all(curl.headers.json);
	curl_global_cleanup();
}

void debug()
{
	ofstream file;
	file.open("response.html");
	file << curl.response;
	file.close();
}
