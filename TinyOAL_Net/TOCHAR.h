#pragma once
#include <vcclr.h>

#define TOCHAR(str) pin_ptr<const unsigned char> pstr = !str->Length?(unsigned char*)"":&System::Text::Encoding::UTF8->GetBytes(str)[0];