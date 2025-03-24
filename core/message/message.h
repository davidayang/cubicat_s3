#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#define Msg_Cursor 1
#define Msg_Orientation 2
#define Msg_XYAxisAngle 3
#define Msg_Collision 4
#define Msg_PlayAnim 5
#define Msg_SetPos 6
#define Msg_MoveTo 7
#define Msg_MountingPointPos 8
#define Msg_StopAnim 9
#define Msg_StartAnim 10
#define CUSTOM_MSG_BEGIN 100


struct MoveToData {
    float     x;
    float     y;
    float     time;
};
#endif