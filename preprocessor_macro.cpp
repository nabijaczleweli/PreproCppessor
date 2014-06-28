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
#include <algorithm>
#include <sstream>

#include "argument_processor.hpp"

using namespace std;

string operator*(string, unsigned long long int);
void print_line(const string &, ostream &);
int process_file(ostream &, istream &, const preprocessor_data &);

unordered_map<string, string> defines;

int main(int, const char * argv[]) {
	preprocessor_data predata;
	if(const int errc = process_args(argv, &predata, defines)) {
		const int retcode = process_args_error(errc, argv);
		if(retcode != -1)
			return retcode;
	}

	ifstream input_file(predata.input_filename);
	ofstream output_file(predata.output_filename);
	if(!input_file) {
		cerr << predata.program_name << ": error: " << predata.input_filename << ": No such file or directory\n";
		return 1;
	}

	if(const int errc = process_file(output_file, input_file, predata))
		return errc;

	for(const auto & pr : defines)
		clog << "Defined " << pr.first << (" for value " + pr.second) * static_cast<bool>(pr.second.size()) << ".\n";
}


int process_file(ostream & output_stream, istream & input_stream, const preprocessor_data & predata) {
	string line, fmtline, command;
	unsigned long long int curline = 1,
	                       curchar = 1;
	while(getline(input_stream, line)) {
		fmtline = line;
		++curline;
		curchar = 1;
		while(isspace(fmtline.front())) {
			fmtline = fmtline.c_str() + 1;
			++curchar;
		}
		if(!fmtline.size()) {
			print_line(line, output_stream);
			continue;
		}
		if(fmtline[0] == '#') {
			fmtline = fmtline.substr(1);
			++curchar;
			while(isspace(fmtline.back()))
				fmtline.pop_back();
			command.erase();
			for(const char & chr : fmtline) {
				++curchar;
				if(isspace(chr))
					break;
				command.push_back(chr);
			}
			if(!command.size()) {
				if(fmtline.size()) {
					cerr << predata.input_filename << ':' << curline << ':' << curchar << ": error: invalid preprocessing directive " << fmtline << "\n " << line << "\n  ^\n";
					return 1;
				}
				print_line(line, output_stream);
				continue;
			}
			if(command == "define") {
				auto name_to_define_itr = find_if(fmtline.begin() + 6, fmtline.end(), static_cast<int (*)(int)>(isalpha));
				if(name_to_define_itr == fmtline.end()) {
					cerr << predata.input_filename << ':' << curline << ':' << line.size() + 1 << ": error: no macro name given in #define directive\n"
					        " " << line << '\n' <<
					        string(" ") * line.size() << " ^";
					return 1;
				}
				auto name_to_define_end_itr = find_if_not(name_to_define_itr, fmtline.end(), static_cast<int (*)(int)>(isalpha));
				string name_to_define(name_to_define_itr, name_to_define_end_itr);
				if(defines.find(name_to_define) != defines.end()) {
					cerr << predata.input_filename << ':' << curline << ":0: warning: \"" << name_to_define << "\" redefined [enabled by default]\n "<< line << "\n ^\n";
					continue;
				}
				auto value_to_define_itr = find_if(name_to_define_end_itr, fmtline.end(), static_cast<int (*)(int)>(isalpha));
				if(value_to_define_itr == fmtline.end()) {
					defines.emplace(name_to_define, "");
					continue;
				}
				string value_to_define(value_to_define_itr, fmtline.end());
				while(isspace(value_to_define.back()))
					value_to_define.pop_back();
				defines.emplace(string(name_to_define_itr, name_to_define_end_itr), value_to_define);
				continue;
			} if(command == "include") {
				bool islocal;
				const char * islocal_itr = find_if_not(fmtline.c_str() + 7, fmtline.c_str() + fmtline.size(), static_cast<int (*)(int)>(isspace));
				if(islocal_itr == fmtline.c_str() + fmtline.size() || !(*islocal_itr == '\"' || *islocal_itr == '<')) {
					cerr << predata.input_filename << ':' << curline << ':' << line.find("include") + 7 << ": error: #include expects \"FILENAME\" or <FILENAME>\n"
					        " " << line << '\n' <<
					        string(" ") * (line.find("include") + 7) << " ^";
					return 1;
				}
				islocal = *islocal_itr == '\"';
				if(islocal_itr + 1 == (fmtline.c_str() + fmtline.size() - 1)) {
					cerr << predata.input_filename << ':' << curline << ':' << (islocal_itr - fmtline.c_str() + 2) << ": error: empty filename in #include\n"
					        " " << line << '\n' <<
					        string(" ") * (islocal_itr - fmtline.c_str()) << "  ^";
					return 1;
				}
				// TODO: end-bracket matching.
				const char * filename_itr = islocal_itr + 1;
				const char * filename_end_itr = filename_itr;
				while(*filename_end_itr++ != (islocal ? '\"' : '>'))
					++filename_end_itr;
				string filename(filename_itr, filename_end_itr);
				if(islocal) {
					preprocessor_data newpredata;
					newpredata.input_filename = filename;
					newpredata.output_filename = predata.output_filename;
					ifstream input_file(filename);
					if(!input_file) {
						cerr << predata.program_name << ": error: " << filename << ": No such file or directory\n";
						return 1;
					}
					if(const int errc = process_file(output_stream, input_file, newpredata))
						return errc;
				} else
					cout << "Non-local includes not yet supported!\n";
				continue;
			} else {
				cerr << predata.input_filename << ':' << curline << ':' << curchar << ": error: invalid preprocessing directive " << fmtline << "\n " << line << "\n  ^\n";
				return 1;
			}
		}
		print_line(line, output_stream);
	}

	return 0;
}


string operator*(string str, unsigned long long int times) {
	stringstream strstrm;
	for(unsigned long long int i = 0; i < times; ++i)
		strstrm << str;
	return strstrm.str();
}

void print_line(const string & _line, ostream & output) {
	if(!_line.size()) {
		output << '\n';
		return;
	}
	if(defines.size()) {
		string line = _line.c_str();
		for(const auto & pr : defines) {
			if(pr.first == pr.second)
				continue;
			size_t pos;
			while((pos = line.find(pr.first)) != string::npos)
				line.replace(pos, pr.first.size(), pr.second);  // TODO: make it isspace() aware!
		}
		output << line << '\n';
	} else
		output << _line << '\n';
}
