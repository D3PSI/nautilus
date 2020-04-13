#ifndef NAUTILUS_LOGGER_CPP
#define NAUTILUS_LOGGER_CPP

#include "NautilusLogger.hpp"

#ifndef _WIN32
#include <curses.h>
#include <ncurses.h>
#endif      // _WIN32

namespace nautilus {

    namespace logger {

        bool                loggerInitialized   = false;
        std::ofstream       standardLog;
        std::mutex          logLock;
        #ifndef _WIN32
        WINDOW*             logWindow;
        WINDOW*             metaDataWindow;
        #endif      // _WIN32
        auto                start               = std::chrono::system_clock::now();
        namespace           fs                  = std::filesystem;

        nautilus::NautilusStatus init(const char* _logdir) {
            if(nautilus::logger::loggerInitialized) return NAUTILUS_STATUS_OK;
            fs::create_directory("logs");
            standardLog.open("logs/nautilus.log", std::ios::trunc);
            #ifndef _WIN32
            initscr();
            start_color();
            noecho();
            nautilus::logger::logWindow = newwin(LINES, std::floor(COLS / 2), 0, 1);
            nautilus::logger::metaDataWindow = newwin(std::floor(LINES / 2), std::floor(COLS / 2), 0, std::floor(COLS / 2));
            scrollok(nautilus::logger::logWindow, true);
            scrollok(nautilus::logger::metaDataWindow, true);
            #endif      // _WIN32
            nautilus::logger::loggerInitialized = true;
            return NAUTILUS_STATUS_OK;
        }

        nautilus::NautilusStatus log(std::string _logEntry, nautilus::NautilusStatus _status) {
            nautilus::logger::init();
            if(nautilus::logger::loggerInitialized) {
                auto now = std::chrono::system_clock::now();
                std::time_t nowT = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::string timeString = std::ctime(&nowT);
                std::chrono::duration< double > elapsedTime = now - nautilus::logger::start;
                std::string timeStringTerm = std::to_string(elapsedTime.count()) + " : ";
                std::string logString = _logEntry + "\n";
                std::string log = timeString + logString; 
                std::scoped_lock< std::mutex > lock(nautilus::logger::logLock);
                #ifndef _WIN32
                wmove(nautilus::logger::logWindow, LINES, 1);
                waddstr(nautilus::logger::logWindow, timeStringTerm.c_str()); 
                waddstr(nautilus::logger::logWindow, logString.c_str());
                wrefresh(nautilus::logger::logWindow);
                #else
                std::cout << log << std::endl;
                #endif      // _WIN32
                standardLog << log << std::endl;
            }
            return NAUTILUS_STATUS_OK;
        }

        NautilusStatus meta() {
            #ifndef _WIN32
            if(nautilus::logger::loggerInitialized) {
                auto now = std::chrono::system_clock::now();
                std::time_t nowT = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::string timeString = std::ctime(&nowT);
                std::chrono::duration< double > elapsedTime = now - nautilus::logger::start;
                std::string meta = "Time: " + std::to_string(elapsedTime.count());
                std::scoped_lock< std::mutex > lock(nautilus::logger::logLock);
                wclear(nautilus::logger::metaDataWindow);
                wmove(nautilus::logger::metaDataWindow, 0, std::floor(COLS / 2));
                waddstr(nautilus::logger::metaDataWindow, meta.c_str());
                wrefresh(nautilus::logger::metaDataWindow);
            }
            #endif      // _WIN32
            return NAUTILUS_STATUS_OK;
        }

        NautilusStatus terminate() {
            #ifndef _WIN32
            endwin();
            #endif      // _WIN32
            return NAUTILUS_STATUS_OK;
        }

    }

}

#endif      // NAUTILUS_LOGGER_CPP