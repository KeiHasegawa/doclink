#include "doclink.h"

namespace doclink {
  namespace func {
    namespace graph {
      namespace flow {
        const int N = 6; // max vertices on 1 page
        const int delta = 10;
        namespace box {
          const int width = 50;
          const int height = 50;
          const int interval = 30;
        } // end of namespace box
        inline int head(int n_vert)
        {
          out << "\\begin{picture}";
          int a = N - n_vert;
          int b = a ? a + 1 : 0;
          int h = height - b * box::interval - a * box::height;
          out << '(' << graph::width << ',' << h << ')';
          out << "(0,0)" << '\n';
          out << "%%";
          out << "\\put(0,0){\\dashbox{5}";
          out << '(' << graph::width << ',' << h << ')';
          out << "}" << '\n';
          return h;
        }
        inline void tail()
        {
          out << "\\end{picture}" << "\n\n";
        }
        struct draw_vert {
          int m_height;
          draw_vert(int h) : m_height(h) {}
          static int y(int h, int m)
          {
            return h - text::height
              - m * (box::interval + box::height);
          }
          int operator()(int nth, const pair<int, file_t>& v)
          {
            out << "\\put(";
            int x = graph::width / 2 - box::width / 2;
            out << x << ',' << y(m_height, nth + 1);
            out << "){\\framebox(" << box::width << ',' << box::height;
            out << "){$B_{" << v.first << "}$}}" << '\n';
            return nth + 1;
          }
        };
        typedef pair<pair<int, int>, pair<int, int> > line_t;
        inline bool overlap(const line_t& s, const line_t& t)
        {
          int sx = s.first.first;
          assert(sx == s.second.first);
          int tx = t.first.first;
          assert(tx == t.second.first);
          if (sx != tx)
            return false;
          int sb = s.first.second;
          int se = s.second.second;
          int tb = t.first.second;
          int te = t.second.second;
          if (sb < se) {
            // left side
            assert(tb < te);
            if (sb < tb)
              return se > tb;
            return te > sb;
          }

          // right side
          assert(tb > te);
          if (sb > tb)
            return se < tb;
          return sb > te;
        }

        inline int get(int x, int y, int B_len, bool right,
                       const vector<line_t>& verticality)
        {
          for (int n = 1 ; ; ++n) {
            int A_len = n * delta;
            int xx = right ? x + A_len : x - A_len;
            int yy = right ? y - B_len : y + B_len;
            pair<int, int> s(xx, y);
            pair<int, int> t(xx, yy);
            line_t line(s, t);
            typedef vector<line_t>::const_iterator IT;
            IT p = find_if(begin(verticality), end(verticality),
                           [line](const line_t& x){return overlap(x, line);});
            if (p == end(verticality))
              return A_len;
          }
        }

        struct inpage {
          int m_height;
          const set<pair<int, int> >& m_edges;
          int m_distance;
          vector<line_t>& m_verticality;
          inpage(int h, const set<pair<int, int> >& e, int d, vector<line_t>& v)
            : m_height(h), m_edges(e), m_distance(d), m_verticality(v) {}
          int self(int nth, const pair<int, file_t>& v)
          {
            pair<int, int> e(v.first, v.first);
            if (m_edges.find(e) == m_edges.end())
              return nth+1;

            /*        +-------+
                   (D)|       |
                   +->|       |
                   |  +-------+
               (C) |      | (A)
                   +------+
                   (B)

            */

            // (A)
            int x = width/2;
            int y = draw_vert::y(m_height, nth + 1);

            out << "\\put";
            out << '(' << x << ',' << y << ')';
            int d = delta;
            out << "{\\line(0,-1){" << d << "}}" << '\n';
            y -= d;

            // (B)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            d = box::width/2 + delta;
            out << "{\\line(-1,0){" << d << "}}" << '\n';
            x -= d;

            // (C)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            pair<int, int> s(x, y);
            d = box::width/2 + delta;
            out << "{\\line(0,1){" << d << "}}" << '\n';
            y += d;
            pair<int, int> t(x, y);
            m_verticality.push_back(line_t(s,t));

            // (D)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            d = delta;
            out << "{\\vector(1,0){" << d << "}}" << '\n';
            return nth + 1;
          }
          void down(int nth)  const
          {
            int x = width/2 + delta;
            int y = draw_vert::y(m_height, nth + 1);
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            int d = box::interval;
            out << "{\\vector(0,-1){" << d << "}}" << '\n';
          }
          void up(int nth) const
          {
            int x = width/2 - delta;
            int y = draw_vert::y(m_height, nth + 1) + box::height;
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            int d = box::interval;
            out << "{\\vector(0,1){" << d << "}}" << '\n';
          }
          int neighbor(int nth, const pair<int, file_t>& v) const
          {
            pair<int, int> a(v.first, v.first + 1);
            if (m_edges.find(a) != m_edges.end())
              down(nth);
            pair<int, int> b(v.first, v.first - 1);
            if (m_edges.find(b) != m_edges.end())
              up(nth);
            if (nth % N == 0) {
              pair<int, int> c(v.first-1, v.first);
              if (m_edges.find(c) != m_edges.end())
                down(nth-1);
            }
            if (nth % N == N - 1) {
              pair<int, int> d(v.first+1, v.first);
              if (m_edges.find(d) != m_edges.end())
                up(nth+1);
            }
            return nth + 1;
          }
          void right_down_left(int nth)
          {
            /*
              +-------+
              |       |
              |       |  (A)
              |       |------+
              +-------+      |
                  .          |
                  .          |
                  .          |
                  .          |  (B)
                  ~          ~
                  .          |
                  .          |
                  .          |
                  .          |
              +-------+      |
              |       |<-----+
              |       |  (C)
              |       |
              +-------+
            */

            // (A)
            int x = width/2 + box::width/2;
            int y = draw_vert::y(m_height, nth + 1) + delta;

            int B_len = delta + m_distance * box::interval
              + (m_distance - 1) * box::height + delta;
            int A_len = get(x, y, B_len, true, m_verticality);

            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\line(1,0){" << A_len << "}}" << '\n';
            x += A_len;

            // (B)
            pair<int, int> s(x,y);
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\line(0,-1){" << B_len << "}}" << '\n';
            y -= B_len;
            pair<int, int> t(x,y);
            m_verticality.push_back(line_t(s,t));

            // (C)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\vector(-1,0){" << A_len << "}}" << '\n';
          }
          void left_up_right(int nth)
          {
            /*
                     +-------+
                     |       |
              (C)    |       |
              +----->|       |
              |      +-------+
              |          .
              |          .
              |          .
           (B)|          .
              ~          ~
              |          .
              |          .
              |          .
              |          .
              |      +-------+
              +------|       |
              (A)    |       |
                     |       |
                     +-------+
            */

            // (A)
            int x = width/2 - box::width/2;
            int y = draw_vert::y(m_height, nth + 1) + box::height - delta;

            int B_len = delta + m_distance * box::interval
              + (m_distance - 1) * box::height + delta;
            int A_len = get(x, y, B_len, false, m_verticality);

            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\line(-1,0){" << A_len << "}}" << '\n';
            x -= A_len;

            // (B)
            pair<int, int> s(x,y);
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\line(0,1){" << B_len << "}}" << '\n';
            y += B_len;
            pair<int, int> t(x,y);
            m_verticality.push_back(line_t(s,t));

            // (C)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\vector(1,0){" << A_len << "}}" << '\n';
          }
          int other(int nth, const pair<int, file_t>& v)
          {
            if (nth + m_distance < N) {
              pair<int, int> a(v.first, v.first + m_distance);
              if (m_edges.find(a) != m_edges.end())
                right_down_left(nth);
            }
            if (nth - m_distance >= 0) {
              pair<int, int> b(v.first, v.first - m_distance);
              if (m_edges.find(b) != m_edges.end())
                left_up_right(nth);
            }
            return nth + 1;
          }
          int operator()(int nth, const pair<int, file_t>& v)
          {
            switch (m_distance) {
            case 0:  return self(nth, v);
            case 1:  return neighbor(nth, v);
            default: return other(nth, v);
            }
          }
        };
        struct outpage {
          int m_height;
          const set<pair<int, int> >& m_edges;
          vector<line_t>& m_verticality;
          outpage(int h, const set<pair<int, int> >& e, vector<line_t>& v)
            : m_height(h), m_edges(e), m_verticality(v) {}
          static bool from_prev_page(const pair<int, int>& e, int v, int nth)
          {
            if (e.second != v)
              return false;
            if (nth == 0)
              nth = 1;
            return e.first < v - nth;
          }
          static bool from_follow_page(const pair<int, int>& e, int v, int nth)
          {
            if (e.second != v)
              return false;
            if (nth == N - 1)
              nth = N - 2;
            return e.first >= v + N - nth;
          }
          static bool to_follow_page(const pair<int, int>& e, int v, int nth)
          {
            if (e.first != v)
              return false;
            if (nth == N - 1)
              nth = N - 2;
            return e.second >= v + N - nth;
          }
          static bool to_prev_page(const pair<int, int>& e, int v, int nth)
          {
            if (e.first != v)
              return false;
            if (nth == 0)
              nth = 1;
            return e.second < v - nth;
          }
          int get2(int x, int y, int* A_len, bool right)
          {
            for (int n = 1 ; ; ++n, *A_len -= delta) {
              int B_len = n * delta;
              if (*A_len <= 0) {
			    // work around
                *A_len += delta;
                return B_len - delta;
              }
              int xx = right ? x + B_len : x - B_len;
              int yy = right ? y + *A_len : y - *A_len;
              pair<int, int> s(xx, yy);
              pair<int, int> t(xx, y);
              line_t line(s, t);
              typedef vector<line_t>::const_iterator IT;
              IT p = find_if(begin(m_verticality), end(m_verticality),
                             [line](const line_t& x){return overlap(x, line);});
              if (p == end(m_verticality))
                return B_len;
            }
          }
          string to_string(const vector<int>& from_prev)
          {
            ostringstream ost;
            ost << '$';
            for (auto v : from_prev)
              ost << "B_" << '{' << v << '}' << ',';
            string s = ost.str();
            s.replace(s.length()-1, 1, "$");
            return s;
          }
          void down_left(int nth, const vector<int>& from_prev)
          {
            /*
              B0, B1, B2 (T)
                  .          |
                  .          |
                  .          | (A)
                  .          |
                  ~          ~
                  .          |
                  .          |
                  .          |
              +-------+      |
              |       |<-----+
              |       |  (B)
              |       |
              +-------+
            */
            int x = width/2 + box::width/2;
            int y = draw_vert::y(m_height, nth + 1) + box::height - delta;
            int A_len = (nth + 1) * box::interval + nth * box::height + delta;
            int B_len = get2(x, y, &A_len, true);

            x += B_len;
            y += A_len;

            // (T)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << '{' << to_string(from_prev) << '}' << '\n';

            // (A)
            if (A_len) {
              pair<int, int> s(x,y);
              out << "\\put";
              out << '(' << x << ',' << y << ')';
              out << "{\\line(0,-1){" << A_len << "}}" << '\n';
              y -= A_len;
              pair<int, int> t(x,y);
              m_verticality.push_back(line_t(s,t));
            }

            // (B)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\vector(-1,0){" << B_len << "}}" << '\n';
          }
          void up_right(int nth, const vector<int>& from_follow)
          {
            /*
                     +-------+
                     |       |
              (B)    |       |
              +----->|       |
              |      +-------+
              |          .
              |          .
           (A)|          .
              |          .
              ~          ~
              |          .
              |          .
              |          .
              B0, B1, B2 (T)
            */
            int x = width/2 - box::width/2;
            int y = draw_vert::y(m_height, nth + 1) + delta;
            int A_len = delta + (N - nth) * box::interval
              + (N - nth - 1) * box::height;
            int B_len = get2(x, y, &A_len, false);

            x -= B_len;
            y -= A_len + text::height;

            // (T)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << '{' << to_string(from_follow) << '}' << '\n';
            y += text::height;

            // (A)
            if (A_len) {
              pair<int, int> s(x,y);
              out << "\\put";
              out << '(' << x << ',' << y << ')';
              out << "{\\line(0,1){" << A_len << "}}" << '\n';
              y += A_len;
              pair<int, int> t(x,y);
              m_verticality.push_back(line_t(s,t));
            }

            // (B)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\vector(1,0){" << B_len << "}}" << '\n';
          }
          void right_down(int nth, int u)
          {
            /*
              +-------+
              |       |
              |       | (A)
              |       |-----+
              +-------+     |
                  .         |
                  .         |
                  .         |
                  .         | (B)
                  ~         ~
                  .         |
                  .         |
                  .         |
                  .         |
                            V
                          B_{u}  (T)
            */
            int x = width/2 + box::width/2;
            int y = draw_vert::y(m_height, nth + 1) + delta;

            int B_len = delta + (N - nth) * box::interval
              + (N - nth - 1) * box::height;
            int A_len = get(x, y, B_len, true, m_verticality);

            // (A)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\line(1,0){" << A_len << "}}" << '\n';
            x += A_len;

            // (B)
            pair<int, int> s(x,y);
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\vector(0,-1){" << B_len << "}}" << '\n';
            y -= B_len;
            pair<int, int> t(x,y);
            m_verticality.push_back(line_t(s,t));

            // (T)
            y -= text::height;
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{$B_{" << u << "}$}" << '\n';
          }
          void left_up(int nth, int u)
          {
            /*
              (T) B_{u}
              |         .
              |         .
              |         .
           (B)|         .
              ~         ~
              |         .
              |         .
              |         .
              |     +-------+
              +-----|       |
                (A) |       |
                    |       |
                    +-------+
            */
            int x = width/2 - box::width/2;
            int y = draw_vert::y(m_height, nth + 1) + box::height - delta;

            int B_len = delta + (nth + 1) * box::interval + nth * box::height;
            int A_len = get(x, y, B_len, false, m_verticality);

            // (A)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\line(-1,0){" << A_len << "}}" << '\n';
            x -= A_len;

            // (B)
            pair<int, int> s(x,y);
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{\\vector(0,1){" << B_len << "}}" << '\n';
            y += B_len;
            pair<int, int> t(x,y);
            m_verticality.push_back(line_t(s,t));

            // (T)
            out << "\\put";
            out << '(' << x << ',' << y << ')';
            out << "{$B_{" << u << "}$}" << '\n';
          }
          int operator()(int nth, const pair<int, file_t>& v)
          {
            vector<int> from_prev;
            for (auto e : m_edges) {
              if (from_prev_page(e, v.first, nth))
                from_prev.push_back(e.first);
            }
            if (!from_prev.empty())
              down_left(nth, from_prev);

            vector<int> from_follow;
            for (auto e : m_edges) {
              if (from_follow_page(e, v.first, nth))
                from_follow.push_back(e.first);
            }
            if (!from_follow.empty())
              up_right(nth, from_follow);

            for (auto e : m_edges) {
              if (to_follow_page(e, v.first, nth))
                right_down(nth, e.second);
            }

            for (auto e : m_edges) {
              if (to_prev_page(e, v.first, nth))
                left_up(nth, e.second);
            }

            return nth + 1;
          }
        };
        void output(const vector<pair<int, file_t> >& vertices,
                    const set<pair<int, int> >& edges)
        {
          if (vertices.size() == 1 && !cmdline::force_output_1_flow)
            return;
          typedef vector<pair<int, file_t> >::const_iterator IT;
          IT p = begin(vertices);
          while (p != end(vertices)) {
            int n = (int)distance(p, end(vertices));
            IT q = (n < N) ? end(vertices) : p + N;
            n = (int)distance(p, q);
            int h = head(n);
            accumulate(p, q, 0, draw_vert(h));
            vector<line_t> verticality;
            for (int i = 0 ; i != N ; ++i)
              accumulate(p, q, 0, inpage(h, edges, i, verticality));
            accumulate(p, q, 0, outpage(h, edges, verticality));
            tail();
            if (n < N)
              break;
            p = q;
          }
        }
      } // end of namespace flow
    } // end of namespace graph
  } // end of namespace func
} // end of namespace doclink
