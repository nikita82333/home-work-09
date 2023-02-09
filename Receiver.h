#ifndef RECEIVER_H
#define RECEIVER_H

#include <string>

/// <summary>
/// Class Receiver in the implementation of the Command pattern.
/// </summary>
class Receiver {
public:
    Receiver() = default;

    void cmd1() {
        //do something
    }
    void cmd2() {
        //do something else
    }
    //etc
};


#endif //RECEIVER_H
