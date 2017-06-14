#define PI 3.14f
#define RADIUS 4
#define AREA ( 2 * PI * RADIUS )
const float area = AREA;

void globalFunction();
namespace Ns {
/*
 * Multiline
 * comment
 */
template <class T, int tntp>
class Foo
{
public:
  Foo(T) { i = tntp; };
  ~Foo() {};

  void setMember(T arg) {
    t = arg;
  }

private:
  T t; T i;
};

typedef Foo<int, 2> FooInt;

struct St;

enum En {
  enum_type1,
  enum_type2
};

union UN // does nothing
{
  FooInt* f;
  St* s;
};

} // namespace Ns


int main(int argc, char *argv[])
{
  // Literals
  int i = 123;
  float f = 45.6f;
  char c = 'c';
  const char* s = "str";

  Ns::FooInt foo = Ns::FooInt(argc);
  Ns::En e = Ns::enum_type1;

  // Յունիկոդ
  typedef Ns::UN ՄԱԿ;
  ՄԱԿ մ;
  մ.s = 0;
  return 0;
}

static int global_static_var;
static int global_static_func();

struct SSS
{
        int i;
        static int s;

        void foo(int);
        static void foo();
};

int SSS::s = 0;

void range_testing()
{
        int a = 1;
        int b = 2;
        int c = a
                + b;
}
