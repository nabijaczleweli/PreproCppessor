// The MIT License (MIT)

// Copyright (c) 2014 nabijaczleweli

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <fstream>
#include <cstring>
#include <unordered_map>

#include "argument_processor.hpp"

using namespace std;

int process_preprocessing_directive(const string &);
constexpr inline int isdrecvbreak(int);


int main(int, const char * argv[]) {
	preprocessor_data predata;
	if(const int errc = process_args(argv, &predata)) {
		const int retcode = process_args_error(errc, argv);
		if(retcode != -1)
			return retcode;
	}

	ifstream input_file(predata.input_filename);
	if(!input_file) {
		cerr << *argv << ": error: " << predata.input_filename << ": No such file or directory\n";
		return 1;
	}

	string line;
	while(getline(input_file, line)) {
		while(isspace(line.front()))
			line = line.c_str() + 1;
		if(!line.size())
			continue;
		if(line[0] == '#') {
			line = line.c_str() + 1;
			while(isspace(line.back()))
				line.pop_back();
			for(unsigned int i = 0; i < line.size(); ++i)
				if(isspace(line[i]) && isspace(line[i + 1])) {
					line = string(line.c_str(), i) + string(line.c_str() + i + 1);
					--i;
				}
			if(!line.size())
				continue;
			process_preprocessing_directive(line);
		}
	}
}


int process_preprocessing_directive(const string & pre_drecv) {
	static unordered_map<string, pair<int (*)(const string &), void (*)(const int)>> commands =
		{
			{
				"include",
				{
					[](const string &) {
						return 0;
					},
					[](const int) {}
				}
			}, {
				"define",
				{
					[](const string &) {return 0;},
					[](const int) {}
				}
			}
		};

	const char * endptr = pre_drecv.c_str();
	while(*endptr && !isdrecvbreak(*endptr))
		++endptr;
	if(endptr == pre_drecv.c_str())
		return 0;
	cout << '\"' << string(pre_drecv.c_str(), endptr) << "\"";
	auto itr = commands.find(string(pre_drecv.c_str(), endptr));
	if(itr == commands.end())
		cerr << "VALUE NOT FOUND!\n";
	else
		cerr << "MEH FUND!\n";

	return 1;
}

constexpr inline int isdrecvbreak(int ch) {
	return !((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}
