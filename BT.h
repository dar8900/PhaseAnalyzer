#ifndef BT_H
#define BT_H
#include <Arduino.h>

extern bool BtDeviceConnected;

void BtInit();
void CheckBtDevConn();
void BtTransaction();

#endif