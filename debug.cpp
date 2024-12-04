#include <SFML/Audio.hpp>
#include <iostream>

int main() {
    // Check if audio recording is supported
    if (!sf::SoundBufferRecorder::isAvailable()) {
        std::cerr << "Audio recording is not supported on this device.\n";
        return -1;
    }

    std::cout << "Audio recording is supported. Starting the recorder...\n";

    // Initialize the recorder
    sf::SoundBufferRecorder recorder;

    // Start recording
    recorder.start(44100);
    std::cout << "Recorder started. Speak or play a sound.\n";

    sf::sleep(sf::seconds(2)); // Record for 2 seconds

    recorder.stop();
    std::cout << "Recorder stopped. Processing the audio...\n";

    const sf::SoundBuffer& buffer = recorder.getBuffer();
    const sf::Int16* samples = buffer.getSamples();
    size_t sampleCount = buffer.getSampleCount();

    std::cout << "Captured " << sampleCount << " samples.\n";

    // Print first 10 samples
    if (sampleCount > 0) {
        std::cout << "First 10 samples:\n";
        for (size_t i = 0; i < std::min(sampleCount, size_t(10)); ++i) {
            std::cout << "Sample[" << i << "]: " << samples[i] << std::endl;
        }
    } else {
        std::cerr << "No audio captured. Check your microphone setup.\n";
    }

    return 0;
}

// g++ debug.cpp -o debug -I/opt/homebrew/opt/sfml/include -I/opt/homebrew/include -L/opt/homebrew/opt/sfml/lib -L/opt/homebrew/lib -lsfml-audio -lsfml-system -lsfml-window -lfftw3 -std=c++11