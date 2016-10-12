//#include <limits.h>
#include "gtest/gtest.h"
#include "minimizer.h"
#include "path.h"
#include "interval.h"
#include <set>
#include <vector>
#include <stdint.h> 
#include <iostream>

using std::set;
using namespace std;

struct Interval;
class Path;
struct Minimizer;

class MinimizerTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test
    // (right before the destructor).
  }
};

TEST_F(MinimizerTest,comparisonCheck){
    deque<Interval> v1 = {Interval(0,15)};
    deque<Interval> v2 = {Interval(1,12), Interval(14,18)};
    deque<Interval> v3 = {Interval(1,16)};
    deque<Interval> v4 = {Interval(0,10), Interval(15,20)};

    Minimizer m1 = Minimizer("abcde", v1);
    Minimizer m2 = Minimizer("abcdg", v1);
    Minimizer m3 = Minimizer("abcde", v2);
    Minimizer m4 = Minimizer("abcde", v3);
    Minimizer m5 = Minimizer("abcde", v4);
    
    set<Minimizer> s;
    s.insert(m1);
    s.insert(m2);
    s.insert(m3);
    s.insert(m4);
    s.insert(m5);

    uint32_t j = 5;
    EXPECT_EQ(s.size(),j) << "size of set of minimizers " << s.size() << " is not equal to 5.";

    vector<Minimizer> v = {m1, m5, m4, m3, m2};
    int i = 0;
    for (std::set<Minimizer>::iterator it=s.begin(); it!=s.end(); ++it)
    {
	EXPECT_EQ(it->miniKmer, v[i].miniKmer) << "miniKmers do not agree: " << it->miniKmer << ", " << v[i].miniKmer;
	EXPECT_EQ(it->startPosOnString, v[i].startPosOnString) << "start positions do not agree: " << it->startPosOnString << ", " << v[i].startPosOnString;
	EXPECT_EQ(it->endPosOnString, v[i].endPosOnString) << "end positions do not agree: " << it->endPosOnString << ", " << v[i].endPosOnString;
    	++i;
    }
}