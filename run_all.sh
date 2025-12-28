#!/bin/bash

# 清理
rm -f *.log *.csv
BIN_DIR=build/macosx/arm64/release

echo "Starting Event Logger..."
$BIN_DIR/event_logger > event_logger_stdout.log 2>&1 &
EV_PID=$!

echo "Starting Trading Engine..."
$BIN_DIR/trading_engine > trading_engine_stdout.log 2>&1 &
ENG_PID=$!

echo "Starting Simulator..."
$BIN_DIR/simulator > simulator_stdout.log 2>&1 &
SIM_PID=$!

echo "Running modular system for 10 seconds..."
sleep 10

echo "Stopping all processes..."
kill $SIM_PID $ENG_PID $EV_PID

echo "Test finished. Checking results..."
echo "--- Latency Stats (First 10 lines) ---"
head -n 10 latency_stats.csv

echo "--- Trade Events (First 10 lines) ---"
head -n 10 trade_events.csv

echo "--- Engine Log (Last 10 lines) ---"
tail -n 10 engine.log
