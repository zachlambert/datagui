#pragma once

#ifdef DATAGUI_DEBUG
#define DATAGUI_LOG(msg, ...) printf((msg "\n"), ##__VA_ARGS__);
#else
#define DATAGUI_LOG(msg, ...) ()
#endif
