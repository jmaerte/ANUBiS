//
// Created by jmaerte on 12.05.20.
//

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
                std::string content;
                if (file.is_open()) {
                    content = {(std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>())};
                } else {
                    throw std::invalid_argument("[IO] ERROR - failed to open file!");
                }
                file.close();
                std::cout << "Read file..." << std::endl;
                size_t pos = content.find(" ");

                while(pos != std::string::npos) {
                    content.replace(pos, 1, "");
                    pos = content.find(" ", pos);
                }

                std::string name;

                pos = content.find(":=");
                if (pos != std::string::npos) {
                    name = content.substr(0, pos);
                    content = content.substr(pos + 2, content.size());
                } else {
                    pos = content.find('=');
                    if (pos != std::string::npos) {
                        name = content.substr(0, pos);
                        content = content.substr(pos + 1, content.size());
                    } else {
                        throw std::invalid_argument("Simplicial Complex file must have a name followed by '=' or ':='.");
                    }
                }

                std::cout << "Found name: " << name << std::endl;

                content = content.substr(1, content.size() - 2);

                boost::regex simplex("[" + set_openers + "](([\\[{][\\w" + sep + "]*[\\]}])|([\\w" + sep + "]*))*[" + set_closers + "]");
                boost::sregex_token_iterator iter(content.begin(), content.end(), simplex, 0);
                boost::sregex_token_iterator end;

                std::map<std::string, unsigned int> labels;
                unsigned int i_labels = 0;

                complex_type complex = constructor(name, sceleton);

                {
                    thread_pool workers(-1);

                    while (iter != end) {
                        std::string s_facet = *iter;
                        s_facet = s_facet.substr(1, s_facet.size() - 2);

                        boost::regex element("[" + set_openers + "][\\w,]*[" + set_closers + "]|[\\w]+");
                        boost::sregex_token_iterator element_iter(s_facet.begin(), s_facet.end(), element, 0);
                        boost::sregex_token_iterator element_end;

                        auto facet = new std::vector<unsigned int>();

                        while (element_iter != element_end) {
                            std::string curr = *element_iter;
                            if (labels.find(curr) == labels.end()) labels[curr] = i_labels++;
                            facet->push_back(labels[curr]);
                            element_iter++;
                        }

//                    std::cout << "Pushing " << s_facet << std::endl;

//                    tree->s_insert(facet, sceleton);
                        std::function<void()> fn = [complex, facet, sceleton]() {
                            complex->facet_insert(facet);
                        };

                        workers.add_work(fn);
                        iter++;
                    }
                    std::cout << "pushed all" << std::endl;
                }
                std::cout << "processed all" << std::endl;

                return complex;
            }

            template<typename complex_constructor>
            auto complex_from_facets(
                complex_constructor constructor,
                std::vector<std::vector<unsigned int> *> &facets,
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

            /***********************************************************************************************************
             * IMPLEMENTATIONS OF COMPLEX IO
             **********************************************************************************************************/

            namespace s_list {
                s_list* s_list::from_file(
                        const std::string &path,
                        int sceleton,
                        const std::string &sep,
                        const std::string &set_openers,
                        const std::string &set_closers
                ) {
                    return complex_from_file([](std::string name, int sceleton) {
                        return new s_list(std::move(name), sceleton);
                    }, path, sceleton, sep, set_openers, set_closers);
                }

                s_list* s_list::from_facets(std::vector<std::vector<unsigned int> *> &facets, std::string name, int sceleton) {
                    return complex_from_facets([](std::string name, int sceleton) {
                        return new s_list(std::move(name), sceleton);
                    }, facets, std::move(name), sceleton);
                }
            }

            namespace s_tree {
                s_tree * s_tree::from_file(
                        const std::string &path,
                        int sceleton,
                        const std::string &sep,
                        const std::string &set_openers,
                        const std::string &set_closers
                ) {
                    return complex_from_file([](std::string name, int sceleton) {
                        return new s_tree(name, sceleton);
                    }, path, sceleton, sep, set_openers, set_closers);
                }

                s_tree *s_tree::from_facets(std::vector<std::vector<unsigned int> *> &facets, std::string name, int sceleton) {
                    return complex_from_facets([](std::string name, int sceleton) {
                        return new s_tree(name, sceleton);
                    }, facets, name, sceleton);
                }
            }

        }
    }
}