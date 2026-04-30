import csv
import matplotlib.pyplot as plt
import numpy as np

def load_csv(filename):
    time = []
    values = []
    with open(filename, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            time.append(float(row[0]))
            values.append(float(row[1]))
    return time, values

signals = {
    'sigA': load_csv('sigA.csv'),
    'sigB': load_csv('sigB.csv'),
    'sigC': load_csv('sigC.csv'),
    'sigD': load_csv('sigD.csv'),
}

tA, vA = signals['sigA']
tB, vB = signals['sigB']
tC, vC = signals['sigC']
tD, vD = signals['sigD']



for name, (t, v) in signals.items():
    n = len(v)
    total_time = t[-1] - t[0]
    Fs = n / total_time
    Ts = 1.0 / Fs
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n/2))]
    Y = np.fft.fft(v) / n
    Y = Y[range(int(n/2))]

    # fig, (ax1, ax2) = plt.subplots(2, 1)
    # ax1.plot(t, v, 'b')
    
    # ax1.set_xlabel('Time (s)')
    # ax1.set_ylabel('Amplitude')
    # ax1.set_title(name + ' - Signal vs Time')
    # ax2.plot(frq, abs(Y), 'b')
    # ax2.set_xlim([0, 100])
    # ax2.set_xlabel('Freq (Hz)')
    # ax2.set_ylabel('|Y(freq)|')
    # ax2.set_title(name + ' - FFT')
    # plt.show()

## MAF filter ##

X_values = {'sigA': 400, 'sigB': 110, 'sigC': 50, 'sigD': 7}

for name, (t, v) in signals.items():
    X = X_values[name]
    v_maf = []

    for i in range(len(v)):
        if i < X:
            v_maf.append(v[i])
        else:
            window = v[i-X:i]
            v_maf.append(sum(window) / X)


    # time domain plot
    plt.figure()
    plt.plot(t, v, 'k')
    plt.plot(t, v_maf, 'r')
    plt.title(name + ' MAF - X = ' + str(X))
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.savefig(name + '_maf.png')
    plt.show()

    # FFT of unfiltered
    n = len(v)
    total_time = t[-1] - t[0]
    Fs = n / total_time
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n/2))]
    Y = np.fft.fft(v) / n
    Y = Y[range(int(n/2))]

    # FFT of filtered
    n_maf = len(v_maf)
    k_maf = np.arange(n_maf)
    T_maf = n_maf / Fs
    frq_maf = k_maf / T_maf
    frq_maf = frq_maf[range(int(n_maf/2))]
    Y_maf = np.fft.fft(v_maf) / n_maf
    Y_maf = Y_maf[range(int(n_maf/2))]

    # FFT comparison plot
    plt.figure()
    plt.plot(frq, abs(Y), 'k')
    plt.plot(frq_maf, abs(Y_maf), 'r')
    plt.xlim([0, 100])
    plt.title(name + ' MAF FFT comparison - X = ' + str(X))
    plt.xlabel('Freq (Hz)')
    plt.ylabel('|Y(freq)|')
    plt.savefig(name + '_maf_fft.png')
    plt.show()


## IIR filter ##

A_values = {'sigA': 0.99, 'sigB': 0.95, 'sigC': 0.85, 'sigD': 0.7}

for name, (t, v) in signals.items():
    A = A_values[name]
    B = 1 - A
    v_iir = []

    for i in range(len(v)):
        if i == 0:
            v_iir.append(v[0])
        else:
            v_iir.append(A * v_iir[i-1] + B * v[i])

    # time domain plot
    plt.figure()
    plt.plot(t, v, 'k')
    plt.plot(t, v_iir, 'r')
    plt.title(name + ' IIR - A = ' + str(A) + ', B = ' + str(round(B, 3)))
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.savefig(name + '_iir.png')
    plt.show()

    # FFT of unfiltered
    n = len(v)
    total_time = t[-1] - t[0]
    Fs = n / total_time
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n/2))]
    Y = np.fft.fft(v) / n
    Y = Y[range(int(n/2))]

    # FFT of filtered
    n_iir = len(v_iir)
    k_iir = np.arange(n_iir)
    T_iir = n_iir / Fs
    frq_iir = k_iir / T_iir
    frq_iir = frq_iir[range(int(n_iir/2))]
    Y_iir = np.fft.fft(v_iir) / n_iir
    Y_iir = Y_iir[range(int(n_iir/2))]

    # FFT comparison plot
    plt.figure()
    plt.plot(frq, abs(Y), 'k')
    plt.plot(frq_iir, abs(Y_iir), 'r')
    plt.xlim([0, 100])
    plt.title(name + ' IIR FFT comparison - A = ' + str(A) + ', B = ' + str(round(B, 3)))
    plt.xlabel('Freq (Hz)')
    plt.ylabel('|Y(freq)|')
    plt.savefig(name + '_iir_fft.png')
    plt.show()


## FIR filter ##

cutoff_values = {'sigA': 5, 'sigB': 5, 'sigC': 1, 'sigD': 2}
num_taps = 79

for name, (t, v) in signals.items():
    n = len(v)
    total_time = t[-1] - t[0]
    Fs = n / total_time

    cutoff = cutoff_values[name]

    # design filter
    h = np.sinc(2 * cutoff / Fs * (np.arange(num_taps) - (num_taps - 1) / 2))
    h *= np.blackman(num_taps)
    h /= np.sum(h)

    # apply filter
    v_fir = []
    for i in range(len(v)):
        acc = 0.0
        for j in range(num_taps):
            if i - j >= 0:
                acc = acc + h[j] * v[i - j]
        v_fir.append(acc)

    # time domain plot
    plt.figure()
    plt.plot(t, v, 'k')
    plt.plot(t, v_fir, 'r')
    plt.title(name + ' FIR - ' + str(num_taps) + ' taps, Blackman, cutoff=' + str(cutoff) + 'Hz')
    plt.xlabel('Time (s)')
    plt.ylabel('Amplitude')
    plt.savefig(name + '_fir.png')
    plt.show()

    # FFT of unfiltered
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n/2))]
    Y = np.fft.fft(v) / n
    Y = Y[range(int(n/2))]

    # FFT of filtered
    n_fir = len(v_fir)
    k_fir = np.arange(n_fir)
    T_fir = n_fir / Fs
    frq_fir = k_fir / T_fir
    frq_fir = frq_fir[range(int(n_fir/2))]
    Y_fir = np.fft.fft(v_fir) / n_fir
    Y_fir = Y_fir[range(int(n_fir/2))]

    # FFT comparison plot
    plt.figure()
    plt.plot(frq, abs(Y), 'k')
    plt.plot(frq_fir, abs(Y_fir), 'r')
    plt.xlim([0, 100])
    plt.title(name + ' FIR FFT - ' + str(num_taps) + ' taps, Blackman, cutoff=' + str(cutoff) + 'Hz')
    plt.xlabel('Freq (Hz)')
    plt.ylabel('|Y(freq)|')
    plt.savefig(name + '_fir_fft.png')
    plt.show()