
local _M = {}

-- time utility
do
  local EPOCH_YEAR = 1970
  local DAYS_TABLE = {
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
  }

  function _M.gettime(ts)
    local year, month, day, hour, minute, second

    local sec_of_day = ts % 86400
    local days       = math.floor(ts / 86400) + 731
    local years      = math.floor(days / (365*4+1)) * 4 - 2
    local days_of_4year = days % (365*4+1);

    hour   = math.floor(sec_of_day / 3600)
    minute = math.floor(sec_of_day / 60) % 60
    second = sec_of_day % 60

    for y = 4, 1, -1 do
      if days_of_4year >= DAYS_TABLE[y][1] then
        year = years + y + EPOCH_YEAR -1
        for m = 12, 1, -1 do
          if days_of_4year >= DAYS_TABLE[y][m] then
            month = m
            day   = days_of_4year - DAYS_TABLE[y][m] + 1
            break
          end
        end
        break
      end
    end

    return {
      year  = year,
      month = month,
      day   = day,
      hour  = hour,
      min   = minute,
      sec   = second,
    }
  end
end


-- register to _G.util
local existed = false
for k, _ in pairs(_G) do
  if k == "util" then
    existed = true
    break
  end
end
if not existed then
  _G.util = _M
else
  for k, v in pairs(_M) do
    _G.util[k] = v
  end
end
