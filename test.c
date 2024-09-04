#include <stdio.h>
#include "enum_definitions.h"

// Définition des enums avec X-Macro
#define X(name, value) name value,
typedef enum { LOG_LEVEL_ENUMS } LogLevel;
typedef enum { CHESS_TILE_ENUM } ChessTile;
typedef enum { CHESS_PIECE_ENUM } ChessPiece;
typedef enum { CHESS_BOOL_INFO_ENUM } ChessBoolInfo;
typedef enum { BTN_STATE_ENUM } BtnState;
typedef enum { BTN_TYPE_ENUM } BtnType;
typedef enum { CLIENT_STATE_ENUM } ClientState;
typedef enum { ROOM_STATE_ENUM } RoomState;
typedef enum { MSG_TYPE_ENUM } MsgType;
typedef enum { MSG_IDX_ENUM } MsgIdx;
#undef X

// Génération automatique des fonctions de conversion de enum en string
#define ENUM_TO_STRING_FUNC(EnumName, EnumDefs) \
    FT_INLINE const char* EnumName##_to_str(EnumName value) { \
        switch (value) { \
            EnumDefs \
            default: return "UNKNOWN"; \
        } \
    }

// Création des cases switch automatiquement
#define X(name, value) case name: return #name;
ENUM_TO_STRING_FUNC(LogLevel, LOG_LEVEL_ENUMS)
ENUM_TO_STRING_FUNC(ChessTile, CHESS_TILE_ENUM)
ENUM_TO_STRING_FUNC(ChessPiece, CHESS_PIECE_ENUM)
ENUM_TO_STRING_FUNC(ChessBoolInfo, CHESS_BOOL_INFO_ENUM)
ENUM_TO_STRING_FUNC(BtnState, BTN_STATE_ENUM)
ENUM_TO_STRING_FUNC(BtnType, BTN_TYPE_ENUM)
ENUM_TO_STRING_FUNC(ClientState, CLIENT_STATE_ENUM)
ENUM_TO_STRING_FUNC(RoomState, ROOM_STATE_ENUM)
ENUM_TO_STRING_FUNC(MsgType, MSG_TYPE_ENUM)
ENUM_TO_STRING_FUNC(MsgIdx, MSG_IDX_ENUM)
#undef X

// Exemple d'utilisation
int main() {
	printf("ChessTile: %s\n", ChessTile_to_str(A3));
	printf("ChessPiece: %s\n", ChessPiece_to_str(WHITE_KING));
	printf("ChessBoolInfo: %s\n", ChessBoolInfo_to_str(WHITE_KING_MOVED));
	printf("BtnState: %s\n", BtnState_to_str(BTN_STATE_PRESSED));
	printf("BtnType: %s\n", BtnType_to_str(BTN_QUIT));
	printf("ClientState: %s\n", ClientState_to_str(CLIENT_STATE_CONNECTED));
	printf("RoomState: %s\n", RoomState_to_str(ROOM_STATE_END));
	printf("MsgType: %s\n", MsgType_to_str(MSG_TYPE_ACK));
	printf("MsgIdx: %s\n", MsgIdx_to_str(IDX_TO));
    return 0;
}
