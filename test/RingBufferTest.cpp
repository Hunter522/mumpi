#include <iostream>
#include <algorithm>
#include "gtest/gtest.h"
#include "RingBuffer.hpp"


static const int NUM_ELEMENTS = 10;

/**
 * @brief Test fixture for RingBuffer
 */
class RingBufferTest : public ::testing::Test {
protected:
	RingBufferTest() {
	}

	virtual ~RingBufferTest() {
	}

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
		_pRingBuffer = new RingBuffer<int>(NUM_ELEMENTS);
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
		delete _pRingBuffer;
	}

	// Objects declared here can be used by all tests in the test case for Foo.
	RingBuffer<int> *_pRingBuffer;
};



TEST_F(RingBufferTest, TestPush) {
	_pRingBuffer->push(0);
	_pRingBuffer->push(1);
	_pRingBuffer->push(2);
	ASSERT_EQ(NUM_ELEMENTS, _pRingBuffer->getSize());
	ASSERT_EQ(3, _pRingBuffer->getRemaining());
	ASSERT_EQ(0, _pRingBuffer->top());
	ASSERT_EQ(1, _pRingBuffer->top());
	ASSERT_EQ(2, _pRingBuffer->top());
	ASSERT_EQ(0, _pRingBuffer->getRemaining());
}

TEST_F(RingBufferTest, TestPushBulk) {
	const int TEMP_BUF_SIZE = 5;
	std::array<int, TEMP_BUF_SIZE> tempBuf;
	std::iota(tempBuf.begin(), tempBuf.end(), 0);

	_pRingBuffer->push(tempBuf.begin(), 0, TEMP_BUF_SIZE);
	ASSERT_EQ(NUM_ELEMENTS, _pRingBuffer->getSize());
	ASSERT_EQ(TEMP_BUF_SIZE, _pRingBuffer->getRemaining());
	for(auto&& i : tempBuf) {
		printf("_pRingBuffer[i]: %d\n", _pRingBuffer->top());
		// ASSERT_EQ(i, _pRingBuffer->top());
	}
	ASSERT_EQ(0, _pRingBuffer->getRemaining());
}

TEST_F(RingBufferTest, TestTopEmpty) {
	ASSERT_TRUE(_pRingBuffer->isEmpty());
	bool caughtException = false;
	try {
		_pRingBuffer->top();
	} catch(EmptyBufferException &e) {
		caughtException = true;
	}
	ASSERT_TRUE(caughtException);
}

TEST_F(RingBufferTest, TestTopBulk) {
	const int TEMP_BUF_SIZE = 5;
	std::array<int, TEMP_BUF_SIZE> tempBuf;
	std::iota(tempBuf.begin(), tempBuf.end(), 0);

	_pRingBuffer->push(tempBuf.begin(), 0, TEMP_BUF_SIZE);
	ASSERT_EQ(NUM_ELEMENTS, _pRingBuffer->getSize());
	ASSERT_EQ(TEMP_BUF_SIZE, _pRingBuffer->getRemaining());
	tempBuf.fill(0);
	_pRingBuffer->top(tempBuf.begin(), 0, TEMP_BUF_SIZE);
	ASSERT_EQ(0, _pRingBuffer->getRemaining());
}

TEST_F(RingBufferTest, TestTopRemaining) {
	std::array<int, 5> tempBuf = {{1,2,3,4,5}};

	_pRingBuffer->push(tempBuf.begin(), 0, 5);
	ASSERT_EQ(NUM_ELEMENTS, _pRingBuffer->getSize());
	ASSERT_EQ(5, _pRingBuffer->getRemaining());
	tempBuf.fill(0);
	const unsigned int elementsRetrieved = _pRingBuffer->topRemaining(tempBuf.begin());
	ASSERT_EQ(5, elementsRetrieved);
	ASSERT_EQ(0, _pRingBuffer->getRemaining());
}

TEST_F(RingBufferTest, TestWrapNoOverwrite) {
	// set up to wrap 2
	std::array<int, 12> tempBuf;
	std::iota(tempBuf.begin(), tempBuf.end(), 0);

	// push 5 in, front should be 0, back should be 5
	_pRingBuffer->push(tempBuf.begin(), 0, 5);
	ASSERT_EQ(5, _pRingBuffer->getRemaining());

	// top 5 off, front should be 5, back should be 5, buffer should be empty
	for(int i = 0; i < 5; i++) {
		ASSERT_EQ(tempBuf[i], _pRingBuffer->top());
	}
	ASSERT_TRUE(_pRingBuffer->isEmpty());

	// push 7 in, should wrap 2 and back should be at idx 3
	_pRingBuffer->push(tempBuf.begin(), 5, 7);
	for(int i = 0; i < 7; i++) {
		ASSERT_EQ(tempBuf[5+i], _pRingBuffer->top());
	}
}

TEST_F(RingBufferTest, TestWrapOverwrite) {
	// 0,1,2,3,4,0,0,0,0,0
	// 10,11,2,3,4,5,6,7,8,9

	// set up to wrap 2
	std::array<int, 12> tempBuf;
	std::iota(tempBuf.begin(), tempBuf.end(), 0);

	// push 5 in, front should be 0, back should be 5
	_pRingBuffer->push(tempBuf.begin(), 0, 5);
	ASSERT_EQ(5, _pRingBuffer->getRemaining());

	ASSERT_FALSE(_pRingBuffer->isEmpty());


	// push 7 in, should wrap 2, overwrite 2 and back should be at idx 3
	_pRingBuffer->push(tempBuf.begin(), 5, 7);
	for(unsigned i = 2; i < 12 && !_pRingBuffer->isEmpty(); i++) {
		ASSERT_EQ(tempBuf[i], _pRingBuffer->top());
	}
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
