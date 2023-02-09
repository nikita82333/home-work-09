#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

#include "Controller.h"

Controller::Controller(unsigned int bulk_size, std::shared_ptr<Receiver> receiver)  :
        _receiver(std::move(receiver)), _bulk_size(bulk_size) {
    std::srand(std::time(nullptr));
    _file1 = std::thread(&Controller::file_worker, this);
    _file2 = std::thread(&Controller::file_worker, this);
    _log = std::thread(&Controller::log_worker, this);
}

Controller::~Controller() {
    _file1.join();
    _file2.join();
    _log.join();
}

void Controller::parse_command(const std::string& line) {
    if (_command_count == 0) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        _log_file_name = "bulk" + std::to_string(time); //as specified in the task
        //_log_file_name = "bulk" + time_to_string(&time); //human-readable log file name
    }

    if (line == "EOF") {
        if (_brackets_count == 0) {
            add_bulk_to_queue();
        }
        _file_exit_flag = true;
        _log_exit_flag = true;
        _file_cv.notify_all();
        _log_cv.notify_one();
    } else if (line == "{") {
        if (_brackets_count == 0) {
            add_bulk_to_queue();
        }
        ++_brackets_count;
    } else if (line == "}") {
        if (_brackets_count > 0) {
            --_brackets_count;
        }
        if (_brackets_count == 0) {
            add_bulk_to_queue();
        }
    } else {
        add_command(std::make_shared<TextCommand>(_receiver, line));
        ++_command_count;
        if (_command_count == _bulk_size && _brackets_count == 0) {
            add_bulk_to_queue();
        }
    }
}

void Controller::add_bulk_to_queue() {
    if (_command_count != 0) {
        {
            std::lock_guard<std::mutex> locker(_file_cv_mutex);
            _file_queue.emplace(_commands, _log_file_name);
        }
        _file_cv.notify_one();

        {
            std::lock_guard<std::mutex> locker(_log_cv_mutex);
            _log_queue.emplace(std::move(_commands));
        }
        _log_cv.notify_one();

        clear_commands();
    }
}

void Controller::add_command(std::shared_ptr<Command> command) {
    _commands.emplace_back(std::move(command));
}

void Controller::file_worker() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    std::string thread_id;
    ss >> thread_id;

    while (true) {
        std::unique_lock<std::mutex> locker(_file_cv_mutex);
        _file_cv.wait(locker, [&](){ return !_file_queue.empty() || _file_exit_flag; });
        if (!_file_queue.empty()) {
            auto bulk = std::move(_file_queue.front());
            _file_queue.pop();
            locker.unlock();

            std::string postfix = "_t" + thread_id
                    + std::to_string(std::rand() % 100) + ".log";//to escape name duplicate
            std::ofstream log_file(bulk.second + postfix);
            if (log_file.is_open()) {
                output_commands(bulk.first, log_file);
                log_file.close();
            } else {
                std::cerr << "File creation error: " << bulk.second + postfix << std::endl;
            }
        } else {
            locker.unlock();
            break;
        }
    }
}

void Controller::log_worker() {
    while (!_log_exit_flag) {
        std::unique_lock<std::mutex> locker(_log_cv_mutex);
        _log_cv.wait(locker, [&](){ return !_log_queue.empty() || _log_exit_flag; });
        while (!_log_queue.empty()) {
            output_commands(_log_queue.front(), std::cout);
            _log_queue.pop();
        }
        locker.unlock();
    }
}

void Controller::output_commands(const command_bulk& commands, std::ostream &out) {
    out << "bulk: ";
    for (auto it = commands.cbegin(); it != commands.cend(); ++it) {
        if (it != commands.cbegin()) {
            out << ", ";
        }
        out << (*it)->get_text();
    }
    out << std::endl;
}

void Controller::execute_commands(const command_bulk& commands) {
    for (const auto& command : commands) {
        command->execute();
    }
}

void Controller::clear_commands() {
    _commands.clear();
    _command_count = 0;
}

std::string Controller::time_to_string(std::time_t *time) {
    std::stringstream ss;
    std::tm* tm = localtime(time);
    ss << tm->tm_year + 1900 << "-" << std::setw(2) << std::setfill('0') << tm->tm_mon + 1 << "-"
       << std::setw(2) << std::setfill('0') << tm->tm_mday << "_"
       << std::setw(2) << std::setfill('0') << tm->tm_hour << "-"
       << std::setw(2) << std::setfill('0') << tm->tm_min << "-"
       << std::setw(2) << std::setfill('0') << tm->tm_sec;
    return ss.str();
}
