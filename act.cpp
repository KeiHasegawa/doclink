#include "doclink.h"

namespace doclink {
  using namespace std;
  namespace appear {
    map<string, int> table;
    int counter;
  }  // end of namespace appear
  struct sweeper {
    char* s;
    char* t;
    sweeper(char* ss, char* tt = 0) : s(ss), t(tt) {}
    ~sweeper(){ free(s); free(t); }
  };
  inline bool match(const common_t& x, const common_t& y)
  {
    return x.m_name == y.m_name && x.m_def == y.m_def;
  }
  string curr_sym;  
  namespace macro {
    vector<macro_t> macros;
    void def(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      macro_t y(name, path, line);
      typedef vector<macro_t>::const_iterator IT;
      IT p = find_if(begin(macros), end(macros),
                     [y](const common_t& x){ return match(x, y); });
      if (p == end(macros))
        macros.push_back(y);
    }
    void ref(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<macro_t>::reverse_iterator IT;
      IT p = find_if(rbegin(macros), rend(macros),
                     [name](const common_t& x){ return cmp_name(x,name); });
      if (p != rend(macros)) {
        p->m_refed.push_back(file_t(path, line));
        return;
      }
      macro_t y(name,"",0);
      y.m_refed.push_back(file_t(path, line));
      macros.push_back(y);
    }
  } // end of namespace macro
  namespace tag {
    vector<tag_t> tags;    
    void decl(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      tag_t y(name, path, line);
      typedef vector<tag_t>::const_iterator IT;
      IT p = find_if(begin(tags), end(tags),
                     [y](const common_t& x){ return match(x, y); });
      if (p == end(tags))
        tags.push_back(y);
    }
    void ref(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<tag_t>::reverse_iterator IT;
      IT p = find_if(rbegin(tags), rend(tags),
                     [name](const common_t& x){ return cmp_name(x,name); });
      if (p == rend(tags))
        return error::undef("tag", name);
      p->m_refed.push_back(make_pair(curr_sym, file_t(path, line)));
    }
  } // end of namespace tag
  namespace var {
    vector<var_t> vars;
    void def(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      curr_sym = name;
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<var_t>::reverse_iterator IT;
      IT p = find_if(rbegin(vars), rend(vars),
                     [name](const common_t& x){ return cmp_name(x, name); });
      if (p != rend(vars)) {
        if (p->m_def.second)
          error::multi_def("var", name, *p);
        p->m_def = file_t(path, line);
        return;
      }
      var_t y(name, path, line);
      vars.push_back(y);
    }
    void decl(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<var_t>::reverse_iterator IT;
      IT p = find_if(rbegin(vars), rend(vars),
                     [name](const common_t& x){ return cmp_name(x, name); });
      if (p != rend(vars)) {
        p->m_decls.push_back(file_t(path, line));
        return;
      }
      var_t y(name, "", 0);
      y.m_decls.push_back(file_t(path, line));
      vars.push_back(y);
    }
    void ref(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<var_t>::reverse_iterator IT;
      IT p = find_if(rbegin(vars), rend(vars),
                     [name](const common_t& x){ return cmp_name(x, name); });
      if (p == rend(vars))
        return error::undef("var", name);
      p->m_refed.push_back(make_pair(curr_sym, file_t(path, line)));
    }
  } // end of namespace var
  namespace func {
    vector<func_t> funcs;
    void def(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      curr_sym = name;
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<func_t>::reverse_iterator IT;
      IT p = find_if(rbegin(funcs), rend(funcs),
                     [name](const common_t& x){ return cmp_name(x, name); });
      if (p != rend(funcs)) {
        if (p->m_def.second)
          error::multi_def("func", name, *p);
        p->m_def = file_t(path, line);
        return;
      }
      func_t y(name, path, line);
      funcs.push_back(y);
    }
    void decl(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<func_t>::reverse_iterator IT;
      IT p = find_if(rbegin(funcs), rend(funcs),
                     [name](const common_t& x){ return cmp_name(x, name); });
      if (p != rend(funcs)) {
        p->m_decls.push_back(file_t(path, line));
        return;
      }
      func_t y(name, "", 0);
      y.m_decls.push_back(file_t(path, line));
      funcs.push_back(y);
    }
    void ref(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<func_t>::reverse_iterator IT;
      IT p = find_if(rbegin(funcs), rend(funcs),
                     [name](const common_t& x){ return cmp_name(x, name); });
      if (p == rend(funcs))
        return error::undef("func", name);
      p->m_refed.push_back(make_pair(curr_sym, file_t(path, line)));

      typedef vector<func_t>::reverse_iterator IT;
      IT q = find_if(rbegin(funcs), rend(funcs),
                     [](const common_t& x){ return cmp_name(x, curr_sym); });
      if (q == rend(funcs))
	return;
      func_t& func = *q;
      q->m_call.push_back(make_pair(name, file_t(path, line)));
    }
    void vertex(int v, char* fn, int line)
    {
      sweeper sweeper(fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<func_t>::reverse_iterator IT;
      IT p = find_if(rbegin(funcs), rend(funcs),
                     [](const common_t& x){ return cmp_name(x, curr_sym); });
      assert(p != rend(funcs));
      func_t& func = *p;
      func.m_vertices.push_back(make_pair(v,file_t(path, line)));
    }
    void edge(int u, int v)
    {
      typedef vector<func_t>::reverse_iterator IT;
      IT p = find_if(rbegin(funcs), rend(funcs),
                     [](const common_t& x){ return cmp_name(x, curr_sym); });
      assert(p != rend(funcs));
      func_t& func = *p;
      func.m_edges.insert(make_pair(u,v));
    }
  } // end of namespace func
  namespace type {
    vector<type_t> types;
    void def(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      file_t curr(path, line);
      typedef vector<type_t>::reverse_iterator IT;
      IT p = find_if(rbegin(types), rend(types),
                     [name](const common_t& x){ return cmp_name(x, name); });
      if (p != rend(types)) {
        if (p->m_def != curr)
          error::multi_def("type", name, *p);
        p->m_def = curr;
        return;
      }
      type_t y(name, path, line);
      types.push_back(y);
    }
    void ref(char* name, char* fn, int line)
    {
      sweeper sweeper(name, fn);
      string path = fn;
      assert(path[0] == '"' && path[path.length()-1] == '"');
      path = path.substr(1,path.length()-2);
      appear::add(path);
      typedef vector<type_t>::reverse_iterator IT;
      IT p = find_if(rbegin(types), rend(types),
                     [name](const common_t& x){ return cmp_name(x,name); });
      if (p == rend(types))
        return error::undef("type", name);
      p->m_refed.push_back(file_t(path, line));      
    }
  } // end of namespace type
} // end of namespace doclink
