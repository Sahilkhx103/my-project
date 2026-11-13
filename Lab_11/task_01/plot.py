import serial
import matplotlib.pyplot as plt
from drawnow import *

# === Setup Serial ===
imuData = serial.Serial('/dev/ttyACM0', 115200)
imuData.reset_input_buffer()  # Discard stale bytes buffered before launch
plt.ion()

accAngles = []
gyroRates = []
filteredAngles = []
time_s = []
cnt = 0.0
dt = 0.02 

# === Plotting Function ===
def makeFig():
    plt.clf()
    plt.title('Live IMU Complementary Filter Data')
    plt.grid(True)
    plt.xlabel('Time (s)')
    plt.ylabel('Angle (deg) / Angular Rate (deg/s)')
    plt.ylim(-120, 120)
    plt.plot(time_s, accAngles, 'r.-', label='Accel Angle (deg)')
    plt.plot(time_s, gyroRates, 'g.-', label='Gyro Rate Y (deg/s)')
    plt.plot(time_s, filteredAngles, 'b-', linewidth=2, label='Filtered Angle (deg)')
    plt.legend(loc='upper left')

# === Main Loop ===
while True:
    while imuData.in_waiting == 0:
        pass

    try:
        updated = False
        # Drain every line currently waiting in the serial buffer
        while imuData.in_waiting > 0:
            line = imuData.readline().decode('utf-8', errors='ignore').strip()
            values = line.split(',')

            if len(values) == 3:
                accAngles.append(float(values[0]))
                gyroRates.append(float(values[1]))
                filteredAngles.append(float(values[2]))
                time_s.append(cnt)
                cnt += dt
                updated = True

                if len(accAngles) > 200:
                    accAngles.pop(0)
                    gyroRates.pop(0)
                    filteredAngles.pop(0)
                    time_s.pop(0)

        # Redraw only once after catching up to the latest sample
        if updated:
            drawnow(makeFig)
            plt.pause(0.001)

    except Exception as e:
        print("Error:", e)