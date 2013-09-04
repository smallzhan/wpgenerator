#ifndef __appcfg_h_
#define __appcfg_h_

#include <string>
#include <vector>
#include <cstdio>

typedef std::vector<std::string> strlist;

namespace appcfg
{
	int parse_args(int argc, char *argv[]);
	int load_conf(const std::string &path);
	void dump_conf(FILE *fp = stdout);
	void* get_handle();
	void set_handle(void *handle);
    bool exist(const std::string &section, const std::string &key, const std::string &value = std::string(""));
	bool read_bool(const std::string &section, const std::string &key, const bool default_val = false);
	int read_int(const std::string &section, const std::string &key, const int default_val = 0);
	double read_double(const std::string &section, const std::string &key, const double default_val = 0.0);
    float read_float(const std::string &section, const std::string &key, const float default_val = 0.0);
	std::string read_string(const std::string &section, const std::string &key, const std::string &default_val = "");
	int read_binary(const std::string &section, const std::string &key, const int default_val = 0);
	strlist read_strlist(const std::string &section, const std::string &key, const strlist &default_val = strlist());
	int read_format(const std::string &section, const std::string &key, const char *format, ...);
	strlist read_keys(const std::string &section);
	strlist read_files(const std::string &path, int type = 0);

    strlist _string_split(const std::string &str);

};

#endif  //__appcfg_h_
