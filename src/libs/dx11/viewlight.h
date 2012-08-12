#pragma once
/// \file	viewlight.h
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
/// Declares the viewlight class.
#ifndef YOLK_DX11_LIGHT_H_
#define YOLK_DX11_LIGHT_H_

#include "light.h"
#include "sm1view.h"

namespace Dx11 {

class ViewLight : public Light {
public:

protected:
	SM1view firstHitTexture;
};

}
#endif