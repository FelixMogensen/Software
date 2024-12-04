#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>

const int SAMPLE_RATE = 44100; // Standard sample rate
const double PI = 3.14159265358979323846; // Pi constant for calculations
const int AMPLITUDE = 30000; // Amplitude
const double DURATION = 0.4; // Tone duration in seconds

// Mapping for DTMF tones
std::map<char, std::pair<int, int>> dtmfFrequencies = {
    {'#', {941, 1477}}, {'*', {941, 1209}}, // Start and End Signals
    {'F(1)', {697, 1209}}, {'B(5)', {770, 1336}}, // Forward and Back
    {'L(9)', {852, 1477}}, {'R(0)', {941, 1336}}, // Left and Right
    {'0', {941, 1336}}, {'1', {697, 1209}}, {'2', {697, 1336}}, {'3', {697, 1477}}, 
    {'4', {770, 1209}}, {'5', {770, 1336}}, {'6', {770, 1477}}, {'7', {852, 1209}}, {'8', {852, 1336}}, {'9', {852, 1477}}  
};

// DTMF tone generation with debugging
void generateDTMFTone(char c, std::vector<sf::Int16>& samples, double durationInSeconds) {
    if (dtmfFrequencies.find(c) == dtmfFrequencies.end()) {
        std::cerr << "Error: Character '" << c << "' not found in DTMF frequencies map.\n";
        return;
    }

    auto frequencies = dtmfFrequencies[c];
    double sampleCount = SAMPLE_RATE * durationInSeconds;
    samples.resize(sampleCount);

    for (int i = 0; i < sampleCount; ++i) {
        double sample = 0.5 * (sin(2 * PI * frequencies.first * i / SAMPLE_RATE) +
                               sin(2 * PI * frequencies.second * i / SAMPLE_RATE));
        samples[i] = static_cast<sf::Int16>(AMPLITUDE * sample);
    }

    // Debugging, Log sample size
    if (samples.empty()) {
        std::cerr << "Error: Samples array is empty for character '" << c << "'.\n";
    } else {
        std::cout << "Generated " << samples.size() << " samples for character '" << c << "'.\n";
    }
}

// Compute checksum as the ASCII value of the command
int computeChecksum(char command) {
    return static_cast<int>(command);
}

// Convert checksum to DTMF-compatible string
std::string encodeChecksum(int checksum) {
    std::stringstream ss;
    ss << checksum; // Convert checksum to string
    return ss.str();
}

// Build the command message
std::string buildMessage(char command) {
    int checksum = computeChecksum(command);
    std::string checksumStr = encodeChecksum(checksum); // Convert checksum to string
    return "#" + std::string(1, command) + checksumStr + "*";
}

// Transmit message function with validation
void transmitMessage(const std::string& message, sf::SoundBuffer& buffer, sf::Sound& sound) {
    for (char c : message) {
        std::vector<sf::Int16> samples;
        generateDTMFTone(c, samples, DURATION);

        if (samples.empty()) {
            std::cerr << "Error: No samples generated for character '" << c << "'. Skipping...\n";
            continue;
        }

        if (buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE)) {
            sound.setBuffer(buffer);
            sound.play();
            sf::sleep(sf::seconds(DURATION)); // Ensure proper timing
        } else {
            std::cerr << "Failed to load sound buffer for character '" << c << "'.\n";
        }
    }
}

int main() {
    sf::SoundBuffer buffer;
    sf::Sound sound;

    std::cout << "Use arrow keys to control the robot. Press 'Q' to quit." << std::endl;

    while (true) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) break;

        char command = '\0';

        // Check for arrow key presses
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) command = 'F';
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) command = 'B';
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) command = 'L';
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) command = 'R';

        if (command != '\0') {
            std::string message = buildMessage(command);
            std::cout << "Sending command: " << message << std::endl;

            transmitMessage(message, buffer, sound);

            // Wait until the key is released to prevent repeated transmissions
            while (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            }
        }
    }

    std::cout << "Program ended." << std::endl;
    return 0;
}

// Compile command on mac:
// g++ DTMF2.cpp -o DTMF2 -I/opt/homebrew/opt/sfml/include -L/opt/homebrew/opt/sfml/lib -lsfml-audio -lsfml-system -lsfml-window -std=c++11