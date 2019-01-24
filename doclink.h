#ifndef _DOC_LINK_H_
#define _DOC_LINK_H_

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <complex>
#include <numeric>
#include <iterator>
#include <set>
#include <map>

#ifndef _MSC_VER
#include <unistd.h>
#endif // _MSC_VER

namespace doclink {
  using namespace std;
  const int section_length = 36;  
  namespace cmdline {
    extern string prog;
    extern bool not_add_ws;
    extern bool output_predefined;
    extern bool output_nodef;
    extern bool not_truncate_string;
    extern vector<string> exclude_dir;
    extern bool exclude_macro;
    extern bool exclude_tag;
    extern bool exclude_type;
    extern bool exclude_var;
    extern bool exclude_func;
    extern bool output_flow;
    extern bool exclude_correlation;
    extern bool force_output_1_flow;
  } // end of namespace cmdline
  extern int lineno;
  extern string curr_file;
  typedef pair<string, int> file_t;
  namespace appear {
    extern map<string, int> table;
    extern int counter;
    inline void add(string fn)
    {
      if (table.find(fn) == table.end())
	table.insert(make_pair(fn, counter++));
    }
    inline bool less(const file_t& x, const file_t& y)
    {
      typedef map<string, int>::const_iterator IT;
      IT p = table.find(x.first);
      assert(p != table.end());
      int nx = p->second;
      IT q = table.find(y.first);
      assert(q != table.end());
      int ny = q->second;
      return make_pair(nx, x.second) < make_pair(ny, y.second);
    }
  } // end of namespace appear

  struct common_t {
    string m_name;
    file_t m_def;
    file_t m_input;
    common_t(string name, string fn, int line)
    : m_name(name), m_def(fn, line), m_input(curr_file, lineno) {}
  };
  inline bool cmp_name(const common_t& x, string name)
  {
    return x.m_name == name;
  }
  inline string conv_(string s)
  {
    string::size_type pos = 0;
    while ( (pos = s.find('_', pos)) != string::npos ) {
      s.replace(pos, 1, "\\_");
      pos += 2;
    }
    return s;
  }
  inline string truncate(string s, int limit)
  {
    if (s.length() <= limit)
      return s;
    int n = (int)s.length() - limit;
    s.replace(limit/3, n+3, "...");
    return s;
  }
  namespace macro {
    struct macro_t : common_t {
      vector<file_t> m_refed;
      macro_t(string name, string path, int line)
        : common_t(name, path, line) {}
    };
    extern vector<macro_t> macros;
    void def(char* name, char* fn, int line);
    void ref(char* name, char* fn, int line);
  } // end of namespace macro
  namespace tag {
    struct tag_t : common_t {
      vector<pair<string,file_t> > m_refed;
      tag_t(string name, string path, int line)
        : common_t(name, path, line) {}
    };
    extern vector<tag_t> tags;    
    void decl(char* name, char* fn, int line);
    void ref(char* name, char* fn, int line);
  } // end of namespace tag
  namespace var {
    struct var_t : common_t {
      vector<file_t> m_decls;
      vector<pair<string, file_t> > m_refed;
      var_t(string name, string fn, int line)
        : common_t(name, fn, line) {}
    };
    extern vector<var_t> vars;
    void def(char* name, char* fn, int line);
    void decl(char* name, char* fn, int line);
    void ref(char* name, char* fn, int line);
  } // end of namespace var
  namespace func {
    struct func_t : common_t {
      vector<file_t> m_decls;
      vector<pair<string, file_t> > m_refed;
      vector<pair<string, file_t> > m_call;
      vector<pair<int,file_t> > m_vertices;
      set<pair<int,int> > m_edges;
      func_t(string name, string fn, int line)
        : common_t(name, fn, line) {}
    };
    extern vector<func_t> funcs;
    void def(char* name, char* fn, int line);
    void decl(char* name, char* fn, int line);
    void ref(char* name, char* fn, int line);
    void vertex(int v, char* fn, int line);
    void edge(int u, int v);
    namespace graph {
      const int width = 300;
      const int height = 550;
      namespace text {
	const int width = 5;
	const int height = 20;
      } // end of namespace text
      namespace flow {
	void output(const vector<pair<int, file_t> >& vertices,
		    const set<pair<int, int> >& edges);
      } // end of namespace flow
      namespace correlation {
	void output();
      } // end of namespace correlation
    }
  } // end of namespace func
  namespace type {
    struct type_t : common_t {
      vector<file_t> m_refed;
      type_t(string name, string path, int line)
        : common_t(name, path, line) {}
    };
    extern vector<type_t> types;
    void def(char* name, char* fn, int line);
    void ref(char* name, char* fn, int line);
  } // end of namespace type
  namespace error {
    extern int counter;
    void unknown_option(string opt);
    void require_argument(string opt);
    void undef(string kind, string name);
    void multi_def(string kind, string name, const common_t& prev);
    void cannot_open(string fn, const file_t& file = file_t());
  } // end of namespace error
  namespace warning {
    void not_found(string fn, int line, const file_t& file,
                   string kind, string name, string search = "");
  } // end of namespace warning
  extern ostream out;
  void generate();
} // end of namespace doclink

void doclink_error(const char* msg);

#endif // _DOC_LINK_H_
