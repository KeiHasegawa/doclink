#include "doclink.h"

int doclink_parse();
extern FILE* doclink_in;

namespace doclink {
  using namespace std;
  ostream out(cout.rdbuf());
  ofstream* ptr_out;
  namespace cmdline {
    string prog;
    vector<string> inputs;
    string output_file;
    bool not_add_ws;
    bool output_predefined;
    bool output_nodef;
    bool not_truncate_string;
    vector<string> exclude_dir;
    bool exclude_macro;
    bool exclude_tag;
    bool exclude_type;
    bool exclude_var;
    bool exclude_func;
    bool output_flow;
    bool exclude_correlation;
    bool force_output_1_flow;
    inline void help_msg()
    {
      cerr << "-o <file> : specify output file." << '\n';
      cerr << "--not-add-ws : not add white space." << '\n';
      cerr << "--output-predefined : output predefined macro." << '\n';
      cerr << "--output-nodef : output variable or function without definition." << '\n';
      cerr << "--not-truncate-string : not truncate string." << '\n';
      cerr << "--exclude-dir <dir> : specify exclude directory." << '\n';
      cerr << "--exclude-macro : exclude macro." << '\n';
      cerr << "--exclude-tag : exclude structure etc." << '\n';
      cerr << "--exclude-type : exclude typedef." << '\n';
      cerr << "--exclude-var : exclude variable." << '\n';
      cerr << "--exclude-func : exclude function." << '\n';
      cerr << "--output-flow : output flow graph." << '\n';
      cerr << "--exclude-correlation : exclude correlation graph." << '\n';
      cerr << "--force-output-1-flow : force output 1 vertex flow graph." << '\n';
      cerr << "-h : print this messages." << '\n';
    }
    inline void set_output(const char* fn)
    {
      output_file = fn;
      ptr_out = new ofstream(fn);
      out.rdbuf(ptr_out->rdbuf());
    }
    inline char** option(char** argv)
    {
      if (string("-o") == *argv) {
        if (++argv && **argv != '-')
          set_output(*argv);
        else
          error::require_argument("-o");
      }
      else if (string("--exclude-dir") == *argv) {
        if (++argv && **argv != '-')
          exclude_dir.push_back(*argv);
        else
          error::require_argument("--exclude-dir");          
      }
      else if (string("--not-add-ws") == *argv)
        not_add_ws = true;
      else if (string("--output-predefined") == *argv)
        output_predefined = true;
      else if (string("--output-nodef") == *argv)
        output_nodef = true;
      else if (string("--not-truncate-string") == *argv)
        not_truncate_string = true;
      else if (string("--exclude-macro") == *argv)
        exclude_macro = true;
      else if (string("--exclude-tag") == *argv)
        exclude_tag = true;
      else if (string("--exclude-type") == *argv)
        exclude_type = true;
      else if (string("--exclude-var") == *argv)
        exclude_var = true;
      else if (string("--exclude-func") == *argv)
        exclude_func = true;
      else if (string("--output-flow") == *argv)
        output_flow = true;
      else if (string("--exclude-correlation") == *argv)
        exclude_correlation = true;
      else if (string("--force-output-1-flow") == *argv)
        force_output_1_flow = true;
      else if (string("-h") == *argv)
        help_msg();
      else
        error::unknown_option(*argv);
      return argv;
    }
    inline void set(int argc, char** argv)
    {
      prog = argv[0];
      while (*++argv) {
        if (**argv == '-')
          argv = option(argv);
        else
          inputs.push_back(*argv);
      }
    }
  } // end of namespace cmdline
  string curr_file;
  int lineno = 1;
  inline void parse(string fn)
  {
    curr_file = fn;
    lineno = 1;
    doclink_in = fopen(fn.c_str(), "r");
    if (!doclink_in)
      return error::cannot_open(fn);
    doclink_parse();
    fclose(doclink_in);
  }
} // end of namespace doclink

int main(int argc, char** argv)
{
  using namespace std;
  using namespace doclink;
  cmdline::set(argc, argv);
  for (auto fn : cmdline::inputs)
    parse(fn);
  generate();
  delete ptr_out;
#ifdef _MSC_VER
#define unlink _unlink
#endif // _MSC_VER
  if (error::counter) {
    if (!cmdline::output_file.empty())
      unlink(cmdline::output_file.c_str());
  }
  return error::counter;
}
