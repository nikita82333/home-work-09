#include <future>

#include "Controller.h"
#include "async.h"

namespace async {

    struct Context {
        std::unique_ptr<Controller> controller;
        std::future<void> future;
    };

    template <typename Worker, typename ...Args>
    std::future<void> then(std::future<void> future, Worker worker, Args ...args) {
        return std::async(
            std::launch::async,
            [future = std::move(future), worker = std::move(worker)](Args ...args) mutable {
                future.wait();
                worker(args...);
            },
            args...
        );
    }

    handle_t connect(std::size_t bulk) {
        auto receiver = std::make_shared<Receiver>();
        auto controller = std::make_unique<Controller>(bulk, receiver);

        auto future = std::async(std::launch::async, [](){});//dummy async thread

        auto context = new Context{std::move(controller), std::move(future)};
        return reinterpret_cast<void *>(context);
    }

    void receive(handle_t handle, const char *data, std::size_t size) {
        auto context = reinterpret_cast<Context*>(handle);

        auto worker = [](Context* context, const char *data, std::size_t size) {
            std::string line;
            for (std::size_t i = 0; i < size; ++i) {
                if (data[i] != '\n') {
                    line += data[i];
                } else {
                    if (!line.empty()) {
                        context->controller->parse_command(line);
                        line.clear();
                    }
                }
            }
            if (!line.empty()) {
                context->controller->parse_command(line);
            }
        };

        auto future = then(std::move(context->future), worker, context, data, size);
        context->future = std::move(future);
    }

    void disconnect(handle_t handle) {
        auto context = reinterpret_cast<Context*>(handle);

        context->future.wait();
        context->controller->parse_command("EOF");

        delete context;
    }
}
