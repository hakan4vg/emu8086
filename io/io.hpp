#ifndef IO_HPP
#define IO_HPP

#include <cstdint>
#include <unordered_map>
#include <functional>
#include <vector>

namespace IO {

    // Type for I/O port handlers
    using InputHandler = std::function<uint8_t(uint16_t port)>;
    using OutputHandler = std::function<void(uint16_t port, uint8_t value)>;

    class IOController {
    private:
        // Maps of port addresses to handler functions
        std::unordered_map<uint16_t, InputHandler> inputHandlers;
        std::unordered_map<uint16_t, OutputHandler> outputHandlers;

        // Default port values
        std::unordered_map<uint16_t, uint8_t> portValues;

    public:
        IOController();

        // Register custom handlers for specific ports
        void registerInputHandler(uint16_t port, InputHandler handler);
        void registerOutputHandler(uint16_t port, OutputHandler handler);

        // Read from and write to I/O ports
        uint8_t readPort(uint16_t port);
        void writePort(uint16_t port, uint8_t value);

        // For word operations (for 16-bit ports)
        uint16_t readPortWord(uint16_t port);
        void writePortWord(uint16_t port, uint16_t value);
    };

    // Common port numbers
    enum CommonPorts {
        KEYBOARD_DATA = 0x60,
        KEYBOARD_CTRL = 0x64,
        TIMER_COUNTER0 = 0x40,
        TIMER_COUNTER1 = 0x41,
        TIMER_COUNTER2 = 0x42,
        TIMER_CTRL = 0x43,
        SERIAL_DATA = 0x3F8,
        SERIAL_CTRL = 0x3FD
    };

} // namespace IO

#endif // IO_HPP 