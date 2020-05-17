//
// Created by jmaerte on 12.05.20.
//

#include <sstream>
#include <string>

#include "ANUBiS/complex/io.hpp"

namespace jmaerte {
    namespace anubis {
        namespace io {

            /***********************************************************************************************************
             * HELPER METHOD FOR READING A COMPLEX FILE
             **********************************************************************************************************/

            template<typename complex_constructor>
            auto complex_from_file(
                    complex_constructor constructor,
                    const std::string & path,
                    int sceleton,
                    const std::string & sep,
                    const std::string & set_openers,
                    const std::string & set_closers
            ) -> typename std::result_of<complex_constructor(std::string, int)>::type {

                using complex_type = typename std::result_of<complex_constructor(std::string, int)>::type;
                static_assert(std::is_base_of<jmaerte::anubis::complex, typename std::remove_pointer<complex_type>::type>::value, "complex_type must be derived from complex.");
                std::cout << "Opening file..." << std::endl;
                std::ifstream file;
                file.open(path);

                if (!file.is_open()) {
                    // logger.throw(Logger.INVALID_ARGUMENT, "failed to open file!");
                    std::cout << "[IO] ERROR - failed to open file!" << std::endl;
                    throw std::invalid_argument("[IO] ERROR - failed to open file!");
                }

                std::cout << "Reading file..." << std::endl;

                std::string name = "";

                std::string line = "";

                bool complex_opened = false;
                bool set_opened = false;
                complex_type complex = NULL;

                std::map<std::string, unsigned int> labels {};
                unsigned int i_labels = 0;

                std::vector<unsigned int> facet {};

                while (std::getline(file, line)) {
                    std::istringstream iss(line);
                    std::string curr;
                    char c;
                    while (iss.get(c)) {
                        if (c == ' ') continue;
                        else if (c == '=') {
                            if (curr.back() == ':') curr = curr.substr(0, curr.size() - 1);
                            name.assign(curr);

                            complex = constructor(name, sceleton);

                            curr = "";
                            std::cout << "[IO] Found complex named " << name << std::endl;
                        } else if (!name.empty()) {

                            if (!complex_opened) {
                                if (set_openers.find(c) != std::string::npos) {
                                    complex_opened = true;
                                }
                            } else {
                                if (set_openers.find(c) != std::string::npos) {
                                    set_opened = true;
                                    curr = "";
                                } else if (set_opened) {
                                    if ((sep + set_closers).find(c) != std::string::npos) {

                                        // add current label to list
                                        if (labels.find(curr) == labels.end()) labels[curr] = i_labels++;
                                        facet.push_back(labels[curr]);
                                        curr = "";
                                        if (set_closers.find(c) != std::string::npos) {
                                            set_opened = false;

                                            // flush
                                            complex->facet_insert(&facet);
                                            facet.clear();
                                        }
                                    } else curr += c;
                                } else {
                                    if (set_closers.find(c) != std::string::npos) {
                                        complex_opened = false;
                                        break;
                                    }
                                }
                            }

                        } else {
                            curr += c;
                        }
                    }
                }
//                delete facet;
                file.close();
                if (complex_opened) throw std::invalid_argument("[IO] ERROR - Simplicial Complex file must define an enclosed set; closing bracket is missing.");
                if (name.empty()) throw std::invalid_argument("[IO] ERROR - Simplicial Complex file must have a name followed by '=' or ':='.");

                return complex;
            }

            template<typename complex_constructor>
            auto complex_from_facets(
                complex_constructor constructor,
                const std::vector<std::vector<unsigned int> *> &facets,
                std::string name,
                int sceleton
            ) -> typename std::result_of<complex_constructor(std::string, int)>::type {

                using complex_type = typename std::result_of<complex_constructor(std::string, int)>::type;
                static_assert(std::is_base_of<jmaerte::anubis::complex, typename std::remove_pointer<complex_type>::type>::value, "complex_type must be derived from complex.");

                complex_type complex = constructor(name, sceleton);
                for (const auto & facet : facets) {
                    complex->facet_insert(facet);
                }
                return complex;
            }
        }
    }
}