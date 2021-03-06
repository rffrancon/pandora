#include "gtest/gtest.h"
#include "minirecord.h"
#include "prg/path.h"
#include "index.h"
#include "interval.h"
#include "inthash.h"
#include <vector>
#include <stdint.h>
#include <iostream>
#include <algorithm>


using namespace std;

TEST(IndexTest, add_record) {
    Index idx;
    KmerHash hash;
    deque<Interval> d = {Interval(3, 5), Interval(9, 12)};
    Path p;
    p.initialize(d);
    pair<uint64_t, uint64_t> kh = hash.kmerhash("ACGTA", 5);
    idx.add_record(min(kh.first, kh.second), 1, p, 0, 0);
    uint32_t j = 1;
    EXPECT_EQ(j, idx.minhash.size());

    // add again - should stay same size
    idx.add_record(min(kh.first, kh.second), 1, p, 0, 0);
    EXPECT_EQ(j, idx.minhash.size());
    EXPECT_EQ(j, idx.minhash[min(kh.first, kh.second)]->size());

    // add a new record with different key
    pair<uint64_t, uint64_t> kh2 = hash.kmerhash("ACTGA", 5);
    idx.add_record(min(kh2.first, kh2.second), 2, p, 0, 0);
    j = 2;
    EXPECT_EQ(j, idx.minhash.size());

    // and a new record which is different but has same key
    idx.add_record(min(kh.first, kh.second), 4, p, 0, 0);
    EXPECT_EQ(j, idx.minhash.size());
    EXPECT_EQ(j, idx.minhash[min(kh.first, kh.second)]->size());
}

TEST(IndexTest, clear) {
    Index idx;
    KmerHash hash;
    deque<Interval> d = {Interval(3, 5), Interval(9, 12)};
    Path p;
    p.initialize(d);
    pair<uint64_t, uint64_t> kh = hash.kmerhash("ACGTA", 5);
    idx.add_record(min(kh.first, kh.second), 1, p, 0, 0);
    kh = hash.kmerhash("ACTGA", 5);
    idx.add_record(min(kh.first, kh.second), 2, p, 0, 0);
    kh = hash.kmerhash("ACGTA", 5);
    idx.add_record(min(kh.first, kh.second), 4, p, 0, 0);
    idx.clear();
    uint32_t j = 0;
    EXPECT_EQ(j, idx.minhash.size());
}

TEST(IndexTest, save) {
    Index idx;
    KmerHash hash;
    deque<Interval> d = {Interval(3, 5), Interval(9, 12)};
    Path p;
    p.initialize(d);
    pair<uint64_t, uint64_t> kh = hash.kmerhash("ACGTA", 5);
    idx.add_record(min(kh.first, kh.second), 1, p, 0, 0);
    kh = hash.kmerhash("ACTGA", 5);
    idx.add_record(min(kh.first, kh.second), 2, p, 0, 0);
    kh = hash.kmerhash("ACGTA", 5);
    idx.add_record(min(kh.first, kh.second), 4, p, 0, 0);
    idx.save("indextext", 1, 5);
    ASSERT_TRUE(fopen("indextext.k5.w1.idx", "r") != NULL);
}

TEST(IndexTest, load) {
    Index idx1, idx2;
    KmerHash hash;
    deque<Interval> d = {Interval(3, 5), Interval(9, 12)};
    Path p;
    p.initialize(d);
    pair<uint64_t, uint64_t> kh1 = hash.kmerhash("ACGTA", 5);
    idx1.add_record(min(kh1.first, kh1.second), 1, p, 0, 0);
    pair<uint64_t, uint64_t> kh2 = hash.kmerhash("ACTGA", 5);
    idx1.add_record(min(kh2.first, kh2.second), 2, p, 0, 0);
    idx1.add_record(min(kh1.first, kh1.second), 4, p, 0, 0);

    idx2.load("indextext", 1, 5);
    EXPECT_EQ(idx1.minhash.size(), idx2.minhash.size());
    EXPECT_EQ(idx1.minhash[min(kh1.first, kh1.second)]->size(), idx2.minhash[min(kh1.first, kh1.second)]->size());
    EXPECT_EQ(idx1.minhash[min(kh2.first, kh2.second)]->size(), idx2.minhash[min(kh2.first, kh2.second)]->size());
    EXPECT_EQ(idx1.minhash[min(kh1.first, kh1.second)]->at(0), idx2.minhash[min(kh1.first, kh1.second)]->at(0));
    EXPECT_EQ(idx1.minhash[min(kh1.first, kh1.second)]->at(1), idx2.minhash[min(kh1.first, kh1.second)]->at(1));
    EXPECT_EQ(idx1.minhash[min(kh2.first, kh2.second)]->at(0), idx2.minhash[min(kh2.first, kh2.second)]->at(0));
}
