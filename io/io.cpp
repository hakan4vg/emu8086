#include "io.hpp"
#include <iostream>
#include <stdexcept>

namespace IO {

    IOController::IOController() {
        // Register default handlers for standard ports
        
        // Keyboard data port - returns simulated keypress
        registerInputHandler(KEYBOARD_DATA, [](uint16_t) -> uint8_t {
            return 0;
        });
        
        registerOutputHandler(SERIAL_DATA, [](uint16_t, uint8_t value) {
            std::cout << static_cast<char>(value);
        });
    }

    void IOController::registerInputHandler(uint16_t port, InputHandler handler) {
        inputHandlers[port] = handler;
    }

    void IOController::registerOutputHandler(uint16_t port, OutputHandler handler) {
        outputHandlers[port] = handler;
    }

    uint8_t IOController::readPort(uint16_t port) {
        // Check if there's a handler for this port
        auto it = inputHandlers.find(port);
        if (it != inputHandlers.end()) {
            return it->second(port);
        }
        
        return portValues.count(port) ? portValues[port] : 0;
    }

    void IOController::writePort(uint16_t port, uint8_t value) {
        // Update the stored value
        portValues[port] = value;
        
        // Check if there's a handler for this port
        auto it = outputHandlers.find(port);
        if (it != outputHandlers.end()) {
            it->second(port, value);
        }
    }

    uint16_t IOController::readPortWord(uint16_t port) {
        // Read low byte from port, high byte from port+1
        return static_cast<uint16_t>(readPort(port)) |
               (static_cast<uint16_t>(readPort(port + 1)) << 8);
    }

    void IOController::writePortWord(uint16_t port, uint16_t value) {
        // Write low byte to port, high byte to port+1
        writePort(port, static_cast<uint8_t>(value & 0xFF));
        writePort(port + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
    }

} // namespace IO 