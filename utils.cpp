string semesterCodeToString(string code)
{
	char c = code[0];
	switch(c) {
		case '1': return string("Spring ").append(code.substr(1,4));
		case '0': return string("Winter ").append(code.substr(1,4));
		case '9': return string("Fall ").append(code.substr(1,4));
		case '7': return string("Summer ").append(code.substr(1,4));
		default: 
				  cerr << "ERROR: Invalid semester" << endl;
				  exit(1);
	}
}

string semesterStringToCode(string str)
{
	list<string> tokens;
	boost::regex_split(std::back_inserter(tokens), str);
	if (tokens.size() != 2)
		return "NULL";
	str = *tokens.begin();
	tokens.pop_front();
	string year = *tokens.begin();
	if (str == "Spring") return string("1") + year;
	else if (str == "Winter") return string("0") + year;
	else if (str == "Fall") return string("9") + year;
	else if (str == "Summer") return string("7") + year;
	else return "NULL";
}

void debug()
{
	ofstream file;
	file.open("response.html");
	file << curl.response;
	file.close();
	cout << "Response code: " << curl.res << endl;
	cout << "Response length: " << curl.respLen << endl;
	cout << "Response header: \n" << curl.responseHeader << endl;
}
