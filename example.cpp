import missingno;

class h {
  int a;
};
class b {
  int a;
};
class file {
  int a;
};

template <typename Tp> struct file_agg {
  file f;
  Tp agg;
};

// This is just an example of the intended usage, so we don't need real
// implementations
mno::req<file> read_file(const char *);
file_agg<h> parse_header(file);
file_agg<b> parse_body(file_agg<h>);

mno::req<b> blah() {
  return read_file("")
      .otherwise("File not found")
      .then(parse_header)
      .then(parse_body)
      .then(&file_agg<b>::agg);
}
