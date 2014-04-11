// configure from command
// an option start by - or --
// -<opt_name> [opt_parameter or opt_parameter_list]
// --<opt_name>=<opt_value>
// --cwd=<new_working_directory>
// --config=<config_path>
//
// configure from file
// line comment (#)
// global section (before any named section)
// named section ([<section-name>])
// include instruction: include(<path>)
// entry = value
//

#include "appcfg.h"
#include "applog.h"

//#include <direct.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <map>
#include <string>
#include <vector>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#else
#define _getcwd getcwd
#define _chdir chdir
#endif

namespace appcfg
{
    static int silent = 1;
	typedef std::map<std::string, std::string> CnfSect;
	typedef std::map<std::string, CnfSect> CnfTab;
	CnfTab __cfg_tab;
	CnfTab *__p_cfg_tab = &__cfg_tab;
	static strlist __loaded_cfg_files;
	static std::string __cmd_str = "<cmd>";
	static std::string __global_str = "<global>";

	void* get_handle() {return __p_cfg_tab;}
	void set_handle(void *handle) {__p_cfg_tab = (CnfTab *)handle;}

	static int _insert_entry(const std::string &section, const std::string &key, const std::string &value)
	{
		int _new = __cfg_tab[section].find(key) == __cfg_tab[section].end();
		if (_new)
		{
            if (!silent)
    			sysclr(YELLOW) << "[insert] " << section << ":" << key << " = " << value << "\n"; sysclr(WHITE);
		}
		else
		{
            if (!silent)
                sysclr(RED) << "[update] " << section << ":" << key << " = " << value << "\n"; sysclr(WHITE);
		}
		__cfg_tab[section][key] = value;
		return _new;
	}

	static std::string _string_strip(const std::string &str)
	{
		const char *p1, *p2;
		p1 = str.c_str();
		p2 = p1 + str.size() - 1;
		while (isspace(*p1)) p1++;
		while (p2 >= p1 && isspace(*p2)) p2--;
		return std::string(p1, p2 - p1 + 1);
	}

	static bool _strlist_find(const strlist &strs, const std::string &str)
	{
		for (int i = 0; i < (int)strs.size(); i++)
		{
			if (strs[i] == str)
				return true;
		}
		return false;
	}

	static std::string _strlist_merge(const strlist &strs, char splitter = ';')
	{
		std::string str;
		int len = (int)strs.size();
		if (len > 0)
			str = strs[0];
		for (int i = 1; i < len; i++)
			str = str + splitter + strs[i];
		return str;
	}

	strlist _string_split(const std::string &str)
	{
		strlist strs;
		const char *p1, *p2;
		p1 = str.c_str();
		p2 = p1;
		while (1)
		{
			if (*p1 == ';' || *p1 == '\0')
			{
				if (p1 > p2)
				{
					std::string val = std::string(p2, p1 - p2);
					val = _string_strip(val);
					if (!val.empty())
						strs.push_back(val);
				}
				if (*p1 == '\0')
					break;
				p2 = p1 + 1;
			}
			p1++;
		}
		return strs;
	}

	static int _load_cfg_file(const std::string &path, bool force = false);

	int load_conf(const std::string &path)
	{
		return _load_cfg_file(path, true);
	}

	bool _is_qualified_identifier(std::string &name)
	{
		// [_a-zA-Z][_a-zA-Z0-9]*
		if (name.empty() || !(name[0] == '_' || name[0] == '@' || name[0] == '$' ||
			name[0] >= 'a' && name[0] <= 'z' ||
			name[0] >= 'A' && name[0] <= 'Z'))
			return false;
		for (int i = 1; i < (int)name.size(); i++)
		{
			if (!(name[0] == '_' || name[0] == '@' || name[0] == '$' ||
				name[0] >= '0' && name[0] <= '9' ||
				name[0] >= 'a' && name[0] <= 'z' ||
				name[0] >= 'A' && name[0] <= 'Z'))
				return false;
		}
		return true;
	}

	static int _parse_line(std::string &line, std::string &cursection)
	{
		std::string::size_type p1 = line.find_first_of('#');
		if (p1 != std::string::npos)
			line = line.substr(0, p1);
		line = _string_strip(line);
		// <path>
		// [section]
		// a = b
		std::string::size_type len = line.size();
		if (line.size() == 0)
			return 0;
		if (line[0] == '<' && line[len - 1] == '>')
		{
			std::string path(line, 1, len - 2);
			path = _string_strip(path);
			int num = _load_cfg_file(path);
			return num >= 0 ? num : 0;
		}
		if (line[0] == '[' && line[len - 1] == ']')
		{
			std::string section(line, 1, len - 2);
			section = _string_strip(section);
			if (_is_qualified_identifier(section))
			{
				if (__p_cfg_tab->find(section) == __p_cfg_tab->end())
					(*__p_cfg_tab)[section] = CnfSect();
				cursection = section;
				return 1;
			}
			return 0;
		}

		p1 = line.find_first_of('=');
		if (p1 != std::string::npos)
		{
			std::string key(line, 0, p1);
			std::string value(line, p1 + 1);
			key = _string_strip(key);
			value = _string_strip(value);
			len = value.size();
			if (len >= 2 &&
				(value[0] == '\"' && value[len - 1] == '\"' ||
				value[0] == '\'' && value[len - 1] == '\''))
			{
				value = value.substr(1, len - 2);
			}
			if (_is_qualified_identifier(key))
			{
				_insert_entry(cursection, key, value);
				return 1;
			}
		}
		return 0;
	}

	static int _load_cfg_file(const std::string &path, bool force)
	{
		if (!force && _strlist_find(__loaded_cfg_files, path))
			return 0;

		__loaded_cfg_files.push_back(path);

        std::vector<std::string> lines;
        {
		    std::ifstream ifile(path.c_str());
		    if (!ifile.is_open())
			    return -1;
            while (!ifile.eof())
            {
                std::string line,text;
                std::getline(ifile, line);
                if (line.empty() || line[0] == '#')
                    continue;
                lines.push_back(line);
            }
        }
		int total = 0;
		std::string cursection = __global_str;
		for (int i = 0; i < (int)lines.size();)
		{
			std::string line = lines[i++],text;
			while (true)
			{
				if (line[line.size() - 1] == '\\')
				{
					text += line.substr(0, line.size() - 1);
					line = lines[i++];
				}
				else
				{
					text += line;
					break;
				}
			}
			if (!text.empty())
            {
				total += _parse_line(text, cursection);
            }

		}

		return total;
	}

	int parse_args(int argc, char *argv[])
	{
		// get program path
		// get current working directory
		__cfg_tab[__global_str] = CnfSect();
		__cfg_tab[__cmd_str] = CnfSect();

		_insert_entry(__cmd_str, "app", argv[0]);

		char _cwd[1024] = "";
		_getcwd(_cwd, 1024);
		std::string cwd(_cwd);

		_insert_entry(__cmd_str, "cwd", cwd);

		strlist cfgfiles;

		for (int i = 1; i < argc; i++)
		{
			if (strncmp(argv[i], "--cwd=", 6) == 0)
			{
				cwd = argv[i] + 6;
				cwd = _string_strip(cwd);
			}
			else if (strncmp(argv[i], "--cfg=", 6) == 0)
			{
				std::string cfg = argv[i] + 6;
				cfg = _string_strip(cfg);
				cfgfiles.push_back(cfg);
			}
			else
			{
				int err = 0;
				if (argv[i][0] != '-')
					err = 1;
				else
				{
					if (argv[i][1] == '-')
					{
						if (argv[i][2] == '-')
							err = 1;
						else
						{
							char *p = strchr(argv[i] + 2, '=');
							if (!p || p == argv[i] + 2)
								err = 1;
							else
							{
								std::string opt(argv[i] + 2, p - (argv[i] + 2));
								std::string val(p + 1);
								opt = _string_strip(opt);
								opt = _string_strip(val);
								_insert_entry(__cmd_str, opt, val);
							}
						}
					}
					else
					{
						std::string opt(argv[i]);
						strlist vlist;
						for (i++; i < argc; i++)
						{
							if (argv[i][0] == '-')
							{
								i--;
								break;
							}
							std::string arg(argv[i]);
							arg = _string_strip(arg);
							if (!arg.empty())
								vlist.push_back(arg);
						}
						_insert_entry(__cmd_str, opt, _strlist_merge(vlist));
					}
				}
				if (err)
				{
                    if (!silent)
                        sysclr(RED) << "wrong option: " << applog::BLUE << argv[i] << "\n"; sysclr(WHITE);
				}
			}
		}

		// try to change working directory
		if (_chdir(cwd.c_str()) == 0)
		{
            if (!silent)
                sysclr(GREEN) << "change current working directory to:\n\t<" << cwd << ">\n"; sysclr(WHITE);
			_insert_entry(__cmd_str, "cwd", cwd);
		}
		else
		{
			std::string msg;
			if (errno == ENOENT)
				msg = "[invalid dir]";
            if (!silent)
                sysclr(RED) << "chdir to <" << cwd << "> failed " << msg << "\n"; sysclr(WHITE);
		}

		if (cfgfiles.empty())
			cfgfiles.push_back("default.ini");
		// recursively load configure file
		for (int i = 0; i < (int)cfgfiles.size(); i++)
			_load_cfg_file(cfgfiles[i]);

		_insert_entry(__cmd_str, "cfg", _strlist_merge(cfgfiles));
		_insert_entry(__cmd_str, "__cfg_loaded__", _strlist_merge(__loaded_cfg_files));

		return 0;
	}

	void dump_conf(FILE *fp /* = stdout */)
	{
		syslog.term << "dump configurations:\n\n";
		CnfTab::const_iterator iter = __p_cfg_tab->begin();
		while (iter != __p_cfg_tab->end())
		{
			syslog.term << "[" << iter->first << "]\n";
			const CnfSect &sect = iter->second;
			CnfSect::const_iterator it = sect.begin();
			while (it != sect.end())
			{
				syslog.term << it->first << " = " << it->second << "\n";
				it++;
			}
			iter++;
			syslog.term << "\n";
		}
	}

	strlist read_keys(const std::string &section)
	{
		strlist strs;
		CnfTab::const_iterator iter = __p_cfg_tab->find(section);
		if (iter != __p_cfg_tab->end())
		{
			const CnfSect &sect = iter->second;
			CnfSect::const_iterator it = sect.begin();
			while (it != sect.end())
			{
				strs.push_back(it->first);
				it++;
			}
		}
		return strs;
	}

	bool exist(const std::string &section, const std::string &key, std::string &value /* = std::string() */ )
	{
		if (section == "")
		{
			// find in <global> and <cmd>
			return (exist("<cmd>", key, value) || exist("<global>", key, value));
		}
		else
		{
			CnfTab::const_iterator iter = __p_cfg_tab->find(section);
			if (iter != __p_cfg_tab->end())
			{
				CnfSect::const_iterator it = iter->second.find(key);
				if (it != iter->second.end())
				{
					value = it->second;
					return true;
				}
			}
		}
		return false;
	}

	std::string read_string(const std::string &section, const std::string &key, const std::string &default_val /* = */ )
	{
		std::string value;
		if (exist(section, key, value))
			return value;
		return default_val;
	}

	bool read_bool(const std::string &section, const std::string &key, const bool default_val /* = false */)
	{
		std::string value;
		if (exist(section, key, value))
		{
			if (value == "1" || value == "true" || value == "on" || value == "TRUE" || value == "ON")
				return true;
			if (value == "0" || value == "false" || value == "off" || value == "FALSE" || value == "OFF")
				return false;
		}
		return default_val;
	}

	int read_int(const std::string &section, const std::string &key, const int default_val /* = 0 */)
	{
		std::string value;
		if (exist(section, key, value))
			return atoi(value.c_str());
		return default_val;
	}

	int read_binary(const std::string &section, const std::string &key, const int default_val /* = 0 */)
	{
		std::string value;
		if (exist(section, key, value))
		{
			int mask = 0;
			int cc = 0;
			for (int i = (int)value.size() - 1; i >= 0; i--)
			{
				if (value[i] == '0') cc++;
				else if (value[i] == '1')
				{
					mask += (1 << cc);
					cc++;
				}
			}
			return mask;
		}
		return default_val;
	}

	double read_double(const std::string &section, const std::string &key, const double default_val /* = 0.0 */)
	{
		std::string value;
		if (exist(section, key, value))
			return atof(value.c_str());
		return default_val;
	}

    float read_float(const std::string &section, const std::string &key, const float default_val /* = 0.0 */)
    {
        std::string value;
        if (exist(section, key, value))
            return (float)atof(value.c_str());
        return default_val;
    }

    strlist read_strlist(const std::string &section, const std::string &key, const strlist &default_val /* = strlist() */)
	{
		std::string value;
		if (exist(section, key, value))
			return  _string_split(value);
		return default_val;
	}

#define IS_DIGIT(c) (c >= '0' && c <= '9' || c == '.' || c == '+' || c == '-')

	int read_format(const std::string &section, const std::string &key, const char *format, ...)
	{
		int readnum = 0;
		std::string value;
		if (exist(section, key, value))
		{
			const char *s = value.c_str();

			va_list args;
			va_start(args, format);
			const char *p = format;
			char *b = (char *)s;
			while (*p) {
				while (*p && *p != '%') p++;
				if (*p == '\0' || (p[1] == 's' && p[1] != 'd' && p[1] != 'f' && (p[1] != 'l' || p[2] != 'f'))) {
					break;
				}
				bool isdbl;
				if (p[1] == 'l') {
					p += 2;
					isdbl = true;
				}
				else {
					p++;
					isdbl = false;
				}
				void *addr = va_arg(args, void *);
				char *start, *end;

				bool ok = true;
				if (*p != 's') {
					while (*b && !IS_DIGIT(*b)) b++;
					if (*b == '\0') {
						break;
					}
					start = b;
					while (IS_DIGIT(*b)) b++;
					end = b - 1;
					int sign = 1;
					if (*start == '-') {
						sign = -1;
						start++;
					}
					if (*p == 'd') {
						int value = 0;
						char *d = start;
						while (d <= end) {
							if (*d == '.' || *d == '+' || *d == '-') {
								ok = false;
								break;
							}
							value = value * 10 + (*d - '0');
							d++;
						}
						if (!ok) {
							break;
						}
						value *= sign;
						(*(int *)addr) = value;
					}
					else if (*p == 'f') {
						char *pos = start;
						while (pos <= end) {
							if (*pos == '.') {
								break;
							}
							pos++;
						}
						int real = 0;
						if (*pos != '.') {
							pos = end + 1;
						}
						char *d = start;
						while (d < pos) {
							if (*d == '.' || *d == '+' || *d == '-') {
								ok = false;
								break;
							}
							real = real * 10 + (*d - '0');
							d++;
						}
						if (!ok) {
							break;
						}
						double image = 0.0;
						if (*pos == '.') {
							start = pos + 1;
							d = end;
							while (d >= start) {
								if (*d == '.' || *d == '+' || *d == '-') {
									ok = false;
									break;
								}
								image = (image + (*d - '0')) * 0.1;
								d--;
							}
							if (!ok) {
								break;
							}
						}
						double value = real * 1.0 + image;
						value *= sign;
						if (isdbl) {
							(*(double *)addr) = value;
						}
						else {
							(*(float *)addr) = (float)value;
						}
					}
				}
				else {
					while (*b && isspace(*b)) b++;
					if (*b == '\0') {
						break;
					}
					start = b;
					while (!isspace(*b)) b++;
					end = b;
					char *dst = (char *)addr;
					memcpy(dst, start, end - start);
					dst[end - start] = '\0';
				}
				if (ok) {
					readnum++;
				}
				else {
					break;
				}
				p++;
			}
			va_end(args);

		}
		return readnum;
	}

	strlist read_files(const std::string &path, int type)
	{
		strlist strs;
		std::string dir;
		int pos = -1;
		for (int i = 0; i < (int)path.size(); i++)
			if (path[i] == '\\' || path[i] == '/')
				pos = i;
		if (pos > 0)
		{
			dir = path.substr(0, pos + 1);
		}
#ifdef _WIN32
		WIN32_FIND_DATA wfd;
		HANDLE handle = FindFirstFile(path.c_str(), &wfd);
		BOOL res = (handle != INVALID_HANDLE_VALUE);
		while (res)
		{
			if (type == 0)
			{
				if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE))
				{
					strs.push_back(dir + wfd.cFileName);
				}
			}
			else if (type == 1)
			{
				strs.push_back(wfd.cFileName);
			}
			else if (type == 2)
			{
				if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					std::string(".") != wfd.cFileName &&
					std::string("..") != wfd.cFileName)
				{
					strs.push_back(dir + wfd.cFileName);
				}
			}
			else if (type == 3)
			{
				if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					std::string(".") != wfd.cFileName &&
					std::string("..") != wfd.cFileName)
				{
					strs.push_back(wfd.cFileName);
				}
			}
			res = FindNextFile(handle, &wfd);
		}
#endif
		return strs;
	}
}
