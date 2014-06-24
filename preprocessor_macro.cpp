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
		if(line[0] == '#') {
			if(!line.size())
				continue;
			while(isspace(line.front()))
				line = line.c_str() + 1;
			while(isspace(line.back()))
				line.pop_back();
			if(!line.size())
				continue;
			process_preprocessing_directive(line);
		}
	}
}

int process_preprocessing_directive(const string & pre_drecv) {
	unordered_map<string, void (*)(const string &)> commands;
	const char * endptr = pre_drecv.c_str();
	while(*endptr && !isspace(*endptr))
		++endptr;
	cout << '\"' << string(pre_drecv.c_str(), endptr) << "\"\n";
	//commands.find()

	return 0;
}
