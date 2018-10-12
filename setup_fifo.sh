#!/bin/bash
TEST_DIR=/var/lib/test
mkdir -p $TEST_DIR
mkfifo $TEST_DIR/fifo0.in
mkfifo $TEST_DIR/fifo0.out
