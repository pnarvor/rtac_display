#pragma once

#include <rtac_display/utils.h>
#include <rtac_display/GLFormat.h>
#include <rtac_display/GLVector.h>

#include <rtac_base/cuda/DeviceVector.h>
#include <rtac_base/cuda/HostVector.h>

namespace rtac { namespace display {


void element(const GLVector<float>& data);

float sum(const GLVector<float>& data);

float sum(cuda::DeviceVector<float>& data);

}; //namespace display
}; //namespace rtac



