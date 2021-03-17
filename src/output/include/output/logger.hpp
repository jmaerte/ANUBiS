//
// Created by jmaerte on 12.05.20.
//

#ifndef ANUBIS_SUPERBUILD_LOGGER_HPP
#define ANUBIS_SUPERBUILD_LOGGER_HPP

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <tuple>
#include <OUTPUT_EXPORT.h>

#define PRIVATE __attribute__((visibility("hidden")))
#define PUBLIC __attribute__((visibility("default")))

namespace jmaerte {
    namespace output {

        PUBLIC extern const OUTPUT_EXPORT unsigned int MAIN_CHANNEL, MEM_CHANNEL, TIME_CHANNEL;

        class logger;

        extern OUTPUT_EXPORT logger& LOGGER;

        class OUTPUT_EXPORT logger {
        public:
            static logger& get_instance() {
                static logger instance;
                return instance;
            }

            logger(const logger&) = delete;
            void operator=(const logger&) = delete;

            static const unsigned int INVALID_ARG = 0u, BAD_ALLOC = 1u; // error ids

            enum license {
                MIT
            };

            unsigned int register_channel(std::string name, std::ostream&);
            unsigned int register_channel_group(std::string name, std::ostream&);

            void release_channel(unsigned int id);
            void release_channel_group(unsigned int id);

            void log(unsigned int id, std::string msg);
            void err(unsigned int id, std::string msg, unsigned int err_id);

            void mute() {
                muted = true;
            }

            void unmute() {
                muted = false;
            }

            void license(license l, std::string year, std::string holder_name, std::string program_name, std::string descript_short, std::string desc) {
                std::string text = "+------------------- " + program_name + " -------------------+" + "\r\n";
                text += "Copyright " + year + " " + holder_name + ".\r\n";

                text += descript_short + "\r\n\r\n";

                text += desc + "\r\n";

                text += "This project is licensed under the terms of ";
                switch(l) {
                    case license::MIT:
                        text += "the MIT License";
                }
                text += ".\r\n";

                text += "+-------------------";
                for (int i = 0; i < program_name.size() + 2; i++) text += "-";
                text += "-------------------+";
                std::cout << text << std::endl;
            }

        private:
            bool muted = false;

            std::map<unsigned int, std::pair<std::string, std::ostream&>> channels;
            std::map<unsigned int, std::vector<unsigned int>> channel_groups;
            std::map<unsigned int, unsigned int> parents;

            logger() {
                channels.insert(std::pair<unsigned int, std::pair<std::string, std::ostream&>>(MAIN_CHANNEL, std::pair<std::string, std::ostream&>(std::string("MAIN"), std::cout)));
                channels.insert(std::pair<unsigned int, std::pair<std::string, std::ostream&>>(MEM_CHANNEL , std::pair<std::string, std::ostream&>(std::string("MEM"), std::cout)));
                channels.insert(std::pair<unsigned int, std::pair<std::string, std::ostream&>>(TIME_CHANNEL, std::pair<std::string, std::ostream&>(std::string("TIME"), std::cout)));
            }

            ~logger() {
                for (auto it = channels.begin(); it != channels.end(); it++) {
                    if (it->first != 0) {
                        log(0u, "Released output channel " + std::to_string(it->first) + ".");
                    }
                }
                log(0u, "Releasing this channel (0) now.");
                channels.clear();
            }
        };

        static std::pair<double, std::string> cast_time(double time) {
            if (time < 1'000) return {time, "µs"};
            else if (time < 1'000'000) return {time / 1'000, "ms"};

            time /= 1'000'000;
            if (time < 60) return {time, "s"};
            else if (time < 3'600) return {time / 60, "min"};
            return {time / 3'600, "h"};
        }

        /**
         * Prints timings of a task with sub-tasks. The times should be given in microseconds.
         */
        static void print_time_resume(std::string task, double time, std::map<std::string, double> sub_tasks) {
            std::pair<double, std::string> time_pair = cast_time(time);
            std::string out = "\t+------------------------------------------------------------------------\r\n"
                              "\t Time elapsed for " + task + ": " + std::to_string(time_pair.first) + time_pair.second + "\r\n";
            for (auto it = sub_tasks.begin(); it != sub_tasks.end(); it++) {
                time_pair = cast_time(it->second);
                out += "\t\tFor " + it->first + ": " + std::to_string(time_pair.first) + time_pair.second + "\r\n";
            }
            out += "\t+------------------------------------------------------------------------";
            LOGGER.log(TIME_CHANNEL, out);
        }

    }
}

#endif //ANUBIS_SUPERBUILD_LOGGER_HPP
