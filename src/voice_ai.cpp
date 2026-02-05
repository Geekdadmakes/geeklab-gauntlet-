/**
 * Voice AI - Wake Word Detection and Command Processing
 * Handles voice interaction with hybrid offline/online processing
 */

#include "voice_ai.h"

VoiceAI::VoiceAI()
    : currentState(VOICE_IDLE)
    , audioBuffer(nullptr)
    , bufferSize(0)
    , listening(false)
    , wakeWordCallback(nullptr)
    , commandCallback(nullptr) {
}

bool VoiceAI::begin() {
    Serial.println("Initializing Voice AI...");

    // Allocate audio buffer (5 seconds at 16kHz, 16-bit mono)
    bufferSize = SAMPLE_RATE * (RECORDING_DURATION / 1000);
    audioBuffer = new int16_t[bufferSize];

    if (!audioBuffer) {
        Serial.println("Failed to allocate audio buffer");
        return false;
    }

    initMicrophone();
    initSpeaker();

    Serial.println("Voice AI initialized");
    return true;
}

void VoiceAI::update() {
    if (listening) {
        recordAudio();

        // Check for wake word
        if (detectWakeWord(audioBuffer, bufferSize)) {
            if (wakeWordCallback) {
                wakeWordCallback();
            }
        }
    }
}

bool VoiceAI::isWakeWordDetected() {
    return currentState == VOICE_LISTENING;
}

void VoiceAI::startListening() {
    listening = true;
    currentState = VOICE_LISTENING;
    Serial.println("Started listening for voice commands");
}

void VoiceAI::stopListening() {
    listening = false;
    currentState = VOICE_IDLE;
    Serial.println("Stopped listening");
}

VoiceCommand VoiceAI::processAudio(const int16_t* audioBuffer, size_t samples) {
    VoiceCommand cmd;
    cmd.type = CMD_UNKNOWN;
    cmd.needsCloud = false;

    // Extract command text from audio
    // In real implementation, this would use speech recognition
    // For now, we'll use a placeholder
    extractCommand(audioBuffer, samples, cmd.raw);

    // Parse the command
    cmd.type = parseCommand(cmd.raw);

    // Determine if cloud processing is needed
    if (cmd.type == CMD_CUSTOM_QUERY || cmd.type == CMD_WEATHER) {
        cmd.needsCloud = true;
    }

    return cmd;
}

CommandType VoiceAI::parseCommand(const char* text) {
    // Simple keyword matching for local commands
    // Convert to lowercase for comparison
    String textStr = String(text);
    textStr.toLowerCase();

    if (textStr.indexOf("message") >= 0 || textStr.indexOf("text") >= 0) {
        return CMD_CHECK_MESSAGES;
    } else if (textStr.indexOf("notification") >= 0) {
        return CMD_SHOW_NOTIFICATIONS;
    } else if (textStr.indexOf("light") >= 0 || textStr.indexOf("turn on") >= 0 || textStr.indexOf("turn off") >= 0) {
        return CMD_DEVICE_CONTROL;
    } else if (textStr.indexOf("time") >= 0 || textStr.indexOf("clock") >= 0) {
        return CMD_TIME_QUERY;
    } else if (textStr.indexOf("battery") >= 0 || textStr.indexOf("power") >= 0) {
        return CMD_BATTERY_STATUS;
    } else if (textStr.indexOf("weather") >= 0) {
        return CMD_WEATHER;
    } else {
        return CMD_CUSTOM_QUERY;
    }
}

void VoiceAI::executeCommand(const VoiceCommand& cmd) {
    if (commandCallback) {
        commandCallback(cmd);
    }
}

void VoiceAI::sendToCloud(const char* query) {
    Serial.print("Sending to cloud: ");
    Serial.println(query);

    // In real implementation, this would:
    // 1. Send audio/text to iPhone via BLE
    // 2. iPhone relays to OpenAI/Siri
    // 3. Response comes back via BLE
    // 4. Display or speak response

    currentState = VOICE_PROCESSING;
}

void VoiceAI::handleCloudResponse(const char* response) {
    Serial.print("Cloud response: ");
    Serial.println(response);

    speak(response);
    currentState = VOICE_RESPONDING;
}

void VoiceAI::speak(const char* text) {
    Serial.print("Speaking: ");
    Serial.println(text);

    // In real implementation, this would:
    // 1. Use local TTS if available
    // 2. Or relay through phone for Siri TTS
    // 3. Play audio through speaker

    currentState = VOICE_RESPONDING;
}

VoiceState VoiceAI::getState() {
    return currentState;
}

void VoiceAI::setState(VoiceState state) {
    currentState = state;
}

void VoiceAI::onWakeWord(void (*callback)()) {
    wakeWordCallback = callback;
}

void VoiceAI::onCommand(void (*callback)(const VoiceCommand&)) {
    commandCallback = callback;
}

// Private methods

void VoiceAI::initMicrophone() {
    // Initialize I2S for MEMS microphone
    // This is a placeholder - real implementation would configure I2S

    Serial.println("Microphone initialized (I2S)");

    // In real implementation:
    // - Configure I2S pins (WS, SD, SCK)
    // - Set sample rate to 16kHz
    // - Configure for mono input
    // - Set up DMA buffer for continuous recording
}

void VoiceAI::initSpeaker() {
    // Initialize I2S for speaker output
    Serial.println("Speaker initialized (I2S)");

    // In real implementation:
    // - Configure I2S for output
    // - Set up MAX98357A amplifier
    // - Configure DMA for audio playback
}

void VoiceAI::recordAudio() {
    // Read audio from I2S microphone
    // This is a placeholder - real implementation would read from I2S DMA buffer

    // In real implementation:
    // - Read samples from I2S
    // - Apply noise reduction
    // - Fill audioBuffer with samples
    // - Implement circular buffer for continuous recording
}

bool VoiceAI::detectWakeWord(const int16_t* buffer, size_t samples) {
    // Wake word detection using simple energy threshold
    // Real implementation would use Picovoice Porcupine or similar

    // Calculate audio energy
    long energy = 0;
    for (size_t i = 0; i < min(samples, (size_t)1000); i++) {
        energy += abs(buffer[i]);
    }
    energy /= min(samples, (size_t)1000);

    // Simple threshold-based detection (placeholder)
    // Real implementation would use ML model
    if (energy > 1000) {
        Serial.println("Wake word detected (placeholder)");
        return true;
    }

    return false;
}

void VoiceAI::extractCommand(const int16_t* buffer, size_t samples, char* output) {
    // Extract command text from audio
    // Real implementation would use speech-to-text

    // Placeholder: simulate command extraction
    strcpy(output, "check messages");

    // In real implementation:
    // - Use lightweight STT model for simple commands
    // - Or send audio to phone for cloud STT
    // - Return transcribed text
}
