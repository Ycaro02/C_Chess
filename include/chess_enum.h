#include "enum_definitions.h"

/* Define the X-Macro enums */
#define X(name, value) name value,
typedef enum { CHESS_TILE_ENUM } ChessTile;
typedef enum { TILE_TYPE_ENUM } TileType;
typedef enum { CHESS_PIECE_ENUM } ChessPiece;
typedef enum { CHESS_BOOL_INFO_ENUM } ChessBoolInfo;
typedef enum { BTN_STATE_ENUM } BtnState;
typedef enum { BTN_TYPE_ENUM } BtnType;
typedef enum { CLIENT_STATE_ENUM } ClientState;
typedef enum { ROOM_STATE_ENUM } RoomState;
typedef enum { MSG_TYPE_ENUM } MsgType;
typedef enum { MSG_IDX_ENUM } MsgIdx;
typedef enum { CHESS_FLAG_ENUM } ChessFlag;
#undef X

/* Automatically generate the enum to string conversion functions */
#define ENUM_TO_STRING_FUNC(EnumName, EnumDefs) \
    FT_INLINE const char* EnumName##_to_str(EnumName value) { \
        switch (value) { \
            EnumDefs \
            default: return "UNKNOWN"; \
        } \
    }

/* Create the switch cases automatically */
#define X(name, value) case name: return #name;
ENUM_TO_STRING_FUNC(ChessTile, CHESS_TILE_ENUM)
ENUM_TO_STRING_FUNC(TileType, TILE_TYPE_ENUM)
ENUM_TO_STRING_FUNC(ChessPiece, CHESS_PIECE_ENUM)
ENUM_TO_STRING_FUNC(ChessBoolInfo, CHESS_BOOL_INFO_ENUM)
ENUM_TO_STRING_FUNC(BtnState, BTN_STATE_ENUM)
ENUM_TO_STRING_FUNC(BtnType, BTN_TYPE_ENUM)
ENUM_TO_STRING_FUNC(ClientState, CLIENT_STATE_ENUM)
ENUM_TO_STRING_FUNC(RoomState, ROOM_STATE_ENUM)
ENUM_TO_STRING_FUNC(MsgType, MSG_TYPE_ENUM)
ENUM_TO_STRING_FUNC(MsgIdx, MSG_IDX_ENUM)
ENUM_TO_STRING_FUNC(ChessFlag, CHESS_FLAG_ENUM)
#undef X
