#include "ros/ros.h"

#include <fstream>

#include "dynamixel.hpp"

#include "dynamixel_control/GetIDs.h"
#include "dynamixel_control/Hold.h"
#include "dynamixel_control/GetActuatorVoltage.h"
#include "dynamixel_control/GetActuatorsVoltages.h"
#include "dynamixel_control/GetActuatorLoad.h"
#include "dynamixel_control/GetActuatorsLoads.h"
#include "dynamixel_control/GetActuatorPosition.h"
#include "dynamixel_control/GetActuatorsPositions.h"
#include "dynamixel_control/SetActuatorPosition.h"
#include "dynamixel_control/SetActuatorSpeed.h"
#include "dynamixel_control/SetActuatorsPositions.h"
#include "dynamixel_control/SetActuatorsSpeeds.h"
#include "dynamixel_control/SetPower.h"
#include "dynamixel_control/SetActualActuatorsPositions.h"

#include <linux/types.h>

#define READ_DURATION 0.02

using namespace dynamixel;

Usb2Dynamixel controller;
std::vector<byte_t> ax12_ids;
std::map<byte_t,int> offsets;

void saveOffsets()
{
    std::ofstream file("offsets.conf",std::ofstream::out|std::ofstream::trunc);
    std::map<byte_t,int>::iterator it = offsets.begin();
    do
    {
        file << (int)(*it).first << " " << (int)(*it).second << " ";
        ++it;
    } while(it != offsets.end());
    file.close();
}

void loadOffsets()
{
    int key;
    int value;

    std::ifstream file("offsets.conf",std::ifstream::in);
    while (file >> key >> value)
        offsets[(byte_t)key] = value;
    file.close();
}

bool GetIDsService(dynamixel_control::GetIDs::Request  &req,
                   dynamixel_control::GetIDs::Response &res)
{
    res.ids = ax12_ids;
}

bool HoldService(dynamixel_control::Hold::Request  &req,
                 dynamixel_control::Hold::Response &res)
{
    dynamixel::Status status;
    for(int i = 0; i < ax12_ids.size(); i++)
    {
        try
        {
            controller.send(dynamixel::ax12::GetPosition(ax12_ids[i]));
            controller.recv(READ_DURATION, status);
            controller.send(dynamixel::ax12::SetPosition(ax12_ids[i], status.decode16()));
            controller.recv(READ_DURATION, status);
        }
        catch (Error e)
        {
            ROS_ERROR("%s",e.msg().c_str());
            return false;
        }

    }
    return true;
}

bool GetActuatorLoadService(dynamixel_control::GetActuatorLoad::Request  &req,
                            dynamixel_control::GetActuatorLoad::Response &res)
{
    if(std::find(ax12_ids.begin(),ax12_ids.end(),req.id) != ax12_ids.end())
    {
        dynamixel::Status status;
        try
        {
            controller.send(dynamixel::ax12::GetLoad(req.id));
            controller.recv(READ_DURATION, status);
        }
        catch (Error e)
        {
            ROS_ERROR("%s",e.msg().c_str());
            return -1;
        }
        int16_t buf = status.decode16();
        ROS_INFO("Raw load : %d",buf);
        // bit n°10 is sign
        res.load = (buf & 0x3FF)*100/1023.0;
        return true;
    }
    else
    {
        return false;
    }
}

bool GetActuatorsLoadsService(dynamixel_control::GetActuatorsLoads::Request  &req,
                              dynamixel_control::GetActuatorsLoads::Response &res)
{
    dynamixel::Status status;
    for(int i = 0; i < req.ids.size(); i++)
    {
        if(std::find(ax12_ids.begin(),ax12_ids.end(),req.ids[i]) != ax12_ids.end())
        {
            try
            {
                controller.send(dynamixel::ax12::GetLoad(req.ids[i]));
                controller.recv(READ_DURATION, status);
            }
            catch (Error e)
            {
                ROS_ERROR("%s",e.msg().c_str());
                return false;
            }
            int16_t buf = status.decode16();
            // bit n°10 is sign
            res.loads.push_back((buf & 0x3FF)*100.0/1023.0);
            //res.loads.push_back(buf);
        }
        else
        {
            res.loads.push_back(0);
        }
    }
    return true;
}

bool GetActuatorVoltageService(dynamixel_control::GetActuatorVoltage::Request  &req,
                               dynamixel_control::GetActuatorVoltage::Response &res)
{
    if(std::find(ax12_ids.begin(),ax12_ids.end(),req.id) != ax12_ids.end())
    {
        dynamixel::Status status;
        try
        {
            controller.send(dynamixel::ax12::GetVoltage(req.id));
            controller.recv(READ_DURATION, status);
        }
        catch (Error e)
        {
            ROS_ERROR("%s",e.msg().c_str());
            return -1;
        }
        res.voltage = (status.decode16()&0xFF)/10.0;
        return true;
    }
    else
    {
        return false;
    }
}

bool GetActuatorsVoltagesService(dynamixel_control::GetActuatorsVoltages::Request  &req,
                                 dynamixel_control::GetActuatorsVoltages::Response &res)
{
    dynamixel::Status status;
    for(int i = 0; i < req.ids.size(); i++)
    {
        if(std::find(ax12_ids.begin(),ax12_ids.end(),req.ids[i]) != ax12_ids.end())
        {
            try
            {
                controller.send(dynamixel::ax12::GetVoltage(req.ids[i]));
                controller.recv(READ_DURATION, status);
            }
            catch (Error e)
            {
                ROS_ERROR("%s",e.msg().c_str());
                return false;
            }
            res.voltages.push_back((status.decode16()&0xFF)/10.0);
        }
        else
        {
            res.voltages.push_back(0);
        }
    }
    return true;
}

bool GetActuatorPositionService(dynamixel_control::GetActuatorPosition::Request  &req,
                                dynamixel_control::GetActuatorPosition::Response &res)
{
    if(std::find(ax12_ids.begin(),ax12_ids.end(),req.id) != ax12_ids.end())
    {
        dynamixel::Status status;
        try
        {
            controller.send(dynamixel::ax12::GetPosition(req.id));
            controller.recv(READ_DURATION, status);
        }
        catch (Error e)
        {
            ROS_ERROR("%s",e.msg().c_str());
            return -1;
        }

        res.pos = status.decode16();
        return true;
    }
    else
    {
        return false;
    }
}

bool GetActuatorsPositionsService(dynamixel_control::GetActuatorsPositions::Request  &req,
                                  dynamixel_control::GetActuatorsPositions::Response &res)
{
    dynamixel::Status status;
    std::vector<int> positions;
    try
    {
        for(int i = 0; i < ax12_ids.size(); i++)
        {
            controller.send(dynamixel::ax12::GetPosition(ax12_ids[i]));
            controller.recv(READ_DURATION, status);
            positions.push_back(status.decode16());
        }
    }
    catch (Error e)
    {
        ROS_ERROR("%s",e.msg().c_str());
        return -1;
    }

    res.ids = ax12_ids;
    res.positions = positions;
}

bool SetActuatorSpeedService(dynamixel_control::SetActuatorSpeed::Request  &req,
                             dynamixel_control::SetActuatorSpeed::Response &res)
{
    if(std::find(ax12_ids.begin(),ax12_ids.end(),req.id) != ax12_ids.end())
    {
        dynamixel::Status status;
        try
        {
            controller.send(dynamixel::ax12::SetSpeed(req.id, req.speed > 0, abs(req.speed)));
            controller.recv(READ_DURATION, status);
        }
        catch (Error e)
        {
            ROS_ERROR("%s",e.msg().c_str());
            return -1;
        }

        return true;
    }
    else
    {
        return false;
    }
}

bool SetActuatorsSpeedsService(dynamixel_control::SetActuatorsSpeeds::Request  &req,
                               dynamixel_control::SetActuatorsSpeeds::Response &res)
{
    dynamixel::Status status;
    std::vector<bool> directions;
    for(int i = 0; i < req.ids.size(); i++)
        directions[i] = (bool)req.directions[i];
    try
    {
        controller.send(dynamixel::ax12::SetSpeeds(req.ids, directions, req.speeds));
        controller.recv(READ_DURATION, status);
    }
    catch (Error e)
    {
        ROS_ERROR("%s",e.msg().c_str());
        return -1;
    }

    return true;
}

bool SetActuatorPositionService(dynamixel_control::SetActuatorPosition::Request  &req,
                                dynamixel_control::SetActuatorPosition::Response &res)
{
    if(std::find(ax12_ids.begin(),ax12_ids.end(),req.id) != ax12_ids.end())
    {
        try
        {
            dynamixel::Status status;
            controller.send(dynamixel::ax12::SetPosition(req.id, req.position + offsets[req.id]));
            controller.recv(READ_DURATION, status);
        }
        catch (Error e)
        {
            ROS_ERROR("%s",e.msg().c_str());
            return -1;
        }

        return true;
    }
    else
    {
        ROS_ERROR("Dynamixel n°%d doesnt exist",(int)req.id);
        return false;
    }
}

bool SetActuatorsPositionsService(dynamixel_control::SetActuatorsPositions::Request  &req,
                                  dynamixel_control::SetActuatorsPositions::Response &res)
{
    dynamixel::Status status;
    try
    {
        std::vector<byte_t> new_ids;
        std::vector<int> new_positions;
        for(int i = 0; i < req.ids.size(); i++)
        {
            int id = req.ids[i];
            if(std::find(ax12_ids.begin(),ax12_ids.end(),id) != ax12_ids.end())
            {
                new_ids.push_back(id);
                new_positions.push_back(req.positions[i] + offsets[id]);
            }
            else
                ROS_ERROR("Dynamixel n°%d doesnt exist",id);
        }
        controller.send(dynamixel::ax12::SetPositions(new_ids, new_positions));
        controller.recv(READ_DURATION, status);
    }
    catch (Error e)
    {
        ROS_ERROR("%s",e.msg().c_str());
        return -1;
    }

    return true;
}

bool SetPowerService(dynamixel_control::SetPower::Request  &req,
                     dynamixel_control::SetPower::Response &res)
{
    dynamixel::Status status;
    try
    {
        for (size_t i = 0; i < ax12_ids.size(); ++i)
        {
            controller.send(dynamixel::ax12::TorqueEnable(ax12_ids[i], req.enable));
            controller.recv(READ_DURATION, status);
        }
    }
    catch (Error e)
    {
        ROS_ERROR("%s",e.msg().c_str());
        return -1;
    }
    return true;
}

bool SetActualActuatorsPositionsService(dynamixel_control::SetActualActuatorsPositions::Request  &req,
                                        dynamixel_control::SetActualActuatorsPositions::Response &res)
{
    dynamixel::Status status;
    try
    {
        for(int i = 0; i < req.ids.size(); i++)
        {
            int id = req.ids[i];
            if(std::find(ax12_ids.begin(),ax12_ids.end(),id) != ax12_ids.end())
            {
                controller.send(dynamixel::ax12::GetPosition(id));
                controller.recv(READ_DURATION, status);
                offsets[id] = status.decode16() - req.positions[i];
            }
            else
                ROS_ERROR("Dynamixel n°%d doesnt exist",id);
        }

        saveOffsets();
    }
    catch (Error e)
    {
        ROS_ERROR("%s",e.msg().c_str());
        return -1;
    }
    return true;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "dynamixel_control");
    ros::NodeHandle nh("/dynamixel_control");

    ros::ServiceServer getIDsService = nh.advertiseService("getids", GetIDsService);
    ros::ServiceServer holdService = nh.advertiseService("hold", HoldService);
    ros::ServiceServer getPositionService = nh.advertiseService("getposition", GetActuatorPositionService);
    ros::ServiceServer getVoltageService = nh.advertiseService("getvoltage", GetActuatorVoltageService);
    ros::ServiceServer getVoltagesService = nh.advertiseService("getvoltages", GetActuatorsVoltagesService);
    ros::ServiceServer getLoadService = nh.advertiseService("getload", GetActuatorLoadService);
    ros::ServiceServer getLoadsService = nh.advertiseService("getloads", GetActuatorsLoadsService);
    ros::ServiceServer getPositionsService = nh.advertiseService("getpositions", GetActuatorsPositionsService);
    ros::ServiceServer setSpeedService = nh.advertiseService("setspeed", SetActuatorSpeedService);
    ros::ServiceServer setPositionService = nh.advertiseService("setposition", SetActuatorPositionService);
    ros::ServiceServer setSpeedsService = nh.advertiseService("setspeeds", SetActuatorsSpeedsService);
    ros::ServiceServer setPositionsService = nh.advertiseService("setpositions", SetActuatorsPositionsService);
    ros::ServiceServer setPowerService = nh.advertiseService("setpower", SetPowerService);
    ros::ServiceServer SetActualPositionsService = nh.advertiseService("setactualpositions", SetActualActuatorsPositionsService);

    // node params :
    std::string port_name;
    int baudrate;

    nh.param<std::string>("port_name", port_name, "/dev/ttyUSB0");
    nh.param<int>("baudrate", baudrate, B1000000);

    ROS_INFO("Port : %s",port_name.c_str());
    ROS_INFO("Baudrate : %d",baudrate);

    try
    {
        controller.open_serial(port_name, baudrate);
        ROS_INFO("Serial port open");

        controller.scan_ax12s();
        ax12_ids = controller.ax12_ids();
        for(int i = 0; i < ax12_ids.size(); i++)
            offsets.insert(std::pair<byte_t,int>(ax12_ids[i],0));
        ROS_INFO("%d dynamixels are connected",(int)ax12_ids.size());;
        for (size_t i = 0; i < ax12_ids.size(); ++i)
            ROS_INFO("id : %d",(int)ax12_ids[i]);
    }
    catch (Error e)
    {
        ROS_ERROR("%s",e.msg().c_str());
        return -1;
    }

    controller.send(dynamixel::ax12::SetAlarmShutdown(0b01011111));
    controller.send(dynamixel::ax12::SetAlarmLed(0b01011111));

    loadOffsets();

    ros::spin();

    return 0;

}
