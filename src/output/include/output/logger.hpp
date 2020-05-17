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

namespace jmaerte {
    namespace output {

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
                channels.insert(std::pair<unsigned int, std::pair<std::string, std::ostream&>>(0u, std::pair<std::string, std::ostream&>(std::string("Main"), std::cout)));
            }
        };
    }
}

#endif //ANUBIS_SUPERBUILD_LOGGER_HPP
