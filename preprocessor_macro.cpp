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
#include <algorithm>
#include <sstream>

#include "argument_processor.hpp"

using namespace std;

constexpr inline int isdrecvbreak(int);
string mulstr(const string &, unsigned long long int);

unordered_map<string, string> defines;

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

	string line, fmtline, command;
	unsigned long long int curline = 1,
	                       curchar = 1;
	while(getline(input_file, line)) {
		fmtline = line;
		++curline;
		curchar = 1;
		while(isspace(fmtline.front())) {
			fmtline = fmtline.c_str() + 1;
			++curchar;
		}
		if(!fmtline.size())
			continue;
		if(fmtline[0] == '#') {
			fmtline = fmtline.c_str() + 1;
			++curchar;
			while(isspace(fmtline.back()))
				fmtline.pop_back();
			command.erase();
			for(const char & chr : fmtline) {
				++curchar;
				if(isdrecvbreak(chr))
					break;
				command.push_back(chr);
			}
			if(!command.size()) {
				if(fmtline.size()) {
					cerr << predata.input_filename << ':' << curline << ':' << curchar << ": error: invalid preprocessing directive " << fmtline << "\n " << line << "\n  ^\n";
					return 1;
				}
				continue;
			}
			if(command == "define") {
				string name, defined_value;
				auto name_to_define_itr = find_if_not(fmtline.begin() + 6, fmtline.end(), isdrecvbreak);
				if(name_to_define_itr == fmtline.end()) {
					cerr << predata.input_filename << ':' << curline << ':' << line.size() + 1 << ": error: no macro name given in #define directive\n " << line << '\n' << mulstr(" ", line.size() + 1) << '^';
					return 1;
				} else {
					auto name_to_define_end_itr = find_if(name_to_define_itr, fmtline.end(), isdrecvbreak);
					string name_to_define(name_to_define_itr, name_to_define_end_itr);
					auto value_to_define_itr = find_if_not(name_to_define_end_itr, fmtline.end(), isdrecvbreak);
					if(value_to_define_itr == fmtline.end()) {
						defines.emplace(string(name_to_define_itr, name_to_define_end_itr), "");
						continue;
					}
					defines.emplace(string(name_to_define_itr, name_to_define_end_itr), string(value_to_define_itr, find_if(value_to_define_itr, fmtline.end(), isdrecvbreak)));
					continue;
				}
			} else {
				cerr << predata.input_filename << ':' << curline << ':' << curchar << ": error: invalid preprocessing directive " << fmtline << "\n " << line << "\n  ^\n";
				return 1;
			}
			cout << command << '\n';
		}
	}
}


constexpr inline int isdrecvbreak(int ch) {
	return !(isalnum(ch) && !isdigit(ch));
}

string mulstr(const string & str, unsigned long long int amount) {
	ostringstream ostr;
	for(unsigned long long int i = 0; i < amount; ++i)
		ostr << str;
	return ostr.str();
}
