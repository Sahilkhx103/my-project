import time
import csv
import serial
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Button

PORT = "/dev/ttyACM0" 
BAUD = 230400          
FS = 10000.0          
VREF = 3.3             
WINDOW_SAMPLES = 500   

ser = None
paused = False
high_byte_cache = None
ring = np.zeros(WINDOW_SAMPLES, dtype=np.float64)

samples_total = 0
rate_window_count = 0
rate_window_start = time.time()
measured_fs = 0.0


def connect_serial():
    global ser, high_byte_cache
    while True:
        try:
            s = serial.Serial(PORT, BAUD, timeout=0.05)
            s.reset_input_buffer()
            high_byte_cache = None
            print(f"[CONNECTED] {PORT} @ {BAUD} bps")
            return s
        except serial.SerialException:
            print(f"[WAITING] {PORT} not ready, retrying...")
            time.sleep(0.5)


def read_available_samples():
    global ser, high_byte_cache, samples_total, rate_window_count, rate_window_start, measured_fs

    if ser is None or not ser.is_open:
        ser = connect_serial()

    try:
        n = ser.in_waiting
        raw = ser.read(n if n > 0 else 64)
    except (serial.SerialException, OSError):
        print("[DISCONNECTED] Wire unplugged! Reconnecting...")
        try:
            ser.close()
        except Exception:
            pass
        ser = None
        high_byte_cache = None
        return np.array([], dtype=np.float64)

    if not raw:
        return np.array([], dtype=np.float64)

    voltages = []
    for b in raw:
        if b & 0x80:
            high_byte_cache = b & 0x3F
        elif high_byte_cache is not None:
            adc_val = (high_byte_cache << 6) | (b & 0x3F)
            voltages.append((adc_val / 4095.0) * VREF)
            high_byte_cache = None

    count = len(voltages)
    if count > 0:
        samples_total += count
        rate_window_count += count
        now = time.time()
        elapsed = now - rate_window_start
        if elapsed >= 1.0:
            measured_fs = rate_window_count / elapsed
            print(f"[1s CHECK] Samples/sec: {rate_window_count} | Fs: {measured_fs:.1f} Hz | Total: {samples_total}")
            rate_window_count = 0
            rate_window_start = now

    return np.array(voltages, dtype=np.float64)


def measure_waveform(y, fs):
    v_min, v_max = float(np.min(y)), float(np.max(y))
    v_pp = v_max - v_min
    v_avg = float(np.mean(y))
    if v_pp < 0.15:
        return v_pp, v_avg, 0.0, 0.0

    mid = 0.5 * (v_max + v_min)
    above = y > mid
    crossings = np.where((~above[:-1]) & (above[1:]))[0]
    if len(crossings) >= 2:
        exact_idx = []
        for idx in crossings:
            dy = y[idx + 1] - y[idx]
            frac = (mid - y[idx]) / dy if dy != 0 else 0.0
            exact_idx.append(idx + frac)
        avg_period_s = float(np.mean(np.diff(exact_idx))) / fs
        freq = 1.0 / avg_period_s if avg_period_s > 0 else 0.0
        return v_pp, v_avg, freq, avg_period_s * 1000.0
    return v_pp, v_avg, 0.0, 0.0


plt.style.use("dark_background")
fig, (ax_time, ax_fft) = plt.subplots(2, 1, figsize=(10, 6.5))
plt.subplots_adjust(bottom=0.16, hspace=0.38)

t_ms = np.arange(WINDOW_SAMPLES) * (1000.0 / FS)
line_time, = ax_time.plot(t_ms, ring, color="#00FF88", lw=1.6)
ax_time.set_ylim(-0.2, 3.5)
ax_time.set_xlim(0, t_ms[-1])
ax_time.set_ylabel("Voltage (V)")
ax_time.set_xlabel("Time (ms)")
ax_time.set_title("STM32F3 Real-Time Digital Oscilloscope (10 kHz)")
ax_time.grid(True, alpha=0.25)

freqs = np.fft.rfftfreq(WINDOW_SAMPLES, d=1.0 / FS)
line_fft, = ax_fft.plot(freqs, np.zeros_like(freqs), color="#38BDF8", lw=1.4)
ax_fft.set_xlim(0, 2500)
ax_fft.set_ylim(0, 2.0)
ax_fft.set_ylabel("Magnitude (V)")
ax_fft.set_xlabel("Frequency (Hz)")
ax_fft.set_title("Live FFT Spectrum")
ax_fft.grid(True, alpha=0.25)

metrics_text = ax_time.text(
    0.015, 0.92, "", transform=ax_time.transAxes,
    fontsize=10, color="#FACC15", verticalalignment="top",
    bbox=dict(boxstyle="round", facecolor="#1E293B", alpha=0.85)
)

ax_pause = plt.axes([0.62, 0.03, 0.14, 0.055])
ax_save = plt.axes([0.78, 0.03, 0.14, 0.055])
btn_pause = Button(ax_pause, "Pause / Run", color="#334155", hovercolor="#475569")
btn_save = Button(ax_save, "Save CSV", color="#334155", hovercolor="#475569")


def toggle_pause(event):
    global paused
    paused = not paused


def save_csv(event):
    fname = f"scope_capture_{int(time.time())}.csv"
    with open(fname, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["Time_ms", "Voltage_V"])
        for t_val, v_val in zip(t_ms, ring):
            writer.writerow([f"{t_val:.4f}", f"{v_val:.4f}"])
    print(f"[SAVED] {fname}")


btn_pause.on_clicked(toggle_pause)
btn_save.on_clicked(save_csv)

ser = connect_serial()
plt.ion()
plt.show()

try:
    while plt.fignum_exists(fig.number):
        new_samples = read_available_samples()
        if not paused and len(new_samples) > 0:
            if len(new_samples) >= WINDOW_SAMPLES:
                ring[:] = new_samples[-WINDOW_SAMPLES:]
            else:
                ring = np.roll(ring, -len(new_samples))
                ring[-len(new_samples):] = new_samples

            line_time.set_ydata(ring)
            ac_sig = ring - np.mean(ring)
            line_fft.set_ydata((2.0 / WINDOW_SAMPLES) * np.abs(np.fft.rfft(ac_sig)))

            v_pp, v_avg, freq, period_ms = measure_waveform(ring, FS)
            metrics_text.set_text(
                f"Freq: {freq:7.2f} Hz | Period: {period_ms:6.2f} ms | "
                f"Vpp: {v_pp:4.2f} V | Vavg: {v_avg:4.2f} V | Fs: {measured_fs:7.1f} S/s"
            )

        fig.canvas.draw_idle()
        fig.canvas.flush_events()
        time.sleep(0.015)
except KeyboardInterrupt:
    pass
finally:
    if ser and ser.is_open:
        ser.close()