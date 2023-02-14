#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <utility>
#include <memory>

#include "Receiver.h"

/// <summary>
/// Parent class Command in the implementation of the Command pattern.
/// </summary>
/// <param name="receiver">Shared pointer to Receiver.</param>
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual std::string get_text() = 0;

protected:
    explicit Command(std::shared_ptr<Receiver> receiver) : _receiver(std::move(receiver)) {}

    std::shared_ptr<Receiver> _receiver;

};

/// <summary>
/// Class TextCommand - concrete command.
/// </summary>
/// <param name="receiver">Shared pointer to Receiver.</param>
/// <param name="text">Text record of the command.</param>
class TextCommand : public Command {
public:
    TextCommand(std::shared_ptr<Receiver> receiver, std::string text) :
        Command(std::move(receiver)), _text(std::move(text)) {}

    void execute() override {
        _receiver->cmd(_text);
    }

    std::string get_text() override {
        return _text;
    }

private:
    std::string _text;

};

#endif //COMMAND_H
