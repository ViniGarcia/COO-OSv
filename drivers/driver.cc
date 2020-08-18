/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "drivers/driver.hh"
#include <osv/pci.hh>
#include <osv/debug.hh>

#include "driver.hh"

using namespace pci;

namespace hw {

    driver_manager* driver_manager::_instance = nullptr;

    driver_manager::driver_manager()
    {

    }

    driver_manager::~driver_manager()
    {
        unload_all();
    }

    void driver_manager::register_driver(std::function<hw_driver* (hw_device*)> probe)
    {
        _probes.push_back(probe);
    }

    void driver_manager::register_unlimited_driver(std::function<hw_driver* (hw_device*)> probe)
    {
        _unlimited_probes.push_back(probe);
    }

    void driver_manager::load_all()
    {
        auto dm = device_manager::instance();
        dm->for_each_device([this] (hw_device* dev) {
            for (auto probe : _probes) {
                if (auto drv = probe(dev)) {
                    dev->set_attached();
                    _drivers.push_back(drv);
                    break;
                }
            }
        });
    }

    void driver_manager::load_unlimited(hw_device* dev)
    {
	for (unsigned int index = 0; index < _unlimited_devices.size(); index++){
	    if (_unlimited_devices[index] == dev){
	        _unlimited_devices[index]->set_attached();
                /*_unlimited_drivers[index]->wake(); //This works to completely reatach the driver (including
						     //the physical address), but we have problems on waking 
						     //the driver and relocate IP or run DHCP. Thus, at the 
						     //moment, we keep the physical data in the interface.*/
                return;
	    }
        }

        for (auto probe : _unlimited_probes) {
            if (auto drv = probe(dev)) {
                dev->set_attached();
                _unlimited_drivers.push_back(drv);
                _unlimited_devices.push_back(dev);
                break;
            }
        }
    }

    void driver_manager::unload_all()
    {
        for (auto drv : _drivers) {
            delete drv;
        }
        _drivers.clear();
	
	for (auto drv : _unlimited_drivers) {
            delete drv;
        }
        _unlimited_drivers.clear();
        _unlimited_devices.clear();
    }

    void driver_manager::unload_unlimited()
    {	
	for (unsigned int index = 0; index < _unlimited_drivers.size(); index++){
            /*_unlimited_drivers[index]->sleep(); //This works to completely detach the driver (including the 							  //physical address), but we have problems on waking the same
						  //driver and relocate IP or run DHCP. Thus, at the moment, we
						  //keep the physical data in the interface.*/
            _unlimited_devices[index]->set_detached();
        }
    }

    void driver_manager::list_drivers()
    {
        for (auto drv : _drivers) {
            drv->dump_config();
        }

	for (auto drv : _unlimited_drivers) {
            drv->dump_config();
        }
    }
}
