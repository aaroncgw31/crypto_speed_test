#pragma once

#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <stdint.h>
#include <string>

int GetDate(int64_t ts);
int GetToday();
int64_t GetNow();

std::string GetNowStr();
std::string FormatTime(int64_t ts);

#endif // _UTILITY_H_
