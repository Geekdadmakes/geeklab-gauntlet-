#ifndef VOICE_AI_H
#define VOICE_AI_H

#include <Arduino.h>
#include "config.h"

// Voice states
enum VoiceState {
    VOICE_IDLE,
    VOICE_LISTENING,
    VOICE_PROCESSING,
    VOICE_RESPONDING
};

// Command types
enum CommandType {
    CMD_CHECK_MESSAGES,
    CMD_SHOW_NOTIFICATIONS,
    CMD_DEVICE_CONTROL,
    CMD_TIME_QUERY,
    CMD_BATTERY_STATUS,
    CMD_WEATHER,
    CMD_CUSTOM_QUERY,
    CMD_UNKNOWN
};

// Command structure
struct VoiceCommand {
    CommandType type;
    char raw[256];
    char parameter[128];
    bool needsCloud;
};

class VoiceAI {
public:
    VoiceAI();
    bool begin();
    void update();

    // Wake word detection
    bool isWakeWordDetected();
    void startListening();
    void stopListening();

    // Command processing
    VoiceCommand processAudio(const int16_t* audioBuffer, size_t samples);
    CommandType parseCommand(const char* text);
    void executeCommand(const VoiceCommand& cmd);

    // Cloud integration
    void sendToCloud(const char* query);
    void handleCloudResponse(const char* response);

    // Text-to-Speech
    void speak(const char* text);

    // State management
    VoiceState getState();
    void setState(VoiceState state);

    // Callback registration
    void onWakeWord(void (*callback)());
    void onCommand(void (*callback)(const VoiceCommand&));

private:
    VoiceState currentState;
    int16_t* audioBuffer;
    size_t bufferSize;
    bool listening;

    void (*wakeWordCallback)();
    void (*commandCallback)(const VoiceCommand&);

    void initMicrophone();
    void initSpeaker();
    void recordAudio();
    bool detectWakeWord(const int16_t* buffer, size_t samples);
    void extractCommand(const int16_t* buffer, size_t samples, char* output);
};

extern VoiceAI voiceAI;

#endif // VOICE_AI_H
