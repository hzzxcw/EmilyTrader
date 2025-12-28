#!/bin/bash

CSV_FILE="latency_stats.csv"

if [ ! -f "$CSV_FILE" ]; then
    echo "Error: $CSV_FILE not found."
    exit 1
fi

echo "=== EmilyTrader Tick-to-Trade (T2T) Analysis ==="

awk -F',' '
NR > 1 {
    t2 = $5;
    t3 = $6;
    if (t2 > 0 && t3 > 0) {
        diff = t3 - t2;
        sum += diff;
        sumsq += diff*diff;
        count++;
        if (min == "" || diff < min) min = diff;
        if (diff > max) max = diff;
        
        # Store for P99 calculation
        values[count] = diff;
    }
}
END {
    if (count > 0) {
        avg = sum / count;
        stddev = sqrt(sumsq / count - avg*avg);
        
        # Simple Sort for Quantiles
        for (i = 1; i <= count; i++) {
            for (j = i + 1; j <= count; j++) {
                if (values[i] > values[j]) {
                    tmp = values[i];
                    values[i] = values[j];
                    values[j] = tmp;
                }
            }
        }
        
        p50 = values[int(count * 0.50) + 1];
        p90 = values[int(count * 0.90) + 1];
        p99 = values[int(count * 0.99) + 1];

        printf "Sample Count: %d\n", count;
        printf "Average T2T:  %.2f ns\n", avg;
        printf "Min T2T:      %d ns\n", min;
        printf "Max T2T:      %d ns\n", max;
        printf "P50 T2T:      %d ns\n", p50;
        printf "P90 T2T:      %d ns\n", p90;
        printf "P99 T2T:      %d ns\n", p99;
        printf "Std Dev:      %.2f ns\n", stddev;
    } else {
        echo "No valid samples found.";
    }
}' "$CSV_FILE"
