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

#include "argument_processor.hpp"
#include "preprocessor_data.hpp"

using namespace std;

int process_args(const char * argv[], preprocessor_data * predata, unordered_map<string, string> & defines) {
	if(!argv[1])
		return 1;
	predata->program_name = *argv;
	for(unsigned int idx = 1; argv[idx]; ++idx) {
		const char * arg = argv[idx];
		const unsigned int arglen = strlen(arg);
		if(*arg == '-' && arg[1]) {
			if(arg[1] == 'o' && !arg[2]) {
				if(!(arg = argv[++idx]))
					return 2;
				predata->output_filename = arg;
			} else if(arg[1] == 'o' && arg[2])
				predata->output_filename = arg + 2;
			else if(arglen >= 6 && !memcmp(arg, "--help", 6)) {
				if(!arg[6])
					return 3;
			} else if(!memcmp(arg, "-D", 2)) {
				if(!arg[2])
					continue;
				string name_to_define, value_to_define;
				const char * eq_sign_pos = strchr(arg + 2, '=');
				if(!eq_sign_pos)
					name_to_define = string(arg + 2, arglen - 2);
				else {
					name_to_define = string(arg + 2, eq_sign_pos);
					value_to_define = string(eq_sign_pos + 1);
				}
				if(defines.find(name_to_define) != defines.end())
					cerr << "<command-line>:0:0: warning: " << name_to_define << " redefined [enabled by default]\n";
				defines.emplace(name_to_define, value_to_define);
			} else if(arglen > 11 && !memcmp(arg, "--sysroots=", 11)) {
				if(!strchr(arg + 11, ';')) {
					predata->system_directories.emplace_front(arg + 11, arg + arglen);
					continue;
				}
				const char * semi_pointer = arg + 11;
				try {
					while(semi_pointer && semi_pointer < (arg + arglen)) {
						const char * new_pos = strchr(semi_pointer, ';');
						string(semi_pointer, new_pos); //  Without this everything crumbles if no ';' at the end
						if(new_pos)
							predata->system_directories.emplace_front(semi_pointer, new_pos);
						semi_pointer = new_pos + 1;
					}
				} catch(length_error le) {
					cerr << "<command-line>:0:0: info: please end your sysroots with \';\' [enabled by default]\n";
					predata->system_directories.emplace_front(semi_pointer, arg + arglen);
				}
			}
		} else
			predata->input_filename = arg;
	}

	if(!predata->input_filename.size())
		return 1;
	if(!predata->output_filename.size())
		predata->output_filename = predata->input_filename + ".ppc";

	return 0;
}

int process_args_error(int errc, const char * argv[]) {
	switch(errc) {
		case 1 :
			cerr << *argv << ": fatal error: no input files\n"
			        "preprocessing terminated.\n";
			return 1;
			break;
		case 2 :
			cerr << *argv << ": error: missing filename after \'-o\'\n";
			return 1;
			break;
		case 3 :
			cout << "Usage: " << *argv << " [options] file\n"
			        "Options:\n"
			        "  --help                   Display this information\n"
			        "  -o <file>                Place the output into <file>\n"
			        "  -D<name>[=[value]]       Define <name> to be [value]. (default: empty)\n"
			        "\n"
			        "For bug reporting, please contact:\n"
			        "<nabijaczleweli@gmail.com>.\n";
			return 0;
			break;
	}
	return -1;
}
