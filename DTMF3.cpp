#include <SFML/Audio.hpp>
#include <cmath>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

// Constants
const int SAMPLE_RATE = 44100;
const double PI = 3.14159265358979323846;

// DTMF Frequencies
std::map<std::pair<int, int>, char> dtmfMap = {
    {{697, 1209}, '1'}, {{697, 1336}, '2'}, {{697, 1477}, '3'},
    {{770, 1209}, '4'}, {{770, 1336}, '5'}, {{770, 1477}, '6'},
    {{852, 1209}, '7'}, {{852, 1336}, '8'}, {{852, 1477}, '9'},
    {{941, 1209}, '*'}, {{941, 1336}, '0'}, {{941, 1477}, '#'},
    {{697, 1209}, 'F'}, {{770, 1336}, 'B'}, {{852, 1477}, 'L'}, {{941, 1336}, 'R'}
};

// Perform FFT (you may use a library like kissFFT for real implementation)
std::vector<double> performFFT(const std::vector<sf::Int16>& samples, int sampleRate) {
    // Placeholder for FFT logic. Replace with an actual FFT implementation.
    std::vector<double> frequencies; // Detected frequencies
    // Perform FFT here to analyze the audio samples and populate `frequencies`.
    return frequencies;
}

// Match frequencies to DTMF characters
char detectDTMF(const std::vector<double>& detectedFrequencies) {
    for (const auto& pair : dtmfMap) {
        int freq1 = pair.first.first;
        int freq2 = pair.first.second;

        if (std::find(detectedFrequencies.begin(), detectedFrequencies.end(), freq1) != detectedFrequencies.end() &&
            std::find(detectedFrequencies.begin(), detectedFrequencies.end(), freq2) != detectedFrequencies.end()) {
            return pair.second; // Return the corresponding DTMF character
        }
    }
    return '\0'; // Return null character if no match is found
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
        sf::sleep(sf::milliseconds(100)); // Polling interval

        // Get recorded samples
        const sf::SoundBuffer& buffer = recorder.getBuffer();
        const sf::Int16* samples = buffer.getSamples();
        size_t sampleCount = buffer.getSampleCount();

        // Convert samples into a vector for processing
        std::vector<sf::Int16> sampleVector(samples, samples + sampleCount);

        // Perform FFT to find frequencies
        std::vector<double> detectedFrequencies = performFFT(sampleVector, SAMPLE_RATE);

        // Detect DTMF tone
        char detectedChar = detectDTMF(detectedFrequencies);
        if (detectedChar != '\0') {
            std::cout << "Detected DTMF character: " << detectedChar << std::endl;
        }
    }

    recorder.stop();
    return 0;
}

// g++ DTMF3.cpp -o DTMF3 -I/opt/homebrew/opt/sfml/include -L/opt/homebrew/opt/sfml/lib -lsfml-audio -lsfml-system -lsfml-window -std=c++11
