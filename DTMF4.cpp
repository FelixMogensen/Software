#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fftw3.h> // Include FFTW header

const int SAMPLE_RATE = 44100; // Audio sample rate
const int N = 2048;            // Number of FFT samples
const double PI = 3.14159265358979323846;

// DTMF Frequencies
std::map<std::pair<int, int>, char> dtmfMap = {
    {{697, 1209}, '1'}, {{697, 1336}, '2'}, {{697, 1477}, '3'},
    {{770, 1209}, '4'}, {{770, 1336}, '5'}, {{770, 1477}, '6'},
    {{852, 1209}, '7'}, {{852, 1336}, '8'}, {{852, 1477}, '9'},
    {{941, 1209}, '*'}, {{941, 1336}, '0'}, {{941, 1477}, '#'},
    {{697, 1209}, 'F'}, {{770, 1336}, 'B'}, {{852, 1477}, 'L'}, {{941, 1336}, 'R'}
};

// Perform FFT using FFTW
std::vector<double> performFFT(const std::vector<sf::Int16>& samples) {
    // Prepare input and output buffers
    double* in = fftw_alloc_real(N);
    fftw_complex* out = fftw_alloc_complex(N / 2 + 1);

    // Copy samples to input buffer, zero-padding if necessary
    for (int i = 0; i < N; ++i) {
        in[i] = (i < samples.size()) ? samples[i] : 0.0;
    }

    // Create FFTW plan
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(plan); // Perform FFT

    // Compute magnitudes
    std::vector<double> magnitudes(N / 2);
    for (int i = 0; i < N / 2; ++i) {
        magnitudes[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]); // sqrt(re^2 + im^2)
    }

    // Clean up
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return magnitudes;
}

// Find the two strongest frequencies
std::pair<int, int> findStrongestFrequencies(const std::vector<double>& magnitudes) {
    int peak1 = 0, peak2 = 0;

    for (int i = 1; i < magnitudes.size(); ++i) {
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

int main() {
    // Initialize audio capture
    sf::SoundBufferRecorder recorder;
    if (!sf::SoundBufferRecorder::isAvailable()) {
        std::cerr << "Audio recording is not supported on this device.\n";
        return -1;
    }

    std::cout << "Listening for DTMF tones. Press Ctrl+C to quit.\n";

    recorder.start(SAMPLE_RATE);

    while (true) {
        sf::sleep(sf::milliseconds(500)); // Polling interval

        // Get recorded samples
        const sf::SoundBuffer& buffer = recorder.getBuffer();
        const sf::Int16* samples = buffer.getSamples();
        size_t sampleCount = buffer.getSampleCount();
        
        // Debugging: Print a few audio samples
        for (size_t i = 0; i < std::min(sampleCount, size_t(10)); ++i) {
            std::cout << "Sample[" << i << "]: " << samples[i] << std::endl;
        }

        // Convert samples into a vector for processing
        std::vector<sf::Int16> sampleVector(samples, samples + std::min(sampleCount, size_t(N)));

        // Perform FFT
        std::vector<double> magnitudes = performFFT(sampleVector);

        // Find strongest frequencies
        std::pair<int, int> strongestFreqs = findStrongestFrequencies(magnitudes);

        // Detect DTMF tone
        char detectedChar = detectDTMF(strongestFreqs);
        if (detectedChar != '\0') {
            std::cout << "Detected DTMF character: " << detectedChar << std::endl;
        }
    }

    recorder.stop();
    return 0;
}

// g++ DTMF4.cpp -o DTMF4 -I/opt/homebrew/opt/sfml/include -I/opt/homebrew/include -L/opt/homebrew/opt/sfml/lib -L/opt/homebrew/lib -lsfml-audio -lsfml-system -lsfml-window -lfftw3 -std=c++11
