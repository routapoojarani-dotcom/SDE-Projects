import pytest
from telemetry_client.stats import (
    compute_packet_loss,
    compute_average_rate_hz,
    summarize_faults,
    summarize_adc_range,
)


def make_packets(seqs, time_step=100, adc=1650, faults=None):
    return [
        {
            "seq": seq,
            "time_ms": i * time_step,
            "adc_mv": adc,
            "faults": faults or [],
        }
        for i, seq in enumerate(seqs)
    ]


def test_packet_loss_zero_when_continuous():
    packets = make_packets([0, 1, 2, 3, 4])
    assert compute_packet_loss(packets) == 0.0


def test_packet_loss_nonzero_when_gap():
    # seqs 0, 2 — seq 1 is missing: 1 lost out of 3 expected
    packets = make_packets([0, 2])
    loss = compute_packet_loss(packets)
    assert loss > 0.0
    assert abs(loss - 1 / 3) < 0.001


def test_average_rate_hz():
    # 11 packets spanning 1000 ms -> 10 intervals -> 10.0 Hz
    packets = make_packets(range(11), time_step=100)
    rate = compute_average_rate_hz(packets)
    assert abs(rate - 10.0) < 0.01


def test_summarize_faults():
    packets = [
        {"seq": 0, "time_ms": 0,   "adc_mv": 1650, "faults": ["ADC_OUT_OF_RANGE"]},
        {"seq": 1, "time_ms": 100, "adc_mv": 1650, "faults": ["ADC_OUT_OF_RANGE", "BUTTON_EVENT"]},
        {"seq": 2, "time_ms": 200, "adc_mv": 1650, "faults": []},
    ]
    result = summarize_faults(packets)
    assert result["ADC_OUT_OF_RANGE"] == 2
    assert result["BUTTON_EVENT"] == 1


def test_summarize_adc_range():
    packets = [
        {"seq": 0, "time_ms": 0,   "adc_mv": 100},
        {"seq": 1, "time_ms": 100, "adc_mv": 200},
        {"seq": 2, "time_ms": 200, "adc_mv": 300},
    ]
    result = summarize_adc_range(packets)
    assert result["min"] == 100
    assert result["max"] == 300
    assert abs(result["mean"] - 200.0) < 0.01
