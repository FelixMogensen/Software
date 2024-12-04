#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <chrono>
#include <fftw3.h>

const int SAMPLE_RATE = 44100; // Audio sample rate
const int N = 4096;            // FFT size
const double PI = 3.14159265358979323846;

// DTMF Frequencies and Characters
std::map<std::pair<int, int>, char> dtmfMap = {
    {{697, 1209}, '1'}, {{697, 1336}, '2'}, {{697, 1477}, '3'},
    {{770, 1209}, '4'}, {{770, 1336}, '5'}, {{770, 1477}, '6'},
    {{852, 1209}, '7'}, {{852, 1336}, '8'}, {{852, 1477}, '9'},
    {{941, 1209}, '*'}, {{941, 1336}, '0'}, {{941, 1477}, '#'}
};

// Perform FFT using FFTW
std::vector<double> performFFT(const std::vector<sf::Int16>& samples) {
    double* in = fftw_alloc_real(N);
    fftw_complex* out = fftw_alloc_complex(N / 2 + 1);

    for (int i = 0; i < N; ++i) {
        in[i] = (i < samples.size()) ? samples[i] : 0.0;
    }

    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    std::vector<double> magnitudes(N / 2);
    for (int i = 0; i < N / 2; ++i) {
        magnitudes[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return magnitudes;
}

// Find the two strongest frequencies above 500 Hz
std::pair<int, int> findStrongestFrequencies(const std::vector<double>& magnitudes) {
    int peak1 = 0, peak2 = 0;

    for (int i = 1; i < magnitudes.size(); ++i) {
        int freq = i * SAMPLE_RATE / N;
        if (freq < 500) continue; // Skip frequencies below 500 Hz

        if (magnitudes[i] > magnitudes[peak1]) {
            peak2 = peak1;
            peak1 = i;
        } else if (magnitudes[i] > magnitudes[peak2]) {
            peak2 = i;
        }
    }

    int freq1 = peak1 * SAMPLE_RATE / N;
    int freq2 = peak2 * SAMPLE_RATE / N;

    return {freq1, freq2};
}

// Match frequencies to DTMF characters
char detectDTMF(const std::pair<int, int>& freqs) {
    for (const auto& pair : dtmfMap) {
        if ((std::abs(freqs.first - pair.first.first) < 20 && std::abs(freqs.second - pair.first.second) < 20) ||
            (std::abs(freqs.first - pair.first.second) < 20 && std::abs(freqs.second - pair.first.first) < 20)) {
            return pair.second;
        }
    }
    return '\0'; // No match found
}

// Custom recorder class
class DTMFRecorder : public sf::SoundRecorder {
private:
    std::chrono::steady_clock::time_point lastFeedbackTime;

public:
    DTMFRecorder() {
        lastFeedbackTime = std::chrono::steady_clock::now();
    }

protected:
    bool onProcessSamples(const sf::Int16* samples, std::size_t sampleCount) override {
        if (sampleCount == 0) return true;

        // Calculate loudness (RMS)
        double rms = 0.0;
        for (std::size_t i = 0; i < sampleCount; ++i) {
            rms += samples[i] * samples[i];
        }
        rms = sqrt(rms / sampleCount);

        // Skip if signal is too quiet
        if (rms < 1000) {
            // Provide periodic feedback about quiet signals
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastFeedbackTime).count() >= 3) {
                std::cout << "Signal too quiet. RMS: " << rms << std::endl;
                lastFeedbackTime = now;
            }
            return true;
        }

        // Convert samples to vector
        std::vector<sf::Int16> sampleVector(samples, samples + std::min(sampleCount, std::size_t(N)));

        // Perform FFT
        std::vector<double> magnitudes = performFFT(sampleVector);

        // Find strongest frequencies
        std::pair<int, int> strongestFreqs = findStrongestFrequencies(magnitudes);

        // Detect DTMF tone
        char detectedChar = detectDTMF(strongestFreqs);
        if (detectedChar != '\0') {
            std::cout << "Detected DTMF tone: " << detectedChar << std::endl;
        } else {
            // Provide periodic feedback about non-DTMF signals
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastFeedbackTime).count() >= 3) {
                std::cout << "Strongest Frequencies: " << strongestFreqs.first << " Hz, " << strongestFreqs.second << " Hz\n";
                lastFeedbackTime = now;
            }
        }

        return true; // Continue recording
    }
};

int main() {
    DTMFRecorder recorder;

    if (!recorder.start(SAMPLE_RATE)) {
        std::cerr << "Failed to start audio recording.\n";
        return -1;
    }

    std::cout << "Listening for DTMF tones...\n";
    while (true) {
        sf::sleep(sf::milliseconds(500));
    }

    recorder.stop();
    return 0;
}

// g++ DTMF5.cpp -o DTMF5 -I/opt/homebrew/opt/sfml/include -I/opt/homebrew/include -L/opt/homebrew/opt/sfml/lib -L/opt/homebrew/lib -lsfml-audio -lsfml-system -lsfml-window -lfftw3 -std=c++11