#pragma once

#include <sdbusplus/bus.hpp>
#include <unistd.h>
#include <fcntl.h>


namespace phosphor
{
namespace fan
{
namespace util
{

class FileDescriptor
{
    public:
        FileDescriptor() = delete;
        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor(FileDescriptor&&) = default;
        FileDescriptor& operator=(const FileDescriptor&) = delete;
        FileDescriptor& operator=(FileDescriptor&&) = default;

        FileDescriptor(int fd) : fd(fd)
        {
        }

        ~FileDescriptor()
        {
            if (fd != -1)
            {
                close(fd);
            }
        }

        int operator()()
        {
            return fd;
        }

        void open(const std::string& pathname, int flags)
        {
            fd = ::open(pathname.c_str(), flags);
            if (-1 == fd)
            {
                throw std::runtime_error(
                    "Failed to open file device: " + pathname);
            }
        }

        bool is_open()
        {
            return fd != -1;
        }

    private:
        int fd = -1;

};

/**
 * @brief Get the inventory service name from the mapper object
 *
 * @return The inventory manager service name
 */
std::string getInvService(sdbusplus::bus::bus& bus);


/**
 * @brief Get the service name from the mapper for the
 *        interface and path passed in.
 *
 * @param[in] path - the dbus path name
 * @param[in] interface - the dbus interface name
 * @param[in] bus - the dbus object
 *
 * @return The service name
 */
std::string getService(const std::string& path,
                       const std::string& interface,
                       sdbusplus::bus::bus& bus);

}
}
}
