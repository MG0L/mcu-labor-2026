#include <stdbool.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rtc.h"
#include "alarm_clock.h"
#include "buttons.h"
#include "rtc.h"
#include "OLED.h"

#define MAX_CHARS_IN_A_ROW (11U)

// main states
typedef enum
{
  NORMAL_OPERATION = 0,
  ALARM_IN_PROGRESS
} MainState_TypeDef;

// substates
typedef enum
{
  SHOW_DATE_TIME = 0,
  SHOW_ALARM_PRESET_TIME,
  SET_DATE_TIME,
  SET_ALARM
} SubState_TypeDef;

// Structure containing the states
typedef struct
{
  volatile MainState_TypeDef MainState;
  SubState_TypeDef SubState;
} State_TypeDef;

// List of optional data types
typedef enum
{
  SELECTED_YEAR = 0,
  SELECTED_MONTH,
  SELECTED_DAY,
  SELECTED_HOUR,
  SELECTED_MINUTE,
  SELECTED_SECOND,
  SELECTED_ALARM_STATE
} Selection_TypeDef;

// Structure for the information displayed on the screen
typedef struct
{
  Selection_TypeDef Selection;
  uint8_t x1;
  uint8_t y1;
} TimeObject_TypeDef;

static State_TypeDef State;

//                                        Jan  Feb  Mar  Apr  Maj  Jun  Jul  Aug  Sep  Okt  Nov  Dec
static const uint8_t MAX_DAYS_MAP[12] = { 31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31 };

/**
 * @brief     Show the date and time
 * @param[in] date Exact date
 * @param[in] time Exact time
 */
static void ShowDateTime(
    const RTC_DateTypeDef * const date,
    const RTC_TimeTypeDef * const time)
{
    char date_string[MAX_CHARS_IN_A_ROW + 1];
    char time_string[MAX_CHARS_IN_A_ROW + 1];
    char bcd_string[MAX_CHARS_IN_A_ROW + 1];

    uint32_t tr = RTC->TR;

    snprintf(date_string, sizeof(date_string),
             "20%02d-%02d-%02d",
             date->Year, date->Month, date->Date);

    snprintf(time_string, sizeof(time_string),
             "%02d:%02d:%02d",
             time->Hours, time->Minutes, time->Seconds);

    snprintf(bcd_string, sizeof(bcd_string),
             "TR:%08lX", (unsigned long)tr);

    OLED_Fill(Black);

    OLED_SetCursor(0, 0);
    OLED_WriteString(date_string, Font_11x18, White);

    OLED_SetCursor(22, 18);
    OLED_WriteString(time_string, Font_11x18, White);

    OLED_SetCursor(0, 36);
    OLED_WriteString(bcd_string, Font_11x18, White);

    OLED_UpdateScreen();
}

/**
 * @brief                   Show the alarm time.
 * @param[in] alarm         Selected alarm time
 * @param[in] alarm_state   State of the alarm
 */
static void ShowAlarmPresetTime(const RTC_AlarmTypeDef * const alarm, const bool alarm_state)
{
  char alarm_string[MAX_CHARS_IN_A_ROW + 1];

  snprintf(alarm_string, sizeof(alarm_string), "%02d:%02d:%02d", alarm->AlarmTime.Hours, alarm->AlarmTime.Minutes,
      alarm->AlarmTime.Seconds);

  OLED_Fill(Black);

  OLED_SetCursor(0, 0);
  OLED_WriteString("Wake up", Font_11x18, White);

  OLED_SetCursor(0, Font_11x18.FontHeight);
  if(alarm_state == true) {
    OLED_WriteString("ON", Font_11x18, White);
  }
  else
  {
    OLED_WriteString("OFF", Font_11x18, White);
  }

  OLED_SetCursor(Font_11x18.FontWidth * 2, Font_11x18.FontHeight * 2);
  OLED_WriteString(alarm_string, Font_11x18, White);

  OLED_UpdateScreen();
}

/**
 * @brief           Return the maximum number of days for a given month
 * @param[in] year  Selected year
 * @param[in] month Selected month
 * @return          Maximum number of days for the selected year and month
 */
static uint8_t GetDayUpperBoundary(const uint8_t year, const uint8_t month)
{
  uint8_t month_idx       = month - 1;
  uint8_t max_num_of_days = MAX_DAYS_MAP[month_idx];
  uint8_t upper_day_value = max_num_of_days;
  bool is_leap_year       = (year % 4 == 0) ? true : false;

  if (is_leap_year == true && month == 2U)
  {
    upper_day_value++; // If it is a leap year, then February is 29 days instead of 28.
  }

  return upper_day_value;
}

/**
 * @brief                     Update the selected value based on the actual state of the buttons
 * @param[in,out] date        Date that we want to update
 * @param[out] time           Time that we want to update
 * @param[out] alarm_state    Alarm state that we want to update
 * @param[in] button_states   State of the buttons
 * @param[in] selection       Selects exactly which slice of time we want to update
 */
static void UpdateSelection(
          RTC_DateTypeDef     * const date         ,
          RTC_TimeTypeDef     * const time         ,
          bool                * const alarm_state  ,
    const Buttons_TypeDef     * const button_states,
    const Selection_TypeDef           selection    )
{
  uint8_t   lower_bound;
  uint8_t   upper_bound;
  uint8_t   new_value;
  uint8_t * value_to_update;
  bool      up   = (button_states->up.updated   == true && button_states->up.state   == BUTTON_PRESSED) ? true : false;
  bool      down = (button_states->down.updated == true && button_states->down.state == BUTTON_PRESSED) ? true : false;

  if (selection == SELECTED_ALARM_STATE)
  {
    if(up == true || down == true)
    {
      *alarm_state = *alarm_state == true ? false : true;
    }
  }
  else
  {
    switch(selection)
    {
    case SELECTED_YEAR:
      lower_bound = 0;
      upper_bound = 99;
      value_to_update = &date->Year;
      break;
    case SELECTED_MONTH:
      lower_bound = 1;
      upper_bound = 12;
      value_to_update = &date->Month;
      break;
    case SELECTED_DAY:
      lower_bound = 1;
      upper_bound = GetDayUpperBoundary(date->Year, date->Month);
      value_to_update = &date->Date;
      break;
    case SELECTED_HOUR:
      lower_bound = 0;
      upper_bound = 23;
      value_to_update = &time->Hours;
      break;
    case SELECTED_MINUTE:
      lower_bound = 0;
      upper_bound = 59;
      value_to_update = &time->Minutes;
      break;
    case SELECTED_SECOND:
      lower_bound = 0;
      upper_bound = 59;
      value_to_update = &time->Seconds;
      break;
    default:
      break;
    }

    if (up == true)
    {
      new_value = *value_to_update + 1;
      *value_to_update = (new_value >= lower_bound && new_value <= upper_bound) ? new_value : lower_bound;
    }

    if (down == true)
    {
      new_value = *value_to_update - 1;
      *value_to_update = (new_value >= lower_bound && new_value <= upper_bound) ? new_value : upper_bound;
    }
  }
}

/**
 * @brief                     Set the date and time
 * @param[in] date            Exact date
 * @param[in] time            Exact time
 * @param[in] button_states   State of the buttons
 */
static void SetDateTime(
    const RTC_DateTypeDef * const date,
    const RTC_TimeTypeDef * const time,
    const Buttons_TypeDef * const button_states)
{
         char             date_string[MAX_CHARS_IN_A_ROW + 1];
         char             time_string[MAX_CHARS_IN_A_ROW + 1];
  static bool             is_first_invocation_in_this_state = true;
  static RTC_DateTypeDef  date_to_set;
  static RTC_TimeTypeDef  time_to_set;
  static uint32_t         selection_idx;

  static TimeObject_TypeDef time_objects[] =
  { //  timeSection      x1  y1
      { SELECTED_YEAR,   22, 33},
      { SELECTED_MONTH,  55, 33},
      { SELECTED_DAY,    88, 33},
      { SELECTED_HOUR,   22, 51},
      { SELECTED_MINUTE, 55, 51},
      { SELECTED_SECOND, 88, 51}
  };

  if (is_first_invocation_in_this_state == true)
  {
    date_to_set.Year    = date->Year;
    date_to_set.Month   = date->Month;
    date_to_set.Date    = date->Date;
    time_to_set.Hours   = time->Hours;
    time_to_set.Minutes = time->Minutes;
    time_to_set.Seconds = time->Seconds;

    is_first_invocation_in_this_state = false;
  }

  // T2: set the time and date -> time and date display
  if (button_states->middle.updated == true && button_states->middle.state == BUTTON_PRESSED)
  {
    State.SubState = SHOW_DATE_TIME;
    // set the time and date
    (void)HAL_RTC_SetDate(&hrtc, &date_to_set, RTC_FORMAT_BIN);
    (void)HAL_RTC_SetTime(&hrtc, &time_to_set, RTC_FORMAT_BIN);
    is_first_invocation_in_this_state = true;
  }
  else // T10: set the date and time
  {
    // read the state of the left and right buttons
    // setting the designation if it is needed
    if (button_states->right.updated == true && button_states->right.state == BUTTON_PRESSED)
    {
      selection_idx = (selection_idx == sizeof(time_objects)/sizeof(TimeObject_TypeDef) - 1) ? 0 : (selection_idx + 1);
    }
    else if (button_states->left.updated == true && button_states->left.state == BUTTON_PRESSED)
    {
      selection_idx = (selection_idx == 0) ? (sizeof(time_objects)/sizeof(TimeObject_TypeDef) - 1) : (selection_idx - 1);
    }

    // Update the values
    Selection_TypeDef selected_value = time_objects[selection_idx].Selection;
    UpdateSelection(&date_to_set, &time_to_set, NULL, button_states, selected_value);

    snprintf(date_string, sizeof(date_string), "20%02d-%02d-%02d", date_to_set.Year, date_to_set.Month, date_to_set.Date);
    snprintf(time_string, sizeof(time_string), "%02d:%02d:%02d", time_to_set.Hours, time_to_set.Minutes, time_to_set.Seconds);

    OLED_Fill(Black);
    OLED_SetCursor(0, 0);
    OLED_WriteString("Set time", Font_11x18, White);
    OLED_SetCursor(0, Font_11x18.FontHeight);
    OLED_WriteString(date_string, Font_11x18, White);
    OLED_SetCursor(Font_11x18.FontWidth * 2, Font_11x18.FontHeight * 2);
    OLED_WriteString(time_string, Font_11x18, White);

    // flashing 2 pixel underline indicates the designation
    if(time->SubSeconds > time->SecondFraction / 2)
    {
      OLED_DrawRectangle(time_objects[selection_idx].x1,
          time_objects[selection_idx].y1,
          time_objects[selection_idx].x1 + (Font_11x18.FontWidth * 2),
          time_objects[selection_idx].y1 + 1, White);
    }

    OLED_UpdateScreen();
  }
}

/**
 * @brief                       Set the alarm time
 * @param[in] alarm             Actual time of the alarm
 * @param[in,out] alarm_state   State of the alarm
 * @param[in] date              Exact date
 * @param[in] time              Exact time
 * @param[in] button_states     State of the buttons
 */
static void SetAlarm(
    const RTC_AlarmTypeDef  * const alarm        ,
          bool              * const alarm_state  ,
    const RTC_DateTypeDef   * const date         ,
    const RTC_TimeTypeDef   * const time         ,
    const Buttons_TypeDef   * const button_states)
{
         char             time_string[MAX_CHARS_IN_A_ROW + 1];
  static bool             is_first_invocation_in_this_state = true;
  static RTC_AlarmTypeDef alarm_to_set;
  static uint32_t         selection_idx;

  static TimeObject_TypeDef time_objects[] =
  { //  timeSection           x1  y1
      { SELECTED_ALARM_STATE,  0, 33},
      { SELECTED_HOUR,        22, 51},
      { SELECTED_MINUTE,      55, 51},
      { SELECTED_SECOND,      88, 51}
  };

  if (is_first_invocation_in_this_state == true)
  {
    alarm_to_set.AlarmTime.Hours    = alarm->AlarmTime.Hours;
    alarm_to_set.AlarmTime.Minutes  = alarm->AlarmTime.Minutes;
    alarm_to_set.AlarmTime.Seconds  = alarm->AlarmTime.Seconds;
    alarm_to_set.AlarmDateWeekDay   = date->Date;

    is_first_invocation_in_this_state = false;
  }

  // T5: setting the alarm --> display the time of the alarm
  if (button_states->middle.updated == true && button_states->middle.state == BUTTON_PRESSED)
  {
    State.SubState = SHOW_ALARM_PRESET_TIME;
    alarm_to_set.Alarm = RTC_ALARM_A;

    // When setting the alarm, you must also enter the day
    // If the user enters a value lower earlier then the current time, the wake-up time must be set for the next day
    if(alarm_to_set.AlarmTime.Hours < time->Hours
        || (alarm_to_set.AlarmTime.Hours == time->Hours && alarm_to_set.AlarmTime.Minutes < time->Minutes)
        || (alarm_to_set.AlarmTime.Hours == time->Hours && alarm_to_set.AlarmTime.Minutes == time->Minutes &&
            alarm_to_set.AlarmTime.Seconds < time->Seconds))
    {
      uint8_t upper_day_bound = GetDayUpperBoundary(date->Year, date->Month);
      uint8_t tomorrow = date->Date + 1;

      if (tomorrow <= upper_day_bound)
      {
        alarm_to_set.AlarmDateWeekDay = tomorrow;
      }
      else
      {
        alarm_to_set.AlarmDateWeekDay = 1; // Overflow must also be managed
      }
    }
    // Set the desired wake-up time
    (void)HAL_RTC_SetAlarm_IT(&hrtc, &alarm_to_set, RTC_FORMAT_BIN);

    // If the user switched off the alarm, then deactivate it
    if(*alarm_state == false)
    {
      (void)HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    }
    is_first_invocation_in_this_state = true;
  }
  else // T12: setting the alarm
  {
    // read the state of the left and right buttons
    // setting the designation if it is needed
    if (button_states->right.updated == true && button_states->right.state == BUTTON_PRESSED)
    {
      selection_idx = (selection_idx == sizeof(time_objects)/sizeof(TimeObject_TypeDef) - 1) ? 0 : (selection_idx + 1);
    }
    else if (button_states->left.updated == true && button_states->left.state == BUTTON_PRESSED)
    {
      selection_idx = (selection_idx == 0) ? (sizeof(time_objects)/sizeof(TimeObject_TypeDef) - 1) : (selection_idx - 1);
    }

    Selection_TypeDef selectedValueOnTheScreen = time_objects[selection_idx].Selection;
    UpdateSelection(NULL, &alarm_to_set.AlarmTime, alarm_state, button_states, selectedValueOnTheScreen);

    snprintf(time_string, sizeof(time_string), "%02d:%02d:%02d",
        alarm_to_set.AlarmTime.Hours, alarm_to_set.AlarmTime.Minutes, alarm_to_set.AlarmTime.Seconds);

    OLED_Fill(Black);

    OLED_SetCursor(0, 0);
    OLED_WriteString("Set alarm", Font_11x18, White);

    OLED_SetCursor(0, Font_11x18.FontHeight);
    if(*alarm_state == true) {
      OLED_WriteString("on", Font_11x18, White);
    }
    else
    {
      OLED_WriteString("off", Font_11x18, White);
    }

    OLED_SetCursor(Font_11x18.FontWidth * 2, Font_11x18.FontHeight * 2);
    OLED_WriteString(time_string, Font_11x18, White);

    // flashing the designation
    if(time->SubSeconds > time->SecondFraction / 2)
    {
      OLED_DrawRectangle(time_objects[selection_idx].x1,
          time_objects[selection_idx].y1,
          time_objects[selection_idx].x1 + (Font_11x18.FontWidth * 2),
          time_objects[selection_idx].y1 + 1, White);
    }

    OLED_UpdateScreen();
  }
}

/**
 * @brief Alarm by flashing the display
 */
static void Alarm(const RTC_TimeTypeDef * const time)
{
  // Flashing the display
  if(time->SubSeconds > time->SecondFraction / 2)
  {
    OLED_Fill(White);
    OLED_SetCursor(0, 0);
    OLED_WriteString("ALARM!!!", Font_16x26, Black);
  }
  else
  {
    OLED_Fill(Black);
    OLED_SetCursor(0, 0);
    OLED_WriteString("ALARM!!!", Font_16x26, White);
  }
  OLED_UpdateScreen();
}

RTC_TimeTypeDef  time;
RTC_DateTypeDef  date;
RTC_AlarmTypeDef alarm;
Buttons_TypeDef  button_states;

/**
 * @brief State machine of the alarm clock
 */
void AlarmClock_MainFunction(void)
{
  static bool      alarmState = true;

  // Get the state of the buttons
  Buttons_GetStates(&button_states);
  bool up     = (button_states.up.updated     == true && button_states.up.state     == BUTTON_PRESSED) ? true : false;
  bool down   = (button_states.down.updated   == true && button_states.down.state   == BUTTON_PRESSED) ? true : false;
  bool middle = (button_states.middle.updated == true && button_states.middle.state == BUTTON_PRESSED) ? true : false;

  // Get the time
  (void)HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
  // Get the date
  (void)HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
  // Get the time of the alarm
  (void)HAL_RTC_GetAlarm(&hrtc, &alarm, RTC_ALARM_A, RTC_FORMAT_BIN);

  if (State.MainState == NORMAL_OPERATION)
  {
    switch (State.SubState)
    {
    case SHOW_DATE_TIME:
      // T1: show the date and time --> set the date and time
      if (middle == true)
      {
        State.SubState = SET_DATE_TIME;
        break;
      }
      // T3: show the date and time --> show the alarm time
      if (up == true || down == true)
      {
        State.SubState = SHOW_ALARM_PRESET_TIME;
        break;
      }
      // T9: show the date and time
      ShowDateTime(&date, &time);
      break;
    case SHOW_ALARM_PRESET_TIME:
      // T4: show the alarm time --> Set the alarm
      if (middle == true)
      {
        State.SubState = SET_ALARM;
        break;
      }
      // T6: Show the alarm time --> show the date and time
      if (up == true || down == true)
      {
        State.SubState = SHOW_DATE_TIME;
        break;
      }
      // T11: Show the alarm time
      ShowAlarmPresetTime(&alarm, alarmState);
      break;
    case SET_DATE_TIME:
      SetDateTime(&date, &time, &button_states);
      break;
    case SET_ALARM:
      SetAlarm(&alarm, &alarmState, &date, &time, &button_states);
      break;
    default:
      break;
    }
  }
  else // ALARM_CLOCK_ALARM_IN_PROGRESS
  {
    // T8: alarm --> normal operation
    if (middle == true)
    {
      State.MainState = NORMAL_OPERATION;
      (void)HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    }
    else // T13: alarm
    {
      Alarm(&time);
    }
  }
}

/**
 * @brief     If there is an interrupt caused by an alarm, then this function will be called
 *            The state of the state machine is changed to alarm from normal operation
 * @param[in] The structure that describe the RTC module
 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  // T7: normal operation --> alarm
  State.MainState = ALARM_IN_PROGRESS;
}
