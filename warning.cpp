#include "doclink.h"

namespace doclink {
  namespace warning {
    void not_found(string fn, int line, const file_t& file,
                   string kind, string name, string text)
    {
      cerr << cmdline::prog << ' ';
      cerr << "warning " << file.first << ':' << file.second << ':' << '\n';
      cerr << fn << ':' << line << ':' << '\n';
      cerr << kind << ' ' << name << ' ';
      if (!text.empty())
        cerr << "for `" << text << "' ";
      cerr << "not found." << '\n';
    }
  } // end of namespace warning
} // end of namespace doclink
