# Telemetry Run Report

Generated: 2024-01-15T10:30:05.000000

## Summary

- Duration: 1.900 s
- Packets received: 20
- Estimated packet loss: 0.00%
- Average packet rate: 10.00 Hz
- Status: FAULT

## ADC Statistics

- Minimum ADC: 1650 mV
- Maximum ADC: 2155 mV
- Average ADC: 1956.0 mV

## Fault Summary

| Fault | Count |
|-------|-------|
| ADC_OUT_OF_RANGE | 1 |

## Notes

Sample report generated from `data/sample_logs/sample_run.jsonl`.
Reproduce with:

```bash
python -m telemetry_client.cli report \
    --input data/sample_logs/sample_run.jsonl \
    --output reports/run_report.md
```
