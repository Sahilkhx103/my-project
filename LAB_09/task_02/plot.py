import serial
import matplotlib.pyplot as plt
from drawnow import *

# === Setup Serial ===
sinWaveData = serial.Serial('/dev/ttyACM0', 115200)
plt.ion()  # Enable interactive mode

ax_vals, ay_vals, az_vals = [], [], []
time_ms = []
cnt = 0  # Time index (1 ms steps)

# === Plotting Function ===
def makeFig():
    plt.clf()
    plt.title('Live 6-Axis Sensor Data')
    plt.grid(True)
    plt.xlabel('Time (ms)')
    plt.ylabel('Accel (g)')
    plt.ylim(-2.5, 2.5)  # Adjust range as needed
    plt.plot(time_ms, ax_vals, 'r.-', label='Accel X')
    plt.plot(time_ms, ay_vals, 'g.-', label='Accel Y')
    plt.plot(time_ms, az_vals, 'b.-', label='Accel Z')
    plt.legend(loc='upper left')

# === Main Loop ===
while True:
    while sinWaveData.inWaiting() == 0:
        pass  # Wait for data

    try:
        line = sinWaveData.readline().decode().strip()
        values = line.split(',')

        if len(values) >= 6:
            ax_vals.append(float(values[0]))
            ay_vals.append(float(values[1]))
            az_vals.append(float(values[2]))
            time_ms.append(cnt)
            cnt += 1

            drawnow(makeFig)
            plt.pause(0.0001)

            # Keep only last 500 samples
            if len(adcValues) > 500:
                ax_vals.pop(0)
                ay_vals.pop(0)
                az_vals.pop(0)
                time_ms.pop(0)

    except Exception as e:
        print("Error:", e)
