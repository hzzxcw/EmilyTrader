#!/bin/bash

# 清理
rm -f *.log *.csv
BIN_DIR=build/macosx/arm64/release

echo "Starting System Engine (Journal Manager)..."
$BIN_DIR/engine config/engine.json > engine_stdout.log 2>&1 &
SYS_PID=$!
sleep 1

echo "Starting Event Logger..."
$BIN_DIR/event_logger config/event_logger.json > event_logger_stdout.log 2>&1 &
EV_PID=$!

echo "Starting Mock Exchange..."
$BIN_DIR/exchange config/exchange.json > exchange_stdout.log 2>&1 &
EXCH_PID=$!

echo "Starting Strategy Loader..."
$BIN_DIR/strategy_loader config/strategy.json > strategy_stdout.log 2>&1 &
STRAT_PID=$!

echo "Starting Simulator..."
$BIN_DIR/simulator config/simulator.json > simulator_stdout.log 2>&1 &
SIM_PID=$!

echo "Running modular system for 20 seconds..."
sleep 20

echo "Stopping all processes..."
kill $SIM_PID $STRAT_PID $EXCH_PID $EV_PID $SYS_PID

echo "Test finished. Checking results..."
echo "--- Latency Stats (First 10 lines) ---"
head -n 10 latency_stats.csv

echo "--- Trade Events (First 10 lines) ---"
head -n 10 trade_events.csv

echo "--- Market Data (First 10 lines) ---"
head -n 10 market_data.csv

echo "--- Exchange Log (Last 10 lines) ---"
tail -n 10 exchange.log

echo "--- Engine Log (Last 10 lines) ---"
tail -n 10 engine.log

echo "--- Strategy Output (Orderbook Snapshot) ---"
tail -n 20 strategy_stdout.log
