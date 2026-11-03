import pytest
from telemetry_client.report_generator import generate_report
from telemetry_client.simulator import generate_simulated_packet


def make_test_packets(n=50):
    return [generate_simulated_packet(i, i * 20) for i in range(n)]


def test_report_file_created(tmp_path):
    path = str(tmp_path / "report.md")
    generate_report(make_test_packets(), path)
    assert (tmp_path / "report.md").exists()


def test_report_contains_packet_count(tmp_path):
    path = str(tmp_path / "report.md")
    generate_report(make_test_packets(10), path)
    content = (tmp_path / "report.md").read_text()
    assert "Packets received: 10" in content


def test_report_contains_packet_loss(tmp_path):
    path = str(tmp_path / "report.md")
    generate_report(make_test_packets(), path)
    content = (tmp_path / "report.md").read_text()
    assert "packet loss" in content.lower()


def test_report_contains_fault_summary(tmp_path):
    path = str(tmp_path / "report.md")
    generate_report(make_test_packets(40), path)
    content = (tmp_path / "report.md").read_text()
    assert "Fault Summary" in content


def test_report_contains_adc_statistics(tmp_path):
    path = str(tmp_path / "report.md")
    generate_report(make_test_packets(), path)
    content = (tmp_path / "report.md").read_text()
    assert "ADC Statistics" in content
