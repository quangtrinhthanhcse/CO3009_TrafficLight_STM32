/*
 * traffic_light.h - Driver điều khiển đèn giao thông 2 tuyến đường
 */

#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include <stdint.h>
#include "global.h"

#define LANE_1  1
#define LANE_2  2

void traffic_init(void);
void traffic_set(uint8_t lane, uint8_t color);

#endif /* INC_TRAFFIC_LIGHT_H_ */
