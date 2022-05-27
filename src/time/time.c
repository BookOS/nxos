/**
 * Copyright (c) 2018-2022, NXOS Development Team
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Contains: system time
 * 
 * Change Logs:
 * Date           Author            Notes
 * 2022-05-14     JasonHu           Init
 */

#include <base/time.h>
#define NX_LOG_NAME "time"
#include <base/log.h>

#include <base/driver.h>
#include <base/initcall.h>

NX_PRIVATE NX_TIME_DEFINE(systemTime);

NX_PRIVATE const char monthDayTable[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

NX_PRIVATE int IsLeapYear(int year)
{
    if (year % 4)
    {
        return 0;
    }
    if (year % 400)
    {
        return 1;
    }
    if (year % 100)
    {
        return 0;
    }
    return 1;
}

NX_PRIVATE int MakeWeekDay(int year, int month, int day)
{
    int c, y, week;
    if (month == 1 || month == 2)
    {
        year--;
        month+=12;
    }
    c = year / 100;
    y = year - c * 100;
    week = (c / 4) - 2 * c + (y + y / 4) + (13 * (month + 1) / 5) + day - 1;
    while(week < 0)
    {
        week += 7;
    }
    week %= 7;
    return week;
}

NX_PRIVATE int MakeMonthDay()
{
    if (systemTime.month == 2)
    {
        return monthDayTable[systemTime.month] + IsLeapYear(systemTime.year);
    }
    else
    {
        return monthDayTable[systemTime.month];
    }
}

NX_PRIVATE int MakeYearDays()
{
    int i;
    int sum = 0;
    for (i = 1; i < systemTime.month; i++)
    {
        if (i == 2)
        {
            sum += monthDayTable[i] + IsLeapYear(systemTime.year);
        }
        else
        {
            sum += monthDayTable[i];
        }
    }

    sum += systemTime.day;
    if (systemTime.month >= 2)
    {
        if (systemTime.month == 2)
        {
            if (systemTime.day == 28)
            {
                sum += IsLeapYear(systemTime.year);    
            }
        }
        else
        {
            sum += IsLeapYear(systemTime.year);
        }
    }
    return sum;
}

void NX_TimeGo(void)
{
    systemTime.second++;
    if(systemTime.second > 59)
    {
        systemTime.minute++;
        systemTime.second = 0;
        if(systemTime.minute > 59)
        {
            systemTime.hour++;
            systemTime.minute = 0;
            if(systemTime.hour > 23)
            {
                systemTime.day++;
                systemTime.hour = 0;
                systemTime.weekDay++;
                if (systemTime.weekDay > 6)
                {
                    systemTime.weekDay = 0;
                }
                systemTime.yearDay = MakeYearDays();
                if(systemTime.day > MakeMonthDay())
                {
                    systemTime.month++;
                    systemTime.day = 1;
                    if(systemTime.month > 12)
                    {
                        systemTime.year++;
                        systemTime.month = 1;
                    }
                }
            }
        }
    }
}

void NX_TimePrint(void)
{
    char *weekDay[] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };
    
    NX_LOG_I("time:%d:%d:%d date:%d/%d/%d",
        systemTime.hour, systemTime.minute, systemTime.second,
        systemTime.year, systemTime.month, systemTime.day);
    NX_LOG_I("week day:%d %s year day:%d", systemTime.weekDay, weekDay[systemTime.weekDay], systemTime.yearDay);
}

NX_Error NX_TimeSet(NX_Time * time)
{
    if (!time)
    {
        return NX_EINVAL;
    }

    systemTime.day = time->day;
    systemTime.hour = time->hour;
    systemTime.minute = time->minute;
    systemTime.month = time->month;
    systemTime.second = time->second;
    systemTime.year = time->year;

    systemTime.weekDay = MakeWeekDay(time->year, time->month, time->day);
    systemTime.yearDay = MakeYearDays();

    return NX_EOK;
}

NX_Error NX_TimeGet(NX_Time * time)
{
    if (!time)
    {
        return NX_EINVAL;
    }
    
    time->day = systemTime.day;
    time->hour = systemTime.hour;
    time->minute = systemTime.minute;
    time->month = systemTime.month;
    time->second = systemTime.second;
    time->weekDay = systemTime.weekDay;
    time->year = systemTime.year;
    time->yearDay = systemTime.yearDay;

    return NX_EOK;
}

NX_PRIVATE void NX_TimeInit(void)
{
    NX_Time time;
    NX_Device * timeDev;
    NX_Error err;
    
    err = NX_DeviceOpen("rtc", 0, &timeDev);
    if (err != NX_EOK)
    {
        NX_LOG_W("no rtc device!");    
        NX_TimeSet(&systemTime); /* set default time */
    }
    else
    {
        err = NX_DeviceRead(timeDev, &time, 0, sizeof(time), NX_NULL);
        if (err != NX_EOK)
        {
            NX_LOG_E("read rtc device error!");
            NX_TimeSet(&systemTime); /* set default time */
        }
        else
        {
            NX_TimeSet(&time);
        }
        NX_DeviceClose(timeDev);
    }
    NX_TimePrint();
}

NX_FINAL_INIT(NX_TimeInit);
