#include "doclink.h"

namespace doclink {
  namespace func {
    namespace graph {
      namespace correlation {
        const int delta = 10;
        const int text_limit = 30;
        typedef pair<int, int> point_t;
        struct tree {
          string m_text;
          point_t m_point;
          enum box_t { NONE, UNDER, DASH, FRAME } m_box;
          vector<tree*> m_children;
          ~tree()
          {
            for (auto t : m_children)
              delete t;
          }
        };
        point_t current_point;
        point_t max_point;
        inline int box_width(string s)
        {
          return text::width * (int)s.length() + delta;
        }
        enum update_t { TOO_WIDE, TOO_HEIGH, OK };
        inline int cdh()
        {
          return current_point.second + delta + text::height;
        }
        inline int hd(int depth)
        {
          if (!depth)
            return graph::height;
          return graph::height - (depth - 1) * (delta + text::height);
        }
        inline update_t update(string s, int depth, bool force)
        {
          int n = current_point.first + box_width(s);
          if (n > graph::width && !force)
            return TOO_WIDE;
          int m = cdh();
          if (m > hd(depth) && !force)
            return TOO_HEIGH;
          current_point.second = m;
          max_point.first = max(max_point.first, n);
          max_point.second = max(max_point.second, m);
          return OK;
        }
        set<const func_t*> done;
        void node(const func_t& func, tree* parent,
                  vector<pair<tree*, point_t> >& result, int depth);
        void lookup(const pair<string, file_t>& p, tree* parent,
                    vector<pair<tree*, point_t> >& result, int depth)
        {
          string name = p.first;
          typedef vector<func_t>::const_iterator IT;
          IT q = find_if(begin(funcs), end(funcs),
                      [name](const common_t& x){ return cmp_name(x, name); });
          assert(q != end(funcs));
          const func_t& func = *q;
          if (done.find(&func) != done.end() && cdh() <= hd(depth)) {
            tree* r = new tree;
            r->m_text = name;
            if (!cmdline::not_truncate_string)
              r->m_text = truncate(r->m_text, text_limit);
            r->m_point = current_point;
            r->m_box = tree::NONE;
            update(r->m_text, depth, true);
            parent->m_children.push_back(r);
            return;
          }
          return node(func, parent, result, depth);
        }
        inline void new_page(const func_t& func, tree* parent,
                             vector<pair<tree*, point_t> >& result,
                             tree* p, update_t r, int depth)
        {
          p->m_box = tree::UNDER;
          assert(parent);
          if (r == TOO_WIDE) {
            update(p->m_text, depth, true);
            parent->m_children.push_back(p);
          }
          else {
            assert(r == TOO_HEIGH);
            if (parent->m_children.empty()) {
              update(p->m_text, depth, true);
              parent->m_children.push_back(p);
            }
            else {
              tree* prev = parent->m_children.back();
              point_t pt = prev->m_point;
              if (pt.second + text::height > hd(depth))
                prev->m_text += " " + func.m_name;
              else {
                update(p->m_text, depth, true);
                parent->m_children.push_back(p);
              }
            }
          }
          if (done.find(&func) != done.end())
            return;
          point_t c = current_point;
          point_t m = max_point;
          current_point = max_point = make_pair(0, 0);
          tree* q = new tree;
          int n = (int)result.size();
          result.push_back(make_pair(q,point_t()));
          q->m_text = parent->m_text;
          q->m_point = current_point;
          q->m_box = tree::NONE;
          update(q->m_text, 0, false);
          current_point.first += 2 * delta;
          node(func, q, result, 1);
          assert(n < result.size());
          result[n].second = max_point;
          current_point = c;
          max_point = m;
        }
        void node(const func_t& func, tree* parent,
                  vector<pair<tree*, point_t> >& result, int depth)
        {
          tree* p = new tree;
          p->m_text = func.m_name;
          if (!cmdline::not_truncate_string)
            p->m_text = truncate(p->m_text, text_limit);
          p->m_point = current_point;
          int line = func.m_def.second;
          p->m_box = line ? tree::FRAME : tree::DASH;
          update_t r = update(p->m_text, depth, false);
          if (r != OK && parent)
            return new_page(func, parent, result, p, r, depth);
          if (parent)
            parent->m_children.push_back(p);
          else
            result.push_back(make_pair(p,point_t()));
          done.insert(&func);
          current_point.first += 2 * delta;
          set<string> tmp;
          for (auto q : func.m_call) {
            string name = q.first;
            if (tmp.find(name) == tmp.end()) {
              lookup(q, p, result, depth+1);
              tmp.insert(name);
            }
          }
          current_point.first -= 2 * delta;
          if (!p->m_children.empty()) {
            tree* q = p->m_children.back();
            if (!cmdline::not_truncate_string) {
              int limit = (graph::width - q->m_point.first) / text::width;
              q->m_text = truncate(q->m_text, limit);
            }
          }
        }
        inline void calc(vector<pair<tree*, point_t> >& result)
        {
          for (auto& func : funcs) {
            int line = func.m_def.second;
            if (line && func.m_refed.empty()) {
              int n = (int)result.size();
              node(func, 0, result, 0);
              assert(n < result.size());
              result[n].second = max_point;
              current_point = max_point = make_pair(0,0);
            }
          }

          for (auto& func :  funcs) {
            int line = func.m_def.second;
            if (line && done.find(&func) == done.end()) {
              int n = (int)result.size();
              node(func, 0, result, 0);
              assert(n < result.size());
              result[n].second = max_point;
              current_point = max_point = make_pair(0,0);
            }
          }
        }
        void conv_coordinate(pair<tree*, point_t> x)
        {
          tree* t = x.first;
          point_t p = x.second;
          t->m_point.second = p.second - t->m_point.second;
          for (auto c : t->m_children)
            conv_coordinate(make_pair(c, p));
        }
        inline void
        down_right(const pair<int, int>& s, const pair<int, int>& t)
        {
          int x = s.first + delta ;
          int y = s.second - text::height;
          out << "\\put" << '(' << x << ',' << y << ')';
          out << '{';
          int down_len = s.second - t.second - text::height/2;
          out << "\\line(0,-1)" << '{' << down_len << '}';
          out << '}' << '\n';
          y -= down_len;
          out << "\\put" << '(' << x << ',' << y << ')';
          out << '{';
          int right_len = t.first - x;
          out << "\\vector(1,0)" << '{' << right_len << '}';
          out << '}' << "\n\n";
        }
        inline void draw(const tree* p)
        {
          out << "\\put";
          switch (p->m_box) {
          case tree::NONE:
          case tree::UNDER:
            out << '(' << p->m_point.first + text::width << ',';
            out << p->m_point.second - 2 * text::height/3 << ')';
            break;
          case tree::DASH:
          case tree::FRAME:
            out << '(' << p->m_point.first << ',';
            out << p->m_point.second - text::height << ')';
            break;
          }
          out << '{';
          string s = p->m_text;
          switch (p->m_box) {
          case tree::NONE:
            out << conv_(s);
            break;
          case tree::UNDER:
            out << "\\underline" << '{' << conv_(s) << '}';
            break;
          case tree::DASH:
            out << "\\dashbox{5}";
            out << '(' << box_width(s) << ',';
            out << text::height << ')' << '{';
            out << conv_(s);
            out << '}';
            break;
          case tree::FRAME:
            out << "\\framebox";
            out << '(' << box_width(s) << ',';
            out << text::height << ')' << '{';
            out << conv_(s);
            out << '}';
            break;
          }
          out << '}' << "\n\n";
          for (auto t : p->m_children) {
            down_right(p->m_point, t->m_point);
            draw(t);
          }
        }
        inline void output(const pair<tree*, point_t>& t)
        {
          tree* p = t.first;
          string s;
          if (p->m_box == tree::FRAME)
            s = p->m_text;
          else {
            assert(p->m_box == tree::NONE);
            assert(p->m_children.size() == 1);
            tree* ch = p->m_children.back();
            s = ch->m_text;
          }
          if (!cmdline::not_truncate_string)
            s = truncate(s, section_length);
          out << "\\section{" << conv_(s) << "}\n\n";
          conv_coordinate(t);
          out << "\\begin{picture}";
          point_t q = t.second;
          out << '(' << q.first << ',' << q.second << ')';
          out << "(0,0)" << "\n";
          out << "%%";
          out << "\\put(0,0){\\dashbox{5}";
          out << '(' << q.first << ',' << q.second << ')';
          out << "}" << "\n\n";
          draw(p);
          out << "\\end{picture}" << "\n\n";
          delete p;
        }
        void output()
        {
          vector<pair<tree*, point_t> > result;
          calc(result);
          for (auto t : result)
            output(t);
        }
      } // end of namespace correlation
    } // end of namespace graph
  } // end of namespace func
} // end of namespace doclink
