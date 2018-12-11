#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <errno.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct BrighntessFunction {
    const std::string name;
    const std::function<double(double)> f;
    const std::function<double(double)> f_inv;
};

const std::string PREFIX = "/sys/class/backlight/";

const std::vector<BrighntessFunction> FUNCTIONS = {
    {"linear", [](double x) { return x; }, [](double y) { return y; }}};

struct BacklightStatus {
    int brightness;
    int max_brightness;
};

bool is_directory(const std::string &path) {
    struct stat s;
    if (stat(path.c_str(), &s) != 0) {
        throw std::runtime_error("Failed to determine type of \"" + path +
                                 "\": errno = " + std::to_string(errno));
    }
    return S_ISDIR(s.st_mode);
}

void ensure_read_access(const std::string &path) {
    if (access(path.c_str(), R_OK) != 0) {
        throw std::runtime_error("Cannot ensure read access to file \"" + path +
                                 "\": errno = " + std::to_string(errno));
    }
}

void ensure_write_access(const std::string &path) {
    if (access(path.c_str(), W_OK) != 0) {
        throw std::runtime_error("Cannot ensure write access to file \"" +
                                 path + "\": errno = " + std::to_string(errno));
    }
}

std::vector<std::string> list_subdirectories(const std::string &directory) {
    const std::string curdir = ".";
    const std::string pardir = "..";
    std::string path;
    std::vector<std::string> result;

    DIR *dir = opendir(directory.c_str());
    if (dir == nullptr) {
        throw std::runtime_error("Failed to access directory \"" + PREFIX +
                                 "\": errno = " + std::to_string(errno));
    }

    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(dir);
        if (entry == nullptr)
            break;

        if (entry->d_name == curdir)
            continue;

        if (entry->d_name == pardir)
            continue;

        path = directory + entry->d_name;
        if (!is_directory(path))
            continue;

        result.push_back(path);
    }

    if (errno != 0) {
        throw std::runtime_error("Failed to iterate through directory \"" +
                                 directory +
                                 "\": errno = " + std::to_string(errno));
    }

    if (closedir(dir) != 0) {
        throw std::runtime_error("Failed to close directory stream for \"" +
                                 directory +
                                 "\": errno = " + std::to_string(errno));
    }

    return result;
}

BacklightStatus get_status(const std::string &path) {
    BacklightStatus ret;
    std::string file_path;
    std::ifstream strm;

    file_path = path + "/brightness";
    ensure_read_access(file_path);
    strm.open(file_path.c_str());
    if (!strm.good()) {
        throw std::runtime_error("Failed to open file \"" + file_path + "\"");
    }
    strm >> ret.brightness;
    strm.close();

    file_path = path + "/max_brightness";
    ensure_read_access(file_path);
    strm.open(file_path.c_str());
    if (!strm.good()) {
        throw std::runtime_error("Failed to open file \"" + file_path +
                                 "\" for reading");
    }
    strm >> ret.max_brightness;
    strm.close();

    return ret;
}

void set_status(const std::string &path, const BacklightStatus &status) {
    std::string file_path = path + "/brightness";
    std::ofstream strm;

    ensure_write_access(file_path);
    strm.open(file_path.c_str());
    if (!strm.good()) {
        throw std::runtime_error("Failed to open file \"" + file_path +
                                 "\" for reading");
    }
    strm << status.brightness;
    strm.close();
}

void usage() { std::cerr << "Usage: chbl list\n"; }
void list_backlights() {
    auto backlights = list_subdirectories(PREFIX);
    for (const auto &backlight : backlights) {
        auto status = get_status(backlight);
        std::cout << backlight << "\t(" << status.brightness << "/"
                  << status.max_brightness << ")\n";
    }
}

void set_brightness(const std::string &backlight, const std::string operation,
                    const BrighntessFunction &function) {
    auto status = get_status(backlight);
    double value;
    double y = double(status.brightness) / double(status.max_brightness);
    double x;
    if (std::isdigit(operation[0])) {
        value = std::strtod(operation.c_str(), nullptr);
        x = function.f_inv(value);
    } else if (operation[0] == '+') {
        value = std::strtod(operation.c_str() + 1, nullptr);
        x = function.f_inv(y) + value;
    } else if (operation[0] == '-') {
        value = std::strtod(operation.c_str() + 1, nullptr);
        x = function.f_inv(y) - value;
    }
    x = std::max(0., std::min(1., x));
    y = std::max(0., std::min(1., function.f(x)));
    status.brightness =
        std::max(0, std::min(status.max_brightness,
                             (int)std::round(y * status.max_brightness)));
    set_status(backlight, status);
}

int main(int argc, char **argv) {
    std::string backlight;
    std::string function;
    std::string operation;
    bool found_set_command = false;

    std::vector<std::string> args(argc - 1);
    for (int i = 1; i < argc; ++i) {
        args.push_back(std::string(argv[i]));
    }

    for (int i = 0; i < (int)args.size(); ++i) {
        if ((args[i] == "-h") || (args[i] == "--help")) {
            usage();
            std::exit(EXIT_FAILURE);
        }

        if (args[i] == "list") {
            list_backlights();
            std::exit(EXIT_SUCCESS);
        }

        if (args[i] == "set") {
            found_set_command = true;
            continue;
        }

        if ((args[i] == "-b") || (args[i] == "--backlight")) {
            if (i + 1 >= argc) {
                usage();
                std::exit(EXIT_FAILURE);
            }
            backlight = args[i + 1];
            continue;
        }

        if ((args[i] == "-f") || (args[i] == "--function")) {
            if (i + 1 >= argc) {
                usage();
                std::exit(EXIT_FAILURE);
            }
            function = args[i + 1];
            continue;
        }

        operation = args[i];
    }

    if (!found_set_command) {
        usage();
        std::exit(EXIT_FAILURE);
    }

    if (backlight.empty()) {
        auto backlights = list_subdirectories(PREFIX);
        if (backlights.empty()) {
            std::runtime_error("There does not seem to be any backlight");
        }
        backlight = backlights[0];
    }

    if (function.empty()) {
        function = FUNCTIONS[0].name;
    }

    for (int i = 0; i < (int)FUNCTIONS.size(); ++i) {
        if (FUNCTIONS[i].name == function) {
            set_brightness(backlight, operation, FUNCTIONS[i]);
            std::exit(EXIT_SUCCESS);
        }
    }

    std::cerr << "Unknown function \"" << function << "\"!\n";
    usage();
    std::exit(EXIT_FAILURE);
}
