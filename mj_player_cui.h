#ifndef MJPLAYER_CUI_H_INCLUDED
#define MJPLAYER_CUI_H_INCLUDED

#include "mj_player_interface.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* CUIクライアントくんのインターフェース取得 */
const struct MJPlayerInterface *MJPlayerCUI_GetInterface(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MJPLAYER_CUI_H_INCLUDED */
