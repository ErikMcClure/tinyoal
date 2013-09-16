#pragma once
#include <vcclr.h>

#define TOCHAR(str) pin_ptr<const char> pstr = &System::Text::Encoding.UTF8->GetBytes(str)[0]