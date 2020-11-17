//
// Created by jmaerte on 12.05.20.
//

#include "output/logger.hpp"
#include <exception>

namespace jmaerte {
    namespace output {
        const unsigned int MAIN_CHANNEL = 0u, MEM_CHANNEL = 1u, TIME_CHANNEL = 2u;


        logger& LOGGER = logger::get_instance();


        unsigned int logger::register_channel(std::string name, std::ostream& stream) {
            int id = 0;
            while (channels.find(id) != channels.end()) {
                id++;
            }
            channels.insert(std::pair(id, std::pair<std::string, std::ostream&>(name, stream)));
            log(0u, "Registered output channel: " + name + " on channel id " + std::to_string(id) + ".");
            return id;
        }

        unsigned int logger::register_channel_group(std::string name, std::ostream& stream) {
            mute();
            unsigned int id = register_channel(name, stream);
            unmute();
            channel_groups.insert(std::make_pair(id, std::vector<unsigned int>{}));
            log(0u, "Registered output channel group: " + name + ".");
            return id;
        }

        void logger::log(unsigned int id, std::string msg) {
            if (muted) return;
            auto it = channels.find(id);
            if (it != channels.end()) channels.at(id).second << "[" << channels.at(id).first << "] " << msg << std::endl;
            else channels.at(0).second << "Unknown channel tried to log..." << std::endl;
        }

        void logger::err(unsigned int id, std::string msg, unsigned int err_id) {
            if (muted) return;
            channels.at(id).second << "[" << channels.at(id).first << "] ";
            switch(err_id) {
                case INVALID_ARG:
                    channels.at(id).second << "INVALID ARGUMENT - ";
                    break;
                case BAD_ALLOC:
                    channels.at(id).second << "BAD ALLOC - ";
                    break;
                default:
                    channels.at(id).second << "ERROR - ";
                    break;
            }
            channels.at(id).second << msg << std::endl;
            throw;
        }

        void logger::release_channel(unsigned int id) {
            channels.erase(id);
            log(0u, "Released output channel: " + std::to_string(id) + ".");
        }

        void logger::release_channel_group(unsigned int id) {
            mute();
            release_channel(id);
            for (unsigned int i : channel_groups.at(id)) {
                release_channel(i);
            }
            unmute();
            channel_groups.erase(id);
            log(0u, "Released output channel group: " + channels.at(id).first + ".");
        }
    }
}