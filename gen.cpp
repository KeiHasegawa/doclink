#pragma warning (disable : 4267)

#include "doclink.h"

namespace doclink {
  const int width = 58;
  const int height = 100;
  const string key_macro = "Macro";
  const string key_tag = "Structure etc";
  const string key_type = "Typedef";
  const string key_var = "Variable";
  const string key_func = "Function";
  const string key_correlation = "Correlation";
  const string key_def = "Defined at";
  const string key_decl = "Declared at";
  const string key_ref = "Referenced at";
  const string key_nowhere = "nowhere";
  const string comm_begin = "/*";
  const string comm_end = "*/";
  const string cxx_comm = "//";
  const int buf_size = 1024;
  const int before_behind = 100;
  const int line_limit = 10;

  inline int get(ifstream& ifs, int line, vector<string>& vs)
  {
    int index = 0;
    for (int n = 1 ; ; ++n) {
      char buf[buf_size];
      ifs.getline(buf, sizeof buf);
      if (ifs.eof())
        break;
      if (line - n > before_behind)
        continue;
      if (n - line > before_behind)
        break;
      string tmp = buf;
      if (!tmp.empty()) {
        string::size_type pos = tmp.size() - 1;
        if (tmp[pos] == '\r')
          tmp.erase(pos, 1);
      }
      vs.push_back(tmp);
      if (n == line)
        index = vs.size() - 1;
    }
    return index;
  }
  inline string to_string(const file_t& file)
  {
    ostringstream ost;
    ost << file.first;
    if (file.second)
      ost << ':' << file.second;
    return ost.str();
  }
  inline string def_file(const file_t& file)
  {
    if (file.first.empty())
      return key_def + ' ' + key_nowhere;
    else
      return key_def + ' ' + to_string(file);
  }
  inline string decl_file(const file_t& file)
  {
    return key_decl + ' ' + to_string(file);
  }
  inline bool exclude(string path)
  {
    string::size_type pos = path.find_last_of('/');
    if (pos == string::npos)
      return false;
    assert(isascii(path[pos-1]));
    path = path.substr(0, pos);
    const vector<string>& v = cmdline::exclude_dir;
    typedef vector<string>::const_iterator IT;
    IT p = find_if(begin(v), end(v), [path](string dir)
                   { return dir == path.substr(0, dir.length()); });
    return p != end(v);
  }
  inline bool ref_file(const vector<file_t>& vf, vector<string>& rf)
  {
    rf.push_back(key_ref);
    if (vf.empty()) {
      rf.back() += ' ' + key_nowhere;
      return false;
    }

    set<file_t> done;    
    for (const auto& file : vf) {
      string fn = file.first;
      if (exclude(fn))
        continue;
      int line = file.second;
      if (!line && !cmdline::output_predefined)
	continue;
      if (done.find(file) != done.end())
        continue;
      done.insert(file);
      string& s = rf.back();
      string t = to_string(file);
      if (s.length() + t.length() < width)
        s += ' ' + t;
      else
        rf.push_back(t);
    }

    return !done.empty();
  }
  inline bool ref_file(const vector<pair<string, file_t> >& vf,
                       vector<string>& rf)
  {
    set<string> tmp;
    for (const auto& x : vf) {
      string fn = x.first;
      if (!exclude(fn))
        tmp.insert(fn);
    }
    rf.push_back(key_ref);
    if (tmp.empty()) {
      rf.back() += ' ' + key_nowhere;
      return false;
    }

    for (string t : tmp) {
      string& s = rf.back();
      if (s.length() + t.length() < width)
        s += ' ' + t;
      else
        rf.push_back(t);
    }
    return true;
  }
  typedef vector<string>::reverse_iterator VSRI;
  inline void
  get_comm(vector<string>& vs, VSRI q,
           string fn, int line, const file_t& file, string kind, string name,
           vector<string>& comment)
  {
    typedef vector<string>::iterator IT;    
    string predecessor = *q;
    if (predecessor.find(comm_end) != string::npos) {
      string::size_type pos;
      VSRI r = find_if(q, rend(vs), [&pos](string s)
                       { return (pos = s.find(comm_begin)) != string::npos; } );
      if (r == rend(vs))
        return warning::not_found(fn, line, file, kind, name, comm_begin);
      IT p = r.base()-1;
      string s = *p;
      string::const_iterator it =
        find_if(begin(s), begin(s) + pos, [](char c){ return isalpha(c); });
      if (it == begin(s) + pos)
        copy(p, q.base(), back_inserter(comment));
      return;
    }

    string::size_type pos = predecessor.find(cxx_comm);
    if (pos != string::npos) {
      const string& s = predecessor;
      string::const_iterator it =
        find_if(begin(s), begin(s) + pos, [](char c){ return isalpha(c); });
      if (it != begin(s) + pos)
        return;
      VSRI r = find_if(q, rend(vs), [](string s){
          return s.find(cxx_comm) == string::npos; });
      copy(r.base(), q.base(), back_inserter(comment));
    }
  }
  inline string conv(string s)
  {
    ostringstream ost;
    if (!cmdline::not_add_ws) {
      while (s.length() < width)
        s += ' ';
    }
    if (!cmdline::not_truncate_string)
      s = truncate(s, width);

    while (!s.empty()) {
      string::size_type pos = s.find('|');
          if (pos)
        ost << "\\verb|" << s.substr(0,pos) << '|';
      if (pos == string::npos)
        break;
      ost << "\\verb+|+";
      s = s.substr(pos+1);
    }
    ost << " \\\\";
    return ost.str();
  }
  inline void multi_line(const vector<string>& vs)
  {
    int n = 0;
    for (auto s : vs) {
      if (++n != line_limit - 2)
        out << conv(s) << '\n';
      else {
        out << conv("...") << '\n';
        out << conv(vs.back()) << '\n';
        break;
      }
    }
  }
  inline void output(string name,
                     const vector<string>& def,
                     string df,
                     const vector<string>& rf,
                     const vector<string>& comment,
                     string decl_f = "")
  {
    if (!cmdline::not_truncate_string)
      name = truncate(name, section_length);
    out << "\\section{" << conv_(name) << "}\n\n";
    out << "\\begin{tabular}{|l|} \\hline" << '\n';
    multi_line(def);
    if (def.empty())
      out << "\\ \\\\";
    out << "\\hline" << '\n';
    out << conv(df) << " \\hline" << '\n';
    if (!decl_f.empty())
      out << conv(decl_f) << " \\hline" << '\n';
    if (!rf.empty()) {
      multi_line(rf);
      out << "\\hline" << '\n';
    }
    multi_line(comment);
    if (comment.empty())
      out << "\\ \\\\ ";
    out << "\\hline" << '\n';
    out << "\\end{tabular}" << "\n\n";
  }
  inline bool operator<(const common_t& x, const common_t& y)
  {
    return appear::less(x.m_def, y.m_def);
  }
  namespace macro {
    const string define = "define";
    inline void
    prog_text(string fn, int line, const file_t& file, string name,
              vector<string>& def, vector<string>& comment)
    {
      if (!line)
        return;
      ifstream ifs(fn);
      if (!ifs)
        return error::cannot_open(fn, file);
      vector<string> vs;
      int index = get(ifs, line, vs);
      assert(index < vs.size());
      VSRI p = rbegin(vs) + vs.size() - index - 1;
      VSRI q = find_if(p, rend(vs), [](string s){
          return s.find(define) != string::npos;} );
      if (q == rend(vs))
        return warning::not_found(fn, line, file, "macro", name, define);
      copy(q.base()-1, p.base(), back_inserter(def));
      if (++q == rend(vs))
        return;
      get_comm(vs, q, fn, line, file, "macro", name, comment);
    }
    inline void gen(const macro_t& x)
    {
      string fn = x.m_def.first;
      int line = x.m_def.second;
      if (!line && !fn.empty() && !cmdline::output_predefined)
        return;
      if (exclude(fn))
        return;
      vector<string> def, comment;
      prog_text(fn, line, x.m_input, x.m_name, def, comment);
      string df = def_file(x.m_def);
      vector<string> rf;
	  bool refed = ref_file(x.m_refed, rf);
      if (fn.empty() && !refed)
        return;
      output(x.m_name, def, df, rf, comment);
    }
  } // end of namespace macro
  namespace tag {
    const string key_struct = "struct";
    const string key_union = "union";
    const string key_enum = "enum";
    const string decl_end = "}";
    inline bool decl_start(string s)
    {
      return s.find(key_struct) != string::npos ||
        s.find(key_union) != string::npos ||
        s.find(key_enum) != string ::npos;
    }
    inline void
    prog_text(string fn, int line, const file_t& file, string name,
              vector<string>& def, vector<string>& comment)
    {
      assert(line);
      ifstream ifs(fn);
      if (!ifs)
        return error::cannot_open(fn, file);
      vector<string> vs;
      int index = get(ifs, line, vs);
      assert(index < vs.size());
      VSRI p = rbegin(vs) + vs.size() - index - 1;
      VSRI q = find_if(p, rend(vs), decl_start);
      if (q == rend(vs)) {
        ostringstream ost;
        ost << key_struct << ',' << key_union << ',' << key_enum;
        return warning::not_found(fn, line, file, "tag", name, ost.str());
      }

      typedef vector<string>::iterator Y;
      Y r = find_if(p.base()-1, end(vs),
                    [](string s){ return s.find(decl_end) != string::npos; });
      if (r == end(vs))
        return warning::not_found(fn, line, file, "tag", name, decl_end);
      copy(q.base()-1, r + 1, back_inserter(def));
      if (++q == rend(vs))
        return;
      get_comm(vs, q, fn, line, file, "tag", name, comment);
    }
    inline void gen(const tag_t& x)
    {
      string fn = x.m_def.first;
      int line = x.m_def.second;
      if (exclude(fn))
        return;
      vector<string> def, comment;
      prog_text(fn, line, x.m_input, x.m_name, def, comment);
      string df = decl_file(x.m_def);
      vector<string> rf;
	  bool refed = ref_file(x.m_refed, rf);
      if (fn.empty() && !refed)
        return;
      output(x.m_name, def, df, rf, comment);
    }
  } // end of namespace tag
  inline bool empty_or_comment(string s)
  {
    if (s.empty())
      return true;

    if (s.find(';') != string::npos)
      return !isspace(s[0]);
    
    string::size_type pos = s.find(comm_end);
    if (pos != string::npos) {
      pos = s.find(comm_begin);
      if (pos == string::npos)
	return true;
      const char* p = find_if(&s[0], &s[pos], isascii);
      return p == &s[pos];
    }

    pos = s.find(cxx_comm);
    if (pos != string::npos) {
      const char* p = find_if(&s[0], &s[pos], isascii);
      return p == &s[pos];
    }

    return false;
  }
  template<class C> inline bool cmp(const C& x, const C& y)
  {
    int lx = x.m_def.second;
    assert(lx || !x.m_decls.empty());
    const file_t& fx = lx ? x.m_def : x.m_decls[0];
    int ly = y.m_def.second;
    assert(ly || !y.m_decls.empty());
    const file_t& fy = ly ? y.m_def : y.m_decls[0];
    return appear::less(fx, fy);
  }
  namespace var {
    inline bool operator<(const var_t& x, const var_t& y)
    {
      return cmp(x,y);
    }
    const string def_end = ";";
    inline void
    prog_text(string fn, int line, const file_t& file, string name,
              vector<string>& def, vector<string>& comment)
    {
      if (!line)
        return;
      ifstream ifs(fn);
      if (!ifs)
        return error::cannot_open(fn, file);
      vector<string> vs;
      int index = get(ifs, line, vs);
      assert(index < vs.size());
      VSRI p = rbegin(vs) + vs.size() - index - 1;
      VSRI q = find_if(p+1, rend(vs), empty_or_comment);
      --q;
      typedef vector<string>::iterator Y;
      Y r = find_if(p.base()-1, end(vs),
                    [](string s){ return s.find(def_end) != string::npos; });
      if (r == end(vs))
        return warning::not_found(fn, line, file, "var", name, def_end);
      copy(q.base()-1, r + 1, back_inserter(def));
      if (++q == rend(vs))
        return;
      get_comm(vs, q, fn, line, file, "var", name, comment);
    }
    inline void gen(const var_t& x)
    {
      string fn = x.m_def.first;
      int line = x.m_def.second;
      if (!line && !cmdline::output_nodef)
        return;
      if (exclude(fn))
        return;
      vector<string> def, comment;
      prog_text(fn, line, x.m_input, x.m_name, def, comment);
      string df = def_file(x.m_def);
      string decl_f;
      const vector<file_t>& d = x.m_decls;
      typedef vector<file_t>::const_iterator IT;
      IT p = find_if(begin(d), end(d),
                     [](const file_t& file){ return !exclude(file.first); });
      if (p != end(d))
        decl_f = decl_file(*p);
      if (!d.empty() && decl_f.empty())
        return;
      if (!line && p != end(d)) {
        assert(def.empty() && comment.empty());
        fn = p->first;
        line = p->second;
        prog_text(fn, line, x.m_input, x.m_name, def, comment);
      }
      vector<string> rf;
      ref_file(x.m_refed, rf);
      output(x.m_name, def, df, rf, comment, decl_f);
    }
  } // end of namespace var
  namespace func {
    const string body_start = "{";
    const string decl_end = ";";
    inline bool operator<(const func_t& x, const func_t& y)
    {
      return cmp(x,y);
    }
    inline void
    prog_text(string fn, int line, const file_t& file, string name,
              vector<string>& def, vector<string>& comment, string xxx)
    {
      if (!line)
        return;
      ifstream ifs(fn);
      if (!ifs)
        return error::cannot_open(fn, file);
      vector<string> vs;
      int index = get(ifs, line, vs);
      assert(index < vs.size());
      VSRI p = rbegin(vs) + vs.size() - index - 1;
      VSRI q = find_if(p+1, rend(vs), empty_or_comment);
      --q;
      typedef vector<string>::iterator Y;
      Y r = find_if(p.base()-1, end(vs),
                    [xxx](string s){ return s.find(xxx) != string::npos; });
      if (r == end(vs))
        return warning::not_found(fn, line, file, "func", name, xxx);
      if (*r == body_start)
        --r;
      copy(q.base()-1, r + 1, back_inserter(def));
      if (++q == rend(vs))
        return;
      get_comm(vs, q, fn, line, file, "func", name, comment);
    }
    inline void gen(const func_t& x)
    {
      string fn = x.m_def.first;
      int line = x.m_def.second;
      if (!line && !cmdline::output_nodef)
        return;
      if (exclude(fn))
        return;
      vector<string> def, comment;
      prog_text(fn, line, x.m_input, x.m_name, def, comment, body_start);
      string df = def_file(x.m_def);
      string decl_f;
      const vector<file_t>& d = x.m_decls;
      typedef vector<file_t>::const_iterator IT;
      IT p = find_if(begin(d), end(d),
                     [](const file_t& file){ return !exclude(file.first); });
      if (p != end(d))
        decl_f = decl_file(*p);
      if (!d.empty() && decl_f.empty())
        return;
      if (!line && p != end(d)) {
        assert(def.empty() && comment.empty());
        fn = p->first;
        line = p->second;
        prog_text(fn, line, x.m_input, x.m_name, def, comment, decl_end);
      }
      vector<string> rf;
      ref_file(x.m_refed, rf);
      output(x.m_name, def, df, rf, comment, decl_f);
      if (cmdline::output_flow)
        graph::flow::output(x.m_vertices, x.m_edges);
    }
  } // end of namespace func
  namespace type {
    const string type_def = "typedef";
    inline void
    prog_text(string fn, int line, const file_t& file, string name,
              vector<string>& def, vector<string>& comment)
    {
      ifstream ifs(fn);
      if (!ifs)
        return error::cannot_open(fn, file);
      vector<string> vs;
      int index = get(ifs, line, vs);
      assert(index < vs.size());
      VSRI p = rbegin(vs) + vs.size() - index - 1;
      VSRI q = find_if(p, rend(vs), [](string s){
          return s.find(type_def) != string::npos;} );
      if (q == rend(vs))
        return warning::not_found(fn, line, file, "type", name, type_def);
      copy(q.base()-1, p.base(), back_inserter(def));
      if (++q == rend(vs))
        return;
      get_comm(vs, q, fn, line, file, "type", name, comment);
    }
    inline void gen(const type_t& x)
    {
      string fn = x.m_def.first;
      int line = x.m_def.second;
      if (exclude(fn))
        return;
      vector<string> def, comment;
      prog_text(fn, line, x.m_input, x.m_name, def, comment);
      string df = def_file(x.m_def);
      vector<string> rf;
      ref_file(x.m_refed, rf);
      output(x.m_name, def, df, rf, comment);
    }
  } // end of namespace type
  void generate()
  {
    using namespace macro;
    if (!cmdline::exclude_macro) {
      if (!macros.empty())
        out << "\\chapter{" << key_macro << "}\n\n";
      for (const auto& x : macros)
        gen(x);
    }

    using namespace tag;
    if (!cmdline::exclude_tag) {
      if (!tags.empty())
        out << "\\chapter{" << key_tag << "}\n\n";
      sort(begin(tags), end(tags));
      for (const auto& x : tags)
        gen(x);
    }

    using namespace type;
    if (!cmdline::exclude_type) {
      if (!types.empty())
        out << "\\chapter{" << key_type << "}\n\n";
      sort(begin(types), end(types));
      for (const auto& x : types)
        gen(x);
    }
    
    using namespace var;
    if (!cmdline::exclude_var) {
      if (!vars.empty())
        out << "\\chapter{" << key_var << "}\n\n";
      sort(begin(vars), end(vars));
      for (const auto& x : vars)
        gen(x);
    }

    using namespace func;
    if (!cmdline::exclude_func) {
      if (!funcs.empty())
        out << "\\chapter{" << key_func << "}\n\n";
      sort(begin(funcs), end(funcs));
      for (const auto& x : funcs)
        gen(x);
    }

    if (!cmdline::exclude_correlation) {
      if (!funcs.empty())
        out << "\\chapter{" << key_correlation << "}\n\n";
      if (cmdline::exclude_func)
        sort(begin(funcs), end(funcs));
      graph::correlation::output();
    }
  }
} // end of namespace doclink
