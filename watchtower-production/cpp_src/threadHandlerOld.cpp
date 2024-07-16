/*
-----------------------------------

Source code for the threading functions of the ground station

Contributors: Arturo, Arkadiusz, Kyril, Hans
The Als Rocketry Club
Sonderborg, Denmark

-----------------------------------
*/

#include "threadHandler.h"
#include <iostream>
#include <thread>

// Define global variables
std::mutex threadHandler::mtx; // Mutex for protecting the message queue
std::condition_variable threadHandler::cv; // Condition for notifying threads
std::queue<std::string> threadHandler::messages;

namespace threadHandler{
    bool finished = false; // Flag to indicate when the threads should stop

    // Message receiving threads -----------------------------------
    // Receiving messages from MQTT
    void mqttInputThread() {
        // Implement if needed
    }

    // Thread function for receiving messages from the terminal
    void emergencyInputThread() {
        std::string input;
        while (!finished) {
            std::getline(std::cin, input);
            if (input == "exit") {
                finished = true;
                cv.notify_one();
                break;
            }
            {
                std::lock_guard<std::mutex> locker(mtx);
                messages.push(input); // Protected code block {}
            } // Guard goes out of scope and mtx is released
            cv.notify_one();
        }
    }

    // Message sending threads -----------------------------------
    void statusThread() {
        while (!finished) {
            std::unique_lock<std::mutex> locker(mtx);
            cv.wait(locker, [] { return !messages.empty() || finished;});
            while (!messages.empty()) {
                std::cout << "Received: " << messages.front() << std::endl;
                messages.pop();
            }

            locker.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "Status thread running..." << std::endl;
        }
    } 

    // Thread control functions -----------------------------------
    // Start the threads
    void startThreads(int protocolType) {
        // Communication threads -----------------------------------
        std::thread input(emergencyInputThread); // Terminal input thread
        std::thread status(statusThread); // Status thread

        // Start the protocol specific internal communication thread
        std::thread protocolThread;
        switch (protocolType) {
            case 0: // MQTT
                protocolThread = std::thread(mqttInputThread); 
                protocolThread.join();
                break;
            case 1: // TCP/IP
                // Implement if needed
                break;
            default: // Default to MQTT
                protocolThread = std::thread(mqttInputThread);
                protocolThread.join();
                break;
        }

        // Wait for the threads to finish -----------------------------------
        input.join();
        status.join();
    }

    // Stop the threads
    void stopThreads() {
        // Implement if needed, for a graceful shutdown 
        finished = true;
        cv.notify_all();
    }
}