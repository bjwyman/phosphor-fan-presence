#include <fcntl.h>
#include <unistd.h>
#include <sdbusplus/bus.hpp>
#include <phosphor-logging/log.hpp>
#include <libevdev/libevdev.h>
#include "utility.hpp"
#include "cooling_type.hpp"

namespace phosphor
{
namespace cooling
{
namespace type
{

std::unique_ptr<libevdev, FreeEvDev>  evdevOpen(int fd)
{
    libevdev* gpioDev = nullptr;

    auto rc = libevdev_new_from_fd(fd, &gpioDev);
    if (!rc)
    {
        return decltype(evdevOpen(0))(gpioDev);
    }

    //TODO - Create error log for failure. openbmc/openbmc#1542
    throw std::runtime_error("Failed to get libevdev from file descriptor"
                             " rc = " + std::to_string(rc));

    return decltype(evdevOpen(0))(nullptr);
}

void CoolingType::setAirCooled()
{
    airCooled = true;
}

void CoolingType::setWaterCooled()
{
    waterCooled = true;
}

void CoolingType::readGpio(const std::string& gpioPath, unsigned int keycode)
{
    using namespace phosphor::logging;

    gpioFd.open(gpioPath.c_str(), O_RDONLY);

    auto gpioDev = evdevOpen(gpioFd());

    auto value = static_cast<int>(0);
    auto fetch_rc = libevdev_fetch_event_value(gpioDev.get(), EV_KEY,
                                               keycode, &value);
    if (0 == fetch_rc)
    {
        //TODO - Create error log for failure. openbmc/openbmc#1542
        throw std::runtime_error(
            "Device does not support event type=EV_KEY and code=" +
            std::to_string(keycode));
    }

    // TODO openbmc/phosphor-fan-presence#6
    if (value > 0)
    {
        setWaterCooled();
    }
    else
    {
        setAirCooled();
    }

}

CoolingType::ObjectMap CoolingType::getObjectMap(const std::string& objpath)
{
    ObjectMap invObj;
    InterfaceMap invIntf;
    PropertyMap invProp;

    invProp.emplace("AirCooled", airCooled);
    invProp.emplace("WaterCooled", waterCooled);
    invIntf.emplace("xyz.openbmc_project.Inventory.Decorator.CoolingType",
                    std::move(invProp));
    invObj.emplace(objpath, std::move(invIntf));

    return invObj;
}

void CoolingType::updateInventory(const std::string& objpath)
{
    using namespace phosphor::logging;

    ObjectMap invObj = getObjectMap(objpath);

    std::string invService;

    invService = phosphor::fan::util::getInvService(bus);

    // Update inventory
    auto invMsg = bus.new_method_call(invService.c_str(),
                                      INVENTORY_PATH,
                                      INVENTORY_INTF,
                                      "Notify");
    invMsg.append(std::move(invObj));
    auto invMgrResponseMsg = bus.call(invMsg);
    if (invMgrResponseMsg.is_method_error())
    {
        throw std::runtime_error(
            "Error in inventory manager call to update inventory");
    }
}

}
}
}
// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
