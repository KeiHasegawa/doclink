#include "doclink.h"

namespace doclink {
  using namespace std;
  namespace error {
    int counter;
    void unknown_option(string opt)
    {
      cerr << cmdline::prog << ' ';
      cerr << "error unknown option : " << opt << '\n';
      ++counter;
    }
    void require_argument(string opt)
    {
      cerr << cmdline::prog << ' ';
      cerr << "error : " << opt << " option requires argument." << '\n';
      ++counter;
    }
    void undef(string kind, string name)
    {
      cerr << cmdline::prog << " error" << '\n';
      cerr << curr_file << ':' << lineno << ':' << '\n';
      cerr << "undefined " << kind << " name `" << name << "'" << '\n';
      ++counter;
    }
    void multi_def(string kind, string name, const common_t& prev)
    {
      cerr << cmdline::prog << " error" << '\n';
      cerr << curr_file << ':' << lineno << ':' << '\n';
      cerr << "multiple defined " << kind << " name `" << name << "'" << '\n';
      const file_t& def = prev.m_def;
      cerr << "previous defined " << def.first << ':' << def.second << ' ';
      const file_t& input = prev.m_input;      
      cerr << "at " << input.first << ':' << input.second << '\n';
      ++counter;
    }
    void cannot_open(string fn, const file_t& file)
    {
      cerr << cmdline::prog << ' ';
      cerr << "error " << "cannot open `" << fn << "'." << '\n';
      if (file.second) {
        cerr << '`' << fn << "' is refered from ";
        cerr << file.first << ':' << file.second << '\n';
      }
      ++counter;
    }
  } // end of namespace error
} // end of namespace doclink

