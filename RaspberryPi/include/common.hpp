#ifndef PCB_INS_COMMON
#define PCB_INS_COMMON

#include <exception>
#include <string>

#define SCREEN_HEIGHT 1600
#define SCREEN_WIDTH 2560

/************************************************
 *              Exception handling
 ***********************************************/

/**
 * @brief Class to process generic errors that stop the execution of the
 * evaluation process
 */
class NotifyError : public std::exception {
public:
  NotifyError(const std::string &message) : message(message) {}

  const char *what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};

#endif // PCB_INS_COMMON
