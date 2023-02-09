#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <list>
#include <memory>
#include <ctime>
#include <string>
#include <utility>
#include <atomic>
#include <queue>
#include <thread>
#include <condition_variable>

#include "Command.h"

using command_bulk = std::list<std::shared_ptr<Command>>;
using context = std::pair<command_bulk, std::string>;

/// <summary>
/// Class Controller - controller (invoker) in the implementation of the Command pattern.
/// </summary>
/// <param name="bulk_size">Command bulk size.</param>
/// <param name="receiver">Shared pointer to Receiver.</param>
class Controller {
public:
    Controller(unsigned int bulk_size, std::shared_ptr<Receiver> receiver);
    ~Controller();

    void parse_command(const std::string& line);
    void add_command(std::shared_ptr<Command> command);

private:
    void file_worker();
    void log_worker();
    void add_bulk_to_queue();
    static void output_commands(const command_bulk& commands, std::ostream& out);
    static void execute_commands(const command_bulk& commands);
    void clear_commands();
    static std::string time_to_string(std::time_t* time);

    std::shared_ptr<Receiver> _receiver;
    command_bulk _commands;

    unsigned int _bulk_size;
    unsigned int _command_count {0};
    unsigned int _brackets_count {0};
    std::string _log_file_name;

    std::queue<context> _file_queue;
    std::queue<command_bulk> _log_queue;

    std::thread _file1;
    std::thread _file2;
    std::thread _log;

    std::atomic<bool> _file_exit_flag {false};
    std::atomic<bool> _log_exit_flag {false};
    std::condition_variable _file_cv;
    std::mutex _file_cv_mutex;
    std::condition_variable _log_cv;
    std::mutex _log_cv_mutex;

};


#endif //CONTROLLER_H
