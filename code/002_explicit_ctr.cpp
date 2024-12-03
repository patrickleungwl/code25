#include <gtest/gtest.h>
#include <vector>

class Widget {
private:
    int i_;
    int j_;
public:
    Widget(int i) : i_(i) {}
    explicit Widget(int i, int j) : i_(i), j_(j) {}
};

TEST(STLContainer, ExplicitCtrTests) {
    std::vector<Widget> v;
    v.emplace_back(1);
    v.push_back(1);
    v.emplace_back(1,2);

    // this does not compile
    // v.push_back(1,2);
    
    // but this is ok
    v.push_back(Widget(1,2));

    ASSERT_EQ(v.size(), 4);
}

// Commentary
//
// the explicit keyword tells the compiler the code cannot implcitly
// create an instance of the class if only the paramters match the
// expected parameters in the constructor. 
//
// The code must explicitly create the instance else the code fails
// at compile time.
