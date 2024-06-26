import missingno;

struct h {
  int a;
};
struct b {
  int a;
};
struct file {
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

mno::req<b> parse_file_serially() {
  return read_file("")
      .if_failed("File not found")
      .map(parse_header)
      .map(parse_body)
      .map([](auto fa) { return fa.agg; });
}

mno::req<char> read_u8();
mno::req<int> read_u16() { return read_u8() | (read_u8() << 8); }

int main() {}
